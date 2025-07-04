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

class LegendBox
{
public:

  SVG::BoundaryBox bb;

  uint32_t nx = 1;      // Number of columns.
  uint32_t sp = 0;      // Spare in last row.

  double weight1 = 0;   // 1st priority weight cost.
  double weight2 = 0;   // 2nd priority weight cost.

  SVG::AnchorX anchor_x;
  SVG::AnchorY anchor_y;

};

}
