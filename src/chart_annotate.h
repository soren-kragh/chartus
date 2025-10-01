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

  std::vector< Source::position_t > anchor_list;

  void do_Layer();

  using Doer = void ( Annotate::* )();
  inline static const std::unordered_map< std::string_view, Doer > doers = {
    { "@Layer", &Annotate::do_Layer },
  };

  void Build( SVG::Group* lower_g, SVG::Group* upper_g );

  struct {
    SVG::Group* lower_g = nullptr;
    SVG::Group* upper_g = nullptr;
    SVG::Group* g = nullptr;
  } state;

};

}
