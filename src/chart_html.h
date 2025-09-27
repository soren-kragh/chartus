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
#include <chart_series.h>

#include <map>
#include <unordered_set>

namespace Chart {

class Ensemble;
class Main;
class Axis;

class HTML
{
public:

  HTML( Ensemble* ensemble ) : ensemble( ensemble ) {}

  void NewChart( Main* main );

  void DefAxisX(
    Main* main, int n, Axis* axis, double val1, double val2,
    NumberFormat number_format,
    bool number_sign, bool logarithmic, bool is_cat = false
  );
  void DefAxisY(
    Main* main, int n, Axis* axis, double val1, double val2,
    NumberFormat number_format,
    bool number_sign, bool logarithmic, bool is_cat = false
  );

  void LegendPos( Series* series, const SVG::BoundaryBox& bb );
  void MoveLegend( Series* series, SVG::U dx, SVG::U dy );
  void MoveLegends( Main* main, SVG::U dx, SVG::U dy );

  // Returns true if the given category index must be included in snap points.
  bool SnapCat( Main* main, cat_idx_t cat_idx );

  bool AllocateSnap( Main* main, SVG::Point p );
  void RecordSnapPoint(
    Series* series, SVG::Point p, cat_idx_t cat_idx,
    std::string_view tag_x, std::string_view tag_y
  );
  void PreserveSnapPoint( Series* series, SVG::Point p );
  void CommitSnapPoints( Series* series, bool force );

  std::string GenHTML( SVG::Canvas* canvas );

  Ensemble* ensemble = nullptr;
  std::vector< Main* > main_list;

  void GenChartData( Main* main, std::ostringstream& oss );

  // Resolution of snap points in points, i.e. how close the snap points are
  // placed (to reduce HTML size). Mouse events are in SVG point unit steps, so
  // a much finer (smaller) resolution than 1.0 does not gain anything.
  static constexpr double snap_resolution = 0.95;
  static constexpr double snap_factor = 1.0 / snap_resolution;

  std::map< Series*, SVG::BoundaryBox > series_legend_map;
};

}
