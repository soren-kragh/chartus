std::cout << R"EOF(
ChartArea: 1000 400

Title: The Dangers of Frozen Yogurt
SubSubTitle: (source: tylervigen.com)

Axis.X.Label: Year

Axis.Y1.Unit:
  pounds per
  person

Axis.Y2.Unit:
  violent crime
     per 100000
   US residents

# You may manually adjust where the axis unit is shown,
# in case it gets too much in the way.
#Axis.Y1.UnitPos: Above
#Axis.Y2.UnitPos: Above

# Nudge axis ranges to get convincing visualization.
Axis.Y1.Range: 0.0 4.0
Axis.Y2.Range: 300 800

# More ticks.
Axis.Y1.Tick: 0.5 2
Axis.Y2.Tick: 50 2

# Try other styles (see chartus -T).
Axis.X.Style: Edge
Axis.Y1.Style: Edge
Axis.Y2.Style: Edge

# Try other types (see chartus -T). XY plot works here because a year
# is a true number; a Line plot is a good alternative in this case.
Series.Type : XY

Series.MarkerSize: 15

# You may disable the somewhat idiosyncratic outlined
# legend style Chartus uses by default.
Series.LegendOutline: Yes

Series.New:
  Pounds per person frozen yogurt
  consumption in the United States
Series.Axis: Y1
Series.MarkerShape: Star

Series.New:
  Violent crime rate per 100000
  residents in the United States
Series.Axis: Y2
Series.MarkerShape: Triangle
Series.LineDash: 4 4

Series.Data:
        1990    2.8     729.6
	1991    3.5     758.2
	1992    3.1     757.7
	1993    3.4     747.1
	1994    3.4     713.6
	1995    3.4     684.5
	1996    2.5     636.6
	1997    2       611
	1998    2.1     567.6
	1999    2       523
	2000    2       506.5
	2001    1.5     504.5
        2002    1.5     494.4
        2003    1.5     475.8
        2004    1.3     463.2
        2005    1.3     469
        2006    1.3     479.3
        2007    1.5     471.8
        2008    1.5     458.6
        2009    0.9     431.9
        2010    1       404.5
        2011    1.2     387.1
        2012    1.1     387.8
        2013    1.4     369.1
        2014    1.3     361.6
        2015    1.4     373.7
        2016    1.2     397.5
        2017    1.2     394.9
        2018    1       383.4
        2019    1       380.8
        2020    0.6     398.5
        2021    0.9     387

Footnote: https://github.com/soren-kragh/chartus
FootnotePos: Right
)EOF";
