std::cout << R"EOF(
# Simple template

ChartBox: Off

Title       : Chart Title
Axis.X.Label: X-Axis Label
Axis.Y.Label: Y-Axis Label

Series.Type : XY
Series.New  : Series A
Series.New  : Series B
Series.Data :
        0       23.7    -15
        7.0     2.3     27
        40      20      -12
        47      10.0    5
        71      4.3     1
        97      14      -17

# Summary of all available specifiers (see -T template for details).
# Note: Axis.* is used here as documentation shorthand - specify each axis
# individually (Axis.X, Axis.Y, Axis.Y2, etc.)
#
# TitleHTML: Chartus
# Margin: 5
# BorderWidth: 5 25
# BorderColor: navy
# Padding: 10
# ForegroundColor: skyblue
# BackgroundColor: darkslategray
# GlobalTitle: Global Title
# GlobalSubTitle: Smaller Global Title
# GlobalSubSubTitle: Even Smaller Global Title
# GlobalTitlePos: Left
# GlobalTitleSize: 1.0
# GlobalTitleLine: On
# GlobalLegendHeading: Metals
# GlobalLegendBox: On
# GlobalLegendPos: Bottom
# GlobalLegendSize: 1.0
# GlobalLegendColor: lavender
# Footnote:
# FootnotePos: Center
# FootnoteLine: On
# FootnoteSize: 1.0
# LetterSpacing: 1.8 1.1 0.8
# GridPadding: 0 12
# NewChartInGrid: 0 0 3 0 Center Bottom
# NewChartInChart: 0 0 Bottom Right
# ChartPadding: -1 0
# ChartArea: 1000 600
# ChartBox: On
# ChartAreaColor: dimgray
# AxisColor: white
# GridColor: green
# TextColor: lightyellow
# BoxColor: aqua 0 0.5
# Title:
# SubTitle:
# SubSubTitle:
# TitleBox: On
# TitlePos: Left
# TitleInside: On
# TitleSize: 1.0
# Axis.X.Orientation: Vertical
# Axis.*.Reverse: Off
# Axis.*.Style: Auto
# Axis.*.Label:
# Axis.*.SubLabel:
# Axis.*.LabelSize: 1.0
# Axis.*.Unit:
# Axis.*.UnitPos: Above
# Axis.*.LogScale: On
# Axis.*.Range: 0 100 90
# Axis.*.Pos: Top
# Axis.*.Tick: 10.0 4
# Axis.*.TickSpacing: 0 5
# Axis.*.Grid: Off On
# Axis.*.GridStyle: Auto
# Axis.*.GridColor: blue
# Axis.*.NumberFormat: Magnitude
# Axis.*.NumberSign: On
# Axis.*.NumberUnit: s
# Axis.*.MinorNumber: On
# Axis.*.NumberPos: Auto
# Axis.*.NumberSize: 1.0
# LegendHeading: Countries
# LegendBox: On
# LegendPos: Below
# LegendSize: 1.0
# BarWidth: 0.8 0.7
# LayeredBarWidth: 1.0
# BarMargin: 0
# Series.Type: XY
# Series.New: Name of series
# Series.Snap: Off
# Series.Prune: 0.5
# Series.GlobalLegend: On
# Series.LegendOutline: Off
# Series.Axis: Y2
# Series.Base: 0
# Series.Style: 32
# Series.MarkerShape: Circle
# Series.MarkerSize: 8
# Series.LineWidth: 1
# Series.LineDash: 3 1
# Series.Lighten: -0.3
# Series.FillTransparency: 0.8
# Series.Color: indigo
# Series.LineColor: black
# Series.FillColor: None
# Series.Tag: On
# Series.TagPos: Below
# Series.TagSize: 0.8
# Series.TagBox: On
# Series.TagTextColor: black
# Series.TagFillColor: lightyellow 0 0.3
# Series.TagLineColor: black
# Series.Data:
# MacroDef: MyMacro
# MacroEnd: MyMacro
# Macro: MyMacro
#
# @PointCoor: Off
# @Axis: Y1
# @Layer: Top
# @LineWidth: Width
# @LineDash: Dash [Hole]
# @LineColor: black
# @FillColor: None
# @TextColor: black
# @TextAnchor: Top Center
# @TextSize: 24
# @TextBold: On
# @LetterSpacing: 1.8 1.1 0.8
# @RectCornerRadius: 10
# @Line: X1 Y1 X2 Y2
# @Rect: X1 Y1 X2 Y2
# @Circle: X Y Radius
# @Ellipse: X Y RadiusX RadiusY
# @Polyline: X1 Y1 X2 Y2 X3 Y3 ...
# @Polygon: X1 Y1 X2 Y2 X3 Y3 ...
# @TextArrow: DX DY [HeadGap [TailGap]]
# @Text: X Y
# @TextBox: X Y
# @Arrow: X1 Y1 X2 Y2 [HeadGap [TailGap]]
# @ArrowWidth: 0
# @Context: {
# @Context: }
)EOF";
