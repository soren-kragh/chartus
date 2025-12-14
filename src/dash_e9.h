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
Series.Color: orange
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
Series.Color: black
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

### Annotations ###

@Context: {
@Layer: Bottom
@LineWidth: 0
@FillColor: blue 0 0.9
@Polygon: L T 10e6 T 100e3 100 L B
@FillColor: yellow 0 0.9
@Polygon: L B 100e3 100 10e6 200 R B
@FillColor: red 0 0.9
@Polygon: R B 10e6 200 100e3 100 10e6 T R T
@Context: }

@Context: {
@TextSize: 1.5
@Text: 3000 4
  Cosmic stability
@Text: 300e3 400
  Divergent pirate coupling flux
@Text: 40e6 30
  Circus formation zone
@Context: }

@Context: {
@LineDash: 4 2
@LineWidth: 2
@FillColor: None
@Circle: 100e3 100 10
@FillColor: black
@LineDash: 0
@LineWidth: 0
@Circle: 100e3 100 3
@Context: }

@Context: {
@ArrowWidth: 15
@FillColor: white 0 0.4
@TextArrow: 65 0 15 5
@TextAnchor: Left
@TextBox: 100e3 100
  Singularity
@Context: }

### Data ###
)EOF";
