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

#pragma once

#include <chart_source.h>

namespace Chart {

class Main;

class Annotate
{
public:

  Annotate( Main* main );
  ~Annotate( void );

  Main* main = nullptr;
  Source* source = nullptr;

  std::vector< Source::position_t > anchor_list;

  void do_Layer();
  void do_LineWidth();
  void do_LineDash();

  using Doer = void ( Annotate::* )();
  inline static const std::unordered_map< std::string_view, Doer > doers = {
    { "@Layer"          , &Annotate::do_Layer           },
    { "@LineWidth"      , &Annotate::do_LineWidth       },
    { "@LineDash"       , &Annotate::do_LineDash        },
  };

  void Build( SVG::Group* upper_g, SVG::Group* lower_g = nullptr );

  struct {
    bool changed = true;

    SVG::Group* upper_g = nullptr;
    SVG::Group* lower_g = nullptr;
    SVG::Group* g = nullptr;

    Chart::Pos layer = Chart::Pos::Top;

    double line_width = 1;
    double line_dash = -1;
    double line_hole = -1;
  } state;

};

}
