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

#include <stack>

#include <chart_source.h>

namespace Chart {

class Main;
class Axis;

class Annotate
{
public:

  Annotate( Source* source );
  ~Annotate( void );

  std::vector< Main* > main_list;
  Source* source = nullptr;

  void AddChart( Main* main )
  {
    main_list.push_back( main );
  }

  std::vector< Source::position_t > anchor_list;

  void BuildArrow(
    SVG::Group* g,
    SVG::Point p1, SVG::Point p2,
    double head_gap, double tail_gap, double arrow_width
  );
  void BuildText( bool boxed );
  void BuildPoly( bool polygon );

  SVG::U GetCoor( bool x_coor = false );
  SVG::Group* CurGroup();

  void do_PointCoor();
  void do_Axis();
  void do_Layer();
  void do_LineWidth();
  void do_LineDash();
  void do_LineColor();
  void do_FillColor();
  void do_TextColor();
  void do_TextAnchor();
  void do_TextSize();
  void do_TextBold();
  void do_LetterSpacing();
  void do_RectCornerRadius();
  void do_Line();
  void do_Rect();
  void do_Circle();
  void do_Ellipse();
  void do_Polyline();
  void do_Polygon();
  void do_TextArrow();
  void do_Text();
  void do_TextBox();
  void do_Arrow();
  void do_ArrowWidth();
  void do_Context();

  using Doer = void ( Annotate::* )();
  inline static const std::unordered_map< std::string_view, Doer > doers = {
    { "PointCoor"       , &Annotate::do_PointCoor               },
    { "Axis"            , &Annotate::do_Axis                    },
    { "Layer"           , &Annotate::do_Layer                   },
    { "LineWidth"       , &Annotate::do_LineWidth               },
    { "LineDash"        , &Annotate::do_LineDash                },
    { "LineColor"       , &Annotate::do_LineColor               },
    { "FillColor"       , &Annotate::do_FillColor               },
    { "TextColor"       , &Annotate::do_TextColor               },
    { "TextAnchor"      , &Annotate::do_TextAnchor              },
    { "TextSize"        , &Annotate::do_TextSize                },
    { "TextBold"        , &Annotate::do_TextBold                },
    { "LetterSpacing"   , &Annotate::do_LetterSpacing           },
    { "RectCornerRadius", &Annotate::do_RectCornerRadius        },
    { "Line"            , &Annotate::do_Line                    },
    { "Rect"            , &Annotate::do_Rect                    },
    { "Circle"          , &Annotate::do_Circle                  },
    { "Ellipse"         , &Annotate::do_Ellipse                 },
    { "Polyline"        , &Annotate::do_Polyline                },
    { "Polygon"         , &Annotate::do_Polygon                 },
    { "TextArrow"       , &Annotate::do_TextArrow               },
    { "Text"            , &Annotate::do_Text                    },
    { "TextBox"         , &Annotate::do_TextBox                 },
    { "Arrow"           , &Annotate::do_Arrow                   },
    { "ArrowWidth"      , &Annotate::do_ArrowWidth              },
    { "Context"         , &Annotate::do_Context                 },
  };

  void Build( SVG::Group* upper_g, SVG::Group* lower_g = nullptr );

  bool new_group = true;
  SVG::Group* current_g = nullptr;

  struct state_t {
    bool point_coor = false;
    std::vector< int > axis_y_n;

    SVG::Group* upper_g = nullptr;
    SVG::Group* lower_g = nullptr;
    Chart::Pos layer = Chart::Pos::Top;

    double line_width = 1.0;
    double line_dash  = 0.0;
    double line_hole  = 0.0;

    SVG::Color line_color{ SVG::ColorName::black };
    SVG::Color fill_color{ SVG::ColorName::white };
    SVG::Color text_color{ SVG::ColorName::black };

    SVG::AnchorX text_anchor_x = SVG::AnchorX::Mid;
    SVG::AnchorY text_anchor_y = SVG::AnchorY::Mid;
    double text_size = 16;
    bool text_bold = false;
    double width_adj    = 1.0;
    double height_adj   = 1.0;
    double baseline_adj = 1.0;

    double text_arrow_dx = 0.0;
    double text_arrow_dy = 0.0;
    double text_arrow_head_gap = 0.0;
    double text_arrow_tail_gap = 0.0;

    double rect_radius = 0.0;
    double arrow_width = 0.0;
  } state;

  std::stack< state_t > context_stack;

};

}
