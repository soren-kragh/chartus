std::cout << R"EOF(
GlobalLegendPos: Top

# The styling is shared for all charts, so define it in a macro.
MacroDef: Setup

ChartArea: 1000 120

TitlePos: Right
TitleSize: 0.5
TitleInside: On

# The title obscures the graph (due to TitleInside), so make the title box a
# little transparent so the graphs can be glimpsed behind the title box.
FrameColor: white 0.0 0.3

# All legends are the same, so make them global instead of
# repeating them for each chart (which is too small anyway).
Series.GlobalLegend: On

# When there is no room to show all the X-axis bins, TickSpacing is very useful
# to control what is shown. Here we choose to only show every 6th hour.
Axis.X.TickSpacing: 0 6
Axis.X.Grid: On

Axis.Y1.NumberUnit: %
Axis.Y1.Label: Load
Axis.Y2.Label: Users

# Make it a little easier to see area plot through the bar plot by
# making the bars a little thinner.
BarWidth: 0.6

Series.Type: Area
Series.New: Load
Series.Axis: Y1
Series.LineColor: midnightblue
Series.FillColor: midnightblue 0 0.5

Series.Type: Bar
Series.New: Users
Series.Axis: Y2
Series.LineColor: orange
Series.FillColor: orange 0 0.5

MacroEnd: Setup

Footnote: https://github.com/soren-kragh/chartus
FootnotePos: Right
)EOF";
