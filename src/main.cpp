//
//  MIT No Attribution License
//
//  Copyright 2025, Soren Kragh
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the
//  “Software”), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so.
//

#include <csignal>
#include <csetjmp>
#include <cfenv>
#include <fstream>
#include <unordered_map>
#include <stack>
#include <functional>
#include <random>
#include <chart_source.h>
#include <chart_ensemble.h>

////////////////////////////////////////////////////////////////////////////////

Chart::Source source;
Chart::Ensemble ensemble{ &source };

bool grid_max_defined = false;
uint32_t grid_max_row = 0;
uint32_t grid_max_col = 0;
Chart::Pos footnote_pos = Chart::Pos::Auto;

struct state_t {
  std::vector< Chart::Series* > series_list;
  std::vector< Chart::SeriesType > type_list;

  bool defining_series = false;
  bool series_type_defined = false;
  Chart::SeriesType series_type = Chart::SeriesType::Line;
  bool snap = true;
  double prune_dist = 0.3;
  Chart::cat_idx_t category_idx = 0;
  bool global_legend = false;
  bool legend_outline = true;
  int axis_y_n = 0;
  double series_base = 0;
  int64_t style = 0;
  Chart::MarkerShape marker_shape = Chart::MarkerShape::Circle;
  double marker_size = -1;
  double line_width = -1;
  double line_dash = -1;
  double line_hole = -1;
  double lighten = 0.0;
  double fill_transparency = -1;
  bool tag_enable = false;
  Chart::Pos tag_pos = Chart::Pos::Auto;
  double tag_size = 1.0;
  bool tag_box = false;
  SVG::Color tag_text_color;
  SVG::Color tag_fill_color;
  SVG::Color tag_line_color;
};

state_t state;

////////////////////////////////////////////////////////////////////////////////

void show_version( void )
{
  std::cout << R"EOF(chartus v1.0.0
This is free software: you are free to change and redistribute it.

Written by Soren Kragh
)EOF";
}

////////////////////////////////////////////////////////////////////////////////

void show_help( void )
{
  std::cout << R"EOF(Usage: chartus [OPTION]... [FILE]...
Generate a chart in SVG or HTML format from FILE(s) to standard output.

With no FILE, or when FILE is -, read standard input.

  -H                Output interactive HTML instead of SVG.
  -t                Output a simple template file; a good starting point.
  -T                Output a full documentation file.
  -eN               Output example N; good for inspiration.
  -h, --help        Display this help and exit.
  -v, --version     Display version.

Examples:
  chartus f - g     Process f's contents, then standard input, then g's
                    contents; output the resulting SVG to standard output.
  chartus           Process standard input and output the resulting SVG
                    to standard output.
)EOF";
}

////////////////////////////////////////////////////////////////////////////////

void gen_template( bool full )
{
  if ( full ) {
    #include <dash_uc_t.h>
  } else {
    #include <dash_t.h>
  }
  return;
}

////////////////////////////////////////////////////////////////////////////////

void gen_example( int N )
{
  std::mt19937 gen{ 16 };
  switch ( N ) {
    case 0:
    {
      #include <dash_e0.h>
      break;
    }
    case 1:
    {
      #include <dash_e1.h>
      break;
    }
    case 2:
    {
      #include <dash_e2.h>
      break;
    }
    case 3:
    {
      #include <dash_e3.h>
      std::uniform_real_distribution< double > rnd{ 0.0, 1.0 };
      for ( int server = 1; server <= 3; server++ ) {
        std::cout << '\n';
        std::cout << "NewChartInGrid:\n";
        std::cout << "Title: Server " << server << '\n';
        std::cout << "SubTitle: 192.42.100." << server << '\n';
        std::cout << "Macro: Setup\n";
        std::cout << "Series.Data:\n";
        double load = 0;
        double usrs = 0;
        for ( int sample = 0; sample <= 4 * 24; sample++ ) {
          load = 0.2 * load + 1.0 * rnd( gen );
          usrs = 0.5 * usrs + 0.6 * rnd( gen );
          load = std::min( 1.0, std::max( 0.0, load ) );
          usrs = std::min( 1.0, std::max( 0.0, usrs ) );
          std::cout << ' ' << sample << 'h';
          std::cout << ' ' << static_cast< int >( 100 * load );
          std::cout << ' ' << static_cast< int >(  20 * usrs );
          std::cout << '\n';
        }
      }
      std::cout << '\n';
      std::cout << "# In case Condensed layout is used (see above) make\n";
      std::cout << "# sure we get the hour ticks for the last chart.\n";
      std::cout << "Axis.X.TickSpacing: 0 6\n";
      break;
    }
    case 4:
    {
      double a[ 4 ] = { 5, 10, 12 };
      double b[ 4 ] = { 8, 10, 40 };
      double c[ 4 ] = { 4, 10,  8 };
      #include <dash_e4.h>
      std::cout << std::fixed << std::setprecision( 1 );
      for ( int x = 0; x < 16; x++ ) {
        std::cout << ' ' << x;
        for ( int series = 0; series < 3; series++ ) {
          double y =
            c[ series ] *
            std::exp( -std::pow( x - a[ series ], 2 ) / b[ series ] );
          std::cout << ' ' << y;
        }
        std::cout << '\n';
      }
      std::cout << "MacroEnd: Series\n";
      std::cout << '\n';
      break;
    }
    case 5:
    {
      std::uniform_real_distribution< double > rnd_dy{ -10e12, +10e12 };
      int number = 3;
      std::vector< double > pos_y( number, 0.0 );
      std::vector< double > neg_y( number, 0.0 );
      for ( int i = 0; i < 3; i++ ) {
        double dy = 0;
        for ( int i = 0; i < number; i++ ) {
          do dy = rnd_dy( gen ); while ( pos_y[ i ] + dy < 0.0 );
          pos_y[ i ] += dy;
          do dy = rnd_dy( gen ); while ( neg_y[ i ] + dy > 0.0 );
          neg_y[ i ] += dy;
        }
      }
      #include <dash_e5.h>
      std::cout << std::scientific << std::setprecision( 1 );
      for ( int sample = 0; sample < 24; sample++ ) {
        double sum = 0;
        double dy = 0;
        for ( int i = 0; i < number; i++ ) {
          do dy = rnd_dy( gen ); while ( pos_y[ i ] + dy < 0.0 );
          pos_y[ i ] += dy;
          do dy = rnd_dy( gen ); while ( neg_y[ i ] + dy > 0.0 );
          neg_y[ i ] += dy;
        }
        std::cout << " \"Hour " << std::setw( 2 ) << sample << '"';
        for ( int i = 0; i < number; i++ ) {
          std::cout << ' ' << pos_y[ i ];
          sum += pos_y[ i ];
        }
        for ( int i = 0; i < number; i++ ) {
          std::cout << ' ' << neg_y[ i ];
          sum += neg_y[ i ];
        }
        std::cout << ' ' << sum;
        std::cout << '\n';
      }
      break;
    }
    case 6:
    {
      #include <dash_e6.h>
      break;
    }
    case 7:
    {
      #include <dash_e7.h>
      break;
    }
    case 8:
    {
      std::normal_distribution< double > md{ 0.0, 1.0 };
      std::uniform_real_distribution< double > ad{ 0.0, 2 * M_PI };
      #include <dash_e8.h>
      std::cout << std::showpos << std::fixed << std::setprecision( 4 );
      double min = -1.25;
      double max = +1.25;
      int bins = 49;
      auto ValToBin =
        [&]( double v )
        {
          return std::lround( (bins + 1) * (v - min) / (max - min) - 1 );
        };
      auto BinToVal =
        [&]( int32_t b )
        {
          return min + (max - min) * (b + 1) / (bins + 1);
        };
      std::vector< uint32_t > bin_x( bins, 0 );
      std::vector< uint32_t > bin_y( bins, 0 );
      int samples = 10000;
      std::cout << "MacroDef: 2d_data" << '\n';
      while ( samples > 0 ) {
        double m = md( gen ) / 2;
        double a = ad( gen );
        double x = m * std::cos( a );
        double y = m * std::sin( a );
        if ( std::abs( m ) <= 1.0 ) {
          std::cout << ' ' << x << ' ' << y << '\n';
          ++bin_x[ ValToBin( x ) ];
          ++bin_y[ ValToBin( y ) ];
          samples--;
        }
      }
      std::cout << "MacroEnd: 2d_data" << '\n';
      auto BinData =
        [&]( int32_t& bin, uint32_t n )
        {
          double a = BinToVal( bin - 1 );
          double b = BinToVal( bin     );
          double c = BinToVal( bin + 1 );
          double p = (a + b) / 2;
          double q = (b + c) / 2;
          std::cout << " [" << p << ':' << q << "] " << n << '\n';
          ++bin;
        };
      {
        std::cout << "MacroDef: x_data" << '\n';
        int32_t bin = 0;
        for ( auto n : bin_x ) {
          BinData( bin, n );
        }
        std::cout << "MacroEnd: x_data" << '\n';
      }
      {
        std::cout << "MacroDef: y_data" << '\n';
        int32_t bin = 0;
        for ( auto n : bin_y ) {
          BinData( bin, n );
        }
        std::cout << "MacroEnd: y_data" << '\n';
      }
      break;
    }
    case 9:
    {
      #include <dash_e9.h>

      double n_min = 0.0;
      double n_max = 3.0;

      auto GetX = [&]( double n ) {
        return std::pow( 10, n );
      };
      auto GetY = [&]( double n ) {
        double x = GetX( n_max - n );
        double y = std::pow( x - 10, 4 ) / (1.0 + std::pow( 1.01, x + 100 ));
        return 0.9e3 + y;
      };

      {
        std::cout << '\n';
        std::cout << "MacroDef: ModelData\n";
        std::cout << "Series.Data:\n";
        uint32_t N = 100;
        for ( uint32_t i = 0; i < N; ++i ) {
          double n = n_min + (n_max - n_min) * i / (N - 1);
          std::cout << ' ' << GetX( n );
          std::cout << ' ' << GetY( n );
          std::cout << '\n';
        }
        std::cout << "MacroEnd: ModelData\n";
      }

      auto field_measurement = [&](
        uint32_t N, std::string name, double error, double uncertainty
      ) {
        std::vector< std::pair< double, double > > points;
        std::uniform_real_distribution< double > n_dev{ -0.2, +0.2 };
        std::uniform_real_distribution< double > y_dev(
          -std::log( error ),
          +std::log( error )
        );
        for ( uint32_t i = 0; i < N; ++i ) {
          double my_min = n_min + 0.3;
          double my_max = n_max - 0.3;
          double n = my_min + (my_max - my_min) * i / (N - 1);
          n += n_dev( gen );
          std::pair< double, double > p;
          p.first  = GetX( n );
          p.second = GetY( n ) * std::exp( y_dev( gen ) );
          points.push_back( p );
        }

        std::cout << '\n';
        std::cout << "MacroDef: " << name << "Data\n";
        std::cout << "Series.Data:\n";
        for ( auto p : points ) {
          std::cout << ' ' << p.first;
          std::cout << ' ' << p.second;
          std::cout << '\n';
        }
        std::cout << "MacroEnd: " << name << "Data\n";

        std::cout << '\n';
        std::cout << "MacroDef: " << name << "Uncertainty\n";
        std::cout << "Series.Data:\n";
        for ( auto p : points ) {
          std::cout << ' ' << p.first;
          std::cout << ' ' << p.second / uncertainty;
          std::cout << '\n';
          std::cout << ' ' << p.first;
          std::cout << ' ' << p.second * uncertainty;
          std::cout << '\n';
          std::cout << " !\n";
        }
        std::cout << "MacroEnd: " << name << "Uncertainty\n";

        return;
      };

      field_measurement( 5, "Bozo", 5, 7 );
      field_measurement( 7, "Krusty", 1.3, 1.7 );

      break;
    }
  }
  return;
}

////////////////////////////////////////////////////////////////////////////////

void do_Pos(
  Chart::Pos& pos, int& axis_y_n
)
{
  std::string_view id = source.GetIdentifier();
  if ( id == "Auto"    ) pos = Chart::Pos::Auto  ; else
  if ( id == "Center"  ) pos = Chart::Pos::Center; else
  if ( id == "Left"    ) pos = Chart::Pos::Left  ; else
  if ( id == "Right"   ) pos = Chart::Pos::Right ; else
  if ( id == "Top"     ) pos = Chart::Pos::Top   ; else
  if ( id == "Bottom"  ) pos = Chart::Pos::Bottom; else
  if ( id == "Above"   ) pos = Chart::Pos::Top   ; else
  if ( id == "Below"   ) pos = Chart::Pos::Bottom; else
  if ( id == "Base"    ) { pos = Chart::Pos::Base; axis_y_n = 0; } else
  if ( id == "BaseY1"  ) { pos = Chart::Pos::Base; axis_y_n = 0; } else
  if ( id == "BasePri" ) { pos = Chart::Pos::Base; axis_y_n = 0; } else
  if ( id == "BaseY2"  ) { pos = Chart::Pos::Base; axis_y_n = 1; } else
  if ( id == "BaseSec" ) { pos = Chart::Pos::Base; axis_y_n = 1; } else
  if ( id == "End"     ) pos = Chart::Pos::End   ; else
  if ( id == "Beyond"  ) pos = Chart::Pos::Beyond; else
  if ( id == "" ) source.ParseErr( "position expected" ); else
  source.ParseErr( "unknown position '" + std::string( id ) + "'", true );
}

void do_Pos(
  Chart::Pos& pos
)
{
  int axis_y_n;
  do_Pos( pos, axis_y_n );
}

////////////////////////////////////////////////////////////////////////////////

bool do_GridPos(
  int64_t& row1, int64_t& col1,
  int64_t& row2, int64_t& col2
)
{
  bool got_pos = false;

  source.SkipWS();

  if ( source.GetInt64( row1 ) ) {
    if ( row1 < 0 || row1 > 99 ) {
      source.ParseErr( "grid row out of range [0;99]", true );
    }

    source.ExpectWS( "column expected" );
    if ( !source.GetInt64( col1 ) ) source.ParseErr( "malformed column" );
    if ( col1 < 0 || col1 > 99 ) {
      source.ParseErr( "grid column out of range [0;99]", true );
    }

    row2 = row1;
    col2 = col1;

    source.SkipWS();

    if ( source.GetInt64( row2 ) ) {
      if ( row2 < 0 || row2 > 99 ) {
        source.ParseErr( "grid row out of range [0;99]", true );
      }

      source.ExpectWS( "column expected" );
      if ( !source.GetInt64( col2 ) ) source.ParseErr( "malformed column" );
      if ( col2 < 0 || col2 > 99 ) {
        source.ParseErr( "grid column out of range [0;99]", true );
      }
    }

    got_pos = true;
  }

  return got_pos;
}

////////////////////////////////////////////////////////////////////////////////

// Indicates if a chart has been started without a preceding New.
bool non_newed_chart = false;

Chart::Main* CurChart( void )
{
  if ( ensemble.Empty() ) {
    non_newed_chart = true;
    source.SavePos( 1 );
    ensemble.NewChart( 0, 0, 0, 0 );
  }
  return ensemble.LastChart();
}

void do_NewChart( bool chart_in_chart )
{
  Chart::Pos pos1 = Chart::Pos::Undef;
  Chart::Pos pos2 = Chart::Pos::Undef;

  int64_t row1 = 0;
  int64_t col1 = 0;
  int64_t row2 = 0;
  int64_t col2 = 0;
  bool grid_given = do_GridPos( row1, col1, row2, col2 );

  source.SkipWS();
  if ( !source.AtEOL() ) {
    do_Pos( pos1 );
    source.SkipWS();
    if ( !source.AtEOL() ) do_Pos( pos2 );
  }

  source.ExpectEOL();

  if ( !grid_given && grid_max_defined ) {
    row1 = grid_max_row + 1;
    row2 = row1;
    col1 = 0;
    col2 = grid_max_col;
  }

  if ( row1 > row2 || col1 > col2 ) {
    source.ParseErr( "malformed grid location" );
  }

  if ( non_newed_chart ) {
    source.RestorePos( 1 );
    source.ToSOL();
    source.ParseErr(
      "chart specifiers must be preceded by NewChartInGrid for multi chart plots"
    );
  }

  bool ok =
    ensemble.NewChart( row1, col1, row2, col2, pos1, pos2, chart_in_chart );
  if ( !ok ) {
    source.ToSOL();
    source.ParseErr( "grid collision" );
  }

  grid_max_row = std::max( grid_max_row, static_cast<uint32_t>( row2 ) );
  grid_max_col = std::max( grid_max_col, static_cast<uint32_t>( col2 ) );
  grid_max_defined = true;

  state = {};
}

void do_NewChartInGrid( void )
{
  do_NewChart( false );
}

void do_NewChartInChart( void )
{
  do_NewChart( true );
  CurChart()->SetPadding( 12, 0 );
}

////////////////////////////////////////////////////////////////////////////////

void do_Margin( void )
{
  double m;
  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "margin expected" );
  source.GetDouble( m );
  if ( m < 0 || m > 1000 ) {
    source.ParseErr( "margin out of range [0;1000]", true );
  }
  source.ExpectEOL();
  ensemble.SetMargin( m );
}

void do_BorderColor( void )
{
  source.GetColor( ensemble.BorderColor() );
}

void do_BorderWidth( void )
{
  double m;
  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "border width expected" );
  source.GetDouble( m );
  if ( m < 0 || m > 1000 ) {
    source.ParseErr( "border width out of range [0;1000]", true );
  }
  source.ExpectEOL();
  ensemble.SetBorderWidth( m );
}

void do_Padding( void )
{
  double m;
  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "padding expected" );
  source.GetDouble( m );
  if ( m < 0 || m > 1000 ) {
    source.ParseErr( "padding out of range [0;1000]", true );
  }
  source.ExpectEOL();
  ensemble.SetPadding( m );
}

void do_GridPadding( void )
{
  double grid_padding;
  double area_padding = 0;

  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "grid padding expected" );
  source.GetDouble( grid_padding );
  if ( grid_padding > 1000 ) {
    source.ParseErr( "grid padding out of range [-inf;1000]", true );
  }

  source.SkipWS();
  if ( !source.AtEOL() ) {
    source.GetDouble( area_padding );
    if ( area_padding < 0 || area_padding > 1000 ) {
      source.ParseErr( "chart area padding out of range [0;1000]", true );
    }
  }

  source.ExpectEOL();
  ensemble.SetGridPadding( grid_padding, area_padding );
}

//------------------------------------------------------------------------------

void do_GlobalLegendHeading( void )
{
  std::string txt;
  source.GetText( txt, true );
  ensemble.SetLegendHeading( txt );
}

void do_GlobalLegendFrame( void )
{
  bool frame;
  source.GetSwitch( frame );
  source.ExpectEOL();
  ensemble.SetLegendFrame( frame );
}

void do_GlobalLegendPos( void )
{
  int64_t row1 = 0;
  int64_t col1 = 0;
  int64_t row2 = 0;
  int64_t col2 = 0;
  Chart::Pos pos;
  source.SkipWS();
  if ( do_GridPos( row1, col1, row2, col2 ) ) {
    Chart::Pos pos1 = Chart::Pos::Auto;
    Chart::Pos pos2 = Chart::Pos::Auto;
    source.SkipWS();
    if ( !source.AtEOL() ) {
      do_Pos( pos1 );
      source.SkipWS();
      if ( !source.AtEOL() ) do_Pos( pos2 );
    }
    source.ExpectEOL();
    if (
      !ensemble.SetLegendPos( row1, col1, row2, col2, pos1, pos2
    ) ) {
      source.ParseErr( "grid collision" );
    }
  } else {
    do_Pos( pos );
    source.ExpectEOL();
    ensemble.SetLegendPos( pos );
  }
}

void do_GlobalLegendSize( void )
{
  double size;
  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "legend size value expected" );
  source.GetDouble( size );
  if ( size < 0.01 || size > 100 ) {
    source.ParseErr( "legend size value out of range", true );
  }
  source.ExpectEOL();
  ensemble.SetLegendSize( size );
}

void do_GlobalLegendColor( void )
{
  source.GetColor( ensemble.LegendColor() );
}

void do_LetterSpacing( void )
{
  double width_adj;
  double height_adj;
  double baseline_adj;
  source.GetLetterSpacing( width_adj, height_adj, baseline_adj );
  ensemble.SetLetterSpacing( width_adj, height_adj, baseline_adj );
}

////////////////////////////////////////////////////////////////////////////////

void do_ChartPadding( void )
{
  double full_padding = -1;
  double area_padding = 0;

  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "padding value expected" );
  source.GetDouble( full_padding );

  source.SkipWS();
  if ( !source.AtEOL() ) {
    source.GetDouble( area_padding );
    if ( area_padding < 0 ) {
      source.ParseErr( "negative area padding not allowed", true );
    }
  }

  source.ExpectEOL();
  CurChart()->SetPadding( full_padding, area_padding );
}

void do_ChartArea( void )
{
  int64_t w;
  int64_t h;

  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "width expected" );
  if ( !source.GetInt64( w ) ) source.ParseErr( "malformed width" );
  if ( w < 10 || w > 100000 ) {
    source.ParseErr( "width out of range [10;100000]", true );
  }

  source.ExpectWS( "height expected" );
  if ( !source.GetInt64( h ) ) source.ParseErr( "malformed height" );
  if ( h < 10 || h > 100000 ) {
    source.ParseErr( "height out of range [10;100000]", true );
  }

  source.ExpectEOL();
  CurChart()->SetChartArea( w, h );
}

void do_ChartBox( void )
{
  bool chart_box;
  source.GetSwitch( chart_box );
  source.ExpectEOL();
  CurChart()->SetChartBox( chart_box );
}

//------------------------------------------------------------------------------

void do_ForegroundColor( void )
{
  source.GetColor( ensemble.ForegroundColor() );
}

void do_BackgroundColor( void )
{
  source.GetColor( ensemble.BackgroundColor() );
}

void do_ChartAreaColor( void )
{
  source.GetColor( CurChart()->ChartAreaColor() );
}

void do_AxisColor( void )
{
  source.GetColor( CurChart()->AxisColor() );
}

void do_GridColor( void )
{
  source.GetColor( CurChart()->AxisX()->GridColor() );
  for ( auto n : { 0, 1 } ) {
    CurChart()->AxisY( n )->GridColor()->Set( CurChart()->AxisX()->GridColor() );
  }
}

void do_TextColor( void )
{
  source.GetColor( CurChart()->TextColor() );
}

void do_FrameColor( void )
{
  source.GetColor( CurChart()->FrameColor() );
}

//------------------------------------------------------------------------------

void do_TitleHTML( void )
{
  std::string txt;
  source.GetText( txt, false );
  ensemble.TitleHTML( txt );
}

void do_GlobalTitle( void )
{
  std::string txt;
  source.GetText( txt, true );
  ensemble.SetTitle( txt );
}

void do_GlobalSubTitle( void )
{
  std::string txt;
  source.GetText( txt, true );
  ensemble.SetSubTitle( txt );
}

void do_GlobalSubSubTitle( void )
{
  std::string txt;
  source.GetText( txt, true );
  ensemble.SetSubSubTitle( txt );
}

void do_GlobalTitlePos( void )
{
  Chart::Pos pos;
  source.SkipWS();
  do_Pos( pos );
  source.ExpectEOL();
  ensemble.SetTitlePos( pos );
}

void do_GlobalTitleSize( void )
{
  double size;
  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "title size value expected" );
  source.GetDouble( size );
  if ( size < 0.01 || size > 100 ) {
    source.ParseErr( "title size value out of range", true );
  }
  source.ExpectEOL();
  ensemble.SetTitleSize( size );
}

void do_GlobalTitleLine( void )
{
  bool title_line;
  source.GetSwitch( title_line );
  source.ExpectEOL();
  ensemble.SetTitleLine( title_line );
}

//------------------------------------------------------------------------------

void do_Title( void )
{
  std::string txt;
  source.GetText( txt, true );
  CurChart()->SetTitle( txt );
}

void do_SubTitle( void )
{
  std::string txt;
  source.GetText( txt, true );
  CurChart()->SetSubTitle( txt );
}

void do_SubSubTitle( void )
{
  std::string txt;
  source.GetText( txt, true );
  CurChart()->SetSubSubTitle( txt );
}

void do_TitleFrame( void )
{
  bool frame;
  source.GetSwitch( frame );
  source.ExpectEOL();
  CurChart()->SetTitleFrame( frame );
}

void do_TitlePos( void )
{
  Chart::Pos pos_x;
  Chart::Pos pos_y = Chart::Pos::Top;

  source.SkipWS();
  do_Pos( pos_x );

  if ( !source.AtEOL() ) {
    source.ExpectWS();
    if ( !source.AtEOL() ) {
      do_Pos( pos_y );
    }
  }

  source.ExpectEOL();
  CurChart()->SetTitlePos( pos_x, pos_y );
}

void do_TitleInside( void )
{
  bool inside;
  source.GetSwitch( inside );
  source.ExpectEOL();
  CurChart()->SetTitleInside( inside );
}

void do_TitleSize( void )
{
  double size;
  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "title size value expected" );
  source.GetDouble( size );
  if ( size < 0.01 || size > 100 ) {
    source.ParseErr( "title size value out of range", true );
  }
  source.ExpectEOL();
  CurChart()->SetTitleSize( size );
}

//------------------------------------------------------------------------------

void do_Footnote( void )
{
  std::string txt;
  source.GetText( txt, true );
  ensemble.AddFootnote( txt );
  ensemble.SetFootnotePos( footnote_pos );
}

void do_FootnotePos( void )
{
  source.SkipWS();
  do_Pos( footnote_pos );
  source.ExpectEOL();
  ensemble.SetFootnotePos( footnote_pos );
}

void do_FootnoteLine( void )
{
  bool footnote_line;
  source.GetSwitch( footnote_line );
  source.ExpectEOL();
  ensemble.SetFootnoteLine( footnote_line );
}

void do_FootnoteSize( void )
{
  double size;
  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "footnote size value expected" );
  source.GetDouble( size );
  if ( size < 0.01 || size > 100 ) {
    source.ParseErr( "footnote size value out of range", true );
  }
  source.ExpectEOL();
  ensemble.SetFootnoteSize( size );
}

//------------------------------------------------------------------------------

void do_Axis_Orientation( Chart::Axis* axis )
{
  bool vertical = false;

  source.SkipWS();
  std::string_view id = source.GetIdentifier();
  if ( id == "Horizontal" ) vertical = false; else
  if ( id == "Vertical"   ) vertical = true ; else
  if ( id == "" ) source.ParseErr( "axis orientation expected" ); else
  source.ParseErr(
    "unknown axis orientation '" + std::string( id ) + "'", true
  );
  source.ExpectEOL();

  vertical = (axis == CurChart()->AxisX()) ? vertical : !vertical;
  CurChart()->AxisX(   )->SetAngle( vertical ? 90 :  0 );
  CurChart()->AxisY( 0 )->SetAngle( vertical ?  0 : 90 );
  CurChart()->AxisY( 1 )->SetAngle( vertical ?  0 : 90 );
}

//------------------------------------------------------------------------------

void do_Axis_Reverse( Chart::Axis* axis )
{
  bool reverse;
  source.GetSwitch( reverse );
  source.ExpectEOL();
  axis->SetReverse( reverse );
}

//------------------------------------------------------------------------------

void do_Axis_Style( Chart::Axis* axis )
{
  Chart::AxisStyle style = Chart::AxisStyle::Auto;
  source.SkipWS();

  std::string_view id = source.GetIdentifier();
  if ( id == "Auto"   ) style = Chart::AxisStyle::Auto ; else
  if ( id == "None"   ) style = Chart::AxisStyle::None ; else
  if ( id == "Line"   ) style = Chart::AxisStyle::Line ; else
  if ( id == "Arrow"  ) style = Chart::AxisStyle::Arrow; else
  if ( id == "Edge"   ) style = Chart::AxisStyle::Edge ; else
  if ( id == "" ) source.ParseErr( "axis style expected" ); else
  source.ParseErr( "unknown axis style '" + std::string( id ) + "'", true );

  source.ExpectEOL();
  axis->SetStyle( style );
}

//------------------------------------------------------------------------------

void do_Axis_Label( Chart::Axis* axis )
{
  std::string txt;
  source.GetText( txt, true );
  axis->SetLabel( txt );
}

//------------------------------------------------------------------------------

void do_Axis_SubLabel( Chart::Axis* axis )
{
  std::string txt;
  source.GetText( txt, true );
  axis->SetSubLabel( txt );
}

//------------------------------------------------------------------------------

void do_Axis_LabelSize( Chart::Axis* axis )
{
  double size;
  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "label size value expected" );
  source.GetDouble( size );
  if ( size < 0.01 || size > 100 ) {
    source.ParseErr( "label size value out of range", true );
  }
  source.ExpectEOL();
  axis->SetLabelSize( size );
}

//------------------------------------------------------------------------------

void do_Axis_Unit( Chart::Axis* axis )
{
  std::string txt;
  source.GetText( txt, true );
  axis->SetUnit( txt );
}

//------------------------------------------------------------------------------

void do_Axis_UnitPos( Chart::Axis* axis )
{
  Chart::Pos pos;
  source.SkipWS();
  do_Pos( pos );
  source.ExpectEOL();
  axis->SetUnitPos( pos );
}

//------------------------------------------------------------------------------

void do_Axis_LogScale( Chart::Axis* axis )
{
  bool log_scale;
  source.GetSwitch( log_scale );
  source.ExpectEOL();
  axis->SetLogScale( log_scale );
}

//------------------------------------------------------------------------------

void do_Axis_Range( Chart::Axis* axis )
{
  double min;
  double max;
  double cross;

  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "min expected" );
  source.GetDouble( min );

  source.ExpectWS( "max expected" );
  source.GetDouble( max );
  if ( !(max > min) ) source.ParseErr( "max must be greater than min", true );

  cross = 0;
  if ( !source.AtEOL() ) {
    source.ExpectWS();
    if ( !source.AtEOL() ) {
      source.GetDouble( cross );
    }
  }

  source.ExpectEOL();

  axis->SetRange( min, max, cross );
}

//------------------------------------------------------------------------------

void do_Axis_Pos( Chart::Axis* axis )
{
  Chart::Pos pos;
  int axis_y_n;
  source.SkipWS();
  do_Pos( pos, axis_y_n );
  source.ExpectEOL();
  axis->SetPos( pos, axis_y_n );
}

//------------------------------------------------------------------------------

void do_Axis_Tick( Chart::Axis* axis )
{
  double major;
  int64_t minor;

  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "major tick expected" );
  source.GetDouble( major );
  if ( !(major > 0) ) source.ParseErr( "major tick must be positive", true );

  source.ExpectWS( "minor tick expected" );
  if ( !source.GetInt64( minor ) ) source.ParseErr( "malformed minor tick" );
  if ( minor < 0 || minor > 100 ) {
    source.ParseErr( "minor tick out of range [0;100]", true );
  }

  source.ExpectEOL();

  axis->SetTick( major, minor );
}

//------------------------------------------------------------------------------

void do_Axis_TickSpacing( Chart::Axis* axis )
{
  int64_t start = 0;
  int64_t stride = 1;

  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "start expected" );
  if ( !source.GetInt64( start ) ) source.ParseErr( "malformed start" );
  if ( start < 0 ) {
    source.ParseErr( "invalid start position", true );
  }

  source.SkipWS();
  if ( source.GetInt64( stride ) ) {
    if ( stride < 1 ) {
      source.ParseErr( "stride must be greater than zero", true );
    }
  }

  source.ExpectEOL();
  axis->SetTickSpacing( start, stride );
}

//------------------------------------------------------------------------------

void do_Axis_Grid( Chart::Axis* axis )
{
  bool major;
  bool minor;

  source.GetSwitch( major );

  minor = major;

  if ( !source.AtEOL() ) {
    source.ExpectWS();
    if ( !source.AtEOL() ) {
      source.GetSwitch( minor );
    }
  }

  source.ExpectEOL();

  axis->SetGrid( major, minor );
}

//------------------------------------------------------------------------------

void do_Axis_GridStyle( Chart::Axis* axis )
{
  Chart::GridStyle style = Chart::GridStyle::Auto;
  source.SkipWS();
  std::string_view id = source.GetIdentifier();
  if ( id == "Auto"  ) style = Chart::GridStyle::Auto ; else
  if ( id == "Dash"  ) style = Chart::GridStyle::Dash ; else
  if ( id == "Solid" ) style = Chart::GridStyle::Solid; else
  if ( id == "" ) source.ParseErr( "grid style expected" ); else
  source.ParseErr( "unknown grid style '" + std::string( id ) + "'", true );
  source.ExpectEOL();
  axis->SetGridStyle( style );
}

//------------------------------------------------------------------------------

void do_Axis_GridColor( Chart::Axis* axis )
{
  source.GetColor( axis->GridColor() );
}

//------------------------------------------------------------------------------

void do_Axis_NumberFormat( Chart::Axis* axis )
{
  Chart::NumberFormat number_format = Chart::NumberFormat::Auto;
  source.SkipWS();

  std::string_view id = source.GetIdentifier();
  if ( id == "Auto"       ) number_format = Chart::NumberFormat::Auto      ; else
  if ( id == "None"       ) number_format = Chart::NumberFormat::None      ; else
  if ( id == "Fixed"      ) number_format = Chart::NumberFormat::Fixed     ; else
  if ( id == "Scientific" ) number_format = Chart::NumberFormat::Scientific; else
  if ( id == "Magnitude"  ) number_format = Chart::NumberFormat::Magnitude ; else
  if ( id == "" ) source.ParseErr( "number format expected" ); else
  source.ParseErr( "unknown number format '" + std::string( id ) + "'", true );

  source.ExpectEOL();
  axis->SetNumberFormat( number_format );
}

//------------------------------------------------------------------------------

void do_Axis_NumberSign( Chart::Axis* axis )
{
  bool number_sign;
  source.GetSwitch( number_sign );
  source.ExpectEOL();
  axis->SetNumberSign( number_sign );
}

//------------------------------------------------------------------------------

void do_Axis_NumberUnit( Chart::Axis* axis )
{
  std::string txt;
  source.GetText( txt, false );
  for ( char& c : txt ) {
    if ( c != '_' ) break;
    c = ' ';
  }
  axis->SetNumberUnit( txt );
}

//------------------------------------------------------------------------------

void do_Axis_MinorNumber( Chart::Axis* axis )
{
  bool minor_num;
  source.GetSwitch( minor_num );
  source.ExpectEOL();
  axis->ShowMinorNumbers( minor_num );
}

//------------------------------------------------------------------------------

void do_Axis_NumberPos( Chart::Axis* axis )
{
  Chart::Pos pos;
  source.SkipWS();
  do_Pos( pos );
  source.ExpectEOL();
  axis->SetNumberPos( pos );
}

//------------------------------------------------------------------------------

void do_Axis_NumberSize( Chart::Axis* axis )
{
  double size;
  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "number size value expected" );
  source.GetDouble( size );
  if ( size < 0.01 || size > 100 ) {
    source.ParseErr( "number size value out of range", true );
  }
  source.ExpectEOL();
  axis->SetNumberSize( size );
}

//------------------------------------------------------------------------------

void do_LegendHeading( void )
{
  std::string txt;
  source.GetText( txt, true );
  CurChart()->SetLegendHeading( txt );
}

void do_LegendFrame( void )
{
  bool frame;
  source.GetSwitch( frame );
  source.ExpectEOL();
  CurChart()->SetLegendFrame( frame );
}

void do_LegendPos( void )
{
  Chart::Pos pos;
  source.SkipWS();
  do_Pos( pos );
  source.ExpectEOL();
  CurChart()->SetLegendPos( pos );
}

void do_LegendSize( void )
{
  double size;
  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "legend size value expected" );
  source.GetDouble( size );
  if ( size < 0.01 || size > 100 ) {
    source.ParseErr( "legend size value out of range", true );
  }
  source.ExpectEOL();
  CurChart()->SetLegendSize( size );
}

//------------------------------------------------------------------------------

void do_BarWidth( void )
{
  double one_width;
  double all_width;

  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "width expected" );
  source.GetDouble( one_width );
  if ( one_width < 0.0 || one_width > 1.0 ) {
    source.ParseErr( "relative width out of range [0.0;1.0]", true );
  }

  all_width = 1.0;
  if ( !source.AtEOL() ) {
    source.ExpectWS();
    if ( !source.AtEOL() ) {
      source.GetDouble( all_width );
      if ( all_width < 0.0 || all_width > 1.0 ) {
        source.ParseErr( "relative width out of range [0.0;1.0]", true );
      }
    }
  }

  source.ExpectEOL();

  CurChart()->SetBarWidth( one_width, all_width );
}

void do_LayeredBarWidth( void )
{
  double width;
  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "width expected" );
  source.GetDouble( width );
  if ( width <= 0.0 || width > 1.0 ) source.ParseErr( "invalid width", true );
  source.ExpectEOL();
  CurChart()->SetLayeredBarWidth( width );
}

void do_BarMargin( void )
{
  double margin;

  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "margin expected" );
  source.GetDouble( margin );
  if ( margin < 0.0 ) source.ParseErr( "invalid margin", true );

  source.ExpectEOL();

  CurChart()->SetBarMargin( margin );
}

//------------------------------------------------------------------------------

void NextSeriesStyle( void )
{
  state.style = (state.style + 1) % 80;
}

void ApplyMarkerSize( Chart::Series* series )
{
  if ( state.marker_size >= 0 ) {
    if (
      state.marker_size == 0 &&
      ( state.series_type == Chart::SeriesType::Scatter ||
        state.series_type == Chart::SeriesType::Point
      )
    ) {
      series->SetMarkerSize( 12 );
    } else {
      series->SetMarkerSize( state.marker_size );
    }
  }
}

void AddSeries( std::string name = "" )
{
  if ( !state.series_type_defined ) {
    state.series_type = Chart::SeriesType::Line;
    state.series_type_defined = true;
  }
  state.type_list.push_back( state.series_type );
  state.series_list.push_back( CurChart()->AddSeries( state.series_type ) );
  auto series = state.series_list.back();
  series->SetName( name );
  series->SetSnap( state.snap );
  series->SetPruneDist( state.prune_dist );
  series->SetGlobalLegend( state.global_legend );
  series->SetLegendOutline( state.legend_outline );
  series->SetAxisY( state.axis_y_n );
  series->SetBase( state.series_base );
  series->SetStyle( state.style );
  NextSeriesStyle();
  series->SetMarkerShape( state.marker_shape );
  ApplyMarkerSize( series );
  if ( state.line_width >= 0 ) {
    series->SetLineWidth( state.line_width );
  }
  if ( state.line_dash >= 0 ) {
    series->SetLineDash( state.line_dash, state.line_hole );
  }
  if ( state.fill_transparency >= 0 ) {
    series->FillColor()->SetTransparency( state.fill_transparency );
  }
  series->LineColor()->Lighten( state.lighten );
  series->FillColor()->Lighten( state.lighten );
  series->SetTagEnable( state.tag_enable );
  series->SetTagPos( state.tag_pos );
  series->SetTagSize( state.tag_size );
  series->SetTagBox( state.tag_box );
  series->TagTextColor()->Set( &state.tag_text_color );
  series->TagFillColor()->Set( &state.tag_fill_color );
  series->TagLineColor()->Set( &state.tag_line_color );
  state.defining_series = true;
}

void do_Series_Type( void )
{
  source.SkipWS();
  std::string_view id = source.GetIdentifier();
  if ( id == "XY"          ) state.series_type = Chart::SeriesType::XY         ; else
  if ( id == "Scatter"     ) state.series_type = Chart::SeriesType::Scatter    ; else
  if ( id == "Line"        ) state.series_type = Chart::SeriesType::Line       ; else
  if ( id == "Point"       ) state.series_type = Chart::SeriesType::Point      ; else
  if ( id == "Lollipop"    ) state.series_type = Chart::SeriesType::Lollipop   ; else
  if ( id == "Bar"         ) state.series_type = Chart::SeriesType::Bar        ; else
  if ( id == "StackedBar"  ) state.series_type = Chart::SeriesType::StackedBar ; else
  if ( id == "LayeredBar"  ) state.series_type = Chart::SeriesType::LayeredBar ; else
  if ( id == "Area"        ) state.series_type = Chart::SeriesType::Area       ; else
  if ( id == "StackedArea" ) state.series_type = Chart::SeriesType::StackedArea; else
  if ( id == "" ) source.ParseErr( "series type expected" ); else
  source.ParseErr( "unknown series type '" + std::string( id ) + "'", true );
  source.ExpectEOL();
  state.series_type_defined = true;
}

void do_Series_New( void )
{
  std::string txt;
  source.GetText( txt, true );
  AddSeries( txt );
}

void do_Series_Snap( void )
{
  source.GetSwitch( state.snap );
  source.ExpectEOL();
  if ( state.defining_series ) {
    state.series_list.back()->SetSnap( state.snap );
  }
}

void do_Series_Prune( void )
{
  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "prune distance expected" );
  source.GetDouble( state.prune_dist );
  if ( state.prune_dist < 0 || state.prune_dist > 100 ) {
    source.ParseErr( "prune distance out of range [0;100]", true );
  }
  source.ExpectEOL();
  if ( state.defining_series ) {
    state.series_list.back()->SetPruneDist( state.prune_dist );
  }
}

void do_Series_GlobalLegend( void )
{
  source.GetSwitch( state.global_legend );
  source.ExpectEOL();
  if ( state.defining_series ) {
    state.series_list.back()->SetGlobalLegend( state.global_legend );
  }
}

void do_Series_LegendOutline( void )
{
  source.GetSwitch( state.legend_outline );
  source.ExpectEOL();
  if ( state.defining_series ) {
    state.series_list.back()->SetLegendOutline( state.legend_outline );
  }
}

void do_Series_Axis( void )
{
  source.SkipWS();
  source.GetAxis( state.axis_y_n );
  source.ExpectEOL();
  if ( state.defining_series ) {
    state.series_list.back()->SetAxisY( state.axis_y_n );
  }
}

void do_Series_Base( void )
{
  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "base expected" );
  source.GetDouble( state.series_base );
  source.ExpectEOL();
  if ( state.defining_series ) {
    state.series_list.back()->SetBase( state.series_base );
  }
}

void do_Series_Style( void )
{
  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "style expected" );
  if ( !source.GetInt64( state.style ) ) source.ParseErr( "malformed style" );
  if ( state.style < 0 || state.style > 79 ) {
    source.ParseErr( "style out of range [0;79]", true );
  }
  source.ExpectEOL();
  if ( state.defining_series ) {
    state.series_list.back()->SetStyle( state.style );
    NextSeriesStyle();
    state.series_list.back()->LineColor()->Lighten( state.lighten );
    state.series_list.back()->FillColor()->Lighten( state.lighten );
    state.series_list.back()->TagTextColor()->Undef();
    state.series_list.back()->TagFillColor()->Undef();
    state.series_list.back()->TagLineColor()->Undef();
  }
  state.marker_size = -1;
  state.line_width = -1;
  state.line_dash = -1;
  state.line_hole = -1;
  state.fill_transparency = -1;
  state.tag_text_color.Undef();
  state.tag_fill_color.Undef();
  state.tag_line_color.Undef();
}

void do_Series_MarkerShape( void )
{
  source.SkipWS();
  std::string_view id = source.GetIdentifier();
  if ( id == "Circle"      ) state.marker_shape = Chart::MarkerShape::Circle     ; else
  if ( id == "Square"      ) state.marker_shape = Chart::MarkerShape::Square     ; else
  if ( id == "Triangle"    ) state.marker_shape = Chart::MarkerShape::Triangle   ; else
  if ( id == "InvTriangle" ) state.marker_shape = Chart::MarkerShape::InvTriangle; else
  if ( id == "Diamond"     ) state.marker_shape = Chart::MarkerShape::Diamond    ; else
  if ( id == "Cross"       ) state.marker_shape = Chart::MarkerShape::Cross      ; else
  if ( id == "Star"        ) state.marker_shape = Chart::MarkerShape::Star       ; else
  if ( id == "LineX"       ) state.marker_shape = Chart::MarkerShape::LineX      ; else
  if ( id == "LineY"       ) state.marker_shape = Chart::MarkerShape::LineY      ; else
  if ( id == "" ) source.ParseErr( "marker shape expected" ); else
  source.ParseErr( "unknown marker shape '" + std::string( id ) + "'", true );
  source.ExpectEOL();
  if ( state.defining_series ) {
    state.series_list.back()->SetMarkerShape( state.marker_shape );
  }
}

void do_Series_MarkerSize( void )
{
  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "marker size expected" );
  source.GetDouble( state.marker_size );
  if ( state.marker_size < 0 || state.marker_size > 100 ) {
    source.ParseErr( "marker size out of range [0;100]", true );
  }
  source.ExpectEOL();
  if ( state.defining_series ) {
    ApplyMarkerSize( state.series_list.back() );
  }
}

void do_Series_LineWidth( void )
{
  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "line width expected" );
  source.GetDouble( state.line_width );
  if ( state.line_width < 0 || state.line_width > 100 ) {
    source.ParseErr( "line width out of range [0;100]", true );
  }
  source.ExpectEOL();
  if ( state.defining_series ) {
    state.series_list.back()->SetLineWidth( state.line_width );
  }
}

void do_Series_LineDash( void )
{
  state.line_dash = 0;
  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "line dash expected" );
  source.GetDouble( state.line_dash );
  if ( state.line_dash < 0 || state.line_dash > 100 ) {
    source.ParseErr( "line dash out of range [0;100]", true );
  }
  state.line_hole = state.line_dash;
  if ( !source.AtEOL() ) {
    source.ExpectWS();
    if ( !source.AtEOL() ) {
      source.GetDouble( state.line_hole );
      if ( state.line_hole < 0 || state.line_hole > 100 ) {
        source.ParseErr( "line hole out of range [0;100]", true );
      }
    }
  }
  source.ExpectEOL();
  if ( state.defining_series ) {
    state.series_list.back()->SetLineDash( state.line_dash, state.line_hole );
  }
}

void do_Series_Lighten( void )
{
  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "lighten value expected" );
  source.GetDouble( state.lighten );
  if ( state.lighten < -1.0 || state.lighten > +1.0 ) {
    source.ParseErr( "lighten value out of range [-1.0;1.0]", true );
  }
  source.ExpectEOL();
  if ( state.defining_series ) {
    state.series_list.back()->LineColor()->Lighten( state.lighten );
    state.series_list.back()->FillColor()->Lighten( state.lighten );
  }
}

void do_Series_FillTransparency( void )
{
  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "transparency value expected" );
  source.GetDouble( state.fill_transparency );
  if ( state.fill_transparency < 0.0 || state.fill_transparency > 1.0 ) {
    source.ParseErr( "transparency value out of range [-1.0;1.0]", true );
  }
  source.ExpectEOL();
  if ( state.defining_series ) {
    state.series_list.back()->
      FillColor()->SetTransparency( state.fill_transparency );
  }
}

void do_Series_Color( void )
{
  if ( !state.defining_series ) {
    source.ParseErr( "Color outside defining series" );
  }
  auto series = state.series_list.back();
  double transparency = -1.0;
  source.GetColor( series->LineColor(), transparency );
  series->LineColor()->Lighten( state.lighten )->SetTransparency( 0.0 );
  series->SetDefaultFillColor();
  if ( transparency >= 0.0 ) {
    series->FillColor()->SetTransparency( transparency );
  } else
  if ( state.fill_transparency >= 0 ) {
    series->FillColor()->SetTransparency( state.fill_transparency );
  }
}

void do_Series_LineColor( void )
{
  if ( !state.defining_series ) {
    source.ParseErr( "LineColor outside defining series" );
  }
  auto series = state.series_list.back();
  source.GetColor( series->LineColor() );
  series->LineColor()->Lighten( state.lighten );
}

void do_Series_FillColor( void )
{
  if ( !state.defining_series ) {
    source.ParseErr( "FillColor outside defining series" );
  }
  auto series = state.series_list.back();
  source.GetColor( series->FillColor() );
  series->FillColor()->Lighten( state.lighten );
  if ( state.fill_transparency >= 0 ) {
    series->FillColor()->SetTransparency( state.fill_transparency );
  }
}

//------------------------------------------------------------------------------

void do_Series_Tag( void )
{
  source.GetSwitch( state.tag_enable );
  source.ExpectEOL();
  if ( state.defining_series ) {
    state.series_list.back()->SetTagEnable( state.tag_enable );
  }
}

void do_Series_TagPos( void )
{
  source.SkipWS();
  do_Pos( state.tag_pos );
  source.ExpectEOL();
  if ( state.defining_series ) {
    state.series_list.back()->SetTagPos( state.tag_pos );
  }
}

void do_Series_TagSize( void )
{
  source.SkipWS();
  if ( source.AtEOL() ) source.ParseErr( "tag size value expected" );
  source.GetDouble( state.tag_size );
  if ( state.tag_size < 0.01 || state.tag_size > 100 ) {
    source.ParseErr( "tag size value out of range", true );
  }
  source.ExpectEOL();
  if ( state.defining_series ) {
    state.series_list.back()->SetTagSize( state.tag_size );
  }
}

void do_Series_TagBox( void )
{
  source.GetSwitch( state.tag_box );
  source.ExpectEOL();
  if ( state.defining_series ) {
    state.series_list.back()->SetTagBox( state.tag_box );
  }
}

void do_Series_TagTextColor( void )
{
  source.GetColor( &state.tag_text_color );
  if ( state.defining_series ) {
    state.series_list.back()->TagTextColor()->Set( &state.tag_text_color );
  }
}

void do_Series_TagFillColor( void )
{
  source.GetColor( &state.tag_fill_color );
  if ( state.defining_series ) {
    state.series_list.back()->TagFillColor()->Set( &state.tag_fill_color );
  }
}

void do_Series_TagLineColor( void )
{
  source.GetColor( &state.tag_line_color );
  if ( state.defining_series ) {
    state.series_list.back()->TagLineColor()->Set( &state.tag_line_color );
  }
}

////////////////////////////////////////////////////////////////////////////////

void parse_series_data( void )
{
  state.defining_series = false;

  uint32_t y_values = 0;
  size_t rows = 0;
  bool no_x_value = false;

  // Do a pre-scan of all the data.
  {
    source.SavePos();
    bool x_is_text = false;
    while ( !source.AtEOF() ) {
      source.SkipWS( true );
      if ( source.AtEOF() ) break;
      bool at_sol = source.AtSOL();
      double d;
      bool got_number = false;
      if ( !x_is_text ) {
        got_number = source.TryGetDoubleOrNone( d );
      }
      if ( !got_number ) {
        std::string_view t;
        bool quoted;
        source.GetCategory( t, quoted );
        if ( !quoted && !t.empty() && at_sol ) {
          if ( t.find( ':' ) != std::string_view::npos ) break;
          auto idx = source.cur_pos.loc.char_idx;
          source.SkipWS();
          if ( source.CurChar() == ':' ) break;
          source.cur_pos.loc.char_idx = idx;
        }
        x_is_text = true;
      }
      ++rows;
      uint32_t n = 0;
      while ( source.AtWS() ) {
        source.SkipWS();
        if ( source.AtEOL() ) break;
        while ( !source.AtSep() ) source.GetChar();
        ++n;
      }
      y_values = std::max( y_values, n );
      source.ExpectEOL();
    }
    if ( rows == 0 ) {
      if ( !source.AtEOF() ) source.ToSOL();
      return;
    }
    if ( state.series_type_defined ) {
      if (
        !x_is_text && y_values == 0 &&
        state.series_type != Chart::SeriesType::XY &&
        state.series_type != Chart::SeriesType::Scatter
      ) {
        no_x_value = true;
      }
    } else {
      if ( !x_is_text ) {
        no_x_value = true;
        ++y_values;
      }
      state.series_type = Chart::SeriesType::Line;
      state.series_type_defined = true;
    }
    if ( y_values == 0 ) y_values = 1;
    source.RestorePos();
  }

  // Auto-add new series if needed.
  for ( uint32_t i = 0; i < y_values; i++ ) {
    if (
      state.series_list.size() == i ||
      state.series_list[ state.series_list.size() - i - 1 ]->datum_num > 0
    )
      AddSeries();
  }

  // Detect series types.
  bool x_is_num = false;
  bool x_is_txt = false;
  for ( uint32_t i = 0; i < y_values; i++ ) {
    auto t = state.type_list[ state.type_list.size() - i - 1 ];
    if (
      t == Chart::SeriesType::XY ||
      t == Chart::SeriesType::Scatter
    )
      x_is_num = true;
    else
      x_is_txt = true;
  }
  if ( x_is_num && x_is_txt ) {
    source.ParseErr(
      "cannot mix XY/Scatter series types with other series types"
    );
  }

  if ( rows > 0 ) {
    source.SkipWS( true );
    source.ToSOL();
  }
  if ( x_is_txt ) {
    CurChart()->SetCategoryAnchor( rows, no_x_value );
  }
  for ( uint32_t i = 0; i < y_values; i++ ) {
    auto series = state.series_list[ state.series_list.size() + i - y_values ];
    series->SetDatumAnchor( rows, state.category_idx, no_x_value, i );
  }

  while ( rows-- ) {
    source.SkipWS( true );
    if ( x_is_txt ) {
      if ( !no_x_value ) {
        std::string_view cat;
        bool quoted;
        source.GetCategory( cat, quoted );
        CurChart()->ParsedCat( state.category_idx, cat );
      } else {
        CurChart()->ParsedCat( state.category_idx, "" );
      }
      state.category_idx++;
    } else {
      double x;
      source.GetDoubleOrNone( x );
    }
    for ( uint32_t n = 0; n < y_values; ++n ) {
      source.SkipWS();
      if ( !source.AtEOL() ) {
        double y;
        source.GetDoubleOrNone( y );
      }
    }
    source.ExpectEOL();
  }

  state.defining_series = false;
  return;
}

void do_Series_Data( void )
{
  source.ExpectEOL();
  source.NextLine();
  parse_series_data();
}

////////////////////////////////////////////////////////////////////////////////

using ChartAction = std::function< void() >;

std::unordered_map< std::string_view, ChartAction > chart_actions = {
  { "Margin"                 , do_Margin                  },
  { "BorderColor"            , do_BorderColor             },
  { "BorderWidth"            , do_BorderWidth             },
  { "Padding"                , do_Padding                 },
  { "GridPadding"            , do_GridPadding             },
  { "GlobalLegendHeading"    , do_GlobalLegendHeading     },
  { "GlobalLegendFrame"      , do_GlobalLegendFrame       },
  { "GlobalLegendPos"        , do_GlobalLegendPos         },
  { "GlobalLegendSize"       , do_GlobalLegendSize        },
  { "GlobalLegendColor"      , do_GlobalLegendColor       },
  { "LetterSpacing"          , do_LetterSpacing           },
  { "NewChartInGrid"         , do_NewChartInGrid          },
  { "NewChartInChart"        , do_NewChartInChart         },
  { "ChartPadding"           , do_ChartPadding            },
  { "ChartArea"              , do_ChartArea               },
  { "ChartBox"               , do_ChartBox                },
  { "ForegroundColor"        , do_ForegroundColor         },
  { "BackgroundColor"        , do_BackgroundColor         },
  { "ChartAreaColor"         , do_ChartAreaColor          },
  { "AxisColor"              , do_AxisColor               },
  { "GridColor"              , do_GridColor               },
  { "TextColor"              , do_TextColor               },
  { "FrameColor"             , do_FrameColor              },
  { "TitleHTML"              , do_TitleHTML               },
  { "GlobalTitle"            , do_GlobalTitle             },
  { "GlobalSubTitle"         , do_GlobalSubTitle          },
  { "GlobalSubSubTitle"      , do_GlobalSubSubTitle       },
  { "GlobalTitlePos"         , do_GlobalTitlePos          },
  { "GlobalTitleSize"        , do_GlobalTitleSize         },
  { "GlobalTitleLine"        , do_GlobalTitleLine         },
  { "Title"                  , do_Title                   },
  { "SubTitle"               , do_SubTitle                },
  { "SubSubTitle"            , do_SubSubTitle             },
  { "TitleFrame"             , do_TitleFrame              },
  { "TitlePos"               , do_TitlePos                },
  { "TitleInside"            , do_TitleInside             },
  { "TitleSize"              , do_TitleSize               },
  { "Footnote"               , do_Footnote                },
  { "FootnotePos"            , do_FootnotePos             },
  { "FootnoteSize"           , do_FootnoteSize            },
  { "FootnoteLine"           , do_FootnoteLine            },
  { "LegendHeading"          , do_LegendHeading           },
  { "LegendFrame"            , do_LegendFrame             },
  { "LegendPos"              , do_LegendPos               },
  { "LegendSize"             , do_LegendSize              },
  { "BarWidth"               , do_BarWidth                },
  { "LayeredBarWidth"        , do_LayeredBarWidth         },
  { "BarMargin"              , do_BarMargin               },
  { "Series.Type"            , do_Series_Type             },
  { "Series.New"             , do_Series_New              },
  { "Series.Snap"            , do_Series_Snap             },
  { "Series.Prune"           , do_Series_Prune            },
  { "Series.GlobalLegend"    , do_Series_GlobalLegend     },
  { "Series.LegendOutline"   , do_Series_LegendOutline    },
  { "Series.Axis"            , do_Series_Axis             },
  { "Series.Base"            , do_Series_Base             },
  { "Series.Style"           , do_Series_Style            },
  { "Series.MarkerShape"     , do_Series_MarkerShape      },
  { "Series.MarkerSize"      , do_Series_MarkerSize       },
  { "Series.LineWidth"       , do_Series_LineWidth        },
  { "Series.LineDash"        , do_Series_LineDash         },
  { "Series.Lighten"         , do_Series_Lighten          },
  { "Series.FillTransparency", do_Series_FillTransparency },
  { "Series.Color"           , do_Series_Color            },
  { "Series.LineColor"       , do_Series_LineColor        },
  { "Series.FillColor"       , do_Series_FillColor        },
  { "Series.Tag"             , do_Series_Tag              },
  { "Series.TagPos"          , do_Series_TagPos           },
  { "Series.TagSize"         , do_Series_TagSize          },
  { "Series.TagBox"          , do_Series_TagBox           },
  { "Series.TagTextColor"    , do_Series_TagTextColor     },
  { "Series.TagFillColor"    , do_Series_TagFillColor     },
  { "Series.TagLineColor"    , do_Series_TagLineColor     },
  { "Series.Data"            , do_Series_Data             },
};

using AxisAction = std::function< void( Chart::Axis* ) >;

std::unordered_map< std::string_view, AxisAction > axis_actions = {
  { "Orientation" , do_Axis_Orientation  },
  { "Reverse"     , do_Axis_Reverse      },
  { "Style"       , do_Axis_Style        },
  { "Label"       , do_Axis_Label        },
  { "SubLabel"    , do_Axis_SubLabel     },
  { "LabelSize"   , do_Axis_LabelSize    },
  { "Unit"        , do_Axis_Unit         },
  { "UnitPos"     , do_Axis_UnitPos      },
  { "LogScale"    , do_Axis_LogScale     },
  { "Range"       , do_Axis_Range        },
  { "Pos"         , do_Axis_Pos          },
  { "Tick"        , do_Axis_Tick         },
  { "TickSpacing" , do_Axis_TickSpacing  },
  { "Grid"        , do_Axis_Grid         },
  { "GridStyle"   , do_Axis_GridStyle    },
  { "GridColor"   , do_Axis_GridColor    },
  { "NumberFormat", do_Axis_NumberFormat },
  { "NumberSign"  , do_Axis_NumberSign   },
  { "NumberUnit"  , do_Axis_NumberUnit   },
  { "MinorNumber" , do_Axis_MinorNumber  },
  { "NumberPos"   , do_Axis_NumberPos    },
  { "NumberSize"  , do_Axis_NumberSize   },
};

bool parse_spec( void )
{
  bool normal_anno = false;
  bool global_anno = false;
  while ( true ) {
    source.SkipWS( true );
    if ( source.AtEOF() ) return false;
    if ( source.AtSOL() ) {
      if ( source.CurChar() == '@' ) {
        if ( source.CurChar( 1 ) == '@' ) {
          if ( !global_anno ) ensemble.AddAnnotationAnchor();
          global_anno = true;
          normal_anno = false;
        } else {
          if ( !normal_anno ) CurChart()->AddAnnotationAnchor();
          normal_anno = true;
          global_anno = false;
        }
      } else {
        normal_anno = false;
        global_anno = false;
      }
    }
    if ( normal_anno || global_anno ) {
      source.NextLine();
      continue;
    }
    break;
  }

  std::string_view key = source.GetKey();

  bool ok = false;

  do {
    if ( key.substr( 0, 5 ) == "Axis." ) {
      size_t i = key.find( '.', 5 );
      if ( i == std::string::npos ) break;
      std::string axis_id{ key.substr( 5, i - 5 ) };
      Chart::Axis* axis = nullptr;
      if ( axis_id == "X"    ) axis = CurChart()->AxisX(   ); else
      if ( axis_id == "Y"    ) axis = CurChart()->AxisY(   ); else
      if ( axis_id == "Y1"   ) axis = CurChart()->AxisY( 0 ); else
      if ( axis_id == "PriY" ) axis = CurChart()->AxisY( 0 ); else
      if ( axis_id == "Y2"   ) axis = CurChart()->AxisY( 1 ); else
      if ( axis_id == "SecY" ) axis = CurChart()->AxisY( 1 ); else
      break;
      auto it = axis_actions.find( key.substr( i + 1 ) );
      if ( it == axis_actions.end() ) break;
      it->second( axis );
      ok = true;
    } else {
      auto it = chart_actions.find( key );
      if ( it == chart_actions.end() ) break;
      it->second();
      ok = true;
    }
  } while ( false );

  if ( !ok ) {
    source.ParseErr( "unknown KEY '" + std::string( key ) + "'", true );
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void parse_lines( void )
{
  source.LoadLine();

  // Support delivering nothing but data (implicit Series.Data).
  parse_series_data();

  while ( parse_spec() ) {}
}

////////////////////////////////////////////////////////////////////////////////

std::jmp_buf sigfpe_jmp;

void sigfpe_handler( int signum )
{
  (void)signum;
  longjmp( sigfpe_jmp, 1 );
}

int main( int argc, char* argv[] )
{
  if ( setjmp( sigfpe_jmp ) ) {
    SVG::Canvas* canvas = new SVG::Canvas();
    SVG::Group* g = canvas->TopGroup();
    g->Add( new SVG::Text( 0, 0, "Floating point exception" ) );
    g->Last()->Attr()->TextFont()->SetSize( 48 )->SetBold();
    SVG::BoundaryBox bb = g->Last()->GetBB();
    g->Add(
      new SVG::Rect(
        bb.min.x - 20, bb.min.y - 20, bb.max.x + 20, bb.max.y + 20, 20
      )
    );
    g->FrontToBack();
    g->Last()->Attr()->SetLineWidth( 10 )->FillColor()->Set( SVG::ColorName::tomato );
    std::cout << canvas->GenSVG( 10 );
    source.Err( "Floating point exception" );
  }
  signal( SIGFPE, sigfpe_handler );
  feenableexcept( FE_DIVBYZERO | FE_INVALID );

  bool out_of_options = false;
  for ( int i = 1; i < argc; i++ ) {
    std::string a( argv[ i ] );
    if ( a == "--" ) {
      out_of_options = true;
      continue;
    }
    if ( !out_of_options ) {
      if ( a == "-H" ) {
        ensemble.EnableHTML( true );
        continue;
      }
      if ( a == "-v" || a == "--version" ) {
        show_version();
        return 0;
      }
      if ( a == "-h" || a == "--help" ) {
        show_help();
        return 0;
      }
      if ( a == "-t" ) {
        gen_template( false );
        return 0;
      }
      if ( a == "-T" ) {
        gen_template( true );
        return 0;
      }
      if ( a == "-e0" ) {
        gen_example( 0 );
        return 0;
      }
      if ( a == "-e1" ) {
        gen_example( 1 );
        return 0;
      }
      if ( a == "-e2" ) {
        gen_example( 2 );
        return 0;
      }
      if ( a == "-e3" ) {
        gen_example( 3 );
        return 0;
      }
      if ( a == "-e4" ) {
        gen_example( 4 );
        return 0;
      }
      if ( a == "-e5" ) {
        gen_example( 5 );
        return 0;
      }
      if ( a == "-e6" ) {
        gen_example( 6 );
        return 0;
      }
      if ( a == "-e7" ) {
        gen_example( 7 );
        return 0;
      }
      if ( a == "-e8" ) {
        gen_example( 8 );
        return 0;
      }
      if ( a == "-e9" ) {
        gen_example( 9 );
        return 0;
      }
      if ( a != "-" && a[ 0 ] == '-' ) {
        source.Err( "Unrecognized option '" + a + "'; try --help" );
      }
    }
    source.AddFile( a );
  }

/*
// TBD
  if ( 1 ) {
    Chart::Grid grid;
    grid.Test();
    return 0;
  }
*/

  source.ReadFiles();

  parse_lines();

  std::cout << ensemble.Build();

  source.Quit( 0 );
}

////////////////////////////////////////////////////////////////////////////////
