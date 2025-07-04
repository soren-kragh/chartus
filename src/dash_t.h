std::cout << R"EOF(
# Simple template

Title       : Chart Title
Axis.X.Label: X-Axis Label
Axis.Y.Label: Y-Axis Label

Series.Type : XY
Series.New  : Name of series
Series.Data :
        0       23.7
        7.0     2.3
        40      20
        47      10.0
        71      4.3
        97      14

# Summary of all available specifiers (see -T template for details):
# Margin: 5
# BorderWidth: 5
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
# GlobalLegendFrame: On
# GlobalLegendPos: Bottom
# GlobalLegendSize: 1.0
# GlobalLegendColor: lavender
# Footnote:
# FootnotePos: Center
# FootnoteLine: On
# FootnoteSize: 1.0
# LetterSpacing: 1.8 1.1 0.8
# GridPadding: 0 12
# New: 0 0 3 0 Center Bottom
# ChartArea: 1000 600
# ChartBox: On
# ChartAreaColor: dimgray
# AxisColor: white
# GridColor: green
# TextColor: lightyellow
# FrameColor: aqua 0 0.5
# Title:
# SubTitle:
# SubSubTitle:
# TitleFrame: On
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
# LegendFrame: On
# LegendPos: Below
# LegendSize: 1.0
# BarWidth: 0.8 0.7
# LayeredBarWidth: 1.0
# BarMargin: 0
# Series.Type: XY
# Series.New: Name of series
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
)EOF";
