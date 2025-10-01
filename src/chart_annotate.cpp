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
}

Annotate::~Annotate( void )
{
}

////////////////////////////////////////////////////////////////////////////////

void Annotate::do_Layer()
{
}

////////////////////////////////////////////////////////////////////////////////

void Annotate::Build( SVG::Group* upper_g, SVG::Group* lower_g )
{
  state.upper_g = upper_g;
  state.lower_g = lower_g;
  state.g = state.upper_g;

  for ( const auto& anchor : anchor_list ) {
    main->ensemble->source->cur_pos = anchor;
    main->ensemble->source->LoadLine();
  }

  auto it = Annotate::doers.find( "@Layer" );
  (this->*( it->second ))();

  return;
}

////////////////////////////////////////////////////////////////////////////////
