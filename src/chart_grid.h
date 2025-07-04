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

#include <chart_common.h>

namespace Chart {

class Main;

class Grid
{
public:

  Grid( void );
  ~Grid( void );

  uint32_t max_x = 0;
  uint32_t max_y = 0;

  SVG::U cell_margin = 0;
  SVG::U area_padding = 0;

  struct element_t {
    Main* chart = nullptr;
    SVG::BoundaryBox full_bb;
    SVG::BoundaryBox area_bb;
    uint32_t grid_x1 = 0;
    uint32_t grid_y1 = 0;
    uint32_t grid_x2 = 0;
    uint32_t grid_y2 = 0;
    bool anchor_x_defined = false;
    bool anchor_y_defined = false;
    SVG::AnchorX anchor_x = SVG::AnchorX::Mid;
    SVG::AnchorY anchor_y = SVG::AnchorY::Mid;
  };

  std::vector< element_t > element_list;

  struct edge_t {
    SVG::U coor      = 0;   // Coordinate of this core chart area edge
    SVG::U adj       = 0;   // Iterative adjustment to coor
    SVG::U pad       = 0;   // Padding caused by decorations outside core area
    bool pad_use     = false;
    SVG::U slack     = 0;
    bool constrained = false;
    bool locked      = false;
  };

  struct cell_t {
    edge_t e1;
    edge_t e2;
  };

  std::vector< cell_t > cell_list_x;
  std::vector< cell_t > cell_list_y;

  void Init( SVG::U cell_margin, SVG::U area_padding );
  uint32_t Solve( std::vector< cell_t >& cell_list );

  struct hole_t {
    uint32_t x1 = 0;
    uint32_t y1 = 0;
    uint32_t x2 = 0;
    uint32_t y2 = 0;
    SVG::BoundaryBox bb;
  };

  void GetHoles( std::vector< Grid::hole_t >& holes );

  void DisplayCoor( std::vector< cell_t >& cell_list );
  void DisplayAdj( std::vector< cell_t >& cell_list );
  void DisplaySlack( std::vector< cell_t >& cell_list );
  void RenumberCoor( std::vector< cell_t >& cell_list );
  void Test( void );

};

}
