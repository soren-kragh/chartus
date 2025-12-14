std::cout << R"EOF(
Title: Climate Data
SubSubTitle: (source: www.woodfortrees.org)

Axis.Y1.Label   : Temperature Anomaly (°C)
Axis.Y1.SubLabel: relative to 1951-1980 mean
Axis.Y2.Label   : Parts per Million CO₂

# Show + for positive temperature anomaly.
Axis.Y1.NumberSign: On
Axis.Y1.Range: -1.5 1.75
Axis.Y1.Tick: 0.25 1

# Make Y2 ticks align with Y1 grid lines.
Axis.Y2.Range: 310 440
Axis.Y2.Tick: 10 1

Axis.X.Unit: Year

# Place legends where they do not obscure the custom annotations (see end of
# this file).
LegendPos: Bottom Right 1

# For the uncertainty interval, StackedBar could also be used instead of
# StackedArea in which case BarWidth defines the width of these error bars.
BarWidth: 0.7
Series.Type: StackedArea

Series.New:
# Set the base below the lowest lower confidence limit so
# we do not get negative stack values relative to the base.
Series.Base: -1.5
Series.Color: None
# We do not want to snap to the error bars in the HTML.
Series.Snap: Off

Series.New:
# Reset base so we only stack the confidence interval
# size on top of the lower confidence interval limit.
Series.Base: 0
Series.LineColor: None
# Do not lighten the color, but make it very transparent.
Series.FillColor: crimson 0.0 0.8

# Use Line plot for the actual data.
Series.Type: Line

Series.New:
  BEST global land/ocean mean
  with 95% confidence interval
# Scary gradient:
Series.LineColor:
  crimson 0.5
  crimson
  Left to Right
Series.LineWidth: 2
# Enable HTML snapping again.
Series.Snap: On

Series.New:
  Mauna Loa atmospheric CO₂
Series.LineColor: skyblue
Series.LineWidth: 4
Series.Axis: Y2

# Manually control what years are shown (every 5th, starting
# with 0 which corresponds to the year 1850).
Axis.X.TickSpacing: 0 5

# Year  Lower  Conf   Temp   CO₂
#       conf   size
#       limit
Series.Data:
  1850 -1.136  0.734 -0.769
  1851 -0.524  0.694 -0.177
  1852 -0.794  1.086 -0.251
  1853 -0.780  0.840 -0.360
  1854 -0.971  0.806 -0.568
  1855 -0.571  1.040 -0.051
  1856 -0.372  0.744  0.000
  1857 -0.907  0.722 -0.546
  1858 -0.639  0.806 -0.236
  1859 -0.688  0.622 -0.377
  1860 -0.696  0.692 -0.350
  1861 -1.407  0.774 -1.020
  1862 -1.353  0.816 -0.945
  1863 -0.500  0.690 -0.155
  1864 -0.959  0.754 -0.582
  1865 -0.248  0.680 +0.092
  1866 -0.315  0.740 +0.055
  1867 -0.521  0.764 -0.139
  1868 -0.790  0.608 -0.486
  1869 -0.537  0.690 -0.192
  1870 -0.545  0.516 -0.287
  1871 -0.825  0.550 -0.550
  1872 -0.870  0.826 -0.457
  1873 -0.577  0.522 -0.316
  1874 -0.453  0.458 -0.224
  1875 -0.675  0.414 -0.468
  1876 -0.630  0.488 -0.386
  1877 -0.446  0.510 -0.191
  1878 -0.301  0.586 -0.008
  1879 -0.340  0.422 -0.129
  1880 -0.602  0.464 -0.370
  1881 -0.341  0.452 -0.115
  1882 -0.077  0.362 +0.104
  1883 -0.595  0.372 -0.409
  1884 -0.420  0.366 -0.237
  1885 -0.884  0.410 -0.679
  1886 -0.806  0.328 -0.642
  1887 -1.165  0.426 -0.952
  1888 -0.526  0.332 -0.360
  1889 -0.267  0.300 -0.117
  1890 -0.609  0.320 -0.449
  1891 -0.648  0.322 -0.487
  1892 -0.422  0.286 -0.279
  1893 -1.058  0.316 -0.900
  1894 -0.683  0.282 -0.542
  1895 -0.586  0.266 -0.453
  1896 -0.349  0.272 -0.213
  1897 -0.296  0.262 -0.165
  1898 -0.305  0.288 -0.161
  1899 -0.331  0.248 -0.207
  1900 -0.536  0.256 -0.408
  1901 -0.417  0.280 -0.277
  1902 -0.407  0.256 -0.279
  1903 -0.369  0.224 -0.257
  1904 -0.783  0.252 -0.657
  1905 -0.481  0.270 -0.346
  1906 -0.474  0.270 -0.339
  1907 -0.527  0.238 -0.408
  1908 -0.528  0.228 -0.414
  1909 -0.869  0.206 -0.766
  1910 -0.482  0.200 -0.382
  1911 -0.777  0.204 -0.675
  1912 -0.323  0.194 -0.226
  1913 -0.628  0.196 -0.530
  1914 -0.102  0.206 +0.001
  1915 -0.351  0.236 -0.233
  1916 -0.296  0.228 -0.182
  1917 -0.787  0.224 -0.675
  1918 -0.642  0.238 -0.523
  1919 -0.322  0.254 -0.195
  1920 -0.282  0.226 -0.169
  1921 -0.228  0.228 -0.114
  1922 -0.385  0.218 -0.276
  1923 -0.392  0.210 -0.287
  1924 -0.361  0.216 -0.253
  1925 -0.516  0.204 -0.414
  1926  0.127  0.212 +0.233
  1927 -0.351  0.214 -0.244
  1928  0.003  0.212 +0.109
  1929 -0.514  0.202 -0.413
  1930 -0.324  0.212 -0.218
  1931 -0.201  0.206 -0.098
  1932  0.008  0.202 +0.109
  1933 -0.349  0.206 -0.246
  1934 -0.354  0.196 -0.256
  1935 -0.473  0.216 -0.365
  1936 -0.399  0.184 -0.307
  1937 -0.156  0.198 -0.057
  1938 -0.014  0.188 +0.080
  1939 -0.129  0.190 -0.034
  1940 -0.037  0.206 +0.066
  1941 -0.209  0.396 -0.011
  1942  0.141  0.402 +0.342
  1943 -0.251  0.406 -0.048
  1944  0.184  0.398 +0.383
  1945 -0.097  0.406 +0.106
  1946 -0.101  0.304 +0.051
  1947 -0.139  0.276 -0.001
  1948 -0.057  0.262 +0.074
  1949 -0.033  0.230 +0.082
  1950 -0.333  0.200 -0.233
  1951 -0.417  0.170 -0.332
  1952  0.102  0.164 +0.184
  1953 -0.001  0.178 +0.088
  1954 -0.302  0.180 -0.212
  1955  0.125  0.168 +0.209
  1956 -0.147  0.146 -0.074
  1957 -0.191  0.138 -0.122
  1958  0.283  0.140 +0.353
  1959  0.040  0.132 +0.106  315.52
  1960 -0.090  0.158 -0.011  316.38
  1961  0.000  0.106 +0.053  316.84
  1962  0.010  0.096 +0.058  317.89
  1963 -0.052  0.098 -0.003  318.69
  1964 -0.136  0.098 -0.087  319.52
  1965 -0.090  0.100 -0.040  319.38
  1966 -0.211  0.104 -0.159  320.57
  1967 -0.141  0.092 -0.095  322.28
  1968 -0.244  0.078 -0.205  322.51
  1969 -0.088  0.090 -0.043  323.95
  1970  0.103  0.068 +0.137  325.01
  1971 -0.091  0.064 -0.059  326.12
  1972 -0.304  0.078 -0.265  326.71
  1973  0.201  0.086 +0.244  328.49
  1974 -0.200  0.082 -0.159  329.30
  1975  0.015  0.068 +0.049  330.84
  1976 -0.074  0.070 -0.039  331.67
  1977  0.276  0.072 +0.312  332.77
  1978  0.101  0.070 +0.136  335.01
  1979  0.140  0.074 +0.177  336.22
  1980  0.339  0.074 +0.376  338.13
  1981  0.532  0.082 +0.573  339.42
  1982  0.090  0.076 +0.128  341.09
  1983  0.449  0.074 +0.486  341.75
  1984  0.249  0.070 +0.284  344.32
  1985  0.247  0.074 +0.284  345.59
  1986  0.186  0.100 +0.236  346.82
  1987  0.308  0.070 +0.343  348.66
  1988  0.523  0.066 +0.556  350.49
  1989  0.033  0.078 +0.072  352.99
  1990  0.379  0.080 +0.419  353.78
  1991  0.400  0.078 +0.439  354.90
  1992  0.459  0.076 +0.497  356.29
  1993  0.359  0.080 +0.399  357.06
  1994  0.260  0.080 +0.300  358.25
  1995  0.492  0.068 +0.526  359.91
  1996  0.296  0.068 +0.330  361.98
  1997  0.327  0.078 +0.366  363.03
  1998  0.613  0.076 +0.651  365.19
  1999  0.466  0.074 +0.503  368.13
  2000  0.290  0.080 +0.330  369.24
  2001  0.474  0.074 +0.511  370.60
  2002  0.731  0.076 +0.769  372.48
  2003  0.767  0.068 +0.801  374.82
  2004  0.612  0.070 +0.647  376.96
  2005  0.748  0.084 +0.790  378.37
  2006  0.607  0.072 +0.643  381.33
  2007  0.953  0.074 +0.990  382.88
  2008  0.259  0.098 +0.308  385.54
  2009  0.676  0.070 +0.711  386.86
  2010  0.785  0.084 +0.827  388.62
  2011  0.578  0.062 +0.609  391.19
  2012  0.439  0.074 +0.476  393.07
  2013  0.699  0.074 +0.736  395.62
  2014  0.749  0.074 +0.786  397.74
  2015  0.830  0.072 +0.866  399.92
  2016  1.205  0.088 +1.249  402.45
  2017  1.053  0.072 +1.089  406.04
  2018  0.831  0.086 +0.874  407.83
  2019  0.911  0.076 +0.949  410.69
  2020  1.163  0.070 +1.198  413.22
  2021  0.858  0.068 +0.892  415.20
  2022  0.905  0.074 +0.942  417.84
  2023  0.882  0.086 +0.925  419.16

Footnote: https://github.com/soren-kragh/chartus
FootnotePos: Right

# Annotations:

# Note that the X-axis is textual with the first bin being the year 1850. So,
# for the chart X-coordinates used in the annotations below X = Year - 1850.

@Context: {
@Layer: Bottom
@LineColor: None
# Scary gradient:
@FillColor:
  0.6 yellow 0 1.0
  1.0 red 0 0.8
  Bottom to Top
@Rect: L B R T
@Context: }

@Context: {
@LineDash: 12 4
@LineWidth: 4

@LineColor: yellow
@Layer: Bottom
@Line: L -0.37 R -0.37
@TextAnchor: Bottom Right
@Layer: Top
@Text: R -0.37
      Pre-industrial
  1850-1900 baseline

@LineColor: red
@Layer: Bottom
@Line: L 1.63 R 1.63
@TextAnchor: Top Left
@Layer: Top
@Text: L 1.63
  Paris Agreement 2 °C goal

@LineColor: orange
@Layer: Bottom
@Line: L 1.13 R 1.13
@TextAnchor: Top Left
@Layer: Top
@Text: L 1.13
  Paris Agreement 1.5 °C goal
@Context: }

@Context: {
@Layer: Top
@RectCornerRadius: 5
@TextSize: 12
@LineWidth: 2
@TextBold: On

@Arrow: 110 1.13-40 110 -0.37 5
@Arrow: 110 1.13-40 110 1.13 5
@TextBox: 110 1.13-40
  1.5 °C

@Arrow: 140 1.63-40 140 -0.37 5
@Arrow: 140 1.63-40 140 1.63 5
@TextBox: 140 1.63-40
  2 °C
@Context: }

@Context: {
@Axis: Y2
@TextArrow: 30 15 5
@TextAnchor: Bottom Right
@FillColor: skyblue 0.5
@TextBox: 109 315.5
  CO₂ data collection starts
  at the Mauna Loa Observatory
@Context: }

)EOF";
