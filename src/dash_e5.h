std::cout << R"EOF(
Footnote: https://github.com/soren-kragh/chartus
FootnotePos: Right

Title: StackedBar and Lollipop
TitleFrame: On

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
)EOF";
