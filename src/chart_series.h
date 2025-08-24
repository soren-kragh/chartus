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
#include <chart_source.h>
#include <chart_legend_box.h>
#include <chart_tag.h>
#include <chart_html.h>

namespace Chart {

class Axis;

class Series
{
public:

  Series( Main* main, SeriesType type );
  ~Series( void );

  void SetName( const std::string& name );

  // Defines if HTML should snap to the series; default is enabled.
  void SetSnap( bool snap_enable = true )
  {
    this->snap_enable = snap_enable;
  }

  // Indicate if legend should be global and potentially shared with other
  // charts.
  void SetGlobalLegend( bool global = true ) { global_legend = global; }

  // Specify if line style legends are shown with an outline around the legend
  // text, or with a small line segment in front of the legend text.
  void SetLegendOutline( bool outline ) { legend_outline = outline; }

  // Select primary (0) or secondary (1) Y-axis; default is primary.
  void SetAxisY( int axis_y_n );

  // Sets the base for area and bar type series.
  void SetBase( double base );

  void SetStyle( int style );

  // Line width also affects outline of hollow markers.
  SVG::Color* LineColor( void ) { return &line_color; }
  void SetLineWidth( SVG::U width );
  void SetLineDash( SVG::U dash );
  void SetLineDash( SVG::U dash, SVG::U hole );

  SVG::Color* FillColor( void ) { return &fill_color; }

  void SetMarkerSize( SVG::U size );
  void SetMarkerShape( MarkerShape shape );

  // Enables tags on data points; will not look good if there are many
  // data points.
  void SetTagEnable( bool enable = true ) { tag_enable = enable; }

  // Position of the tag relative to the data point.
  void SetTagPos( Pos pos ) { tag_pos = pos; }

  // Tag size scaling factor.
  void SetTagSize( float size ) { tag_size = size; }

  // Show the tag in a small box.
  void SetTagBox( bool enable = true ) { tag_box = enable; }

  // Tag text color, the fill color of the tag box (if any), and the color of
  // the line around the tag box (if any).
  SVG::Color* TagTextColor( void ) { return &tag_text_color; }
  SVG::Color* TagFillColor( void ) { return &tag_fill_color; }
  SVG::Color* TagLineColor( void ) { return &tag_line_color; }

  void SetPruneDist( SVG::U dist )
  {
    prune_dist = dist;
    prune_dist_inv = 0.0;
    if ( prune_dist > 0.0 ) {
      prune_dist_inv = 1.0 / prune_dist;
    }
  }

  double x_of_fst_valid = 0.0;
  double x_of_lst_valid = 0.0;
  bool x_of_valid_defined = false;

  double ToDouble( const std::string_view sv );

  // Anchor the series at the current position in the source. The no_x indicates
  // that no X-value is present and y_idx indicates the Y-value associated with
  // this series.
  void SetDatumAnchor(
    size_t num, size_t cat_ofs, bool no_x, uint32_t y_idx
  );

  Source::position_t datum_pos;
  size_t datum_num = 0;
  size_t datum_cat_ofs = 0;
  bool datum_no_x = false;
  uint32_t datum_y_idx = 0;

  // Used to iterate through the datums directly in the source.
  void DatumBegin()
  {
    source->cur_pos = datum_pos;
    source->LoadLine();
  }
  void DatumNext()
  {
    source->NextLine();
    source->SkipWS( true );
  }

  Source* source = nullptr;
  Main* main = nullptr;

  void ApplyFillStyle( SVG::Object* obj );
  void ApplyLineStyle( SVG::Object* obj );
  void ApplyMarkStyle( SVG::Object* obj );
  void ApplyHoleStyle( SVG::Object* obj );
  void ApplyTagStyle ( SVG::Object* obj );

  // Pruning removes data points that do not contribute significantly to the
  // overall rendering of the SVG.

  struct prune_state_t {
    size_t cnt = 0;

    // Pruned points are stored here.
    std::vector< SVG::Point > points;

    // p1 and p2 are the start and end points of the collection, which is all
    // points from p1 to p2 both inclusive.
    SVG::Point p1;
    SVG::Point p2;

    // e1 and e2 are the extreme start/end points of the line making up
    // collection. All points in the collection are spaced less than prune_dist
    // from the line from e1 to e2. Having e1/e2 enables us to prune points even
    // when there is a lot of zigzagging back and forth along (or almost along
    // as dictated by prune_dist) the e1/e2 line, as would be the case for
    // example with noisy sensor data etc.

    SVG::Point e1;
    SVG::Point e2;

    // Indicates if the extreme start/end points are also the start/end points.
    bool e1_is_p1;
    bool e2_is_p2;

    // d1/d2 is the distance of furthest point in the collection to the
    // left/right from the e1-to-e2 line.
    SVG::U d1;
    SVG::U d2;

    // Used when pruning isolated points.
    std::unordered_map< uint64_t, SVG::Point > iso_exists;
    std::vector< SVG::Point > iso_points;
  };

  void PrunePolyAdd( prune_state_t& ps, SVG::Point p );
  void PrunePolyEnd( prune_state_t& ps, bool no_html = false );

  void PrunePointsAdd( prune_state_t& ps, SVG::Point p );
  void PrunePointsEnd( prune_state_t& ps );

  bool Inside( const SVG::Point p, const SVG::BoundaryBox& bb );
  bool Inside( const SVG::Point p )
  {
    return Inside( p, chart_area );
  }

  int ClipLine(
    SVG::Point& c1, SVG::Point& c2, SVG::Point p1, SVG::Point p2,
    const SVG::BoundaryBox& bb
  );
  int ClipLine(
    SVG::Point& c1, SVG::Point& c2, SVG::Point p1, SVG::Point p2
  ) {
    return ClipLine( c1, c2, p1, p2, chart_area );
  }

  SVG::Point MoveInside( SVG::Point p );

  void UpdateLegendBoxes(
    SVG::Point p1, SVG::Point p2,
    bool p1_inc = true, bool p2_inc = true
  );

  // Computes if the series must stack above base or below base:
  //    +1 : Stack above base.
  //    -1 : Stack below base.
  //     0 : No preferred stack direction.
  void ComputeStackDir();

  // Updated by ComputeStackDir
  int stack_dir = 0;

  void BuildArea(
    SVG::Group* fill_g,
    SVG::Group* line_g,
    SVG::Group* mark_g,
    SVG::Group* hole_g,
    SVG::Group* tag_g,
    std::vector< double >* base_ofs,
    std::vector< SVG::Point >* base_pts
  );
  void BuildBar(
    SVG::Group* fill_g,
    SVG::Group* tbar_g,         // Used for thin bars
    SVG::Group* line_g,
    SVG::Group* mark_g,
    SVG::Group* hole_g,
    SVG::Group* tag_g,
    uint32_t bar_num,
    uint32_t bar_tot,
    std::vector< double >* ofs_pos,
    std::vector< double >* ofs_neg
  );
  void BuildLine(
    SVG::Group* line_g,
    SVG::Group* mark_g,
    SVG::Group* hole_g,
    SVG::Group* tag_g
  );
  void Build(
    SVG::Group* main_g,
    SVG::Group* line_g,
    SVG::Group* area_fill_g,
    SVG::Group* marker_g,
    SVG::Group* tag_g,
    uint32_t bar_num,
    uint32_t bar_tot,
    std::vector< double >* ofs_pos = nullptr,
    std::vector< double >* ofs_neg = nullptr,
    std::vector< SVG::Point >* base_pts = nullptr
  );

  uint32_t id;

  // The area within which the graphs are plotted.
  SVG::BoundaryBox chart_area;

  Axis* axis_x;
  Axis* axis_y;
  int axis_y_n;

  bool global_legend = false;
  bool legend_outline = false;

  SeriesType type;
  std::string name;
  bool snap_enable = true;
  double base;

  std::vector< LegendBox >* lb_list;

  Tag* tag_db;
  bool tag_enable;
  Pos tag_pos;
  float tag_size;
  bool tag_box;
  SVG::Color tag_text_color;
  SVG::Color tag_fill_color;
  SVG::Color tag_line_color;
  SVG::U tag_dist_x;
  SVG::U tag_dist_y;

  HTML* html_db;

  // Used by Chart::HTML
  bool has_snap = false;
  uint32_t line_color_same_cnt = 0;
  uint32_t fill_color_same_cnt = 0;

  // Used by Chart::Legend
  Series* same_legend_series = nullptr;

  std::vector< SVG::Color > color_list;
  SVG::Color line_color;
  SVG::U line_width;
  SVG::U line_dash;
  SVG::U line_hole;

  SVG::Color fill_color;

  // Used for floating point precision issues.
  double e1 = 0;
  double e2 = 0;

  uint32_t bar_layer_num = 0;
  uint32_t bar_layer_tot = 1;

  SVG::U prune_dist = 0.0;
  SVG::U prune_dist_inv = 0.0;
  SVG::U prune_dist_min = 0.001;

  SVG::U      marker_size;
  MarkerShape marker_shape;

  typedef struct {
    SVG::U x1;
    SVG::U y1;
    SVG::U x2;
    SVG::U y2;
  } MarkerDims;

  // Derived marker variables:
  bool       marker_show;
  bool       marker_show_out;
  bool       marker_show_int;
  MarkerDims marker_int;        // Interior marker dimension.
  MarkerDims marker_out;        // Outer marker dimension.

  bool has_line;
  bool has_fill;

  bool fill_color_shown = false;
  bool line_color_shown = false;

  // Compute derived marker_* variables and other visual properties.
  void DetermineVisualProperties( void );

  // Returns true if the two given series have the same legend.
  static bool SameLegend( Series* s1, Series* s2 );

  // Build marker based on marker_* variables.
  void BuildMarker( SVG::Group* g, const MarkerDims& m, SVG::Point p );

  // Determine min/max data values.
  void DetermineMinMax(
    std::vector< double >& ofs_pos,
    std::vector< double >& ofs_neg
  );

  bool   def_x = false;
  double min_x;
  double max_x;

  bool   def_y = false;
  double min_y;
  double max_y;
  bool   min_y_is_base = false;
  bool   max_y_is_base = false;

  size_t max_tag_x_size = 0;
  size_t max_tag_y_size = 0;
};

}
