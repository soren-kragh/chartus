std::cout << R"EOF(
# This is an example of multiple charts arranged in a 2-by-2 grid:
#
#            Col 0    Col 1
#       +------------++---+
#       |            || S |
#       |            || i |
# Row 0 |    Main    || d |
#       |            || e |
#       |            ||   |
#       |            || Y |
#       +------------++---+
#       +------------+
# Row 1 |   Side X   |
#       +------------+

# Select to only let the actual core chart areas determine the distance between
# the charts. This gives equal gap between the chart boxes, but you must ensure
# that decorations do not collide since -1 disables collision check for
# decorations, i.e. labels, numbers, titles, etc. that are external to the core
# chart area. In this example we have ensured that there are no decorations
# between the main chart and the side panels.
GridPadding: -1 4

# The side panels share the legend as they are the same. You can of course
# remove this heading, it's just an example.
GlobalLegendHeading:
  Global
  Shared
  Legend

# You can also place the shared legend for the side panels manually.
#GlobalLegendPos: Bottom

GlobalTitle:
  2D Sample Distribution
GlobalSubSubTitle:
  This plot has many data points, so it may be a good idea to convert it to
  bitmap before sharing or using in documents etc., e.g.:

  ⟫ chartus -e3 | chartus | svg2png >example3.png

GlobalTitlePos: Left
GlobalTitleLine: On

# Grid position of the main chart.
New: 0 0

ChartArea: 600 600
ChartBox: On

Axis.X.Range: -1.25 +1.25
Axis.Y.Range: -1.25 +1.25

Axis.X.Style: None
Axis.Y.Style: None

Axis.X.Pos: Bottom
Axis.Y.Pos: Left

# Show axis numbers inside the chart area.
Axis.X.NumberPos: Above
Axis.Y.NumberPos: Right

# Show plus sign for positive axis numbers.
Axis.X.NumberSign: On
Axis.Y.NumberSign: On

# Make axis numbers a little bigger.
Axis.X.NumberSize: 1.4
Axis.Y.NumberSize: 1.4

# You must select the type before defining the series.
Series.Type: Scatter
Series.New:
  Scatter plot with
  density effect

# Use a very transparent color so we get a nice density effect for the markers.
# By having both marker outline and marker fill, we get a cool effect. You may
# disable this by setting ether line- or fill-color to None.
# Google "svg colors" for a list of the standard SVG color names.
Series.MarkerSize: 15
Series.LineWidth: 5
Series.LineColor: blue -0.5 0.9
Series.FillColor: yellow 0 0.9

# The actual data values are stored in a macro.
Series.Data:
Macro: 2d_data

# Use a macro for the side panel layout.
#vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv#
MacroDef: SidePanel
ChartBox: On
# Adjust so the center of the bars are placed exactly at "whole" fractional
# numbers (0.25, 0.5 etc.) as opposed to being placed between them:
BarMargin: 0.5
# Also try Lollipop:
Series.Type: Bar
Series.New: Samples
# The side panels share the same legend, so make it global:
Series.GlobalLegend: On
Series.Style: 7
Axis.Y.Range: 0 1500
Axis.Y.GridStyle: Solid
# Hide the bin ranges, which is achieved by selecting the first one to "show"
# way past the number of bins. Also try removing TickSpacing to show bins:
Axis.X.TickSpacing: 999
TitleSize: 1.2
TitleInside: On
TitleFrame: Off
TitlePos: Right Bottom
MacroEnd: SidePanel
#^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^#

# Define grid position of new chart for the X side panel.
New: 1 0
Title: X
ChartArea: 600 120
Macro: SidePanel
# In case the bin ranges are shown (see TickSpacing above), make sure to show
# them where they don't collide with the main plot, remember, we disabled
# collision check for decorations (see GridPadding earlier).
Axis.X.Pos: Bottom
Axis.Y.Reverse: On
# Move Y-axis numbers into the chart area like so:
#Axis.Y.NumberPos: Right
Series.Data:
Macro: x_data

# Define grid position of new chart for the Y side panel.
New: 0 1
Title: Y
ChartArea: 120 600
Macro: SidePanel
# See comment for Axis.X.Pos above.
Axis.X.Pos: Right
# Normally a textual X-axis goes from top to bottom, but we want the bins to
# follow the mathematical Y coordinates:
Axis.X.Reverse: On
Axis.X.Orientation: Vertical
Axis.Y.Pos: Top
# Move Y-axis numbers into the chart area like so:
#Axis.Y.NumberPos: Below
Series.Data:
Macro: y_data

Footnote: https://github.com/soren-kragh/chartus
FootnotePos: Right
FootnoteLine: On

)EOF";
