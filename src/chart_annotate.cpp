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
  state.g = (state.layer == Chart::Pos::Top) ? state.upper_g : state.lower_g;
  if ( !state.g ) source->ParseErr( "illegal layer", true );
  source->ExpectEOL();
}

////////////////////////////////////////////////////////////////////////////////

void Annotate::Build( SVG::Group* upper_g, SVG::Group* lower_g )
{
  state.upper_g = upper_g;
  state.lower_g = lower_g;
  state.g = state.upper_g;

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
