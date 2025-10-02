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
  void do_LineColor();
  void do_FillColor();
  void do_TextColor();

  using Doer = void ( Annotate::* )();
  inline static const std::unordered_map< std::string_view, Doer > doers = {
    { "@Layer"          , &Annotate::do_Layer           },
    { "@LineWidth"      , &Annotate::do_LineWidth       },
    { "@LineDash"       , &Annotate::do_LineDash        },
    { "@LineColor"      , &Annotate::do_LineColor       },
    { "@FillColor"      , &Annotate::do_FillColor       },
    { "@TextColor"      , &Annotate::do_TextColor       },
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
    SVG::Color line_color{ SVG::ColorName::black };
    SVG::Color fill_color{ SVG::ColorName::white };
    SVG::Color text_color{ SVG::ColorName::black };
  } state;

};

}
