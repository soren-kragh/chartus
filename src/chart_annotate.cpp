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

#include <chart_annotate.h>
#include <chart_main.h>
#include <chart_ensemble.h>

using namespace SVG;
using namespace Chart;

////////////////////////////////////////////////////////////////////////////////

Annotate::Annotate( Main* main )
{
  this->main = main;
  this->source = main->ensemble->source;
}

Annotate::~Annotate( void )
{
}

////////////////////////////////////////////////////////////////////////////////

void Annotate::do_Layer()
{
  source->SkipWS();
  std::string_view id = source->GetIdentifier( true );
  if ( id == "Top"     ) state.layer = Chart::Pos::Top   ; else
  if ( id == "Bottom"  ) state.layer = Chart::Pos::Bottom; else
  if ( id == "" ) source->ParseErr( "Top or Bottom expected" ); else
  source->ParseErr( "unknown layer '" + std::string( id ) + "'", true );
  Group* g = (state.layer == Chart::Pos::Top) ? state.upper_g : state.lower_g;
  if ( !g ) source->ParseErr( "illegal layer", true );
  source->ExpectEOL();
  state.changed = true;
}

void Annotate::do_LineWidth()
{
  source->SkipWS();
  if ( source->AtEOL() ) source->ParseErr( "line width expected" );
  if ( !source->GetDouble( state.line_width ) ) {
    source->ParseErr( "malformed line width" );
  }
  if ( state.line_width < 0 ) {
    source->ParseErr( "invalid line width", true );
  }
  source->ExpectEOL();
  state.changed = true;
}

void Annotate::do_LineDash()
{
  state.line_dash = 0;
  source->SkipWS();
  if ( source->AtEOL() ) source->ParseErr( "line dash expected" );
  if ( !source->GetDouble( state.line_dash ) ) {
    source->ParseErr( "malformed line dash" );
  }
  if ( state.line_dash < 0 ) {
    source->ParseErr( "invalid line dash", true );
  }
  state.line_hole = state.line_dash;
  if ( !source->AtEOL() ) {
    source->ExpectWS();
    if ( !source->AtEOL() ) {
      if ( !source->GetDouble( state.line_hole ) ) {
        source->ParseErr( "malformed line hole" );
      }
      if ( state.line_hole < 0 ) {
        source->ParseErr( "invalid line hole", true );
      }
    }
  }
  source->ExpectEOL();
  state.changed = true;
}

void Annotate::do_LineColor()
{
  source->GetColor( &state.line_color );
  state.changed = true;
}

void Annotate::do_FillColor()
{
  source->GetColor( &state.fill_color );
  state.changed = true;
}

void Annotate::do_TextColor( )
{
  source->GetColor( &state.text_color );
  state.changed = true;
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
    std::string_view id = source->GetIdentifier( true );
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
  state.changed = true;
}

void Annotate::do_TextSize()
{
  source->SkipWS();
  if ( source->AtEOL() ) source->ParseErr( "text size expected" );
  if ( !source->GetDouble( state.text_size ) ) {
    source->ParseErr( "malformed text size" );
  }
  if ( state.text_size <= 0 ) {
    source->ParseErr( "invalid text size", true );
  }
  source->ExpectEOL();
  state.changed = true;
}

void Annotate::do_TextBold()
{
  source->GetSwitch( state.text_bold );
  state.changed = true;
}

////////////////////////////////////////////////////////////////////////////////

void Annotate::Build( SVG::Group* upper_g, SVG::Group* lower_g )
{
  state.upper_g = upper_g;
  state.lower_g = lower_g;

  for ( const auto& anchor : anchor_list ) {
    source->cur_pos = anchor;
    source->LoadLine();
    while ( true ) {
      source->SkipWS( true );
      if ( source->AtEOF() ) break;
      char c = source->CurChar();
      bool sol = source->AtSOL();
      if ( !sol ) source->ParseErr( "KEY must be unindented" );
      if ( c != '@' ) break;
      std::string_view key = source->GetIdentifier();
      source->SkipWS();
      if ( key.empty() ) source->ParseErr( "KEY expected", true );
      if ( source->CurChar() != ':' ) source->ParseErr( "':' expected" );
      source->GetChar();
      auto it = doers.find( key );
      if ( it == doers.end() ) {
        source->ParseErr( "unknown KEY '" + std::string( key ) + "'", true );
      }
      (this->*( it->second ))();
    }
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////
