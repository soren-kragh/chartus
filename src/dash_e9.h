std::cout << R"EOF(
Title: Breakthrough in Science
SubTitle: model vs field measurements

ChartBox: Off

### Axis Setup ###

Axis.X.LogScale: On
Axis.X.NumberFormat: Scientific
Axis.X.Orientation: Vertical
Axis.X.Reverse: On
Axis.X.Unit: pirates × GeV⁻¹

Axis.Y.LogScale: On
Axis.Y.MinorNumber: Off
Axis.Y.Unit: clowns/parsec³

### Model Data ###

Series.Type: XY
Series.New: Model prediction
Series.LineWidth: 5
Series.LineColor: orange
Series.LineDash: 10 8
Macro: ModelData

# In the following we start with the uncertainty intervals. These are done using
# segmented XY plot using ! to separate the segments. Then the data points are
# shown on top of the uncertainty intervals.

### Bozo ###

Series.Type: XY
Series.New:
Macro: UncertaintyStyle
Macro: BozoUncertainty

Series.Type: Scatter
Series.New: Bozo et al., 1953
Macro: DataStyle
Series.MarkerShape: InvTriangle
Macro: BozoData

### Krusty ###

Series.Type: XY
Series.New:
Macro: UncertaintyStyle
Macro: KrustyUncertainty

Series.Type: Scatter
Series.New: Krusty et al., 1998
Macro: DataStyle
Series.MarkerShape: Diamond
Macro: KrustyData

### Style macros ###

MacroDef: UncertaintyStyle
# Series.Style resets style to avoid carry-over of persistent style modifiers.
Series.Style: 0
Series.LineWidth: 1
Series.LineColor: black
Series.MarkerShape: LineX
Series.MarkerSize: 20
MacroEnd: UncertaintyStyle

MacroDef: DataStyle
# Series.Style resets style to avoid carry-over of persistent style modifiers.
Series.Style: 0
Series.LineWidth: 2
Series.LineColor: black
Series.FillColor: white
Series.MarkerSize: 12
MacroEnd: DataStyle

### Footnote ###

Footnote: https://github.com/soren-kragh/chartus
FootnotePos: Right

### Data ###
)EOF";
