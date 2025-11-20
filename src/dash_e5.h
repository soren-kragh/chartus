std::cout << R"EOF(
Footnote: https://github.com/soren-kragh/chartus
FootnotePos: Right

########################################

NewChartInGrid:

ChartArea: 1000 480

Title: StackedBar and Lollipop
TitleBox: On

LegendPos: Above

# Also try Horizontal
Axis.X.Orientation: Vertical
Axis.X.Pos: Base
Axis.X.Style: Line

Axis.Y.NumberFormat: Scientific
Axis.Y.Pos: Top
Axis.Y.Style: Edge
Axis.Y.GridStyle: Solid

# Also try StackedArea
Series.Type: StackedBar

Series.LineWidth: 1

Series.New: Positive A
Series.LineColor: white
Series.FillColor: blue 0.0

Series.New: Positive B
Series.LineColor: white
Series.FillColor: blue 0.3

Series.New: Positive C
Series.LineColor: white
Series.FillColor: blue 0.6

Series.New: Negative A
Series.LineColor: white
Series.FillColor: red 0.0

Series.New: Negative B
Series.LineColor: white
Series.FillColor: red 0.3

Series.New: Negative C
Series.LineColor: white
Series.FillColor: red 0.6

Series.Type: Lollipop
Series.New: Sum
Series.LineColor: black 0.1
Series.LineWidth: 5
Series.MarkerShape: LineX
Series.MarkerSize: 15

Series.Tag: On
Series.TagBox: On
Series.TagSize: 1
Series.TagPos: Beyond
Series.TagFillColor: black
Series.TagLineColor: white
Series.TagTextColor: white

Series.Data:
Macro: BarLollipopData

########################################

NewChartInGrid:

ChartArea: 1000 240

Title:
  Using color gradient to
  distinguish positive and
  negative values
TitleInside: On
TitlePos: Bottom Left
TitleSize: 0.5

# Only show every 4th (starting from 0) to
# avoid that the X-axis numbers are rotated.
Axis.X.TickSpacing: 0 4

# X-axis grid is normally disabled for
# non-numerical X-axis.
Axis.X.Grid: On

Series.Type: Bar
Series.New: 1.0×Sin(x)
# Abrupt change over baseline:
Series.Color:
  red
  Base red 0 0.8
  Base blue 0 0.8
  blue

# Note: Area type plots are always
# shown behind other plots.
Series.Type: Area
Series.New: 1.6×Cos(x)
# Smooth change over baseline:
Series.Color:
  orange
  Base gray 0 0.8
  seagreen

Series.Data:
Macro: WaveData

########################################

)EOF";
