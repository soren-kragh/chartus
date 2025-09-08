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

#include <chart_series.h>
#include <chart_main.h>
#include <chart_ensemble.h>

#include <unordered_set>
#include <charconv>

using namespace SVG;
using namespace Chart;

////////////////////////////////////////////////////////////////////////////////

Series::Series( Main* main, SeriesType type )
{
  this->type = type;
  this->is_cat = type != SeriesType::XY && type != SeriesType::Scatter;
  this->source = main->ensemble->source;
  this->main = main;
  id = 0;

  axis_x = nullptr;
  axis_y = nullptr;
  axis_y_n = 0;
  base = 0;
  tag_db = nullptr;
  tag_enable = false;
  tag_pos = Pos::Auto;
  tag_size = 1.0;
  tag_box = false;

  html_db = nullptr;

  color_list.emplace_back(); color_list.back().Set( ColorName::royalblue     );
  color_list.emplace_back(); color_list.back().Set( ColorName::tomato        );
  color_list.emplace_back(); color_list.back().Set( ColorName::darkseagreen  );
  color_list.emplace_back(); color_list.back().Set( ColorName::darkturquoise );
  color_list.emplace_back(); color_list.back().Set( ColorName::darkmagenta   );
  color_list.emplace_back(); color_list.back().Set( ColorName::deepskyblue   );
  color_list.emplace_back(); color_list.back().Set( ColorName::orange        );
  color_list.emplace_back(); color_list.back().Set( ColorName::brown         );
  color_list.emplace_back(); color_list.back().Set( ColorName::chartreuse    );
  color_list.emplace_back(); color_list.back().Set( ColorName::slategrey     );

  SetName( "" );
  SetAxisY( 0 );

  SetLineWidth(
    (type == SeriesType::Area || type == SeriesType::StackedArea) ? 0 : 1
  );
  line_color.Set( ColorName::black );
  SetLineDash( 0 );

  fill_color.Clear();

  SetMarkerSize( 0 );
  SetMarkerShape( MarkerShape::Circle );

  marker_show = false;
  marker_show_out = false;
  marker_show_int = false;
  has_line = false;
  has_fill = false;
}

Series::~Series( void )
{
}

////////////////////////////////////////////////////////////////////////////////

void Series::SetName( const std::string& name )
{
  this->name = name;
}

void Series::SetAxisY( int axis_y_n )
{
  this->axis_y_n = axis_y_n;
}

void Series::SetBase( double base )
{
  this->base = base;
}

void Series::SetDefaultFillColor()
{
  fill_color.Set( &line_color );
  if (
    type == SeriesType::Bar ||
    type == SeriesType::StackedBar ||
    type == SeriesType::LayeredBar ||
    type == SeriesType::Area ||
    type == SeriesType::StackedArea
  ) {
    if ( type == SeriesType::Area || type == SeriesType::StackedArea ) {
      fill_color.Lighten( 0.2 );
      fill_color.SetTransparency( 0.5 );
    } else {
      fill_color.Lighten( 0.5 );
      fill_color.SetTransparency( 0.2 );
    }
  } else {
    fill_color.Lighten( 0.5 );
  }
}

void Series::SetStyle( int style )
{
  line_color.Set( &color_list[ style % color_list.size() ] );
  SetDefaultFillColor();
  style = style / color_list.size();
  style = style % 8;
  if (
    type == SeriesType::Bar ||
    type == SeriesType::StackedBar ||
    type == SeriesType::LayeredBar ||
    type == SeriesType::Area ||
    type == SeriesType::StackedArea
  ) {
    SetLineWidth( 1 );
    SetLineDash( 0 );
  } else {
    if ( style == 0 ) {
      SetLineWidth( 4 );
      SetLineDash( 0 );
    }
    if ( style == 1 ) {
      SetLineWidth( 4 );
      SetLineDash( 4, 2 );
    }
    if ( style == 2 ) {
      SetLineWidth( 4 );
      SetLineDash( 8, 2 );
    }
    if ( style == 3 ) {
      SetLineWidth( 4 );
      SetLineDash( 16, 2 );
    }
    if ( style == 4 ) {
      SetLineWidth( 2 );
      SetLineDash( 0 );
    }
    if ( style == 5 ) {
      SetLineWidth( 2 );
      SetLineDash( 4, 2 );
    }
    if ( style == 6 ) {
      SetLineWidth( 2 );
      SetLineDash( 8, 2 );
    }
    if ( style == 7 ) {
      SetLineWidth( 2 );
      SetLineDash( 16, 2 );
    }
  }
  SetMarkerSize(
    ( type == SeriesType::Scatter ||
      type == SeriesType::Point ||
      type == SeriesType::Lollipop
    )
    ? 12
    : 0
  );
}

//------------------------------------------------------------------------------

void Series::SetLineWidth( SVG::U width )
{
  if ( width > 0 ) {
    line_width = width;
  } else {
    line_width = 0;
  }
}

void Series::SetLineDash( SVG::U dash )
{
  line_dash = dash;
  line_hole = dash;
}

void Series::SetLineDash( SVG::U dash, SVG::U hole )
{
  line_dash = dash;
  line_hole = hole;
  if ( line_hole == 0 ) line_dash = 0;
  if ( line_dash == 0 ) line_hole = 0;
}

//------------------------------------------------------------------------------

void Series::SetMarkerSize( SVG::U size )
{
  marker_size = std::max( 0.0, +size );
}

void Series::SetMarkerShape( MarkerShape shape )
{
  marker_shape = shape;
}

//------------------------------------------------------------------------------

void Series::ApplyFillStyle( SVG::Object* obj )
{
  obj->Attr()->LineColor()->Clear();
  obj->Attr()->FillColor()->Set( &fill_color );
}

void Series::ApplyLineStyle( SVG::Object* obj )
{
  obj->Attr()->SetLineWidth( line_width );
  if ( line_width > 0 ) {
    if ( line_dash > 0 ) {
      obj->Attr()->SetLineDash( line_dash, line_hole );
    }
    if ( marker_show ) {
      obj->Attr()->SetLineJoin( LineJoin::Round );
    }
    obj->Attr()->LineColor()->Set( &line_color );
  } else {
    obj->Attr()->LineColor()->Clear();
  }
  obj->Attr()->FillColor()->Clear();
}

void Series::ApplyMarkStyle( SVG::Object* obj )
{
  obj->Attr()->LineColor()->Clear();
  if (
    marker_shape == MarkerShape::Cross ||
    marker_shape == MarkerShape::LineX ||
    marker_shape == MarkerShape::LineY
  ) {
    ApplyLineStyle( obj );
    obj->Attr()->SetLineDash( 0 );
  } else {
    if ( line_width > 0 ) {
      obj->Attr()->FillColor()->Set( &line_color );
      if ( type != SeriesType::Scatter && type != SeriesType::Point ) {
        obj->Attr()->FillColor()->SetOpacity( 1.0 );
      }
    } else {
      obj->Attr()->FillColor()->Clear();
    }
  }
}

void Series::ApplyHoleStyle( SVG::Object* obj )
{
  obj->Attr()->LineColor()->Clear();
  obj->Attr()->FillColor()->Set( &fill_color );
  if ( type != SeriesType::Scatter && type != SeriesType::Point ) {
    obj->Attr()->FillColor()->SetOpacity( 1.0 );
  }
}

void Series::ApplyTagStyle( SVG::Object* obj )
{
  obj->Attr()->LineColor()->Set( &tag_line_color );
  obj->Attr()->FillColor()->Set( &tag_fill_color );
  obj->Attr()->TextColor()->Set( &tag_text_color );
  obj->Attr()->SetLineWidth( 1 );
  obj->Attr()->TextFont()->SetSize( 12 * tag_size );
  obj->Attr()->TextFont()->SetBold();
}

////////////////////////////////////////////////////////////////////////////////

void Series::PrunePolyAdd( prune_state_t& ps, SVG::Point p )
{
  // Returns the distance from p to the line going from e1 to e2. The sign of
  // the returned distance indicates if p lies to the left (positive) or the
  // right (negative) of the line.
  auto dist2line = []( Point e1, Point e2, Point p )
  {
    double dx = e2.x - e1.x;
    double dy = e2.y - e1.y;
    double px = p.x - e1.x;
    double py = p.y - e1.y;

    double cross = dx * py - dy * px;

    return cross / std::sqrt( dx * dx + dy * dy );
  };

  // Returns true if p was integrated into the p1 to p2 collection thereby
  // causing the previous point (p2) to be pruned.
  auto prune = [&]()
  {
    auto new_e1 = ps.e1;
    auto new_e2 = ps.e2;

    double vex = ps.e2.x - ps.e1.x;
    double vey = ps.e2.y - ps.e1.y;

    bool vex_tiny = std::abs( vex ) < epsilon;
    bool vey_tiny = std::abs( vey ) < epsilon;

    if ( vex_tiny && vey_tiny ) {
      new_e2 = p;
    } else {
      double dot1 = (p.x - ps.e1.x) * vex + (p.y - ps.e1.y) * vey;
      double dot2 = (p.x - ps.e2.x) * vex + (p.y - ps.e2.y) * vey;
      double d;
      if ( dot1 < 0 || dot2 > 0 ) {
        // p is before/after the current e1 to e2 line, so extend e1 or e2.
        if ( dot2 > 0 ) {
          // Extend e2.
          d = dist2line( ps.e1, p, ps.e2 );
        } else {
          // Swap e1/e2 direction and extend new e2 (previous e1).
          d = dist2line( ps.e2, p, ps.e1 );
          std::swap( ps.d1, ps.d2 );
          new_e1 = ps.e2;
        }
        new_e2 = p;
        // Do not accept pruning that causes vertical/horizontal lines to
        // become slightly skewed, as this is a much more visible artifact:
        if ( (vex_tiny || vey_tiny) && std::abs( d ) > epsilon ) return false;
        // We use the distance form the old e2 to the new extended e1/e2 line
        // and update d1/d2 accordingly. This is not mathematically correct,
        // ideally all points from p1 to p2 should be reexamined. But this
        // heuristic is judged to be a good enough to avoid O(n^2) complexity.
        if ( d > 0 ) {
          ps.d1 = ps.d1 + d;
          ps.d2 = std::max( 0.0, ps.d2 - d );
        } else {
          ps.d1 = std::max( 0.0, ps.d1 + d );
          ps.d2 = ps.d2 - d;
        }
      } else {
        d = dist2line( ps.e1, ps.e2, p );
        if ( d > 0 ) {
          ps.d1 = std::max( +ps.d1, +d );
        } else {
          ps.d2 = std::max( +ps.d2, -d );
        }
      }
      if ( ps.d1 > prune_dist || ps.d2 > prune_dist ) return false;
    }

    ps.e1 = new_e1;
    ps.e2 = new_e2;
    ps.p2 = p;
    return true;
  };

  ps.cnt++;

  if ( ps.cnt > 2 ) {
    if ( prune_dist < prune_dist_min || !prune() ) {
      if ( ps.html_enable ) {
        html_db->PreserveSnapPoint( this, ps.p1 );
        html_db->PreserveSnapPoint( this, ps.e1 );
        html_db->PreserveSnapPoint( this, ps.p2 );
        html_db->PreserveSnapPoint( this, ps.e2 );
        html_db->CommitSnapPoints( this, false );
      }
      if ( ps.e1 != ps.p1 ) ps.points.push_back( ps.e1 );
      if ( ps.e2 != ps.p2 ) ps.points.push_back( ps.e2 );
      ps.p1 = ps.e1 = ps.p2;
      ps.points.push_back( ps.p1 );
      ps.p2 = ps.e2 = p;
      ps.d1 = ps.d2 = 0;
    }
  } else {
    if ( ps.cnt == 1 ) {
      ps.points.clear();
      ps.p1 = ps.e1 = p;
      ps.points.push_back( ps.p1 );
    } else {
      ps.p2 = ps.e2 = p;
      ps.d1 = ps.d2 = 0;
    }
  }

  return;
}

void Series::PrunePolyEnd( prune_state_t& ps )
{
  if ( ps.html_enable ) {
    html_db->PreserveSnapPoint( this, ps.p1 );
    html_db->PreserveSnapPoint( this, ps.e1 );
    html_db->PreserveSnapPoint( this, ps.p2 );
    html_db->PreserveSnapPoint( this, ps.e2 );
    html_db->CommitSnapPoints( this, true );
  }
  if ( ps.cnt > 0 ) {
    if ( ps.cnt > 1 ) {
      if ( ps.e1 != ps.p1 ) ps.points.push_back( ps.e1 );
      if ( ps.e2 != ps.p2 ) ps.points.push_back( ps.e2 );
      ps.points.push_back( ps.p2 );
    }
  } else {
    ps.points.clear();
  }
  ps.cnt = 0;
}

////////////////////////////////////////////////////////////////////////////////

uint64_t Series::PrunePointsKey( SVG::Point p )
{
  return
    (static_cast< uint64_t >( p.y * prune_dist_inv ) << 32) |
    (static_cast< uint64_t >( p.x * prune_dist_inv ) <<  0);
}

void Series::PrunePointsAdd( prune_state_t& ps, SVG::Point p )
{
  if ( type != SeriesType::Scatter && prune_dist >= prune_dist_min ) {
    // Make sure extremes are included; for Scatter plot this does not make
    // sense as the points are totally random.
    PrunePolyAdd( ps, p );
  } else {
    if ( ++ps.cnt == 1 ) {
      ps.points.clear();
    }
    if ( ps.html_enable ) {
      html_db->CommitSnapPoints( this, false );
    }
  }
  if ( ps.cnt == 1 ) {
    ps.iso_exists.clear();
    ps.iso_points.clear();
  }
  if ( prune_dist >= prune_dist_min ) {
    uint64_t key = PrunePointsKey( p );
    if ( ps.iso_exists.insert( { key, p } ).second ) {
      ps.iso_points.push_back( p );
      if ( ps.html_enable ) {
        html_db->PreserveSnapPoint( this, p );
      }
    }
  } else {
    ps.iso_points.push_back( p );
    if ( ps.html_enable ) {
      html_db->PreserveSnapPoint( this, p );
    }
  }
}

void Series::PrunePointsEnd( prune_state_t& ps )
{
  if ( ps.cnt == 0 ) {
    ps.points.clear();
    ps.iso_exists.clear();
    ps.iso_points.clear();
  }
  if ( type != SeriesType::Scatter && prune_dist >= prune_dist_min ) {
    PrunePolyEnd( ps );
  } else {
    if ( ps.html_enable ) {
      html_db->CommitSnapPoints( this, true );
    }
  }
  ps.cnt = 0;
  for ( const auto& p : ps.points ) {
    uint64_t key = PrunePointsKey( p );
    auto it = ps.iso_exists.find( key );
    if ( it == ps.iso_exists.end() || it->second != p ) {
      ps.iso_points.push_back( p );
    }
  }
  ps.points = std::move( ps.iso_points );
}

////////////////////////////////////////////////////////////////////////////////

double Series::ToDouble( const std::string_view sv )
{
  if ( sv.empty() ) return Chart::num_skip;

  if ( sv.size() == 1 ) {
    switch ( sv[ 0 ] ) {
      case '!' : return Chart::num_invalid;
      case '-' : return Chart::num_skip;
      default : break;
    }
  }

  const char* p1 = sv.data() + ((sv[ 0 ] == '+') ? 1 : 0);
  const char* p2 = sv.data() + sv.size();
  double d = Chart::num_skip;
  std::from_chars( p1, p2, d );
  return d;
}

void Series::SetDatumAnchor(
  size_t num, size_t cat_ofs, bool no_x, uint32_t y_idx
)
{
  datum_pos = source->cur_pos;
  datum_cat_ofs = cat_ofs;
  datum_num = num;
  datum_no_x = no_x;
  datum_y_idx = y_idx;
}

////////////////////////////////////////////////////////////////////////////////

bool Series::Inside( const SVG::Point p, const SVG::BoundaryBox& bb )
{
  return
    p.x >= bb.min.x && p.x <= bb.max.x &&
    p.y >= bb.min.y && p.y <= bb.max.y;
}

////////////////////////////////////////////////////////////////////////////////

// Returns:
//   0 : No intersection.
//   1 : One intersection; c1 is the point.
//   2 : Two intersections; c1 and c2 are the points.
int Series::ClipLine(
  SVG::Point& c1, SVG::Point& c2, SVG::Point p1, SVG::Point p2,
  const SVG::BoundaryBox& bb
)
{
  // Record original p1.
  Point o1 = p1;

  auto intersect_x = []( U x, Point p1, Point p2 )
  {
    U dx = p1.x - p2.x;
    U dy = p1.y - p2.y;
    U cp = p1.x * p2.y - p1.y * p2.x;
    U y = (dy * x + cp) / dx;
    return y;
  };
  auto intersect_y = []( U y, Point p1, Point p2 )
  {
    U dx = p1.x - p2.x;
    U dy = p1.y - p2.y;
    U cp = p1.x * p2.y - p1.y * p2.x;
    U x = (dx * y - cp) / dy;
    return x;
  };
  auto near = [&]( Point p1, Point p2 )
  {
    return
      std::abs( p1.x - p2.x ) < e1 &&
      std::abs( p1.y - p2.y ) < e1;
  };

  // The clip coordinate for the four sides and a flag telling if it is valid.
  U bot_x{ 0 }; bool bot_v = false;
  U top_x{ 0 }; bool top_v = false;
  U lft_y{ 0 }; bool lft_v = false;
  U rgt_y{ 0 }; bool rgt_v = false;

  // Detect bottom and top clippings.
  if ( p1.y > p2.y ) std::swap( p1, p2 );
  if ( p1.y < bb.min.y && p2.y >= bb.min.y ) {
    bot_x = intersect_y( bb.min.y, p1, p2 );
    bot_v = bot_x > (bb.min.x - e2) && bot_x < (bb.max.x + e2);
  }
  if ( p1.y <= bb.max.y && p2.y > bb.max.y ) {
    top_x = intersect_y( bb.max.y, p1, p2 );
    top_v = top_x > (bb.min.x - e2) && top_x < (bb.max.x + e2);
  }

  // Detect left and right clippings.
  if ( p1.x > p2.x ) std::swap( p1, p2 );
  if ( p1.x < bb.min.x && p2.x >= bb.min.x ) {
    lft_y = intersect_x( bb.min.x, p1, p2 );
    lft_v = lft_y > (bb.min.y - e2) && lft_y < (bb.max.y + e2);
  }
  if ( p1.x <= bb.max.x && p2.x > bb.max.x ) {
    rgt_y = intersect_x( bb.max.x, p1, p2 );
    rgt_v = rgt_y > (bb.min.y - e2) && rgt_y < (bb.max.y + e2);
  }

  // The four potential clip points.
  Point bot_c{ bot_x, bb.min.y };
  Point top_c{ top_x, bb.max.y };
  Point lft_c{ bb.min.x, lft_y };
  Point rgt_c{ bb.max.x, rgt_y };

  // Prune very close clip-detections in the corners.
  if ( bot_v && lft_v && near( bot_c, lft_c ) ) lft_v = false;
  if ( bot_v && rgt_v && near( bot_c, rgt_c ) ) rgt_v = false;
  if ( top_v && lft_v && near( top_c, lft_c ) ) lft_v = false;
  if ( top_v && rgt_v && near( top_c, rgt_c ) ) rgt_v = false;

  // Deliver result.
  Point* c = &c1;
  int n = 0;
  if ( bot_v ) { *c = bot_c; c = &c2; n++; }
  if ( top_v ) { *c = top_c; c = &c2; n++; }
  if ( lft_v ) { *c = lft_c; c = &c2; n++; }
  if ( rgt_v ) { *c = rgt_c; c = &c2; n++; }

  // When we have two clip points, we must make sure that the order of points on
  // line is p1:c1:c2:p2, where p1 and p2 are the original arguments.
  if ( n == 2 ) {
    p1 = o1;    // Restore original argument.
    double dx1 = c1.x - p1.x;
    double dy1 = c1.y - p1.y;
    double dx2 = c2.x - p1.x;
    double dy2 = c2.y - p1.y;
    if ( dx1*dx1 + dy1*dy1 > dx2*dx2 + dy2*dy2 ) {
      std::swap( c1, c2 );
    }
  }

  return n;
}

SVG::Point Series::MoveInside( SVG::Point p )
{
  if ( p.x < chart_area.min.x ) p.x = chart_area.min.x;
  if ( p.x > chart_area.max.x ) p.x = chart_area.max.x;
  if ( p.y < chart_area.min.y ) p.y = chart_area.min.y;
  if ( p.y > chart_area.max.y ) p.y = chart_area.max.y;
  return p;
}

////////////////////////////////////////////////////////////////////////////////

void Series::UpdateLegendBoxes(
  Point p1, Point p2,
  bool p1_inc, bool p2_inc
)
{
  Point c1;
  Point c2;
  for ( LegendBox& lb : *lb_list ) {
    if ( p1.x < lb.bb.min.x && p2.x < lb.bb.min.x ) continue;
    if ( p1.x > lb.bb.max.x && p2.x > lb.bb.max.x ) continue;
    if ( p1.y < lb.bb.min.y && p2.y < lb.bb.min.y ) continue;
    if ( p1.y > lb.bb.max.y && p2.y > lb.bb.max.y ) continue;
    bool p1_inside = Inside( p1, lb.bb );
    bool p2_inside = Inside( p2, lb.bb );
    if ( p1_inside && p1_inc ) lb.weight1 += 1;
    if ( p2_inside && p2_inc ) lb.weight1 += 1;
    if ( p1_inside && p2_inside ) {
      c1 = p1;
      c2 = p2;
    } else {
      int c = ClipLine( c1, c2, p1, p2, lb.bb );
      if ( p1_inside || p2_inside ) {
        if ( c != 1 ) continue;
        c2 = p1_inside ? p1 : p2;
      } else {
        if ( c != 2 ) continue;
      }
    }
    double dx = c1.x - c2.x;
    double dy = c1.y - c2.y;
    lb.weight2 += std::sqrt( dx*dx + dy*dy );
  }
}

////////////////////////////////////////////////////////////////////////////////

void Series::DetermineVisualProperties( void )
{
  marker_show = false;
  marker_show_out = false;
  marker_show_int = false;
  has_line = false;
  has_fill = false;
  line_color_shown = false;
  fill_color_shown = false;

  if (
    type == SeriesType::XY ||
    type == SeriesType::Line ||
    type == SeriesType::Lollipop ||
    type == SeriesType::Bar ||
    type == SeriesType::StackedBar ||
    type == SeriesType::LayeredBar ||
    type == SeriesType::Area ||
    type == SeriesType::StackedArea
  ) {
    has_line = line_width > 0 && !line_color.IsClear();
    line_color_shown = has_line;
  }

  if (
    type == SeriesType::Bar ||
    type == SeriesType::StackedBar ||
    type == SeriesType::LayeredBar ||
    type == SeriesType::Area ||
    type == SeriesType::StackedArea
  ) {
    has_fill = !fill_color.IsClear();
    fill_color_shown = has_fill;
  }

  // Minimal tag distance from center of data point.
  tag_dist_x = tag_dist_y = has_line ? (line_width / 2) : 0;

  if (
    type != SeriesType::XY &&
    type != SeriesType::Scatter &&
    type != SeriesType::Line &&
    type != SeriesType::Point &&
    type != SeriesType::Lollipop &&
    type != SeriesType::Area &&
    type != SeriesType::StackedArea
  ) {
    return;
  }

  U radius = marker_size / 2;

  auto compute = [&]( MarkerDims& m, U delta )
  {
    switch ( marker_shape ) {
      case MarkerShape::Square :
      case MarkerShape::Cross :
        m.x1 = -1.0000 * (0.9 * radius + delta);
        m.x2 = +1.0000 * (0.9 * radius + delta);
        m.y1 = -1.0000 * (0.9 * radius + delta);
        m.y2 = +1.0000 * (0.9 * radius + delta);
        break;
      case MarkerShape::Triangle :
        m.x1 = -1.7320 * (0.7 * radius + delta);
        m.x2 = +1.7320 * (0.7 * radius + delta);
        m.y1 = -1.0000 * (0.7 * radius + delta);
        m.y2 = +2.0000 * (0.7 * radius + delta);
        break;
      case MarkerShape::InvTriangle :
        m.x1 = -1.7320 * (0.7 * radius + delta);
        m.x2 = +1.7320 * (0.7 * radius + delta);
        m.y1 = -2.0000 * (0.7 * radius + delta);
        m.y2 = +1.0000 * (0.7 * radius + delta);
        break;
      case MarkerShape::Diamond :
        m.x1 = -1.4142 * (0.9 * radius + delta);
        m.x2 = +1.4142 * (0.9 * radius + delta);
        m.y1 = -1.4142 * (0.9 * radius + delta);
        m.y2 = +1.4142 * (0.9 * radius + delta);
        break;
      case MarkerShape::Star :
        m.x1 = -2.0 * (0.7 * radius + delta);
        m.x2 = +2.0 * (0.7 * radius + delta);
        m.y1 = -2.0 * (0.7 * radius + delta);
        m.y2 = +2.0 * (0.7 * radius + delta);
        break;
      case MarkerShape::LineX :
        m.x1 = (axis_x->angle == 0) ? -radius : 0.0;
        m.x2 = (axis_x->angle == 0) ? +radius : 0.0;
        m.y1 = (axis_x->angle == 0) ? 0.0 : -radius;
        m.y2 = (axis_x->angle == 0) ? 0.0 : +radius;
        break;
      case MarkerShape::LineY :
        m.x1 = (axis_x->angle != 0) ? -radius : 0.0;
        m.x2 = (axis_x->angle != 0) ? +radius : 0.0;
        m.y1 = (axis_x->angle != 0) ? 0.0 : -radius;
        m.y2 = (axis_x->angle != 0) ? 0.0 : +radius;
        break;
      default :
        m.x1 = -1.0000 * (1.0 * radius + delta);
        m.x2 = +1.0000 * (1.0 * radius + delta);
        m.y1 = -1.0000 * (1.0 * radius + delta);
        m.y2 = +1.0000 * (1.0 * radius + delta);
        break;
    }
    return;
  };

  U lw = line_width;

  if ( radius > 0 ) {
    marker_show_out = !line_color.IsClear() && line_width > 0;
    marker_show_int = !fill_color.IsClear();
    if (
      marker_shape == MarkerShape::Cross ||
      marker_shape == MarkerShape::LineX ||
      marker_shape == MarkerShape::LineY
    ) {
      marker_show_int = false;
    } else {
      if ( 2 * radius < 3 * line_width ) {
        if ( has_line && 2 * radius < line_width ) {
          lw = line_width / 2 - radius;
          radius = line_width / 2;
          marker_show_out = marker_show_out && lw > 0;
        } else {
          marker_show_int = marker_show_int && !marker_show_out;
        }
      }
    }
  }
  marker_show = marker_show_out || marker_show_int;
  if ( !marker_show_out || !marker_show_int ) lw = 0;

  if ( marker_show_int ) fill_color_shown = true;
  if ( marker_show_out ) line_color_shown = true;

  compute( marker_int, -lw );
  compute( marker_out, 0 );

  tag_dist_x =
    std::max(
      marker_show ? ((marker_out.x2 - marker_out.x1) / 2) : 0, +tag_dist_x
    );
  tag_dist_y =
    std::max(
      marker_show ? ((marker_out.y2 - marker_out.y1) / 2) : 0, +tag_dist_y
    );

  return;
}

////////////////////////////////////////////////////////////////////////////////

bool Series::SameLegend( Series* s1, Series* s2 )
{
  auto is_point = []( Series* s )
  {
    return
      s->type == SeriesType::Point ||
      s->type == SeriesType::Scatter;
  };

  auto is_line = []( Series* s )
  {
    return
      s->type == SeriesType::Line ||
      s->type == SeriesType::XY;
  };

  auto is_bar = []( Series* s )
  {
    return
      s->type == SeriesType::Bar ||
      s->type == SeriesType::StackedBar ||
      s->type == SeriesType::LayeredBar;
  };

  auto is_area = []( Series* s )
  {
    return
      s->type == SeriesType::Area ||
      s->type == SeriesType::StackedArea;
  };

  bool same =
    (is_point( s1 ) && is_point( s2 )) ||
    (is_line ( s1 ) && is_line ( s2 )) ||
    (is_bar  ( s1 ) && is_bar  ( s2 )) ||
    (is_area ( s1 ) && is_area ( s2 )) ||
    (s1->type == s2->type);

  same =
    same &&
    s1->name             == s2->name &&
    s1->marker_show      == s2->marker_show &&
    s1->marker_show_out  == s2->marker_show_out &&
    s1->marker_show_int  == s2->marker_show_int &&
    s1->has_line         == s2->has_line &&
    s1->has_fill         == s2->has_fill &&
    s1->line_color_shown == s2->line_color_shown &&
    s1->fill_color_shown == s2->fill_color_shown;

  if ( same && s1->line_color_shown ) {
    same = s1->line_color == s2->line_color;
  }
  if ( same && s1->fill_color_shown ) {
    same = s1->fill_color == s2->fill_color;
  }

  if ( same && s1->has_line ) {
    same =
      s1->legend_outline == s2->legend_outline &&
      s1->line_width     == s2->line_width &&
      s1->line_dash      == s2->line_dash &&
      s1->line_hole      == s2->line_hole;
  }

  if ( same && s1->marker_show ) {
    same =
      s1->marker_size  == s2->marker_size &&
      s1->marker_shape == s2->marker_shape;
  }

  return same;
}

////////////////////////////////////////////////////////////////////////////////

void Series::BuildMarker( Group* g, const MarkerDims& m, SVG::Point p )
{
  Poly* poly;

  switch ( marker_shape ) {
    case MarkerShape::Circle :
      g->Add( new Circle( p, m.x2 ) );
      break;
    case MarkerShape::Square :
      g->Add( new Rect(
        p.x + m.x1, p.y + m.y1,
        p.x + m.x2, p.y + m.y2
      ) );
      break;
    case MarkerShape::Triangle :
      poly =
        new Poly(
          { p.x, p.y + m.y2,
            p.x + m.x2, p.y + m.y1,
            p.x + m.x1, p.y + m.y1
          }
        );
      poly->Close();
      g->Add( poly );
      break;
    case MarkerShape::InvTriangle :
      poly =
        new Poly(
          { p.x, p.y + m.y1,
            p.x + m.x2, p.y + m.y2,
            p.x + m.x1, p.y + m.y2
          }
        );
      poly->Close();
      g->Add( poly );
      break;
    case MarkerShape::Diamond :
      poly =
        new Poly(
          { p.x + m.x2, p.y,
            p.x, p.y + m.y2,
            p.x + m.x1, p.y,
            p.x, p.y + m.y1
          }
        );
      poly->Close();
      g->Add( poly );
      break;
    case MarkerShape::Cross :
      g = g->AddNewGroup();
      g->Add(
        new Line(
          p.x + m.x1, p.y + m.y1,
          p.x + m.x2, p.y + m.y2
        )
      );
      g->Add(
        new Line(
          p.x + m.x2, p.y + m.y1,
          p.x + m.x1, p.y + m.y2
        )
      );
      break;
    case MarkerShape::Star : {
      const double d = 0.35;
      poly =
        new Poly(
          { p.x + m.x2, p.y,
            p.x + m.x2 * d, p.y + m.y2 * d,
            p.x, p.y + m.y2,
            p.x + m.x1 * d, p.y + m.y2 * d,
            p.x + m.x1, p.y,
            p.x + m.x1 * d, p.y + m.y1 * d,
            p.x, p.y + m.y1,
            p.x + m.x2 * d, p.y + m.y1 * d
          }
        );
      poly->Close();
      g->Add( poly );
      break;
    }
    case MarkerShape::LineX :
    case MarkerShape::LineY :
      g->Add(
        new Line(
          p.x + m.x1, p.y + m.y1,
          p.x + m.x2, p.y + m.y2
        )
      );
      break;
    default :
      break;
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////

void Series::ComputeStackDir()
{
  stack_dir = 0;
  if ( type != SeriesType::StackedArea ) return;

  DatumBegin();
  for (
    size_t i = 0; i < datum_num;
    ++i, DatumNext()
  ) {
    std::string_view svx;
    std::string_view svy;
    source->GetDatum( svx, svy, datum_no_x, datum_y_idx );
    double y = ToDouble( svy ) - base;
    if ( y < 0 ) {
      stack_dir = -1;
      return;
    }
    if ( y > 0 ) {
      stack_dir = +1;
      return;
    }
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////

void Series::DetermineMinMax(
  std::vector< double >& ofs_pos,
  std::vector< double >& ofs_neg
)
{
  bool stackable =
    type == SeriesType::Bar ||
    type == SeriesType::StackedBar ||
    type == SeriesType::StackedArea;

  def_x = false;
  min_x = axis_x->log_scale ? 10 : 0;
  max_x = axis_x->log_scale ? 10 : 0;

  def_y = false;
  min_y = axis_y->log_scale ? 10 : 0;
  max_y = axis_y->log_scale ? 10 : 0;
  min_y_is_base = false;
  max_y_is_base = false;

  max_tag_x_size = 0;
  max_tag_y_size = 0;

  if (
    stackable ||
    type == SeriesType::LayeredBar ||
    type == SeriesType::Lollipop ||
    type == SeriesType::Area
  ) {
    double y = base;
    if ( axis_y->Valid( y ) ) {
      if ( !def_y || min_y > y ) {
        min_y = y;
        min_y_is_base = true;
      }
      if ( !def_y || max_y < y ) {
        max_y = y;
        max_y_is_base = true;
      }
      def_y = true;
    }
  }

  DatumBegin();
  for ( size_t i = 0; i < datum_num; ++i, DatumNext() ) {
    std::string_view svx;
    std::string_view svy;
    source->GetDatum( svx, svy, datum_no_x, datum_y_idx );
    double x = datum_cat_ofs + i;
    double y = ToDouble( svy );
    if ( !is_cat ) x = ToDouble( svx );
    if ( !axis_x->Valid( x ) ) continue;
    if ( !axis_y->Valid( y ) ) continue;
    {
      if ( !x_of_valid_defined ) x_of_fst_valid = x;
      x_of_lst_valid = x;
      x_of_valid_defined = true;
    }
    if ( stackable ) {
      size_t i = x;
      y -= base;
      if ( stack_dir < 0 || (stack_dir == 0 && y < 0) ) {
        y += ofs_neg.at( i );
        ofs_neg[ i ] = y;
      } else {
        y += ofs_pos.at( i );
        ofs_pos[ i ] = y;
      }
      if ( !axis_y->Valid( y ) ) continue;
    }
    max_tag_x_size = std::max( max_tag_x_size, svx.size() );
    max_tag_y_size = std::max( max_tag_y_size, svy.size() );
    if ( !def_x || min_x > x ) min_x = x;
    if ( !def_x || max_x < x ) max_x = x;
    if ( !def_y || min_y > y ) {
      min_y = y;
      min_y_is_base = false;
    }
    if ( !def_y || max_y < y ) {
      max_y = y;
      max_y_is_base = false;
    }
    def_x = true;
    def_y = true;
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////

void Series::BuildArea(
  Group* fill_g,
  Group* line_g,
  Group* mark_g,
  Group* hole_g,
  Group* tag_g,
  std::vector< double >* base_ofs,
  std::vector< SVG::Point >* base_pts
)
{
  prune_state_t fill_ps;
  prune_state_t line_ps;
  prune_state_t mark_ps;
  prune_state_t base_ps;

  line_ps.html_enable = html_db != nullptr && !marker_show;
  mark_ps.html_enable = html_db != nullptr && marker_show;

  Pos tag_direction;
  bool reverse = axis_y->reverse ^ (stack_dir < 0);
  if ( axis_x->angle == 0 ) {
    tag_direction = reverse ? Pos::Bottom : Pos::Top;
  } else {
    tag_direction = reverse ? Pos::Left : Pos::Right;
  }

  bool first_in_stack = true;

  if ( type == SeriesType::StackedArea ) {
    first_in_stack = base_pts->empty();
    // Initialize the fill polygon with the points from the top of the previous
    // polygon, which are contained in base_pts.
    if ( has_fill ) {
      for ( auto it = base_pts->rbegin(); it != base_pts->rend(); ++it ) {
        PrunePolyAdd( fill_ps, *it );
      }
    }
    base_pts->clear();
    base_pts->shrink_to_fit();
  }

  auto commit_line = [&]( void )
  {
    PrunePolyEnd( line_ps );
    if ( has_line && !line_ps.points.empty() ) {
      auto it = line_ps.points.cbegin();
      uint64_t d = (line_ps.points.size() + max_poly - 1) / max_poly;
      uint64_t n = 0;
      for ( uint64_t i = 1; i <= d; ++i ) {
        uint64_t m = line_ps.points.size() * i / d;
        Poly* poly = new Poly();
        line_g->Add( poly );
        while ( n < m ) {
          poly->Add( *(it++) );
          ++n;
        }
      }
    }
  };

  Point ap_prv_p;
  size_t ap_line_cnt = 0;
  auto add_point =
  [&](
    Point p, size_t cat_idx, std::string_view tag_x, std::string_view tag_y,
    bool is_datum, bool on_line
  )
  {
    if ( on_line ) {
      UpdateLegendBoxes(
        (ap_line_cnt == 0) ? p : ap_prv_p, p, false, is_datum
      );
    }
    if ( type == SeriesType::StackedArea ) {
      PrunePolyAdd( base_ps, p );
    }
    if ( has_fill ) {
      PrunePolyAdd( fill_ps, p );
    }
    if ( on_line ) {
      PrunePolyAdd( line_ps, p );
    }
    if ( is_datum ) {
      if ( marker_show ) PrunePointsAdd( mark_ps, p );
      if ( html_db ) {
        html_db->RecordSnapPoint( this, p, cat_idx, tag_x, tag_y );
      }
    }
    if ( tag_enable ) {
      tag_db->LineTag(
        this, tag_g, p, tag_x, tag_y, is_datum,
        has_line && on_line && ap_line_cnt > 0, tag_direction
      );
    }
    if ( on_line && (is_datum || ap_line_cnt == 0) ) {
      ap_line_cnt++;
    } else {
      commit_line();
      ap_line_cnt = 0;
    }
    ap_prv_p = p;
    return;
  };

  Point dp_prv_p;
  bool dp_prv_on_line = false;
  bool dp_prv_inside = false;
  bool dp_first = true;
  auto do_point =
  [&](
    Point p, size_t cat_idx, std::string_view tag_x, std::string_view tag_y,
    bool on_line = true
  )
  {
    if ( axis_x->angle != 0 ) std::swap( p.x, p.y );
    bool inside = Inside( p );
    if ( dp_first ) {
      if ( inside ) {
        add_point( p, cat_idx, tag_x, tag_y, on_line, on_line );
      }
    } else {
      if ( dp_prv_inside && inside ) {
        // Common case when we stay inside the chart area.
        add_point( p, cat_idx, tag_x, tag_y, on_line, on_line );
      } else {
        // Handle clipping in and out of the chart area.
        Point c1, c2;
        int n = ClipLine( c1, c2, dp_prv_p, p );
        if ( dp_prv_inside ) {
          // We went from inside to now outside.
          if ( n == 1 ) {
            add_point(
              c1, cat_idx, tag_x, tag_y, false, on_line && dp_prv_on_line
            );
          }
        } else
        if ( inside ) {
          // We went from outside to now inside.
          if ( n == 1 ) {
            add_point(
              c1, cat_idx, tag_x, tag_y, false, on_line && dp_prv_on_line
            );
          }
          add_point( p, cat_idx, tag_x, tag_y, on_line, on_line );
        } else
        if ( n == 2 ) {
          // We are still outside, but the line segment passes through the
          // chart area.
          add_point( c1, cat_idx, tag_x, tag_y, false, on_line && dp_prv_on_line );
          add_point( c2, cat_idx, tag_x, tag_y, false, on_line && dp_prv_on_line );
        }
      }
    }
    if ( !inside ) {
      add_point( MoveInside( p ), cat_idx, tag_x, tag_y, false, false );
    }
    dp_prv_p = p;
    dp_prv_on_line = on_line;
    dp_prv_inside = inside;
    dp_first = false;
    return;
  };

  if ( main->category_num > 0 ) {
    double beg_y = base;
    double end_y = base;
    if ( type == SeriesType::StackedArea ) {
      beg_y = base_ofs->front();
      end_y = base_ofs->back();
    }
    Point beg_p{
      axis_x->Coor( 0 ),
      axis_y->Coor( beg_y )
    };
    Point end_p{
      axis_x->Coor( main->category_num - 1 ),
      axis_y->Coor( end_y )
    };
    if ( first_in_stack ) do_point( beg_p, 0, "", "", false );
    double prv_base = 0;
    bool prv_valid = false;
    bool first = true;
    for ( size_t cat_idx = 0; cat_idx < main->category_num; ++cat_idx ) {
      std::string_view svx;
      std::string_view svy;
      double y = num_invalid;
      if ( cat_idx == datum_cat_ofs ) {
        DatumBegin();
      } else
      if ( cat_idx > datum_cat_ofs && cat_idx < datum_cat_ofs + datum_num ) {
        DatumNext();
      }
      if ( x_of_valid_defined ) {
        if ( cat_idx >= x_of_fst_valid && cat_idx <= x_of_lst_valid ) {
          source->GetDatum( svx, svy, datum_no_x, datum_y_idx );
          y = ToDouble( svy );
        }
      }
      if ( axis_y->Skip( y ) ) {
        continue;
      }
      bool valid = axis_y->Valid( y );
      y -= base;
      if ( !first && prv_valid && !valid ) {
        Point p{ axis_x->Coor( cat_idx - 1 ), axis_y->Coor( base ) };
        do_point( p, cat_idx, svx, svy, false );
      }
      if ( !valid ) y = 0;
      if ( type == SeriesType::StackedArea ) {
        prv_base = base_ofs->at( cat_idx );
        y += prv_base;
        base_ofs->at( cat_idx ) = y;
      } else {
        y += base;
      }
      if ( !first && !prv_valid && valid ) {
        Point p{ axis_x->Coor( cat_idx ), axis_y->Coor( base ) };
        do_point( p, cat_idx, svx, svy, false );
      }
      Point p{ axis_x->Coor( cat_idx ), axis_y->Coor( y ) };
      do_point( p, cat_idx, svx, svy, valid );
      prv_valid = valid;
      first = false;
    }
    if ( first_in_stack ) do_point( end_p, 0, "", "", false );
  }

  PrunePolyEnd( fill_ps );
  if ( !fill_ps.points.empty() ) {
    Poly* poly = new Poly();
    fill_g->Add( poly );
    for ( auto& p : fill_ps.points ) {
      poly->Add( p );
    }
    poly->Close();
  }

  commit_line();

  PrunePointsEnd( mark_ps );
  for ( auto& p : mark_ps.points ) {
    if ( marker_show_out ) BuildMarker( mark_g, marker_out, p );
    if ( marker_show_int ) BuildMarker( hole_g, marker_int, p );
  }

  if ( type == SeriesType::StackedArea ) {
    PrunePointsEnd( base_ps );
    *base_pts = std::move( base_ps.points );
  }

  return;
}

//------------------------------------------------------------------------------

void Series::BuildBar(
  Group* fill_g,
  Group* tbar_g,
  Group* line_g,
  Group* mark_g,
  Group* hole_g,
  Group* tag_g,
  uint32_t bar_num,
  uint32_t bar_tot,
  std::vector< double >* ofs_pos,
  std::vector< double >* ofs_neg
)
{
  Pos zero_direction = Pos::Auto;
  {
    bool has_pos_bar = false;
    bool has_neg_bar = false;
    DatumBegin();
    for ( size_t i = 0; i < datum_num; ++i, DatumNext() ) {
      std::string_view svx;
      std::string_view svy;
      source->GetDatum( svx, svy, datum_no_x, datum_y_idx );
      double y = ToDouble( svy );
      if ( axis_y->Valid( y ) ) {
        if ( y - base > 0 ) has_pos_bar = true;
        if ( y - base < 0 ) has_neg_bar = true;
      }
    }
    if ( axis_x->angle == 0 ) {
      if ( axis_y->reverse ) {
        zero_direction = (has_pos_bar || !has_neg_bar) ? Pos::Bottom : Pos::Top;
      } else {
        zero_direction = (has_pos_bar || !has_neg_bar) ? Pos::Top : Pos::Bottom;
      }
    } else {
      if ( axis_y->reverse ) {
        zero_direction = (has_pos_bar || !has_neg_bar) ? Pos::Left : Pos::Right;
      } else {
        zero_direction = (has_pos_bar || !has_neg_bar) ? Pos::Right : Pos::Left;
      }
    }
  }

  double wx;    // Width of bar.
  double cx;    // Center of bar.
  {
    double sa = 1.0 - main->bar_all_width;
    double so = 1.0 - main->bar_one_width;
    so = main->bar_all_width * so / (bar_tot - so);
    if ( sa < so ) {
      so = (1 - main->bar_one_width) / bar_tot;
      sa = so;
    }
    wx = (1.0 - sa + so) / bar_tot - so;
    cx = sa/2 - 0.5 + wx/2 + bar_num * (wx + so);
    if ( bar_layer_tot > 1 ) {
      wx -=
        bar_layer_num * wx * (1.0 - main->bar_layered_width)
        / (bar_layer_tot - 1);
    }
  }

  Point p1;
  Point p2;

  DatumBegin();
  for ( size_t i = 0; i < datum_num; ++i, DatumNext() ) {
    size_t cat_idx = datum_cat_ofs + i;
    double x = cat_idx + cx;
    std::string_view svx;
    std::string_view svy;
    source->GetDatum( svx, svy, datum_no_x, datum_y_idx );
    double y = ToDouble( svy );
    if ( !axis_y->Valid( y ) ) continue;

    U q = axis_x->Coor( x );
    p1.x = p2.x = q;
    if ( type == SeriesType::Lollipop ) {
      p1.y = axis_y->Coor( base );
      p2.y = axis_y->Coor( y );
    } else {
      double yb = y - base;
      if ( yb < 0 ) {
        p1.y = axis_y->Coor( ofs_neg->at( cat_idx ) );
        ofs_neg->at( cat_idx ) += yb;
        p2.y = axis_y->Coor( ofs_neg->at( cat_idx ) );
      } else {
        p1.y = axis_y->Coor( ofs_pos->at( cat_idx ) );
        ofs_pos->at( cat_idx ) += yb;
        p2.y = axis_y->Coor( ofs_pos->at( cat_idx ) );
      }
    }
    if ( axis_x->angle != 0 ) {
      std::swap( p1.x, p1.y );
      std::swap( p2.x, p2.y );
    }

    bool p1_inside = Inside( p1 );
    bool p2_inside = Inside( p2 );
    if ( !p1_inside || !p2_inside ) {
      Point c1, c2;
      int n = ClipLine( c1, c2, p1, p2 );
      if ( p1_inside ) {
        if ( n != 1 ) continue;
        p2 = c1;
      } else
      if ( p2_inside ) {
        if ( n != 1 ) continue;
        p1 = c1;
      } else
      {
        if ( n != 2 ) continue;
        p1 = c1;
        p2 = c2;
      }
      if ( axis_x->angle == 0 ) {
        p1.x = p2.x = q;
      } else {
        p1.y = p2.y = q;
      }
    }

    if ( html_db && p2_inside ) {
      html_db->RecordSnapPoint( this, p2, cat_idx, svx, svy );
      html_db->PreserveSnapPoint( this, p2 );
      html_db->CommitSnapPoints( this, true );
    }

    if ( tag_enable ) {
      Pos direction = zero_direction;
      if ( p2.x > p1.x ) direction = Pos::Right;
      if ( p2.x < p1.x ) direction = Pos::Left;
      if ( p2.y > p1.y ) direction = Pos::Top;
      if ( p2.y < p1.y ) direction = Pos::Bottom;
      tag_db->BarTag( this, tag_g, p1, p2, svy, direction );
    }

    if ( type == SeriesType::Lollipop ) {
      line_g->Add( new Line( p1, p2 ) );
      if ( p2_inside && marker_show ) {
        if ( marker_show_out ) BuildMarker( mark_g, marker_out, p2 );
        if ( marker_show_int ) BuildMarker( hole_g, marker_int, p2 );
      }
      UpdateLegendBoxes( p1, p2, false, true );
    }

    if (
      y != base &&
      ( type == SeriesType::Bar ||
        type == SeriesType::LayeredBar ||
        type == SeriesType::StackedBar
      )
    ) {
      U w = std::abs( axis_x->Coor( wx / 2 ) - axis_x->Coor( 0 ) );
      bool cut_bot = false;
      bool cut_top = false;
      bool cut_lft = false;
      bool cut_rgt = false;
      if ( axis_x->angle == 0 ) {
        p1.x -= w;
        p2.x += w;
        if ( p1.y < p2.y ) {
          if ( !p1_inside ) cut_bot = true;
          if ( !p2_inside ) cut_top = true;
          cut_bot = true;
        }
        if ( p1.y > p2.y ) {
          if ( !p1_inside ) cut_top = true;
          if ( !p2_inside ) cut_bot = true;
          cut_top = true;
        }
      } else {
        p1.y -= w;
        p2.y += w;
        if ( p1.x < p2.x ) {
          if ( !p1_inside ) cut_lft = true;
          if ( !p2_inside ) cut_rgt = true;
          cut_lft = true;
        }
        if ( p1.x > p2.x ) {
          if ( !p1_inside ) cut_rgt = true;
          if ( !p2_inside ) cut_lft = true;
          cut_rgt = true;
        }
      }
      if ( p1.x > p2.x ) std::swap( p1.x, p2.x );
      if ( p1.y > p2.y ) std::swap( p1.y, p2.y );
      UpdateLegendBoxes( Point( p1.x, p1.y ), Point( p1.x, p2.y ) );
      UpdateLegendBoxes( Point( p1.x, p1.y ), Point( p2.x, p1.y ) );
      UpdateLegendBoxes( Point( p2.x, p1.y ), Point( p2.x, p2.y ) );
      UpdateLegendBoxes( Point( p1.x, p2.y ), Point( p2.x, p2.y ) );
      bool has_interior =
        p2.x - p1.x > line_width &&
        p2.y - p1.y > line_width;
      if ( has_interior ) {
        if ( has_fill ) {
          Point c1{ p1 };
          Point c2{ p2 };
          if ( has_line ) {
            U q = (line_dash > 0) ? 0 : (line_width / 2);
            c1.x += cut_lft ? 0 : +q;
            c2.x -= cut_rgt ? 0 : +q;
            c1.y += cut_bot ? 0 : +q;
            c2.y -= cut_top ? 0 : +q;
          }
          fill_g->Add( new Rect( c1, c2 ) );
        }
        if ( has_line ) {
          U d = line_width / 2;
          U q = std::min( 0.25, +d );
          if ( cut_bot && cut_top ) {
            line_g->Add( new Line( p1.x + d, p1.y - q, p1.x + d, p2.y + q ) );
            line_g->Add( new Line( p2.x - d, p1.y - q, p2.x - d, p2.y + q ) );
          } else
          if ( cut_lft && cut_rgt ) {
            line_g->Add( new Line( p1.x - q, p1.y + d, p2.x + q, p1.y + d ) );
            line_g->Add( new Line( p1.x - q, p2.y - d, p2.x + q, p2.y - d ) );
          } else
          if ( cut_bot ) {
            line_g->Add( new Poly(
              { p1.x + d, p1.y - q, p1.x + d, p2.y - d,
                p2.x - d, p2.y - d, p2.x - d, p1.y - q
              }
            ) );
          } else
          if ( cut_top ) {
            line_g->Add( new Poly(
              { p1.x + d, p2.y + q, p1.x + d, p1.y + d,
                p2.x - d, p1.y + d, p2.x - d, p2.y + q
              }
            ) );
          } else
          if ( cut_lft ) {
            line_g->Add( new Poly(
              { p1.x - q, p1.y + d, p2.x - d, p1.y + d,
                p2.x - d, p2.y - d, p1.x - q, p2.y - d
              }
            ) );
          } else
          if ( cut_rgt ) {
            line_g->Add( new Poly(
              { p2.x + q, p1.y + d, p1.x + d, p1.y + d,
                p1.x + d, p2.y - d, p2.x + q, p2.y - d
              }
            ) );
          } else {
            line_g->Add( new Rect( p1.x + d, p1.y + d, p2.x - d, p2.y - d ) );
          }
        }
      } else {
        tbar_g->Add( new Rect( p1, p2 ) );
      }
    }

  }

  return;
}

//------------------------------------------------------------------------------

void Series::BuildLine(
  Group* line_g,
  Group* mark_g,
  Group* hole_g,
  Group* tag_g
)
{
  prune_state_t line_ps;
  prune_state_t mark_ps;

  line_ps.html_enable = html_db != nullptr && has_line;
  mark_ps.html_enable = html_db != nullptr && !has_line;

  bool adding_segments = false;

  Pos tag_direction;
  if ( axis_x->angle == 0 ) {
    tag_direction = axis_y->reverse ? Pos::Bottom : Pos::Top;
  } else {
    tag_direction = axis_y->reverse ? Pos::Left : Pos::Right;
  }

  Point prv;
  auto add_point =
  [&](
    Point p, double x, std::string_view tag_x, std::string_view tag_y,
    bool clipped = false
  )
  {
    if ( has_line ) {
      PrunePolyAdd( line_ps, p );
      if ( adding_segments ) {
        UpdateLegendBoxes( prv, p );
      }
    } else {
      UpdateLegendBoxes( p, p, true, false );
    }
    if ( !clipped ) {
      if ( marker_show ) PrunePointsAdd( mark_ps, p );
      if ( html_db ) {
        size_t cat_idx = static_cast< size_t >( x );
        html_db->RecordSnapPoint( this, p, cat_idx, tag_x, tag_y );
      }
    }
    if ( !has_line && !marker_show && html_db ) {
      html_db->CommitSnapPoints( this, true );
    }
    if ( tag_enable ) {
      tag_db->LineTag(
        this, tag_g, p, tag_x, tag_y, !clipped,
        adding_segments && has_line, tag_direction
      );
    }
    prv = p;
    adding_segments = true;
  };

  auto end_point = [&]( void )
  {
    PrunePolyEnd( line_ps );
    if ( !line_ps.points.empty() ) {
      auto it = line_ps.points.cbegin();
      uint64_t d = (line_ps.points.size() + max_poly - 1) / max_poly;
      uint64_t n = 0;
      for ( uint64_t i = 1; i <= d; ++i ) {
        uint64_t m = line_ps.points.size() * i / d;
        Poly* poly = new Poly();
        line_g->Add( poly );
        while ( n < m ) {
          poly->Add( *(it++) );
          ++n;
        }
      }
    }
    PrunePointsEnd( mark_ps );
    for ( auto& p : mark_ps.points ) {
      if ( marker_show_out ) BuildMarker( mark_g, marker_out, p );
      if ( marker_show_int ) BuildMarker( hole_g, marker_int, p );
    }
    adding_segments = false;
    tag_db->EndLineTag();
  };

  bool first = true;
  Point cur;
  Point old;

  DatumBegin();
  for ( size_t i = 0; i < datum_num; ++i, DatumNext() ) {
    std::string_view svx;
    std::string_view svy;
    source->GetDatum( svx, svy, datum_no_x, datum_y_idx );
    double x = datum_cat_ofs + i;
    double y = ToDouble( svy );
    if ( !is_cat ) x = ToDouble( svx );

    old = cur;
    if ( axis_x->angle == 0 ) {
      cur.x = axis_x->Coor( x );
      cur.y = axis_y->Coor( y );
    } else {
      cur.y = axis_x->Coor( x );
      cur.x = axis_y->Coor( y );
    }
    bool valid = axis_x->Valid( x ) && axis_y->Valid( y );
    bool inside = Inside( cur );
    if ( !valid ) {
      if ( axis_x->Skip( x ) || (axis_x->Valid( x ) && axis_y->Skip( y )) ) {
        cur = old;
      } else {
        end_point();
        first = true;
      }
    } else
    if ( first ) {
      if ( inside ) {
        add_point( cur, x, svx, svy );
      }
      first = false;
    } else {
      if ( adding_segments && inside ) {
        // Common case when we stay inside the chart area.
        add_point( cur, x, svx, svy );
      } else {
        // Handle clipping in and out of the chart area.
        Point c1, c2;
        int n = ClipLine( c1, c2, old, cur );
        if ( !adding_segments ) {
          // We were outside.
          if ( inside ) {
            // We went from outside to now inside.
            if ( n == 1 ) {
              add_point( c1, x, svx, svy, true );
            }
            add_point( cur, x, svx, svy );
          } else {
            if ( n == 2 ) {
              // We are still outside, but the line segment passes through the
              // chart area.
              add_point( c1, x, svx, svy, true );
              add_point( c2, x, svx, svy, true );
              end_point();
            }
          }
        } else {
          // We went from inside to now outside.
          if ( n == 1 ) {
            add_point( c1, x, svx, svy, true );
          }
          end_point();
        }
      }
    }
  }
  end_point();
}

//------------------------------------------------------------------------------

void Series::Build(
  SVG::Group* main_g,
  SVG::Group* line_g,
  SVG::Group* area_fill_g,
  SVG::Group* marker_g,
  SVG::Group* tag_g,
  uint32_t bar_num,
  uint32_t bar_tot,
  std::vector< double >* ofs_pos,
  std::vector< double >* ofs_neg,
  std::vector< SVG::Point >* base_pts
)
{
  // Used for extra margin in comparisons to account for precision issues. This
  // may cause an unintended extra clip-detection near the corners, but the
  // points will in that case be very near each other and will thus be detected
  // by near(). Use different epsilons for near-detection and clip-detection in
  // order to ensure that any spurious clip-detection will be caught by the
  // more inclusive near-detection.
  e1 =
    std::max(
      chart_area.max.x - chart_area.min.x,
      chart_area.max.y - chart_area.min.y
    ) * epsilon;
  e2 = e1 * 0.1;

  // Enlarge the chart area a little bit to ensure that points lying exactly at
  // the boundary are not excluded due to precision issues.
  chart_area.min.x -= e1;
  chart_area.min.y -= e1;
  chart_area.max.x += e1;
  chart_area.max.y += e1;

  Group* fill_g = nullptr;
  Group* tbar_g = nullptr;
  Group* mark_g = nullptr;
  Group* hole_g = nullptr;

  if ( type == SeriesType::Area || type == SeriesType::StackedArea ) {
    fill_g = area_fill_g->AddNewGroup();
  } else {
    fill_g = main_g->AddNewGroup();
  }
  ApplyFillStyle( fill_g );

  // Tiny bars.
  if ( has_line ) {
    tbar_g = line_g->AddNewGroup();
    tbar_g->Attr()->LineColor()->Clear();
    tbar_g->Attr()->FillColor()->Set( &line_color );
  } else {
    tbar_g = main_g->AddNewGroup();
    ApplyFillStyle( tbar_g );
  }

  if ( bar_layer_tot > 1 ) {
    line_g = main_g->AddNewGroup();
  } else {
    line_g = line_g->AddNewGroup();
    if ( type == SeriesType::Bar || type == SeriesType::StackedBar ) {
      line_g->ParrentGroup()->FrontToBack();
    }
  }
  ApplyLineStyle( line_g );

  if ( marker_g != nullptr ) {
    mark_g = marker_g->AddNewGroup();
    ApplyMarkStyle( mark_g );
    hole_g = marker_g->AddNewGroup();
    ApplyHoleStyle( hole_g );
  }

  tag_g = tag_g->AddNewGroup();
  ApplyTagStyle( tag_g );

  if (
    type == SeriesType::Area ||
    type == SeriesType::StackedArea
  ) {
    BuildArea(
      fill_g, line_g, mark_g, hole_g, tag_g,
      ofs_pos, base_pts
    );
  }

  if (
    type == SeriesType::Lollipop ||
    type == SeriesType::Bar ||
    type == SeriesType::StackedBar ||
    type == SeriesType::LayeredBar
  ) {
    BuildBar(
      fill_g, tbar_g, line_g, mark_g, hole_g, tag_g,
      bar_num, bar_tot,
      ofs_pos, ofs_neg
    );
  }

  if (
    type == SeriesType::XY ||
    type == SeriesType::Scatter ||
    type == SeriesType::Line ||
    type == SeriesType::Point
  ) {
    BuildLine(
      line_g, mark_g, hole_g, tag_g
    );
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////
