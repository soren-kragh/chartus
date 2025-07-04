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

  const double num_lo      = 1e-300;
  const double num_hi      = 1e+300;
  const double num_invalid = 0.56598313e+301;   // Magic reserved value.
  const double num_skip    = 0.90870847e+301;   // Magic reserved value.
  const double coor_hi     = 1e24;

  // Correction factor for floating point precision issues in comparisons etc.
  const double epsilon = 1e-6;

  // Spacing around various boxes.
  const SVG::U box_spacing = 8;

  // Spacing from series to tag.
  const SVG::U tag_spacing = box_spacing / 2;

  enum class Pos {
    Auto, Center, Right, Left, Top, Bottom, Base, End, Beyond
  };

  enum class Dir { Right, Left, Up, Down };

  enum class NumberFormat { Auto, None, Fixed, Scientific, Magnitude };

  enum class AxisStyle { Auto, None, Line, Arrow, Edge };

  enum class GridStyle { Auto, Dash, Solid };

  enum class SeriesType {
    XY, Scatter, Line, Point, Lollipop,
    Bar, StackedBar, LayeredBar,
    Area, StackedArea
  };

  enum class MarkerShape {
    Circle, Square, Triangle, InvTriangle, Diamond, Cross, LineX, LineY
  };

  // Determines if coordinates are so near as to be considered the same.
  inline bool CoorNear( SVG::U c1, SVG::U c2 )
  {
    return std::abs( c1 - c2 ) < epsilon;
  }

  SVG::Object* Collides(
    SVG::Object* obj, const std::vector< SVG::Object* >& objects,
    SVG::U margin_x = 0, SVG::U margin_y = 0
  );

  void MoveObjs(
    Dir dir,
    const std::vector< SVG::Object* >& move_objs,
    const std::vector< SVG::Object* >& avoid_objs,
    SVG::U margin_x = 0, SVG::U margin_y = 0
  );

  void MoveObj(
    Dir dir,
    SVG::Object* obj,
    const std::vector< SVG::Object* >& avoid_objs,
    SVG::U margin_x = 0, SVG::U margin_y = 0
  );

  void ShowObjBB( SVG::Object* obj );

  void MakeColorVisible(
    SVG::Color* color, SVG::Color* bg_color, float min_visibility = 0.3
  );

  // Detect if the given string most likely contains only normal width UTF-8
  // characters.
  bool NormalWidthUTF8( const std::string& s );

}
