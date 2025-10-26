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

#include <chart_legend.h>
#include <chart_ensemble.h>

using namespace SVG;
using namespace Chart;

////////////////////////////////////////////////////////////////////////////////

Legend::Legend( Ensemble* ensemble )
{
  this->ensemble = ensemble;
}

Legend::~Legend( void )
{
}

////////////////////////////////////////////////////////////////////////////////

void Legend::Add( Series* series )
{
  for ( Series* s : series_list ) {
    if ( Series::SameLegend( s, series ) ) {
      series->same_legend_series = s->same_legend_series;
      s->same_legend_series = series;
      return;
    }
  }
  series_list.push_back( series );
  return;
}

uint32_t Legend::Cnt( void )
{
  return series_list.size();
}

////////////////////////////////////////////////////////////////////////////////

SVG::U Legend::MarginX( bool boxed )
{
  return (boxed ? 1 : 2) * box_spacing;
}

SVG::U Legend::MarginY( bool boxed )
{
  (void)boxed;
  return box_spacing;
}

////////////////////////////////////////////////////////////////////////////////

void Legend::CalcLegendDims(
  Group* g, Legend::LegendDims& legend_dims
)
{
  legend_dims.ch = 0;
  legend_dims.ow = 0;
  legend_dims.cr = 0;
  legend_dims.mw = 0;
  legend_dims.mh = 0;
  legend_dims.ss = 0;
  legend_dims.lx = 0;
  legend_dims.rx = 0;
  legend_dims.tx = 0;
  legend_dims.dx = 8;
  legend_dims.dy = 4;
  legend_dims.sx = 0;
  legend_dims.sy = 0;
  legend_dims.hx = 0;
  legend_dims.hy = 0;

  g->Add( new Text( "X" ) );
  BoundaryBox bb = g->Last()->GetBB();
  g->DeleteFront();
  U char_w = bb.max.x - bb.min.x;
  U char_h = bb.max.y - bb.min.y;

  if ( !heading.empty() ) {
    Label::CreateLabel( g, heading, char_h * 1.2 );
    BoundaryBox bb = g->Last()->GetBB();
    g->DeleteFront();
    legend_dims.hx = bb.max.x - bb.min.x;
    legend_dims.hy = bb.max.y - bb.min.y + char_h / 2;
  }

  U ox = char_h / 3;    // Text to outline X spacing.
  U oy = char_h / 5;    // Text to outline Y spacing.

  bool symbol_shown = false;
  bool line_wo_symbol = false;
  for ( auto series : series_list ) {
    if ( series->name.empty() ) continue;
    if (
      ( series->marker_show &&
        ( ( series->marker_shape != MarkerShape::LineX &&
            series->marker_shape != MarkerShape::LineY
          ) ||
          series->type == SeriesType::Scatter ||
          series->type == SeriesType::Point
        )
      ) ||
      series->type == SeriesType::Bar ||
      series->type == SeriesType::StackedBar ||
      series->type == SeriesType::LayeredBar ||
      series->type == SeriesType::Area ||
      series->type == SeriesType::StackedArea
    ) {
      symbol_shown = true;
    } else {
      line_wo_symbol = true;
    }
  }

  for ( auto series : series_list ) {
    if ( series->name.empty() ) continue;
    if ( symbol_shown && line_wo_symbol ) {
      // Disable outline if there is a chance that it'll look misaligned.
      series->legend_outline = false;
    }
    if ( series->line_width > char_h * 0.8 ) {
      // No outline if it is too fat.
      series->legend_outline = false;
    }
    bool has_outline =
      series->legend_outline &&
      series->has_line &&
      series->type != SeriesType::Bar &&
      series->type != SeriesType::StackedBar &&
      series->type != SeriesType::LayeredBar &&
      series->type != SeriesType::Area &&
      series->type != SeriesType::StackedArea;
    if ( has_outline ) {
      legend_dims.ow = std::max( legend_dims.ow, series->line_width );
    }
  }
  U how = legend_dims.ow / 2;

  for ( auto series : series_list ) {
    if ( series->name.empty() ) continue;
    if (
      series->marker_show &&
      series->type != SeriesType::Area &&
      series->type != SeriesType::StackedArea &&
      ( ( series->marker_shape != MarkerShape::LineX &&
          series->marker_shape != MarkerShape::LineY
        ) ||
        series->type == SeriesType::Scatter ||
        series->type == SeriesType::Point
      )
    ) {
      Series::MarkerDims md = series->marker_out;
      legend_dims.mw = std::max( +legend_dims.mw, md.x2 - md.x1 );
      legend_dims.mh = std::max( +legend_dims.mh, md.y2 - md.y1 );
    }
  }

  legend_dims.ss = std::max( legend_dims.mw, legend_dims.mh ) / 2;

  U line_symbol_width = -1;
  for ( auto series : series_list ) {
    if ( series->name.empty() ) continue;
    if (
      series->type == SeriesType::Bar ||
      series->type == SeriesType::StackedBar ||
      series->type == SeriesType::LayeredBar ||
      series->type == SeriesType::Area ||
      series->type == SeriesType::StackedArea
    ) {
      if ( series->has_fill || series->has_line ) {
        legend_dims.ss = std::max( +legend_dims.ss, (char_h + 8) / 2 );
      }
      if ( series->has_line ) {
        legend_dims.ss = std::max( +legend_dims.ss, 2 * series->line_width );
        legend_dims.ss =
          std::max(
            +legend_dims.ss, (series->line_dash + series->line_hole) * 0.75
          );
      }
    }
    if (
      series->has_line && !series->legend_outline &&
      ( series->type == SeriesType::XY ||
        series->type == SeriesType::Line ||
        series->type == SeriesType::Lollipop
      )
    ) {
      if ( line_symbol_width < 0 ) {
        line_symbol_width = 2.8 * char_w;
      }
      legend_dims.ss = std::max( +legend_dims.ss, series->line_width / 2 );
      line_symbol_width =
        std::max(
          +line_symbol_width, 3 * series->line_dash + 2 * series->line_hole
        );
      line_symbol_width = std::max( +line_symbol_width, 3 * series->line_width );
      line_symbol_width = std::max( +line_symbol_width, 3 * legend_dims.mw );
    }
  }
  if ( line_symbol_width < 0 ) {
    line_symbol_width = 0;
  }

  legend_dims.ch = char_h;
  legend_dims.lx = std::max( +legend_dims.lx, legend_dims.ss - how );
  legend_dims.lx = std::max( +legend_dims.lx, line_symbol_width / 2 - how );
  legend_dims.dx += legend_dims.lx;
  legend_dims.tx = how + legend_dims.lx + ox;

  if ( how > 0 ) {
    legend_dims.cr = how + char_h / 4;
  }

  for ( auto series : series_list ) {
    if ( series->name.empty() ) continue;

    uint32_t max_lines = 1;
    uint32_t max_chars = 1;
    {
      uint32_t cur_chars = 0;
      size_t idx = 0;
      while ( idx < series->name.size() ) {
        auto c = series->name[ idx ];
        if ( Text::UTF8_CharAdv( series->name, idx ) ) {
          if ( c == '\n' ) {
            max_lines++;
            max_chars = std::max( max_chars, cur_chars );
            cur_chars = 0;
          } else {
            cur_chars++;
          }
        }
      }
      max_chars = std::max( max_chars, cur_chars );
    }
    U text_w = char_w * max_chars;
    U text_h = char_h * max_lines;

    bool has_outline =
      series->legend_outline &&
      series->has_line &&
      series->type != SeriesType::Bar &&
      series->type != SeriesType::StackedBar &&
      series->type != SeriesType::LayeredBar &&
      series->type != SeriesType::Area &&
      series->type != SeriesType::StackedArea;

    if ( has_outline ) legend_dims.rx = legend_dims.lx;

    legend_dims.sx =
      std::max(
        +legend_dims.sx,
        2 * how + legend_dims.lx + ox + text_w + ox +
        (has_outline ? (2 * how) : 0)
      );

    legend_dims.sy =
      std::max(
        +legend_dims.sy,
        text_h +
        (has_outline ? (2 * (oy + series->line_width / 2 + how)) : 0)
      );
    legend_dims.sy =
      std::max(
        +legend_dims.sy,
        has_outline
        ? (legend_dims.mh + 2*(legend_dims.cr + how))
        : (2 * legend_dims.ss)
      );
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////

void Legend::GetDims(
  SVG::U& w, SVG::U& h,
  Legend::LegendDims& legend_dims, bool boxed, uint32_t nx
)
{
  uint32_t ny = (Cnt() + nx - 1) / nx;
  w = nx * legend_dims.sx + (nx - 1) * legend_dims.dx;
  h = ny * legend_dims.sy + (ny - 1) * legend_dims.dy;
  w += legend_dims.lx + legend_dims.rx;
  h += legend_dims.hy;
  w = std::max( w, legend_dims.hx );
  if ( boxed ) {
    w += 2 * box_spacing;
    h += 2 * box_spacing;
  }
}

//------------------------------------------------------------------------------

bool Legend::GetBestFit(
  Legend::LegendDims& legend_dims, uint32_t& nx, bool boxed,
  SVG::U avail_x, SVG::U avail_y,
  SVG::U soft_x, SVG::U soft_y
)
{
  if ( soft_x <= 0.0 ) soft_x = avail_x;
  if ( soft_y <= 0.0 ) soft_y = avail_y;

  U need_x;
  U need_y;

  auto CalcAspect = [&]( void )
  {
    double a = 1.0;
    double b = 1.0;
    if ( avail_x > 0 && avail_y > 0 )
    {
      a = need_x / need_y;
      b = avail_x / avail_y;
    } else
    if ( avail_x > 0 )
    {
      a = need_x;
      b = avail_x;
    } else
    if ( avail_y > 0 )
    {
      a = need_y;
      b = avail_y;
    }
    return std::max( a / b, b / a );
  };

  uint32_t best_nx = 0;
  uint32_t best_rem = 0;
  bool best_fits = false;
  double best_exceed = num_hi;
  double best_aspect = num_hi;

  for ( uint32_t nx = 1; nx <= Cnt(); ++nx ) {
    GetDims( need_x, need_y, legend_dims, boxed, nx );
    uint32_t rem = Cnt() % nx;
    if ( rem > 0 ) rem = nx - rem;
    bool fits =
      (avail_x <= 0 || avail_x >= need_x) &&
      (avail_y <= 0 || avail_y >= need_y);
    double exceed =
      std::max(
        std::max( 0.0, need_x - soft_x ),
        std::max( 0.0, need_y - soft_y )
      );
    double aspect = CalcAspect();

    bool better = best_nx == 0 || (fits && !best_fits);
    if ( fits == best_fits ) {
      if ( exceed < best_exceed ) better = true;
      if ( exceed == best_exceed ) {
        if ( fits && rem < best_rem ) better = true;
        if ( !fits || rem == best_rem ) {
          if ( aspect < best_aspect ) better = true;
        }
      }
    }

    if ( better ) {
      best_nx = nx;
      best_rem = rem;
      best_fits = fits;
      best_exceed = exceed;
      best_aspect = aspect;
    }
  }

  nx = best_nx;
  return best_fits;
}

////////////////////////////////////////////////////////////////////////////////

void Legend::BuildLegends(
  bool boxed,
  SVG::Color* box_line_color,
  SVG::Color* box_fill_color,
  Group* g, int nx
)
{
  g->Attr()->SetTextAnchor( AnchorX::Min, AnchorY::Max );
  Legend::LegendDims legend_dims;
  CalcLegendDims( g, legend_dims );
  int ny = (Cnt() + nx - 1) / nx;

  {
    U mx = boxed ? box_spacing : U( 0 );
    U my = boxed ? box_spacing : U( 0 );
    U w = nx * legend_dims.sx + (nx - 1) * legend_dims.dx;
    U h = ny * legend_dims.sy + (ny - 1) * legend_dims.dy;
    w += legend_dims.lx + legend_dims.rx;
    U ey = legend_dims.hy;
    U ex = std::max( 0.0, legend_dims.hx - w );
    Point r1{
      -mx - legend_dims.lx - ex / 2,
      +my + ey
    };
    Point r2{
      r1.x + w + ex + 2 * mx,
      r1.y - h - ey - 2 * my
    };
    g->Add( new Rect( r1, r2, boxed ? box_spacing : U( 0 ) ) );
    if ( boxed ) {
      g->Last()->Attr()->LineColor()->Set( box_line_color );
      g->Last()->Attr()->SetLineWidth( 1 );
      if ( box_fill_color->IsDefined() ) {
        g->Last()->Attr()->FillColor()->Set( box_fill_color );
      }
    } else {
      g->Last()->Attr()->FillColor()->Clear();
      g->Last()->Attr()->LineColor()->Clear();
      g->Last()->Attr()->SetLineWidth( 0 );
    }
    if ( !heading.empty() ) {
      Object* obj = Label::CreateLabel( g, heading, legend_dims.ch * 1.2 );
      obj->MoveTo( AnchorX::Mid, AnchorY::Max, (r1.x + r2.x)/2, r1.y - my );
    }
  }

  int n = 0;
  for ( auto series : series_list ) {
    if ( series->name.empty() ) continue;
    U px = (n % nx) * +(legend_dims.sx + legend_dims.dx);
    U py = (n / nx) * -(legend_dims.sy + legend_dims.dy);
    Point marker_p{ px + legend_dims.ow/2, py - legend_dims.sy/2 };

    if ( ensemble->enable_html ) {
      BoundaryBox bb;
      bb.min.x = px - legend_dims.lx;
      bb.min.y = py - legend_dims.sy;
      bb.max.x = px + legend_dims.rx + legend_dims.sx;
      bb.max.y = py;
      for ( Series* s = series; s != nullptr; s = s->same_legend_series ) {
        ensemble->html_db->LegendPos( s, bb );
      }
    }

    U line_w = series->line_width;
    if ( !series->has_line ) line_w = 0;

    bool has_outline =
      series->legend_outline &&
      series->has_line &&
      series->type != SeriesType::Bar &&
      series->type != SeriesType::StackedBar &&
      series->type != SeriesType::LayeredBar &&
      series->type != SeriesType::Area &&
      series->type != SeriesType::StackedArea;

    if ( has_outline ) {
      g->Add(
        new Rect(
          px + legend_dims.ow/2,
          py - legend_dims.ow/2,
          px - legend_dims.ow/2 + legend_dims.sx,
          py + legend_dims.ow/2 - legend_dims.sy,
          legend_dims.cr
        )
      );
      series->ApplyLineStyle( g->Last() );
    }

    if (
      series->has_line && !series->legend_outline &&
      ( series->type == SeriesType::XY ||
        series->type == SeriesType::Line ||
        series->type == SeriesType::Lollipop
      )
    ) {
      g->Add(
        new Line(
          marker_p.x - legend_dims.ow/2 - legend_dims.lx, marker_p.y,
          marker_p.x + legend_dims.ow/2 + legend_dims.lx, marker_p.y
        )
      );
      series->ApplyLineStyle( g->Last() );
    }

    if (
      series->marker_show &&
      series->type != SeriesType::Area &&
      series->type != SeriesType::StackedArea &&
      ( ( series->marker_shape != MarkerShape::LineX &&
          series->marker_shape != MarkerShape::LineY
        ) ||
        series->type == SeriesType::Scatter ||
        series->type == SeriesType::Point
      )
    ) {
      marker_p.y -= (series->marker_out.y1 + series->marker_out.y2) / 2;
      if ( series->marker_show_out ) {
        series->BuildMarker( g, series->marker_out, marker_p );
        series->ApplyMarkStyle( g->Last() );
      }
      if ( series->marker_show_int ) {
        series->BuildMarker( g, series->marker_int, marker_p );
        series->ApplyHoleStyle( g->Last() );
      }
    }

    if (
      series->type == SeriesType::Bar ||
      series->type == SeriesType::StackedBar ||
      series->type == SeriesType::LayeredBar ||
      series->type == SeriesType::Area ||
      series->type == SeriesType::StackedArea
    ) {
      Point p1{ marker_p.x - legend_dims.ss, marker_p.y - legend_dims.ss };
      Point p2{ marker_p.x + legend_dims.ss, marker_p.y + legend_dims.ss };
      if ( series->has_fill ) {
        Point c1{ p1 };
        Point c2{ p2 };
        U q = (series->line_dash > 0) ? 0 : (line_w / 2);
        c1.x += q; c2.x -= q;
        c1.y += q; c2.y -= q;
        g->Add( new Rect( c1, c2 ) );
        series->ApplyFillStyle( g->Last() );
      }
      if ( line_w > 0 ) {
        U d = line_w / 2;
        p1.x += d; p2.x -= d;
        p1.y += d; p2.y -= d;
        g->Add( new Rect( p1, p2 ) );
        series->ApplyLineStyle( g->Last() );
        g->Last()->Attr()->SetLineJoin( LineJoin::Sharp );
      }
    }

    int lines = 1;
    for ( char c : series->name ) if ( c == '\n' ) lines++;
    px += legend_dims.ow / 2 + legend_dims.tx;
    py -= (legend_dims.sy - lines * legend_dims.ch) / 2;
    std::string s;
    size_t cidx = 0;
    while ( cidx < series->name.size() ) {
      size_t oidx = cidx;
      if ( Text::UTF8_CharAdv( series->name, cidx ) ) {
        if ( series->name[ oidx ] != '\n' ) {
          s += series->name.substr( oidx, cidx - oidx );
        }
        if ( series->name[ oidx ] == '\n' || cidx == series->name.size() ) {
          if ( !s.empty() ) {
            g->Add( new Text( px, py, s ) );
          }
          py -= legend_dims.ch;
          s = "";
        }
      }
    }

    n++;
  }
}

////////////////////////////////////////////////////////////////////////////////
