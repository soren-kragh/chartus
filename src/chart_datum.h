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

#include <svg_canvas.h>

namespace Chart {

class Datum
{
public:

  Datum() {}
  Datum( double x, double y )
    : x( x ), y( y ) {}
  Datum( double x, double y, std::string_view tag_x, std::string_view tag_y )
    : x( x ), y( y ), tag_x( tag_x ), tag_y( tag_y ) {}

  double x;
  double y;
  std::string_view tag_x;
  std::string_view tag_y;

};

}
