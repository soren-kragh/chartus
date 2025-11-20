std::cout << R"EOF(

BorderWidth: 5 20
BorderColor: midnightblue 0.4
ForegroundColor: white
BackgroundColor: midnightblue -0.4

ChartArea: 1000 500

Title: Programming Language Performance
SubSubTitle: (source: Rui Pereira et al., 2020)

AxisColor: skyblue
GridColor: skyblue

# Try it:
#Axis.X.NumberSize: 0.9
#Axis.X.Orientation: Vertical
#Axis.X.Reverse: On
#Axis.Y.Reverse: On
#BarMargin: 0.5
#Series.Staircase: On
#Series.FillTransparency: 0.5

Axis.Y.LogScale: On
Axis.Y.NumberFormat: Fixed

LegendHeading:
  Performance Metric
     (normalized)
LegendSize: 1.2

Series.Type: Bar
Series.New:
  Execution time
# Change bar starting base as the default base
# of zero does not map well to logarithmic Y-axis.
Series.Base: 0.5
# Use a gradient fill from invisible blue to darkened almost opaque red; make
# the red more prominent by completing the transition at 0.9 instead of 1.0:
Series.FillColor:
  blue 0.0 1.0
  0.9 red -0.2 0.3
Series.Tag: On
Series.TagBox: Off
Series.TagSize: 0.9
# Also try Beyond:
Series.TagPos: Base
Series.TagTextColor: white

Series.Type: Line
Series.New:
  Memory usage
Series.MarkerSize: 10
Series.Tag: On
Series.TagBox: Off
Series.TagSize: 1
Series.TagPos: Auto
Series.TagTextColor: yellow

Series.Data:
  C            1.00   1.17
  Rust         1.04   1.54
  C++          1.56   1.34
  Ada          1.85   1.47
  Java         1.89   6.01
  Chapel       2.14   4.00
  Go           2.83   1.05
  Pascal       3.02   1.00
  Ocaml        3.09   2.82
  C#           3.14   2.85
  Lisp         3.40   1.92
  Haskell      3.55   2.45
  Swift        4.20   2.71
  Fortran      4.20   1.24
  F#           6.30   4.25
  JavaScript   6.52   4.59
  Dart         6.67   8.64
  Racket      11.27   3.52
  Hack        26.99   3.34
  PHP         27.64   2.57
  Erlang      36.71   7.20
  Jruby       43.44  19.84
  TypeScript  46.20   4.69
  Ruby        59.34   3.97
  Perl        65.79   6.62
  Python      71.90   2.80
  Lua         82.91   6.72

FootnoteLine: On
Footnote: https://github.com/soren-kragh/chartus
FootnotePos: Right
)EOF";
