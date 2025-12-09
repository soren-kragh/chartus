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

#include <list>

#include <chart_common.h>
#include <chart_source.h>
#include <chart_annotate.h>
#include <chart_label.h>
#include <chart_tag.h>
#include <chart_html.h>
#include <chart_series.h>
#include <chart_axis.h>
#include <chart_legend.h>
#include <chart_legend_box.h>

namespace Chart {

class Ensemble;

class Main
{
public:

  Main( Ensemble* ensemble, SVG::Group* svg_g );
  ~Main( void );

  SVG::Group* GetGroup( void ) { return svg_g; }

  // Used to move the completed chart (i.e. after Build()) to its
  // final position in the grid,
  void Move( SVG::U dx, SVG::U dy );

  // Inform that this is an embedded chart.
  void SetEmbedded( bool embedded = true )
  {
    this->embedded = embedded;
  }

  void SetPadding( SVG::U full_padding, SVG::U area_padding )
  {
    SetPaddingX( full_padding, area_padding );
    SetPaddingY( full_padding, area_padding );
  }
  void SetPaddingX( SVG::U full_padding, SVG::U area_padding )
  {
    full_padding_x = full_padding;
    area_padding_x = area_padding;
  }
  void SetPaddingY( SVG::U full_padding, SVG::U area_padding )
  {
    full_padding_y = full_padding;
    area_padding_y = area_padding;
  }
  void SetFrame( SVG::U width, SVG::U padding, SVG::U radius );

  SVG::Color* FrameColor( void ) { return &frame_color; }
  SVG::Color* CanvasColor( void ) { return &canvas_color; }

  void SetChartArea( SVG::U width, SVG::U height );
  void SetChartBox( bool chart_box = true );

  SVG::Color* ChartAreaColor( void ) { return &chart_area_color; }
  SVG::Color* AxisColor( void ) { return &axis_color; }
  SVG::Color* TextColor( void ) { return &text_color; }

  // Specify alternative background color of title box and legend box.
  SVG::Color* BoxColor( void ) { return &box_color; }

  void SetTitle( const std::string& txt );
  void SetSubTitle( const std::string& txt );
  void SetSubSubTitle( const std::string& txt );
  void SetTitlePos( Pos pos1 = Pos::Undef, Pos pos2 = Pos::Undef );
  void SetTitleInside( bool inside = true );
  void SetTitleSize( double size ) { title_size = size; }

  // Force the title box or not instead of it being determined automatically.
  void SetTitleBox( bool enable = true );

  void SetLegendHeading( const std::string& txt );

  // Force the legend box or not instead of it being determined automatically.
  void SetLegendBox( bool enable = true );

  // Normally it will strive to place the series legends somewhere inside the
  // chart area, but if the legends obscure too much of the charts you may
  // specify a location outside the chart area (if only pos1 is given). If both
  // pos1 and pos2 are give, the legends will be placed inside the chart area at
  // the given position. If force_nx > 0 the number of legends in the horizontal
  // direction is forced to that value.
  void SetLegendPos( Pos pos1, Pos pos2 = Pos::Undef, uint32_t force_nx = 0 );

  // Legend text size scaling factor.
  void SetLegendSize( double size );

  // Specify the relative width of bars (0.0 to 1.0) and the relative width (0.0
  // to 1.0) of all bars belonging to the same X-value.
  void SetBarWidth( double one_width, double all_width );

  // Specify the relative width of the topmost layered bar.
  void SetLayeredBarWidth( double width );

  // Set extra start/end margin in units of bar buckets.
  void SetBarMargin( double margin );

  Axis* AxisX( void ) { return axis_x; }
  Axis* AxisY( int n = 0 ) { return axis_y[ n ]; }

  // A default style a automatically assigned new series, but style properties
  // can subsequently be changed.
  Series* AddSeries( SeriesType type );

  // Anchor a new range of num categories at the current position in the source.
  // The empty flag indicates if the category isn't given en the source and thus
  // is empty.
  void SetCategoryAnchor( cat_idx_t num, bool empty );

  // Called for each category as they are parsed from the source.
  void ParsedCat( cat_idx_t cat_idx, std::string_view cat );

  // Used to iterate through the categories directly in the source.
  void CategoryBegin();
  void CategoryLoad();
  void CategoryNext();
  void CategoryGet( std::string_view& cat );

  // Called each time current position is at a new streak of annotation
  // specifiers.
  void AddAnnotationAnchor();

  void Build( void );

  Ensemble* ensemble = nullptr;
  SVG::Group* svg_g = nullptr;
  SVG::U g_dx = 0;
  SVG::U g_dy = 0;

  SVG::U full_padding_x = -1;
  SVG::U area_padding_x = 0;
  SVG::U full_padding_y = -1;
  SVG::U area_padding_y = 0;

  SVG::U frame_width   = -1;
  SVG::U frame_padding = 8;
  SVG::U frame_radius  = 0;
  SVG::Color frame_color;
  SVG::Color canvas_color;

  SVG::U chart_w       = 1000;
  SVG::U chart_h       = 600;
  bool   chart_box     = false;
  bool   chart_box_set = false;

  void CalcLegendBoxes(
    SVG::Group* g, std::vector< LegendBox >& lb_list,
    const std::vector< SVG::Object* >& avoid_objects
  );
  void PlaceLegends(
    std::vector< SVG::Object* >& avoid_objects,
    const std::vector< LegendBox >& lb_list,
    SVG::Group* legend_g
  );

  void AxisPrepare( SVG::Group* tag_g );

  void SeriesPrepare(
    std::vector< LegendBox >* lb_list
  );

  void BuildSeries(
    SVG::Group* below_axes_g,
    SVG::Group* above_axes_g,
    SVG::Group* tag_g
  );

  void BuildTitle(
    std::vector< SVG::Object* >& avoid_objects
  );

  void BuildFrame();

  // Get the extra space around the core chart area required to account for
  // markers and/or lines which due to their width may spill out of of the chart
  // area.
  SVG::U GetAreaOverhang( void );

  // Transfer various information to the HTML object (ensemble->html_db).
  void PrepareHTML( void );

  // Indicates that this is an embedded chart.
  bool embedded = false;

  SVG::Color chart_area_color;
  SVG::Color axis_color;
  SVG::Color text_color;
  SVG::Color box_color;

  std::string title;
  std::string sub_title;
  std::string sub_sub_title;
  Pos         title_pos_x;
  Pos         title_pos_y;
  bool        title_inside;
  double      title_size;
  bool        title_box;
  bool        title_box_specified;

  uint32_t bar_tot = 0;
  uint32_t lol_tot = 0;

  Legend* legend_obj;
  bool    legend_box;
  bool    legend_box_specified;

  double bar_one_width     = 1.00;
  double bar_all_width     = 0.85;
  double bar_layered_width = 0.50;
  double bar_margin        = 0.00;

  Label* label_db;
  Tag* tag_db;

  std::vector< Series* > series_list;

  struct category_anchor_t {
    Source::position_t pos;
    cat_idx_t num = 0;
    bool empty = false;
  };

  std::vector< category_anchor_t > category_anchor_list;

  // This state is used by CategoryBegin(), CategoryNext(), and CategoryGet().
  cat_idx_t cat_list_idx = 0;
  cat_idx_t cat_list_cnt = 0;
  bool      cat_list_empty = true;

  // Number of categories across all series.
  cat_idx_t category_num = 0;

  // State maintained by ParsedCat().
  struct parse_cat_t {
    bool      non_empty_seen = false;
    bool      stride_found = false;
    cat_idx_t idx = 0;

    // Defines if all categories have normal character width.
    bool normal_width = true;

    // Minimum distance between non-empty categories.
    cat_idx_t empty_stride = 1;
  } parse_cat;

  Axis* axis_x;
  Axis* axis_y[ 2 ];

  // Used by HTML class.
  struct html_t {
    std::unordered_set< uint64_t > snap_set;
    std::unordered_set< cat_idx_t > cat_set;

    // Informs if all snap points are in line; for multiple bars per category
    // this will not be the case.
    bool all_inline = true;

    // Specify if the chart X-axis is vertical.
    bool axis_swap = false;

    struct axis_t {
      Axis*        axis = nullptr;
      bool         is_cat;
      NumberFormat number_format;
      bool         number_sign;
      bool         logarithmic;
      double       val1;
      double       val2;
    };

    axis_t x_axis[ 2 ];
    axis_t y_axis[ 2 ];
  };
  html_t html;

  Annotate* annotate = nullptr;

};

}
