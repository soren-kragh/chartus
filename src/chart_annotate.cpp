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

#include <cstring>

#include <chart_annotate.h>
#include <chart_main.h>
#include <chart_ensemble.h>

using namespace SVG;
using namespace Chart;

////////////////////////////////////////////////////////////////////////////////

Annotate::Annotate( Source* source, bool global )
{
  this->source = source;
  this->global = global;
}

Annotate::~Annotate( void )
{
}

////////////////////////////////////////////////////////////////////////////////

size_t Annotate::GetMainIdx()
{
  int64_t i = 0;
  if ( global ) {
    if ( !source->GetInt64( i, false ) ) {
      source->ParseErr( "invalid chart number" );
    }
    if ( source->CurChar() != ':' ) {
      source->ParseErr( "':' expected" );
    }
    source->GetChar();
  }
  if ( i < 0 || i >= static_cast< int64_t >( main_list.size() ) ) {
    source->ParseErr( "chart number out of range", true );
  }
  return i;
}

////////////////////////////////////////////////////////////////////////////////

void Annotate::do_PointCoor()
{
  source->GetSwitch( state.point_coor );
  source->ExpectEOL();
}

void Annotate::do_Axis()
{
  source->SkipWS();
  size_t main_idx = GetMainIdx();
  source->GetAxis( state.axis_y_n[ main_idx ] );
  source->ExpectEOL();
  if ( !main_list[ main_idx ]->axis_y[ state.axis_y_n[ main_idx ] ]->show ) {
    source->ParseErr( "no Y2-axis is defined", true );
  }
}

void Annotate::do_Layer()
{
  source->SkipWS();
  std::string_view id = source->GetIdentifier();
  if ( id == "Top"     ) state.layer = Chart::Pos::Top   ; else
  if ( id == "Bottom"  ) state.layer = Chart::Pos::Bottom; else
  if ( id == "" ) source->ParseErr( "Top or Bottom expected" ); else
  source->ParseErr( "unknown layer '" + std::string( id ) + "'", true );
  Group* g = (state.layer == Chart::Pos::Top) ? state.upper_g : state.lower_g;
  if ( !g ) source->ParseErr( "illegal layer", true );
  source->ExpectEOL();
  new_group = true;
}

void Annotate::do_LineWidth()
{
  source->SkipWS();
  if ( source->AtEOL() ) source->ParseErr( "line width expected" );
  source->GetDouble( state.line_width );
  if ( state.line_width < 0 ) {
    source->ParseErr( "invalid line width", true );
  }
  source->ExpectEOL();
  new_group = true;
}

void Annotate::do_LineDash()
{
  state.line_dash = 0;
  source->SkipWS();
  if ( source->AtEOL() ) source->ParseErr( "line dash expected" );
  source->GetDouble( state.line_dash );
  if ( state.line_dash < 0 ) {
    source->ParseErr( "invalid line dash", true );
  }
  state.line_hole = state.line_dash;
  if ( !source->AtEOL() ) {
    source->ExpectWS();
    if ( !source->AtEOL() ) {
      source->GetDouble( state.line_hole );
      if ( state.line_hole < 0 ) {
        source->ParseErr( "invalid line hole", true );
      }
    }
  }
  if ( state.line_hole == 0 ) state.line_dash = 0;
  source->ExpectEOL();
  new_group = true;
}

void Annotate::do_LineColor()
{
  source->GetColor( &state.line_color );
  new_group = true;
}

void Annotate::do_FillColor()
{
  source->GetColor( &state.fill_color );
  new_group = true;
}

void Annotate::do_TextColor( )
{
  source->GetColor( &state.text_color );
}

void Annotate::do_TextAnchor( )
{
  source->SkipWS();
  state.text_anchor_x = SVG::AnchorX::Mid;
  state.text_anchor_y = SVG::AnchorY::Mid;
  if ( source->AtEOL() ) {
    source->ParseErr( "anchor expected" );
  }
  while ( !source->AtEOL() ) {
    std::string_view id = source->GetIdentifier();
    if ( id == "Left"    ) state.text_anchor_x = SVG::AnchorX::Min; else
    if ( id == "Right"   ) state.text_anchor_x = SVG::AnchorX::Max; else
    if ( id == "Bottom"  ) state.text_anchor_y = SVG::AnchorY::Min; else
    if ( id == "Top"     ) state.text_anchor_y = SVG::AnchorY::Max; else
    if ( id != "Center"  ) {
      source->ParseErr( "unknown anchor '" + std::string( id ) + "'", true );
    }
    source->SkipWS();
  }
  source->ExpectEOL();
}

void Annotate::do_TextSize()
{
  source->SkipWS();
  if ( source->AtEOL() ) source->ParseErr( "text size expected" );
  source->GetDouble( state.text_size );
  if ( state.text_size <= 0 ) {
    source->ParseErr( "invalid text size", true );
  }
  source->ExpectEOL();
}

void Annotate::do_TextBold()
{
  source->GetSwitch( state.text_bold );
  source->ExpectEOL();
}

void Annotate::do_LetterSpacing()
{
  source->GetLetterSpacing(
    state.width_adj, state.height_adj, state.baseline_adj
  );
}

void Annotate::do_RectCornerRadius()
{
  source->SkipWS();
  if ( source->AtEOL() ) source->ParseErr( "radius size expected" );
  source->GetDouble( state.rect_radius );
  if ( state.rect_radius < 0 ) {
    source->ParseErr( "invalid radius", true );
  }
  source->ExpectEOL();
}

////////////////////////////////////////////////////////////////////////////////

SVG::U Annotate::GetCoor( bool x_coor )
{
  source->SkipWS();
  if ( source->AtEOL() ) {
    source->ParseErr( "coordinate expected" );
  }

  size_t main_idx = GetMainIdx();
  bool y_axis = false;
  Axis* axis;
  if ( x_coor ^ (main_list[ main_idx ]->axis_x->angle != 0) ) {
    axis = main_list[ main_idx ]->axis_x;
  } else {
    y_axis = true;
    axis = main_list[ main_idx ]->axis_y[ state.axis_y_n[ main_idx ] ];
  }

  double d1 = 0;
  double d2 = 0;

  auto& loc = source->cur_pos.loc;
  const char* ptr = loc.buf.data() + loc.char_idx;
  size_t num = source->segments[ loc.seg_idx ].byte_cnt - loc.char_idx;

  source->ref_idx = source->cur_pos.loc.char_idx;

  if ( y_axis && *ptr == 'Y' && num >= 3 ) {
    if ( strncmp( ptr, "Y1:", 3 ) == 0 ) {
      axis = main_list[ main_idx ]->axis_y[ 0 ];
      ptr += 3;
      loc.char_idx += 3;
    }
    if ( strncmp( ptr, "Y2:", 3 ) == 0 ) {
      axis = main_list[ main_idx ]->axis_y[ 1 ];
      ptr += 3;
      loc.char_idx += 3;
    }
    if ( !axis->show ) {
      source->ParseErr( "no Y2-axis is defined", true );
    }
  }

  source->ref_idx = source->cur_pos.loc.char_idx;

  bool d1_is_coor = false;
  if ( num > 0 && *ptr >= 'A' && *ptr <= 'Z' ) {
    if ( x_coor ) {
      if ( num >= 4 && strncmp( ptr, "Left", 4 ) == 0 ) {
        d1 = 0;
        loc.char_idx += 4;
      } else
      if ( *ptr == 'L' ) {
        d1 = 0;
        loc.char_idx += 1;
      } else
      if ( num >= 5 && strncmp( ptr, "Right", 5 ) == 0 ) {
        d1 = axis->length;
        loc.char_idx += 5;
      } else
      if ( *ptr == 'R' ) {
        d1 = axis->length;
        loc.char_idx += 1;
      } else
      if ( num >= 6 && strncmp( ptr, "Center", 6 ) == 0 ) {
        d1 = axis->length / 2;
        loc.char_idx += 6;
      } else
      if ( *ptr == 'C' ) {
        d1 = axis->length / 2;
        loc.char_idx += 1;
      } else {
        source->ParseErr( "invalid coordinate", true );
      }
    } else {
      if ( num >= 6 && strncmp( ptr, "Bottom", 6 ) == 0 ) {
        d1 = 0;
        loc.char_idx += 6;
      } else
      if ( *ptr == 'B' ) {
        d1 = 0;
        loc.char_idx += 1;
      } else
      if ( num >= 3 && strncmp( ptr, "Top", 3 ) == 0 ) {
        d1 = axis->length;
        loc.char_idx += 3;
      } else
      if ( *ptr == 'T' ) {
        d1 = axis->length;
        loc.char_idx += 1;
      } else
      if ( num >= 6 && strncmp( ptr, "Center", 6 ) == 0 ) {
        d1 = axis->length / 2;
        loc.char_idx += 6;
      } else
      if ( *ptr == 'C' ) {
        d1 = axis->length / 2;
        loc.char_idx += 1;
      } else {
        source->ParseErr( "invalid coordinate", true );
      }
    }
    d1_is_coor = true;
  } else {
    source->GetDoubleFull( d1, false, false, true );
  }

  if ( source->AtSep() && state.point_coor ) d1_is_coor = true;
  if ( !d1_is_coor ) {
    if ( !axis->Valid( d1 ) ) {
      source->ParseErr( "illegal value", true );
    }
    d1 = axis->Coor( d1 );
  }
  if ( !source->AtSep() ) {
    if ( source->CurChar() != '+' && source->CurChar() != '-' ) {
      source->ParseErr( "invalid coordinate", true );
    }
    source->GetDouble( d2 );
  }

  double d3 =
    x_coor
    ? main_list[ main_idx ]->g_dx
    : main_list[ main_idx ]->g_dy;
  return d1 + d2 + d3;
}

////////////////////////////////////////////////////////////////////////////////

SVG::Group* Annotate::CurGroup()
{
  if ( new_group ) {
    Group* g = (state.layer == Chart::Pos::Top) ? state.upper_g : state.lower_g;
    g = g->AddNewGroup();
    g->Attr()->SetLineWidth( state.line_width );
    g->Attr()->SetLineDash( state.line_dash, state.line_hole );
    g->Attr()->LineColor()->Set( &state.line_color );
    g->Attr()->FillColor()->Set( &state.fill_color );
    current_g = g;
  }
  new_group = false;
  return current_g;
}

////////////////////////////////////////////////////////////////////////////////

void Annotate::do_Line()
{
  Point p1;
  Point p2;
  p1.x = GetCoor( true );
  p1.y = GetCoor();
  p2.x = GetCoor( true );
  p2.y = GetCoor();
  source->ExpectEOL();
  CurGroup()->Add( new Line( p1, p2 ) );
}

void Annotate::do_Rect()
{
  Point p1;
  Point p2;
  p1.x = GetCoor( true );
  p1.y = GetCoor();
  p2.x = GetCoor( true );
  p2.y = GetCoor();
  source->ExpectEOL();
  CurGroup()->Add( new Rect( p1, p2, state.rect_radius ) );
}

void Annotate::do_Circle()
{
  Point p;
  p.x = GetCoor( true );
  p.y = GetCoor();
  double r = 0;
  source->SkipWS();
  source->GetDouble( r );
  source->ExpectEOL();
  CurGroup()->Add( new Circle( p, r ) );
}

void Annotate::do_Ellipse()
{
  Point p;
  p.x = GetCoor( true );
  p.y = GetCoor();
  double rx = 0;
  double ry = 0;
  source->SkipWS();
  source->GetDouble( rx );
  source->SkipWS();
  source->GetDouble( ry );
  source->ExpectEOL();
  CurGroup()->Add( new Ellipse( p, rx, ry ) );
}

void Annotate::BuildPoly( bool polygon )
{
  SVG::Poly* poly = new Poly();
  CurGroup()->Add( poly );
  while ( true ) {
    source->SkipWS();
    if ( source->AtEOL() ) break;
    Point p;
    p.x = GetCoor( true );
    p.y = GetCoor();
    poly->Add( p );
  }
  if ( polygon ) {
    poly->Close();
  } else {
    poly->Attr()->FillColor()->Clear();
  }
}

void Annotate::do_Polyline()
{
  BuildPoly( false );
}

void Annotate::do_Polygon()
{
  BuildPoly( true );
}

void Annotate::do_TextArrow()
{
  state.text_arrow_head_gap = 0.0;
  state.text_arrow_tail_gap = 0.0;
  source->SkipWS();
  source->GetDouble( state.text_arrow_dx );
  source->SkipWS();
  source->GetDouble( state.text_arrow_dy );
  source->SkipWS();
  if ( !source->AtEOL() ) {
    source->GetDouble( state.text_arrow_head_gap );
    source->SkipWS();
    if ( !source->AtEOL() ) {
      source->GetDouble( state.text_arrow_tail_gap );
    }
  }
  source->ExpectEOL();
}

void Annotate::BuildArrow(
  SVG::Group* g,
  SVG::Point p1, SVG::Point p2,
  double head_gap, double tail_gap, double arrow_width
)
{
  double base_width  = (arrow_width > 0) ? arrow_width : state.line_width;
  double head_length = std::max( 2 * base_width, 8.0 );
  double head_width  = head_length * 1.2;

  double dx     = p1.x - p2.x;
  double dy     = p1.y - p2.y;
  double theta  = std::atan2( dy, dx ) * 180.0 / M_PI;
  double length = std::sqrt( dx * dx + dy * dy ) - head_gap - tail_gap;
  length        = std::max( 0.0, length );
  head_length   = std::min( head_length, length );

  if ( length == 0.0 || base_width == 0.0 ) return;

  SVG::Poly* poly = new Poly();
  g->Add( poly );
  poly->Add( head_gap, 0 );
  poly->Add( head_gap + head_length, +head_width / 2 );
  poly->Add( head_gap + head_length, +base_width / 2 );
  poly->Add( head_gap + length     , +base_width / 2 );
  poly->Add( head_gap + length     , -base_width / 2 );
  poly->Add( head_gap + head_length, -base_width / 2 );
  poly->Add( head_gap + head_length, -head_width / 2 );
  poly->Close();
  if ( arrow_width == 0 ) {
    poly->Attr()->SetLineWidth( 0 );
    poly->Attr()->FillColor()->Set( &state.line_color );
  }
  poly->Rotate( theta, 0, 0 );
  poly->Move( p2.x, p2.y );
}

void Annotate::BuildText( bool boxed )
{
  Point p;
  p.x = GetCoor( true );
  p.y = GetCoor();
  source->ExpectEOL();
  std::string txt;
  source->GetText( txt, true );

  double dx{ state.text_arrow_dx };
  double dy{ state.text_arrow_dy };
  double arrow_length = std::sqrt( dx * dx + dy * dy );
  bool arrow = dx != 0 || dy != 0;
  Point pa{ 0.0, 0.0 };

  Group* cur_g = CurGroup();
  Group* all_g = cur_g;

  if ( arrow ) {
    all_g = cur_g->AddNewGroup();
    if ( state.text_anchor_x == AnchorX::Min ) dx = -std::abs( dx );
    if ( state.text_anchor_x == AnchorX::Max ) dx = +std::abs( dx );
    if ( state.text_anchor_y == AnchorY::Min ) dy = -std::abs( dy );
    if ( state.text_anchor_y == AnchorY::Max ) dy = +std::abs( dy );
    if ( state.text_anchor_x == AnchorX::Mid ) {
      if ( state.text_anchor_y != AnchorY::Mid ) {
        dx = 0;
        if ( state.text_anchor_y == AnchorY::Min ) dy = -arrow_length;
        if ( state.text_anchor_y == AnchorY::Max ) dy = +arrow_length;
      }
    }
    if ( state.text_anchor_y == AnchorY::Mid ) {
      if ( state.text_anchor_x != AnchorX::Mid ) {
        dy = 0;
        if ( state.text_anchor_x == AnchorX::Min ) dx = -arrow_length;
        if ( state.text_anchor_x == AnchorX::Max ) dx = +arrow_length;
      }
    }
    pa = Point{ -dx, -dy };
    BuildArrow(
      all_g, pa, Point{ 0.0, 0.0 },
      state.text_arrow_head_gap, state.text_arrow_tail_gap, state.arrow_width
    );
  }

  Group* txt_g = all_g->AddNewGroup();
  if ( !arrow ) all_g = txt_g;

  Group* lab_g = Label::CreateLabel( txt_g, txt, state.text_size );
  lab_g->Attr()->TextFont()->SetWidthFactor( state.width_adj );
  lab_g->Attr()->TextFont()->SetHeightFactor( state.height_adj );
  lab_g->Attr()->TextFont()->SetBaselineFactor( state.baseline_adj );
  lab_g->Attr()->TextFont()->SetBold( state.text_bold );
  lab_g->Attr()->TextColor()->Set( &state.text_color );
  BoundaryBox bb = lab_g->GetBB();
  U mx = state.text_size / 4;
  U my = mx;
  // Add temporary dummy to get margin when not boxed.
  txt_g->Add(
    new Rect(
      bb.min.x - mx, bb.min.y - my,
      bb.max.x + mx, bb.max.y + my
    )
  );
  if ( boxed ) {
    mx += state.line_width / 2;
    my += state.line_width / 2;
    txt_g->Add(
      new Rect(
        bb.min.x - mx, bb.min.y - my,
        bb.max.x + mx, bb.max.y + my,
        state.rect_radius
      )
    );
    txt_g->FrontToBack();
    if ( arrow ) {
      dx = (dx / arrow_length) * state.rect_radius;
      dy = (dy / arrow_length) * state.rect_radius;
      if ( state.text_anchor_x == AnchorX::Min ) {
        if ( state.text_anchor_y == AnchorY::Min ) {
          pa.x -= state.rect_radius + dx;
          pa.y -= state.rect_radius + dy;
        }
        if ( state.text_anchor_y == AnchorY::Max ) {
          pa.x -= state.rect_radius + dx;
          pa.y += state.rect_radius - dy;
        }
      }
      if ( state.text_anchor_x == AnchorX::Max ) {
        if ( state.text_anchor_y == AnchorY::Min ) {
          pa.x += state.rect_radius - dx;
          pa.y -= state.rect_radius + dy;
        }
        if ( state.text_anchor_y == AnchorY::Max ) {
          pa.x += state.rect_radius - dx;
          pa.y += state.rect_radius - dy;
        }
      }
    }
  }
  txt_g->MoveTo( state.text_anchor_x, state.text_anchor_y, pa );
  txt_g->DeleteFront(); // Delete the dummy again.
  all_g->Move( p.x, p.y );
}

void Annotate::do_Text()
{
  BuildText( false );
}

void Annotate::do_TextBox()
{
  BuildText( true );
}

void Annotate::do_Arrow()
{
  Point p1;
  Point p2;
  p1.x = GetCoor( true );
  p1.y = GetCoor();
  p2.x = GetCoor( true );
  p2.y = GetCoor();
  double head_gap = 0.0;
  double tail_gap = 0.0;
  source->SkipWS();
  if ( !source->AtEOL() ) {
    source->GetDouble( head_gap );
    source->SkipWS();
    if ( !source->AtEOL() ) {
      source->GetDouble( tail_gap );
    }
  }
  source->ExpectEOL();

  BuildArrow( CurGroup(), p1, p2, head_gap, tail_gap, state.arrow_width );
}

void Annotate::do_ArrowWidth()
{
  source->SkipWS();
  if ( source->AtEOL() ) source->ParseErr( "arrow width expected" );
  source->GetDouble( state.arrow_width );
  if ( state.arrow_width < 0 ) {
    source->ParseErr( "invalid arrow width", true );
  }
  source->ExpectEOL();
}

////////////////////////////////////////////////////////////////////////////////

void Annotate::do_Context()
{
  source->SkipWS();
  switch ( source->CurChar() ) {
    case '{':
      context_stack.push( state );
      break;
    case '}':
      if ( context_stack.empty() ) {
        source->ParseErr( "unmatched context brace" );
      }
      state = context_stack.top();
      context_stack.pop();
      new_group = true;
      break;
    default:
      source->ParseErr( "brace expected" );
  }
  source->GetChar();
  source->ExpectEOL();
}

////////////////////////////////////////////////////////////////////////////////

void Annotate::Build( SVG::Group* upper_g, SVG::Group* lower_g )
{
  state.upper_g = upper_g;
  state.lower_g = lower_g;
  state.axis_y_n.resize( main_list.size(), 0 );

  for ( const auto& anchor : anchor_list ) {
    source->cur_pos = anchor;
    source->LoadLine();
    while ( true ) {
      source->SkipWS( true );
      if ( source->AtEOF() ) break;
      if ( source->AtSOL() ) {
        if ( source->CurChar() != '@' ) break;
        if ( (source->CurChar( 1 ) == '@') ? !global : global ) break;
      }
      std::string_view key = source->GetKey();
      auto it = doers.find( key.substr( global ? 2 : 1 ) );
      if ( it == doers.end() ) {
        source->ParseErr( "unknown KEY '" + std::string( key ) + "'", true );
      }
      (this->*( it->second ))();
    }
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////
