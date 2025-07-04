std::cout << R"EOF(
GlobalTitle: Different Bar Plot Types

New: 0 0
Title: Bar
Series.Type: Bar
Macro: Setup
Macro: Series

New: 0 1
Title: StackedBar
Series.Type: StackedBar
Macro: Setup
Macro: Series

New: 1 0
Title: LayeredBar
Series.Type: LayeredBar
Macro: Setup
Macro: Series

New: 1 1
Title: LayeredBar
SubTitle:
  LayeredBarWidth: 1.0
  Series.FillTransparency: 0.7
Series.Type: LayeredBar
Macro: Setup
LayeredBarWidth: 1.0
Series.FillTransparency: 0.7
Macro: Series

New: 2 0
Title: Lollipop
SubTitle:
  BarWidth: 1 0
SubSubTitle:
  Causes all lollipops to be
  drawn on top of each other
Series.Type: Lollipop
Macro: Setup
BarWidth: 1 0
Series.LineWidth: 2
Series.LineDash: 2 2
Macro: Series

Footnote: https://github.com/soren-kragh/chartus
FootnotePos: Right

MacroDef: Setup
ChartArea: 500 200
TitlePos: Left
TitleSize: 0.7
TitleInside: On
Series.GlobalLegend: On
MacroEnd: Setup

MacroDef: Series
Series.New: Series A
Series.New: Series B
Series.New: Series C
Series.Data:
)EOF";
