//
//  MIT No Attribution License
//
//  Copyright 2025, Soren Kragh
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the
//  “Software”), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so.
//

#include <chart_html.h>
#include <chart_ensemble.h>

using namespace SVG;
using namespace Chart;

////////////////////////////////////////////////////////////////////////////////

void HTML::NewChart( Main* main )
{
  main_list.push_back( main );
}

//------------------------------------------------------------------------------

void HTML::DefAxisX(
  Main* main, int n, Axis* axis, double val1, double val2,
  NumberFormat number_format,
  bool number_sign, bool logarithmic, bool is_cat
)
{
  main->html.x_axis[ n ] = {
    .axis          = axis,
    .is_cat        = is_cat,
    .number_format = number_format,
    .number_sign   = number_sign,
    .logarithmic   = logarithmic,
    .val1          = val1,
    .val2          = val2
  };
}

void HTML::DefAxisY(
  Main* main, int n, Axis* axis, double val1, double val2,
  NumberFormat number_format,
  bool number_sign, bool logarithmic, bool is_cat
)
{
  main->html.y_axis[ n ] = {
    .axis          = axis,
    .is_cat        = is_cat,
    .number_format = number_format,
    .number_sign   = number_sign,
    .logarithmic   = logarithmic,
    .val1          = val1,
    .val2          = val2
  };
}

//------------------------------------------------------------------------------

void HTML::LegendPos( Series* series, const SVG::BoundaryBox& bb )
{
  series_legend_map[ series ] = bb;
}

void HTML::MoveLegend( Series* series, SVG::U dx, SVG::U dy )
{
  for ( Series* s = series; s != nullptr; s = s->same_legend_series ) {
    auto it = series_legend_map.find( s );
    if ( it != series_legend_map.end() ) {
      it->second.min.x += dx;
      it->second.min.y += dy;
      it->second.max.x += dx;
      it->second.max.y += dy;
    }
  }
}

void HTML::MoveLegends( Main* main, SVG::U dx, SVG::U dy )
{
  for ( auto series : main->legend_obj->series_list ) {
    MoveLegend( series, dx, dy );
  }
}

//------------------------------------------------------------------------------

bool HTML::SnapCat( Main* main, cat_idx_t cat_idx )
{
  if ( main->category_num <= main->axis_x->length ) return true;
  cat_idx_t x = cat_idx;
  cat_idx_t A = main->category_num - 1;
  cat_idx_t B = static_cast< cat_idx_t >( snap_factor * main->axis_x->length);
  if ( x >= A/2 ) x = A - x;
  cat_idx_t i = (x*B + A/2) / A;
  cat_idx_t p = (i*A + B/2) / B;
  return x == p;
}

bool HTML::AllocateSnap( Main* main, SVG::Point p )
{
  uint64_t key =
    (static_cast< uint64_t >( p.y * snap_factor ) << 32) |
    (static_cast< uint64_t >( p.x * snap_factor ) <<  0);
  return main->html.snap_set.insert( key ).second;
}

void HTML::RecordSnapPoint(
  Series* series, SVG::Point p, cat_idx_t cat_idx,
  std::string_view tag_x, std::string_view tag_y
)
{
  Series::html_t::snap_point_t sp;
  sp.p = p;
  sp.cat_idx = cat_idx;
  sp.tag_y = tag_y;
  if ( !series->is_cat ) sp.tag_x = tag_x;
  series->html.uncommitted_snap_points.push_back( sp );
  series->html.has_snap = true;
}

void HTML::PreserveSnapPoint( Series* series, SVG::Point p )
{
  series->html.preserve_set.insert( p );
}

void HTML::CommitSnapPoints( Series* series, bool force )
{
  if ( !force && series->html.uncommitted_snap_points.size() < 100000 ) {
    // Do not commit when we only have relatively few points, as this increases
    // the chance of not committing non-preservable points.
    return;
  }
  auto main = series->main;
  bool is_cat = series->is_cat;
  for ( const auto& sp : series->html.uncommitted_snap_points ) {
    bool add =
      series->html.preserve_set.count( sp.p ) > 0 ||
      (is_cat && SnapCat( main, sp.cat_idx ));
    if ( add ) {
      if ( is_cat ) main->html.cat_set.insert( sp.cat_idx );
      series->html.snap_points.push_back( sp );
      AllocateSnap( main, sp.p );
    }
  }
  for ( const auto& sp : series->html.uncommitted_snap_points ) {
    bool add = AllocateSnap( main, sp.p );
    if ( add ) {
      if ( is_cat ) main->html.cat_set.insert( sp.cat_idx );
      series->html.snap_points.push_back( sp );
    }
  }
  series->html.uncommitted_snap_points.clear();
}

////////////////////////////////////////////////////////////////////////////////

std::string quoteJS( std::string_view s ) {
  std::ostringstream oss;
  oss << '"';
  for ( char c : s ) {
    if ( static_cast<unsigned char>( c ) < ' ' ) {
        oss << ' ';
    } else if ( c == '"' ) {
        oss << "\\\"";
    } else if ( c == '\\' ) {
        oss << "\\\\";
    } else {
        oss << c;
    }
  }
  oss << '"';
  return oss.str();
}

//------------------------------------------------------------------------------

void HTML::GenChartData( Main* main, std::ostringstream& oss )
{
  BoundaryBox chart_bb = main->GetGroup()->GetBB();

  BoundaryBox area_bb;
  // Standard SVG coordinates (Y direction down) given here.
  area_bb.min.x = main->g_dx;
  area_bb.min.y = -(main->g_dy + main->chart_h);
  area_bb.max.x = main->g_dx + main->chart_w;
  area_bb.max.y = -(main->g_dy);

  Color bg_color;
  if ( !main->ChartAreaColor()->IsClear() ) {
    bg_color.Set( main->ChartAreaColor() );
  } else {
    bg_color.Set( main->ensemble->BackgroundColor() );
  }
  if ( bg_color.IsClear() ) bg_color.Set( ColorName::white );

  oss << "{\n";

  // Never hide the mouse cursor as it causes stuttering for large SVGs.
  bool hide_mouse_cursor = false;
  {
    Color crosshairLineColor{ main->AxisColor() };
    MakeColorVisible( &crosshairLineColor, &bg_color );

    Color crosshairFillColor{ &bg_color };

    Color axisBoxLineColor{ &crosshairLineColor };

    Color axisBoxFillColor;
    axisBoxFillColor.Set( &crosshairFillColor, &crosshairLineColor, 0.2 );

    Color highlightColor{ "gold" };
    MakeColorVisible( &crosshairLineColor, &bg_color );

    if ( !crosshairLineColor.IsDefined() ) crosshairLineColor.Clear();
    if ( !crosshairFillColor.IsDefined() ) crosshairFillColor.Clear();
    if ( !axisBoxLineColor.IsDefined() ) axisBoxLineColor.Clear();
    if ( !axisBoxFillColor.IsDefined() ) axisBoxFillColor.Clear();

    oss
      << "backgroundColor : " << bg_color.SVG() << ",\n"
      << "crosshairLineColor : " << crosshairLineColor.SVG() << ",\n"
      << "crosshairFillColor : " << crosshairFillColor.SVG() << ",\n"
      << "axisBoxLineColor : " << axisBoxLineColor.SVG() << ",\n"
      << "axisBoxFillColor : " << axisBoxFillColor.SVG() << ",\n"
      << "highlightColor : " << highlightColor.SVG() << ",\n";

    if ( crosshairLineColor.IsClear() ) hide_mouse_cursor = false;
  }

  oss  << "axisFontSize : 14,\n";
  oss  << "infoFontSize : 14,\n";

  oss << "area:{";
  oss << "x1:" << area_bb.min.x.SVG( false ) << ',';
  oss << "y1:" << area_bb.min.y.SVG( false ) << ',';
  oss << "x2:" << area_bb.max.x.SVG( false ) << ',';
  oss << "y2:" << area_bb.max.y.SVG( false ) << "},\n";

  // Convert to standard SVG coordinates.
  std::swap( chart_bb.min.y, chart_bb.max.y );
  chart_bb.min.y = -chart_bb.min.y;
  chart_bb.max.y = -chart_bb.max.y;

  oss << "chart:{";
  oss << "x1:" << chart_bb.min.x.SVG( false ) << ',';
  oss << "y1:" << chart_bb.min.y.SVG( false ) << ',';
  oss << "x2:" << chart_bb.max.x.SVG( false ) << ',';
  oss << "y2:" << chart_bb.max.y.SVG( false ) << "},\n";

  oss << "axisX : [";
  for ( auto a : main->html.x_axis ) {
    oss << "{";
    oss << "show:" << (a.axis != nullptr) << ',';
    if ( a.axis ) {
      oss << "areaVal1:" << a.val1 << ',';
      oss << "areaVal2:" << a.val2 << ',';
      oss << "isX:" << (a.axis == main->axis_x) << ',';
      oss << "isCategory:" << a.is_cat << ',';
      oss << "logarithmic:" << a.logarithmic << ',';
      oss << "showSign:" << a.number_sign << ',';
      if ( a.number_format == NumberFormat::Magnitude ) {
        oss << "format:\"Engineering\",";
      } else
      if ( a.number_format == NumberFormat::Scientific ) {
        oss << "format:\"Scientific\",";
      } else {
        oss << "format:\"Fixed\",";
      }
    }
    oss << "},";
  }
  oss << "],\n";

  oss << "axisY : [";
  for ( auto a : main->html.y_axis ) {
    oss << "{";
    oss << "show:" << (a.axis != nullptr) << ',';
    if ( a.axis ) {
      oss << "areaVal1:" << a.val1 << ',';
      oss << "areaVal2:" << a.val2 << ',';
      oss << "isX:" << (a.axis == main->axis_x) << ',';
      oss << "isCategory:" << a.is_cat << ',';
      oss << "logarithmic:" << a.logarithmic << ',';
      oss << "showSign:" << a.number_sign << ',';
      if ( a.number_format == NumberFormat::Magnitude ) {
        oss << "format:\"Engineering\",";
      } else
      if ( a.number_format == NumberFormat::Scientific ) {
        oss << "format:\"Scientific\",";
      } else {
        oss << "format:\"Fixed\",";
      }
    }
    oss << "},";
  }
  oss << "],\n";

  oss << "axisSwap : " << main->html.axis_swap << ",\n";
  oss << "hideMouseCursor : " << hide_mouse_cursor << ",\n";
  oss << "inLine : " << main->html.all_inline << ",\n";

  for ( auto s1 : main->series_list ) {
    if ( !s1->html.has_snap ) continue;
    for ( auto s2 : main->series_list ) {
      if ( !s2->html.has_snap || s1 == s2 ) continue;
      if (
        s1->line_color_shown && s2->line_color_shown &&
        Color::Diff( s1->LineColor(), s2->LineColor() ) < 0.1
      )
        s1->html.line_color_same_cnt++;
      if (
        s1->fill_color_shown && s2->fill_color_shown &&
        Color::Diff( s1->FillColor(), s2->FillColor() ) < 0.1
      )
        s1->html.fill_color_same_cnt++;
    }
  }

  oss << "seriesList : [\n";
  for ( auto series : main->series_list ) {
    oss << "{";
    if ( series_legend_map.count( series ) > 0 ) {
      BoundaryBox bb = series_legend_map[ series ];
      if ( !series->global_legend ) {
        bb.min.x += main->g_dx; bb.min.y += main->g_dy;
        bb.max.x += main->g_dx; bb.max.y += main->g_dy;
      }
      std::swap( bb.min.y, bb.max.y );
      bb.min.y = -bb.min.y;
      bb.max.y = -bb.max.y;
      oss << "legendBB:{";
      oss << "x1:" << bb.min.x.SVG( false ) << ',';
      oss << "y1:" << bb.min.y.SVG( false ) << ',';
      oss << "x2:" << bb.max.x.SVG( false ) << ',';
      oss << "y2:" << bb.max.y.SVG( false ) << "},";
    }
    int idx_x = -1;
    int idx_y = -1;
    for ( int i = 0; i < 2; i++ ) {
     if ( series->axis_x == main->html.x_axis[ i ].axis ) idx_x = i;
     if ( series->axis_y == main->html.x_axis[ i ].axis ) idx_x = i;
     if ( series->axis_x == main->html.y_axis[ i ].axis ) idx_y = i;
     if ( series->axis_y == main->html.y_axis[ i ].axis ) idx_y = i;
    }
    if ( idx_x >= 0 ) {
      oss << "axisX:" << idx_x << ',';
    }
    if ( idx_y >= 0 ) {
      oss << "axisY:" << idx_y << ',';
    }
    {
      Color fg;
      {
        Color c1{ series->line_color };
        Color c2{ series->fill_color };
        if ( !series->line_color_shown ) c1.Set( &bg_color );
        if ( !series->fill_color_shown ) c2.Set( &bg_color );
        c1.SetTransparency( 0.0 );
        c2.SetTransparency( 0.0 );
        float d1 = Color::Diff( &c1, &bg_color );
        float d2 = Color::Diff( &c2, &bg_color );
        fg.Set( &c1 );
        if ( series->fill_color_shown ) {
          if (
            !series->line_color_shown ||
            (d1 < 0.1 && d2 > d1) ||
            ( d1 > 0.1 && d2 > 0.1 &&
              series->html.fill_color_same_cnt < series->html.line_color_same_cnt
            )
          ) {
            fg.Set( &c2 );
          }
        }
      }
      Color bg{ &bg_color };
      if ( !fg.IsClear() ) {
        bg.Set( &bg_color, &fg, 0.2 );
      }
      Color tx{ main->AxisColor() };
      MakeColorVisible( &tx, &bg_color );
      if ( !fg.IsDefined() ) fg.Clear();
      if ( !bg.IsDefined() ) bg.Clear();
      if ( !tx.IsDefined() ) tx.Clear();
      oss << "fgColor:" << fg.SVG() << ',';
      oss << "bgColor:" << bg.SVG() << ',';
      oss << "txColor:" << tx.SVG() << ',';
    }
    oss << "},\n";
  }
  oss << "],\n";

  oss << "snapPoints : [\n";
  for ( auto series : main->series_list ) {
    for ( const auto& sp : series->html.snap_points ) {
      U X = +(sp.p.x + main->g_dx);
      U Y = -(sp.p.y + main->g_dy);
      oss << "{s:" << series->id << ',';
      if ( series->is_cat ) {
        oss << "x:" << sp.cat_idx << ',';
      } else {
        oss << "x:" << quoteJS( sp.tag_x ) << ',';
      }
      oss << "y:" << quoteJS( sp.tag_y ) << ",";
      oss << "X:" << X.SVG( false ) << ',';
      oss << "Y:" << Y.SVG( false ) << "},\n";
    }
  }
  oss << "],\n";

  if ( main->category_num > 0 ) {
    oss << "catCnt : " << main->category_num << ",\n";
    oss << "categories : [\n";
    cat_idx_t j = 0;
    main->CategoryBegin();
    for (
      cat_idx_t i = 0; i < main->category_num;
      ++i, main->CategoryNext()
    ) {
      bool snappable = SnapCat( main, i );
      if ( snappable || main->html.cat_set.count( i ) > 0 ) {
        std::string_view cat;
        main->CategoryGet( cat );
        if ( !cat.empty() ) {
          if ( j < i ) {
            oss << i << ',';
            j = i;
          }
          if ( snappable ) {
            oss << ',';
          }
          oss << quoteJS( cat ) << ",\n";
          ++j;
        }
      }
    }
    oss << "],\n";
  }

  oss << "},\n";

  return;
}

//------------------------------------------------------------------------------

std::string HTML::GenHTML( SVG::Canvas* canvas )
{
  std::ostringstream oss;
  oss << std::boolalpha;

  #include <chart_html_part1.h>

  oss << "<title>" << ensemble->title_html << "</title>\n";
  oss << "</head>\n";
  oss << "<body>\n";

  BoundaryBox ensemble_bb = canvas->TopGroup()->GetBB();

  {
    U ensemble_w = ensemble_bb.max.x - ensemble_bb.min.x;
    U ensemble_h = ensemble_bb.max.y - ensemble_bb.min.y;
    oss << "<div style=\"";
    oss << "width:" << ensemble_w.SVG( false ) << "px;";
    oss << "height:" << ensemble_h.SVG( false ) << "px;";
    oss << "position:relative;margin:0 auto;\">\n";
  }

  oss << canvas->GenSVG( 0, "style=\"pointer-events: none;\" id=\"svgChart\"" );

  {
    Canvas cursor_canvas;
    Group* g = cursor_canvas.TopGroup();
    g->Add( new Rect( ensemble_bb.min, ensemble_bb.max ) );
    g->Attr()->SetLineWidth( 0 );
    g->Attr()->LineColor()->Clear();
    g->Attr()->FillColor()->Clear();
    oss << cursor_canvas.GenSVG( 0, "style=\"pointer-events: none;\" id=\"svgCursor\"" );
  }

  {
    Canvas snap_canvas;
    Group* g = snap_canvas.TopGroup();
    g->Add( new Rect( ensemble_bb.min, ensemble_bb.max ) );
    g->Attr()->SetLineWidth( 0 );
    g->Attr()->LineColor()->Clear();
    g->Attr()->FillColor()->Clear();
    oss << snap_canvas.GenSVG( 0, "id=\"svgSnap\"" );
  }

  oss << "</div>\n";

  oss << "\n<script>\n\n";

  oss << "const chart_list = [" << '\n';
  for ( auto main : main_list ) {
    GenChartData( main, oss );
  }
  oss << "];" << '\n';

  #include <chart_html_part2.h>

  return oss.str();
}

////////////////////////////////////////////////////////////////////////////////
