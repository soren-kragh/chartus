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
#include <chart_ensemble.h>

////////////////////////////////////////////////////////////////////////////////

Chart::Ensemble ensemble;

bool grid_max_defined = false;
uint32_t grid_max_row = 0;
uint32_t grid_max_col = 0;
Chart::Pos footnote_pos = Chart::Pos::Auto;

struct state_t {
  std::vector< Chart::Series* > series_list;
  std::vector< Chart::SeriesType > type_list;

  bool defining_series = false;
  bool series_type_defined = false;
  Chart::SeriesType series_type = Chart::SeriesType::XY;
  double prune_dist = 0.3;
  int32_t category_idx = 0;
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

#define ERR( MSG_ ) \
  do \
  { \
    std::cerr << "*** ERROR: " << MSG_ << std::endl; \
    exit( 1 ); \
  } while ( 0 )

////////////////////////////////////////////////////////////////////////////////

void show_version( void )
{
  std::cout << R"EOF(chartus v0.10.0
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
  std::random_device rd{};
  std::mt19937 gen{ rd() };
  switch ( N ) {
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
      std::normal_distribution< double > md{ 0.0, 1.0 };
      std::uniform_real_distribution< double > ad{ 0.0, 2 * M_PI };
      #include <dash_e3.h>
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
    case 4:
    {
      #include <dash_e4.h>
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
      double a[ 4 ] = { 5, 10, 12 };
      double b[ 4 ] = { 8, 10, 40 };
      double c[ 4 ] = { 4, 10,  8 };
      #include <dash_e6.h>
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
  }
  return;
}

////////////////////////////////////////////////////////////////////////////////

bool is_ws( char c )
{
  return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

void trunc_ws( std::string& s )
{
  while ( s.length() > 0 && is_ws( s.back() ) ) {
    s.pop_back();
  }
}

void trunc_nl( std::string& s )
{
  while ( s.length() > 0 ) {
    if ( s.back() != '\r' && s.back() != '\n' ) break;
    s.pop_back();
  }
}

////////////////////////////////////////////////////////////////////////////////

std::vector< std::string > file_names;

struct LineRec {
  std::string line;
  size_t      line_number;
  uint32_t    file_name_idx;
  bool        macro = false;
  bool        macro_end = false;
};

using LineRecIter = std::vector< LineRec >::iterator;

std::vector< LineRec > lines;
LineRecIter            cur_line;
size_t                 cur_col;

std::map< std::string, size_t > macros;

std::vector< LineRecIter > macro_stack;

void next_line( void );

std::vector< LineRecIter > saved_macro_stack[ 2 ];
LineRecIter                saved_line[ 2 ];

void save_line_pos( uint32_t context = 0 )
{
  saved_line[ context ] = cur_line;
  saved_macro_stack[ context ] = macro_stack;
}

void restore_line_pos( uint32_t context = 0 )
{
  cur_line = saved_line[ context ];
  cur_col = 0;
  macro_stack = saved_macro_stack[ context ];
}

// Start column of last parsed identifier.
size_t id_col = 0;

////////////////////////////////////////////////////////////////////////////////

bool at_eof( void )
{
  return cur_line == lines.end();
}

bool at_eol( void )
{
  if ( at_eof() ) return true;
  if ( cur_col >= cur_line->line.length() ) return true;
  if ( cur_col == 0 && cur_line->line[ 0 ] == '#' ) return true;
  return false;
}

bool at_ws( void )
{
  return !at_eol() && is_ws( cur_line->line[ cur_col ] );
}

void skip_ws( bool multi_line = false )
{
  while ( !at_eof() ) {
    while ( !at_eol() ) {
      if ( !is_ws( cur_line->line[ cur_col ] ) ) return;
      cur_col++;
    }
    if ( !multi_line ) break;
    next_line();
  }
}

void parse_err( const std::string& msg, bool revert_col = false )
{
  auto show_pos = [&]( LineRecIter lri, size_t col, bool stack = false )
  {
    std::cerr
      << file_names[ lri->file_name_idx ] << " ("
      << lri->line_number << ','
      << col << ')'
      << (stack ? '>' : ':')
      << '\n';
  };

  if ( revert_col ) cur_col = id_col;
  std::cerr << "*** PARSE ERROR: " << msg << "\n";
  for ( auto lri : macro_stack ) {
    show_pos( lri, 0, true );
  }
  if ( at_eof() ) {
    std::cerr << "at EOF";
  } else {
    if ( cur_col > cur_line->line.length() ) cur_col = cur_line->line.length();
    show_pos( cur_line, cur_col );
    std::cerr << cur_line->line << '\n';
    for ( size_t i = 0; i < cur_col; i++ ) {
      std::cerr << ' ';
    }
    std::cerr << '^';
  }
  std::cerr << '\n';
  exit( 1 );
}

////////////////////////////////////////////////////////////////////////////////

char get_char( bool adv = true )
{
  if ( at_eol() ) return '\r';
  char c = cur_line->line[ cur_col ];
  if ( adv ) cur_col++;
  return c;
}

std::string get_identifier( bool all_non_ws = false )
{
  id_col = cur_col;
  std::string id = "";
  while ( !at_eol() ) {
    char c = cur_line->line[ cur_col ];
    if (
      (all_non_ws && !is_ws( c ) ) ||
      (c >= 'a' && c <= 'z') ||
      (c >= 'A' && c <= 'Z') ||
      (c >= '0' && c <= '9') ||
      (c == '.' || c == '-' || c == '+' || c == '_')
    ) {
      id.push_back( c );
      cur_col++;
    } else {
      break;
    }
  }
  return id;
}

// Attempts to parse an int64_t or double from the current position. If
// successful the function returns true and the current position is advanced,
// otherwise the function returns false and the current position is left
// unchanged.
bool get_int64( int64_t& i )
{
  id_col = cur_col;
  try {
    std::string str = cur_line->line.substr( cur_col );
    str.push_back( ' ' );
    std::istringstream iss( str );
    int64_t num;
    if ( !(iss >> num).fail() ) {
      cur_col += iss.tellg();
      if ( !at_eol() && !at_ws() ) {
        cur_col = id_col;
        return false;
      }
      i = num;
    } else {
      return false;
    }
  } catch ( const std::exception& e ) {
    return false;
  }
  return true;
}
bool get_double( double& d, bool none_allowed = false )
{
  id_col = cur_col;
  if ( none_allowed ) {
    char c = get_char();
    if ( (c == '-' || c == '!') && (at_eol() || at_ws()) ) {
      d = (c == '!') ? Chart::num_invalid : Chart::num_skip;
      return true;
    }
  }
  cur_col = id_col;
  try {
    std::string str = cur_line->line.substr( cur_col );
    str.push_back( ' ' );
    std::istringstream iss( str );
    double num;
    if ( !(iss >> num).fail() ) {
      cur_col += iss.tellg();
      if ( !at_eol() && !at_ws() ) {
        cur_col = id_col;
        return false;
      }
      d = num;
    } else {
      return false;
    }
  } catch ( const std::exception& e ) {
    return false;
  }
  if ( std::abs( d ) > Chart::num_hi ) {
    parse_err( "number too big", true );
  }
  return true;
}

// Read in a text based X-value which defines a category for the series.
bool get_category( std::string& t, bool& quoted )
{
  t.clear();
  quoted = false;
  id_col = cur_col;
  bool in_quote = false;
  while ( !at_eol() ) {
    char c = cur_line->line[ cur_col ];
    if ( c == '"' ) {
      if ( cur_col == id_col ) {
        in_quote = true;
        cur_col++;
        continue;
      } else {
        if ( in_quote ) {
          in_quote = false;
          quoted = true;
          cur_col++;
        }
        break;
      }
    }
    if ( is_ws( c ) ) {
      if ( !in_quote ) break;
    }
    t.push_back( c );
    cur_col++;
  }
  if ( in_quote ) return false;
  if ( !quoted && t == "-" ) t.clear();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool get_key( std::string& key )
{
  key = "";
  skip_ws( true );
  if ( at_eof() ) return false;
  if ( cur_col > 0 ) parse_err( "KEY must be unindented" );
  key = get_identifier();
  skip_ws();
  if ( key == "" ) parse_err( "KEY expected" );
  if ( get_char( false ) != ':' ) parse_err( "':' expected" );
  get_char();
  return true;
}

void get_text( std::string& txt, bool multi_line )
{
  txt = "";
  skip_ws();
  while ( !at_eol() ) txt.push_back( get_char() );
  trunc_ws( txt );
  if ( txt != "" || !multi_line ) return;
  next_line();
  save_line_pos();
  size_t indent = 0;
  while ( !at_eof() ) {
    while ( at_ws() ) cur_col++;
    if ( !at_eol() ) {
      if ( cur_col == 0 ) break;
      if ( indent == 0 || cur_col < indent ) indent = cur_col;
    }
    next_line();
  }
  restore_line_pos();
  if ( indent < 1 ) return;
  while ( !at_eof() ) {
    if ( txt != "" ) txt.push_back( '\n' );
    while ( at_ws() ) cur_col++;
    if ( !at_eol() ) {
      if ( cur_col < indent ) break;
      cur_col = indent;
      while ( !at_eol() ) txt.push_back( get_char() );
      trunc_ws( txt );
    }
    next_line();
  }
  trunc_ws( txt );
}

////////////////////////////////////////////////////////////////////////////////

void expect_eol( void )
{
  skip_ws();
  if ( !at_eol() ) parse_err( "garbage at EOL" );
}

void expect_ws( const std::string err_msg_if_eol = "" )
{
  auto old_col = cur_col;
  skip_ws();
  if ( cur_col > old_col && !at_eol() ) return;
  if ( at_eol() && err_msg_if_eol != "" ) parse_err( err_msg_if_eol );
  if ( cur_col == old_col ) parse_err( "whitespace expected" );
}

////////////////////////////////////////////////////////////////////////////////

void next_line( void )
{
  ++cur_line;

  while ( true ) {
    while ( cur_line != lines.end() && macro_stack.empty() && cur_line->macro ) {
      ++cur_line;
    }
    if ( cur_line == lines.end() ) break;

    if (
      cur_line->line.size() >= 6 &&
      cur_line->line.compare( 0, 6, "Macro:" ) == 0
    ) {
      cur_col = 6;
      skip_ws();
      std::string macro_name = get_identifier();
      expect_eol();
      auto it = macros.find( macro_name );
      if ( it == macros.end() ) {
        parse_err( "macro '" + macro_name + "' is undefined", true );
      }
      for ( auto lri : macro_stack ) {
        if ( lri == cur_line ) {
          parse_err( "circular macro call", true );
        }
      }
      macro_stack.push_back( cur_line );
      cur_line = lines.begin() + it->second + 1;
      continue;
    }

    if ( cur_line->macro_end ) {
      cur_line = ++macro_stack.back();
      macro_stack.pop_back();
      continue;
    }

    break;
  }

  cur_col = 0;
  return;
}

////////////////////////////////////////////////////////////////////////////////

void do_Pos(
  Chart::Pos& pos, int& axis_y_n
)
{
  std::string id = get_identifier( true );
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
  if ( id == "" ) parse_err( "position expected" ); else
  parse_err( "unknown position '" + id + "'", true );
}

void do_Pos(
  Chart::Pos& pos
)
{
  int axis_y_n;
  do_Pos( pos, axis_y_n );
}

void do_Switch(
  bool& flag
)
{
  skip_ws();
  std::string id = get_identifier( true );
  if ( id == "On"  ) flag = true ; else
  if ( id == "Off" ) flag = false; else
  if ( id == "Yes" ) flag = true ; else
  if ( id == "No"  ) flag = false; else
  if ( id == "" ) parse_err( "On/Off (Yes/No) expected" ); else
  parse_err( "On/Off (Yes/No) expected, saw '" + id + "'", true );
}

void do_Color(
  SVG::Color* color
)
{
  skip_ws();
  std::string color_id = get_identifier( true );

  bool color_ok = true;

  color->Clear();
  color->SetTransparency( 0.0 );

  if ( color_id != "None" ) {
    if (color_id.size() != 7 || color_id[0] != '#') {
      color_ok = false;
    }
    for ( char c : color_id.substr( 1 ) ) {
      if ( !std::isxdigit( c ) ) color_ok = false;
    }
    if ( color_ok ) {
      uint8_t r = static_cast<uint8_t>( std::stoi( color_id.substr(1, 2), nullptr, 16) );
      uint8_t g = static_cast<uint8_t>( std::stoi( color_id.substr(3, 2), nullptr, 16) );
      uint8_t b = static_cast<uint8_t>( std::stoi( color_id.substr(5, 2), nullptr, 16) );
      color->Set( r, g, b );
    } else {
      color_ok = color->Set( color_id ) == color;
    }
  }

  if ( !color_ok ) {
    parse_err( "invalid color", true );
  }

  if ( !at_eol() ) {
    double lighten = 0.0;
    expect_ws();
    if ( !at_eol() ) {
      if ( !get_double( lighten ) ) {
        parse_err( "malformed lighten value" );
      }
      if ( lighten < -1.0 || lighten > 1.0 ) {
        parse_err( "lighten value out of range [-1.0;1.0]", true );
      }
      if ( lighten < 0 )
        color->Darken( -lighten );
      else
        color->Lighten( lighten );
    }
  }

  if ( !at_eol() ) {
    double transparency = 0.0;
    expect_ws();
    if ( !at_eol() ) {
      if ( !get_double( transparency ) ) {
        parse_err( "malformed transparency value" );
      }
      if ( transparency < 0.0 || transparency > 1.0 ) {
        parse_err( "transparency value out of range [0.0;1.0]", true );
      }
      color->SetTransparency( transparency );
    }
  }

  expect_eol();
}

////////////////////////////////////////////////////////////////////////////////

bool do_GridPos(
  int64_t& row1, int64_t& col1,
  int64_t& row2, int64_t& col2
)
{
  bool got_pos = false;

  skip_ws();

  if ( get_int64( row1 ) ) {
    if ( row1 < 0 || row1 > 99 ) {
      parse_err( "grid row out of range [0;99]", true );
    }

    expect_ws( "column expected" );
    if ( !get_int64( col1 ) ) parse_err( "malformed column" );
    if ( col1 < 0 || col1 > 99 ) {
      parse_err( "grid column out of range [0;99]", true );
    }

    row2 = row1;
    col2 = col1;

    skip_ws();

    if ( get_int64( row2 ) ) {
      if ( row2 < 0 || row2 > 99 ) {
        parse_err( "grid row out of range [0;99]", true );
      }

      expect_ws( "column expected" );
      if ( !get_int64( col2 ) ) parse_err( "malformed column" );
      if ( col2 < 0 || col2 > 99 ) {
        parse_err( "grid column out of range [0;99]", true );
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
    save_line_pos( 1 );
    ensemble.NewChart( 0, 0, 0, 0 );
  }
  return ensemble.LastChart();
}

void do_New( void )
{
  Chart::Pos align_hor = Chart::Pos::Auto;
  Chart::Pos align_ver = Chart::Pos::Auto;

  int64_t row1 = 0;
  int64_t col1 = 0;
  int64_t row2 = 0;
  int64_t col2 = 0;
  bool grid_given = do_GridPos( row1, col1, row2, col2 );

  skip_ws();
  if ( !at_eol() ) {
    do_Pos( align_hor );
    expect_ws( "vertical position expected" );
    do_Pos( align_ver );
  }

  expect_eol();

  if ( !grid_given && grid_max_defined ) {
    row1 = grid_max_row + 1;
    row2 = row1;
    col1 = 0;
    col2 = grid_max_col;
  }

  if ( row1 > row2 || col1 > col2 ) {
    parse_err( "malformed grid location" );
  }

  if ( non_newed_chart ) {
    restore_line_pos( 1 );
    parse_err(
      "chart specifiers must be preceded by New for multi chart plots"
    );
  }

  if ( !ensemble.NewChart( row1, col1, row2, col2, align_hor, align_ver ) ) {
    parse_err( "grid collision" );
  }

  grid_max_row = std::max( grid_max_row, static_cast<uint32_t>( row2 ) );
  grid_max_col = std::max( grid_max_col, static_cast<uint32_t>( col2 ) );
  grid_max_defined = true;

  state = {};
}

////////////////////////////////////////////////////////////////////////////////

void do_Margin( void )
{
  double m;
  skip_ws();
  if ( at_eol() ) parse_err( "margin expected" );
  if ( !get_double( m ) ) parse_err( "malformed margin" );
  if ( m < 0 || m > 1000 ) {
    parse_err( "margin out of range [0;1000]", true );
  }
  expect_eol();
  ensemble.SetMargin( m );
}

void do_BorderColor( void )
{
  do_Color( ensemble.BorderColor() );
}

void do_BorderWidth( void )
{
  double m;
  skip_ws();
  if ( at_eol() ) parse_err( "border width expected" );
  if ( !get_double( m ) ) parse_err( "malformed border width" );
  if ( m < 0 || m > 1000 ) {
    parse_err( "border width out of range [0;1000]", true );
  }
  expect_eol();
  ensemble.SetBorderWidth( m );
}

void do_Padding( void )
{
  double m;
  skip_ws();
  if ( at_eol() ) parse_err( "padding expected" );
  if ( !get_double( m ) ) parse_err( "malformed padding" );
  if ( m < 0 || m > 1000 ) {
    parse_err( "padding out of range [0;1000]", true );
  }
  expect_eol();
  ensemble.SetPadding( m );
}

void do_GridPadding( void )
{
  double grid_padding;
  double area_padding = 0;

  skip_ws();
  if ( at_eol() ) parse_err( "grid padding expected" );
  if ( !get_double( grid_padding ) ) parse_err( "malformed grid padding" );
  if ( grid_padding > 1000 ) {
    parse_err( "grid padding out of range [-inf;1000]", true );
  }

  skip_ws();
  if ( get_double( area_padding ) ) {
    if ( area_padding < 0 || area_padding > 1000 ) {
      parse_err( "chart area padding out of range [0;1000]", true );
    }
  }

  expect_eol();
  ensemble.SetGridPadding( grid_padding, area_padding );
}

//------------------------------------------------------------------------------

void do_GlobalLegendHeading( void )
{
  std::string txt;
  get_text( txt, true );
  ensemble.SetLegendHeading( txt );
}

void do_GlobalLegendFrame( void )
{
  bool frame;
  do_Switch( frame );
  expect_eol();
  ensemble.SetLegendFrame( frame );
}

void do_GlobalLegendPos( void )
{
  int64_t row1 = 0;
  int64_t col1 = 0;
  int64_t row2 = 0;
  int64_t col2 = 0;
  Chart::Pos pos;
  skip_ws();
  if ( do_GridPos( row1, col1, row2, col2 ) ) {
    Chart::Pos align_hor = Chart::Pos::Auto;
    Chart::Pos align_ver = Chart::Pos::Auto;
    skip_ws();
    if ( !at_eol() ) {
      do_Pos( align_hor );
      expect_ws( "vertical position expected" );
      do_Pos( align_ver );
    }
    expect_eol();
    if (
      !ensemble.SetLegendPos( row1, col1, row2, col2, align_hor, align_ver
    ) ) {
      parse_err( "grid collision" );
    }
  } else {
    do_Pos( pos );
    expect_eol();
    ensemble.SetLegendPos( pos );
  }
}

void do_GlobalLegendSize( void )
{
  double size;
  skip_ws();
  if ( at_eol() ) parse_err( "legend size value expected" );
  if ( !get_double( size ) ) {
    parse_err( "malformed Legend size value" );
  }
  if ( size < 0.01 || size > 100 ) {
    parse_err( "legend size value out of range", true );
  }
  expect_eol();
  ensemble.SetLegendSize( size );
}

void do_GlobalLegendColor( void )
{
  do_Color( ensemble.LegendColor() );
}

void do_LetterSpacing( void )
{
  double width_adj    = 1.0;
  double height_adj   = 1.0;
  double baseline_adj = 1.0;

  skip_ws();
  if ( at_eol() ) parse_err( "width adjustment expected" );
  if ( !get_double( width_adj ) ) {
    parse_err( "malformed width adjustment" );
  }
  if ( width_adj < 0 || width_adj > 100 ) {
    parse_err( "width adjustment out of range [0;100]", true );
  }

  if ( !at_eol() ) {
    expect_ws();
    if ( !at_eol() ) {
      if ( !get_double( height_adj ) ) {
        parse_err( "malformed height adjustment" );
      }
      if ( height_adj < 0 || height_adj > 100 ) {
        parse_err( "height adjustment out of range [0;100]", true );
      }
    }
  }

  if ( !at_eol() ) {
    expect_ws();
    if ( !at_eol() ) {
      if ( !get_double( baseline_adj ) ) {
        parse_err( "malformed baseline adjustment" );
      }
      if ( baseline_adj < 0 || baseline_adj > 100 ) {
        parse_err( "baseline adjustment out of range [0;100]", true );
      }
    }
  }

  expect_eol();

  ensemble.SetLetterSpacing( width_adj, height_adj, baseline_adj );
}

////////////////////////////////////////////////////////////////////////////////

void do_ChartArea( void )
{
  int64_t w;
  int64_t h;

  skip_ws();
  if ( at_eol() ) parse_err( "width expected" );
  if ( !get_int64( w ) ) parse_err( "malformed width" );
  if ( w < 10 || w > 100000 ) {
    parse_err( "width out of range [10;100000]", true );
  }

  expect_ws( "height expected" );
  if ( !get_int64( h ) ) parse_err( "malformed height" );
  if ( h < 10 || h > 100000 ) {
    parse_err( "height out of range [10;100000]", true );
  }

  expect_eol();
  CurChart()->SetChartArea( w, h );
}

void do_ChartBox( void )
{
  bool chart_box;
  do_Switch( chart_box );
  expect_eol();
  CurChart()->SetChartBox( chart_box );
}

//------------------------------------------------------------------------------

void do_ForegroundColor( void )
{
  do_Color( ensemble.ForegroundColor() );
}

void do_BackgroundColor( void )
{
  do_Color( ensemble.BackgroundColor() );
}

void do_ChartAreaColor( void )
{
  do_Color( CurChart()->ChartAreaColor() );
}

void do_AxisColor( void )
{
  do_Color( CurChart()->AxisColor() );
}

void do_GridColor( void )
{
  do_Color( CurChart()->AxisX()->GridColor() );
  for ( auto n : { 0, 1 } ) {
    CurChart()->AxisY( n )->GridColor()->Set( CurChart()->AxisX()->GridColor() );
  }
}

void do_TextColor( void )
{
  do_Color( CurChart()->TextColor() );
}

void do_FrameColor( void )
{
  do_Color( CurChart()->FrameColor() );
}

//------------------------------------------------------------------------------

void do_TitleHTML( void )
{
  std::string txt;
  get_text( txt, false );
  ensemble.TitleHTML( txt );
}

void do_GlobalTitle( void )
{
  std::string txt;
  get_text( txt, true );
  ensemble.SetTitle( txt );
}

void do_GlobalSubTitle( void )
{
  std::string txt;
  get_text( txt, true );
  ensemble.SetSubTitle( txt );
}

void do_GlobalSubSubTitle( void )
{
  std::string txt;
  get_text( txt, true );
  ensemble.SetSubSubTitle( txt );
}

void do_GlobalTitlePos( void )
{
  Chart::Pos pos;
  skip_ws();
  do_Pos( pos );
  expect_eol();
  ensemble.SetTitlePos( pos );
}

void do_GlobalTitleSize( void )
{
  double size;
  skip_ws();
  if ( at_eol() ) parse_err( "title size value expected" );
  if ( !get_double( size ) ) {
    parse_err( "malformed title size value" );
  }
  if ( size < 0.01 || size > 100 ) {
    parse_err( "title size value out of range", true );
  }
  expect_eol();
  ensemble.SetTitleSize( size );
}

void do_GlobalTitleLine( void )
{
  bool title_line;
  do_Switch( title_line );
  expect_eol();
  ensemble.SetTitleLine( title_line );
}

//------------------------------------------------------------------------------

void do_Title( void )
{
  std::string txt;
  get_text( txt, true );
  CurChart()->SetTitle( txt );
}

void do_SubTitle( void )
{
  std::string txt;
  get_text( txt, true );
  CurChart()->SetSubTitle( txt );
}

void do_SubSubTitle( void )
{
  std::string txt;
  get_text( txt, true );
  CurChart()->SetSubSubTitle( txt );
}

void do_TitleFrame( void )
{
  bool frame;
  do_Switch( frame );
  expect_eol();
  CurChart()->SetTitleFrame( frame );
}

void do_TitlePos( void )
{
  Chart::Pos pos_x;
  Chart::Pos pos_y = Chart::Pos::Top;

  skip_ws();
  do_Pos( pos_x );

  if ( !at_eol() ) {
    expect_ws();
    if ( !at_eol() ) {
      do_Pos( pos_y );
    }
  }

  expect_eol();
  CurChart()->SetTitlePos( pos_x, pos_y );
}

void do_TitleInside( void )
{
  bool inside;
  do_Switch( inside );
  expect_eol();
  CurChart()->SetTitleInside( inside );
}

void do_TitleSize( void )
{
  double size;
  skip_ws();
  if ( at_eol() ) parse_err( "title size value expected" );
  if ( !get_double( size ) ) {
    parse_err( "malformed title size value" );
  }
  if ( size < 0.01 || size > 100 ) {
    parse_err( "title size value out of range", true );
  }
  expect_eol();
  CurChart()->SetTitleSize( size );
}

//------------------------------------------------------------------------------

void do_Footnote( void )
{
  std::string txt;
  get_text( txt, true );
  ensemble.AddFootnote( txt );
  ensemble.SetFootnotePos( footnote_pos );
}

void do_FootnotePos( void )
{
  skip_ws();
  do_Pos( footnote_pos );
  expect_eol();
  ensemble.SetFootnotePos( footnote_pos );
}

void do_FootnoteLine( void )
{
  bool footnote_line;
  do_Switch( footnote_line );
  expect_eol();
  ensemble.SetFootnoteLine( footnote_line );
}

void do_FootnoteSize( void )
{
  double size;
  skip_ws();
  if ( at_eol() ) parse_err( "footnote size value expected" );
  if ( !get_double( size ) ) {
    parse_err( "malformed footnote size value" );
  }
  if ( size < 0.01 || size > 100 ) {
    parse_err( "footnote size value out of range", true );
  }
  expect_eol();
  ensemble.SetFootnoteSize( size );
}

//------------------------------------------------------------------------------

void do_Axis_Orientation( Chart::Axis* axis )
{
  bool vertical;

  skip_ws();
  std::string id = get_identifier( true );
  if ( id == "Horizontal" ) vertical = false; else
  if ( id == "Vertical"   ) vertical = true ; else
  if ( id == "" ) parse_err( "axis orientation expected" ); else
  parse_err( "unknown axis orientation '" + id + "'", true );
  expect_eol();

  vertical = (axis == CurChart()->AxisX()) ? vertical : !vertical;
  CurChart()->AxisX(   )->SetAngle( vertical ? 90 :  0 );
  CurChart()->AxisY( 0 )->SetAngle( vertical ?  0 : 90 );
  CurChart()->AxisY( 1 )->SetAngle( vertical ?  0 : 90 );
}

//------------------------------------------------------------------------------

void do_Axis_Reverse( Chart::Axis* axis )
{
  bool reverse;
  do_Switch( reverse );
  expect_eol();
  axis->SetReverse( reverse );
}

//------------------------------------------------------------------------------

void do_Axis_Style( Chart::Axis* axis )
{
  Chart::AxisStyle style;
  skip_ws();

  std::string id = get_identifier( true );
  if ( id == "Auto"   ) style = Chart::AxisStyle::Auto ; else
  if ( id == "None"   ) style = Chart::AxisStyle::None ; else
  if ( id == "Line"   ) style = Chart::AxisStyle::Line ; else
  if ( id == "Arrow"  ) style = Chart::AxisStyle::Arrow; else
  if ( id == "Edge"   ) style = Chart::AxisStyle::Edge ; else
  if ( id == "" ) parse_err( "axis style expected" ); else
  parse_err( "unknown axis style '" + id + "'", true );

  expect_eol();
  axis->SetStyle( style );
}

//------------------------------------------------------------------------------

void do_Axis_Label( Chart::Axis* axis )
{
  std::string txt;
  get_text( txt, true );
  axis->SetLabel( txt );
}

//------------------------------------------------------------------------------

void do_Axis_SubLabel( Chart::Axis* axis )
{
  std::string txt;
  get_text( txt, true );
  axis->SetSubLabel( txt );
}

//------------------------------------------------------------------------------

void do_Axis_LabelSize( Chart::Axis* axis )
{
  double size;
  skip_ws();
  if ( at_eol() ) parse_err( "label size value expected" );
  if ( !get_double( size ) ) {
    parse_err( "malformed label size value" );
  }
  if ( size < 0.01 || size > 100 ) {
    parse_err( "label size value out of range", true );
  }
  expect_eol();
  axis->SetLabelSize( size );
}

//------------------------------------------------------------------------------

void do_Axis_Unit( Chart::Axis* axis )
{
  std::string txt;
  get_text( txt, true );
  axis->SetUnit( txt );
}

//------------------------------------------------------------------------------

void do_Axis_UnitPos( Chart::Axis* axis )
{
  Chart::Pos pos;
  skip_ws();
  do_Pos( pos );
  expect_eol();
  axis->SetUnitPos( pos );
}

//------------------------------------------------------------------------------

void do_Axis_LogScale( Chart::Axis* axis )
{
  bool log_scale;
  do_Switch( log_scale );
  expect_eol();
  axis->SetLogScale( log_scale );
}

//------------------------------------------------------------------------------

void do_Axis_Range( Chart::Axis* axis )
{
  double min;
  double max;
  double cross;

  skip_ws();
  if ( at_eol() ) parse_err( "min expected" );
  if ( !get_double( min ) ) parse_err( "malformed min" );

  expect_ws( "max expected" );
  if ( !get_double( max ) ) parse_err( "malformed max" );
  if ( !(max > min) ) parse_err( "max must be greater than min", true );

  cross = 0;
  if ( !at_eol() ) {
    expect_ws();
    if ( !at_eol() ) {
      if ( !get_double( cross ) ) parse_err( "malformed orthogonal axis cross" );
    }
  }

  expect_eol();

  axis->SetRange( min, max, cross );
}

//------------------------------------------------------------------------------

void do_Axis_Pos( Chart::Axis* axis )
{
  Chart::Pos pos;
  int axis_y_n;
  skip_ws();
  do_Pos( pos, axis_y_n );
  expect_eol();
  axis->SetPos( pos, axis_y_n );
}

//------------------------------------------------------------------------------

void do_Axis_Tick( Chart::Axis* axis )
{
  double major;
  int64_t minor;

  skip_ws();
  if ( at_eol() ) parse_err( "major tick expected" );
  if ( !get_double( major ) ) parse_err( "malformed major tick" );
  if ( !(major > 0) ) parse_err( "major tick must be positive", true );

  expect_ws( "minor tick expected" );
  if ( !get_int64( minor ) ) parse_err( "malformed minor tick" );
  if ( minor < 0 || minor > 100 ) {
    parse_err( "minor tick out of range [0;100]", true );
  }

  expect_eol();

  axis->SetTick( major, minor );
}

//------------------------------------------------------------------------------

void do_Axis_TickSpacing( Chart::Axis* axis )
{
  int64_t start = 0;
  int64_t stride = 1;

  skip_ws();
  if ( at_eol() ) parse_err( "start expected" );
  if ( !get_int64( start ) ) parse_err( "malformed start" );
  if ( start < 0 ) {
    parse_err( "invalid start position", true );
  }

  skip_ws();
  if ( get_int64( stride ) ) {
    if ( stride < 1 ) {
      parse_err( "stride must be greater than zero", true );
    }
  }

  expect_eol();
  axis->SetTickSpacing( start, stride );
}

//------------------------------------------------------------------------------

void do_Axis_Grid( Chart::Axis* axis )
{
  bool major;
  bool minor;

  do_Switch( major );

  minor = major;

  if ( !at_eol() ) {
    expect_ws();
    if ( !at_eol() ) {
      do_Switch( minor );
    }
  }

  expect_eol();

  axis->SetGrid( major, minor );
}

//------------------------------------------------------------------------------

void do_Axis_GridStyle( Chart::Axis* axis )
{
  Chart::GridStyle style;
  skip_ws();
  std::string id = get_identifier( true );
  if ( id == "Auto"  ) style = Chart::GridStyle::Auto ; else
  if ( id == "Dash"  ) style = Chart::GridStyle::Dash ; else
  if ( id == "Solid" ) style = Chart::GridStyle::Solid; else
  if ( id == "" ) parse_err( "grid style expected" ); else
  parse_err( "unknown grid style '" + id + "'", true );
  expect_eol();
  axis->SetGridStyle( style );
}

//------------------------------------------------------------------------------

void do_Axis_GridColor( Chart::Axis* axis )
{
  do_Color( axis->GridColor() );
}

//------------------------------------------------------------------------------

void do_Axis_NumberFormat( Chart::Axis* axis )
{
  Chart::NumberFormat number_format;
  skip_ws();

  std::string id = get_identifier( true );
  if ( id == "Auto"       ) number_format = Chart::NumberFormat::Auto      ; else
  if ( id == "None"       ) number_format = Chart::NumberFormat::None      ; else
  if ( id == "Fixed"      ) number_format = Chart::NumberFormat::Fixed     ; else
  if ( id == "Scientific" ) number_format = Chart::NumberFormat::Scientific; else
  if ( id == "Magnitude"  ) number_format = Chart::NumberFormat::Magnitude ; else
  if ( id == "" ) parse_err( "number format expected" ); else
  parse_err( "unknown number format '" + id + "'", true );

  expect_eol();
  axis->SetNumberFormat( number_format );
}

//------------------------------------------------------------------------------

void do_Axis_NumberSign( Chart::Axis* axis )
{
  bool number_sign;
  do_Switch( number_sign );
  expect_eol();
  axis->SetNumberSign( number_sign );
}

//------------------------------------------------------------------------------

void do_Axis_NumberUnit( Chart::Axis* axis )
{
  std::string txt;
  get_text( txt, false );
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
  do_Switch( minor_num );
  expect_eol();
  axis->ShowMinorNumbers( minor_num );
}

//------------------------------------------------------------------------------

void do_Axis_NumberPos( Chart::Axis* axis )
{
  Chart::Pos pos;
  skip_ws();
  do_Pos( pos );
  expect_eol();
  axis->SetNumberPos( pos );
}

//------------------------------------------------------------------------------

void do_Axis_NumberSize( Chart::Axis* axis )
{
  double size;
  skip_ws();
  if ( at_eol() ) parse_err( "number size value expected" );
  if ( !get_double( size ) ) {
    parse_err( "malformed number size value" );
  }
  if ( size < 0.01 || size > 100 ) {
    parse_err( "number size value out of range", true );
  }
  expect_eol();
  axis->SetNumberSize( size );
}

//------------------------------------------------------------------------------

void do_LegendHeading( void )
{
  std::string txt;
  get_text( txt, true );
  CurChart()->SetLegendHeading( txt );
}

void do_LegendFrame( void )
{
  bool frame;
  do_Switch( frame );
  expect_eol();
  CurChart()->SetLegendFrame( frame );
}

void do_LegendPos( void )
{
  Chart::Pos pos;
  skip_ws();
  do_Pos( pos );
  expect_eol();
  CurChart()->SetLegendPos( pos );
}

void do_LegendSize( void )
{
  double size;
  skip_ws();
  if ( at_eol() ) parse_err( "legend size value expected" );
  if ( !get_double( size ) ) {
    parse_err( "malformed Legend size value" );
  }
  if ( size < 0.01 || size > 100 ) {
    parse_err( "legend size value out of range", true );
  }
  expect_eol();
  CurChart()->SetLegendSize( size );
}

//------------------------------------------------------------------------------

void do_BarWidth( void )
{
  double one_width;
  double all_width;

  skip_ws();
  if ( at_eol() ) parse_err( "width expected" );
  if ( !get_double( one_width ) ) parse_err( "malformed width" );
  if ( one_width < 0.0 || one_width > 1.0 ) {
    parse_err( "relative width out of range [0.0;1.0]", true );
  }

  all_width = 1.0;
  if ( !at_eol() ) {
    expect_ws();
    if ( !at_eol() ) {
      if ( !get_double( all_width ) ) parse_err( "malformed width" );
      if ( all_width < 0.0 || all_width > 1.0 ) {
        parse_err( "relative width out of range [0.0;1.0]", true );
      }
    }
  }

  expect_eol();

  CurChart()->SetBarWidth( one_width, all_width );
}

void do_LayeredBarWidth( void )
{
  double width;
  skip_ws();
  if ( at_eol() ) parse_err( "width expected" );
  if ( !get_double( width ) ) parse_err( "malformed width" );
  if ( width <= 0.0 || width > 1.0 ) parse_err( "invalid width", true );
  expect_eol();
  CurChart()->SetLayeredBarWidth( width );
}

void do_BarMargin( void )
{
  double margin;

  skip_ws();
  if ( at_eol() ) parse_err( "margin expected" );
  if ( !get_double( margin ) ) parse_err( "malformed margin" );
  if ( margin < 0.0 ) parse_err( "invalid margin", true );

  expect_eol();

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

void AddSeries( std::string name = "", bool anonymous_snap = false )
{
  if ( !state.series_type_defined ) {
    cur_col = 0;
    parse_err( "undefined SeriesType" );
  }
  state.type_list.push_back( state.series_type );
  state.series_list.push_back( CurChart()->AddSeries( state.series_type ) );
  state.series_list.back()->SetName( name );
  state.series_list.back()->SetAnonymousSnap( anonymous_snap );
  state.series_list.back()->SetPruneDist( state.prune_dist );
  state.series_list.back()->SetGlobalLegend( state.global_legend );
  state.series_list.back()->SetLegendOutline( state.legend_outline );
  state.series_list.back()->SetAxisY( state.axis_y_n );
  state.series_list.back()->SetBase( state.series_base );
  state.series_list.back()->SetStyle( state.style );
  NextSeriesStyle();
  state.series_list.back()->SetMarkerShape( state.marker_shape );
  ApplyMarkerSize( state.series_list.back() );
  if ( state.line_width >= 0 ) {
    state.series_list.back()->SetLineWidth( state.line_width );
  }
  if ( state.line_dash >= 0 ) {
    state.series_list.back()->SetLineDash( state.line_dash, state.line_hole );
  }
  if ( state.fill_transparency >= 0 ) {
    state.series_list.back()->FillColor()->SetTransparency( state.fill_transparency );
  }
  state.series_list.back()->LineColor()->Lighten( state.lighten );
  state.series_list.back()->FillColor()->Lighten( state.lighten );
  state.series_list.back()->SetTagEnable( state.tag_enable );
  state.series_list.back()->SetTagPos( state.tag_pos );
  state.series_list.back()->SetTagSize( state.tag_size );
  state.series_list.back()->SetTagBox( state.tag_box );
  state.series_list.back()->TagTextColor()->Set( &state.tag_text_color );
  state.series_list.back()->TagFillColor()->Set( &state.tag_fill_color );
  state.series_list.back()->TagLineColor()->Set( &state.tag_line_color );
  state.defining_series = true;
}

void do_Series_Type( void )
{
  skip_ws();
  std::string id = get_identifier( true );
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
  if ( id == "" ) parse_err( "series type expected" ); else
  parse_err( "unknown series type '" + id + "'", true );
  expect_eol();
  state.series_type_defined = true;
}

void do_Series_New( void )
{
  std::string txt;
  get_text( txt, true );
  AddSeries( txt );
}

void do_Series_Prune( void )
{
  skip_ws();
  if ( at_eol() ) parse_err( "prune distance expected" );
  if ( !get_double( state.prune_dist ) ) parse_err( "malformed prune distance" );
  if ( state.prune_dist < 0 || state.prune_dist > 100 ) {
    parse_err( "prune distance out of range [0;100]", true );
  }
  expect_eol();
  if ( state.defining_series ) {
    state.series_list.back()->SetPruneDist( state.prune_dist );
  }
}

void do_Series_GlobalLegend( void )
{
  do_Switch( state.global_legend );
  expect_eol();
  if ( state.defining_series ) {
    state.series_list.back()->SetGlobalLegend( state.global_legend );
  }
}

void do_Series_LegendOutline( void )
{
  do_Switch( state.legend_outline );
  expect_eol();
  if ( state.defining_series ) {
    state.series_list.back()->SetLegendOutline( state.legend_outline );
  }
}

void do_Series_Axis( void )
{
  skip_ws();
  std::string id = get_identifier( true );
  if ( id == "Primary"   ) state.axis_y_n = 0; else
  if ( id == "Y1"        ) state.axis_y_n = 0; else
  if ( id == "Secondary" ) state.axis_y_n = 1; else
  if ( id == "Y2"        ) state.axis_y_n = 1; else
  if ( id == "" ) parse_err( "Primary/Y1 or Secondary/Y2 expected" ); else
  parse_err( "unknown Y-axis '" + id + "'", true );
  expect_eol();
  if ( state.defining_series ) {
    state.series_list.back()->SetAxisY( state.axis_y_n );
  }
}

void do_Series_Base( void )
{
  skip_ws();
  if ( at_eol() ) parse_err( "base expected" );
  if ( !get_double( state.series_base ) ) parse_err( "malformed base" );
  expect_eol();
  if ( state.defining_series ) {
    state.series_list.back()->SetBase( state.series_base );
  }
}

void do_Series_Style( void )
{
  skip_ws();
  if ( at_eol() ) parse_err( "style expected" );
  if ( !get_int64( state.style ) ) parse_err( "malformed style" );
  if ( state.style < 0 || state.style > 79 ) {
    parse_err( "style out of range [0;79]", true );
  }
  expect_eol();
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
  skip_ws();
  std::string id = get_identifier( true );
  if ( id == "Circle"      ) state.marker_shape = Chart::MarkerShape::Circle     ; else
  if ( id == "Square"      ) state.marker_shape = Chart::MarkerShape::Square     ; else
  if ( id == "Triangle"    ) state.marker_shape = Chart::MarkerShape::Triangle   ; else
  if ( id == "InvTriangle" ) state.marker_shape = Chart::MarkerShape::InvTriangle; else
  if ( id == "Diamond"     ) state.marker_shape = Chart::MarkerShape::Diamond    ; else
  if ( id == "Cross"       ) state.marker_shape = Chart::MarkerShape::Cross      ; else
  if ( id == "Star"        ) state.marker_shape = Chart::MarkerShape::Star       ; else
  if ( id == "LineX"       ) state.marker_shape = Chart::MarkerShape::LineX      ; else
  if ( id == "LineY"       ) state.marker_shape = Chart::MarkerShape::LineY      ; else
  if ( id == "" ) parse_err( "marker shape expected" ); else
  parse_err( "unknown marker shape '" + id + "'", true );
  expect_eol();
  if ( state.defining_series ) {
    state.series_list.back()->SetMarkerShape( state.marker_shape );
  }
}

void do_Series_MarkerSize( void )
{
  skip_ws();
  if ( at_eol() ) parse_err( "marker size expected" );
  if ( !get_double( state.marker_size ) ) parse_err( "malformed marker size" );
  if ( state.marker_size < 0 || state.marker_size > 100 ) {
    parse_err( "marker size out of range [0;100]", true );
  }
  expect_eol();
  if ( state.defining_series ) {
    ApplyMarkerSize( state.series_list.back() );
  }
}

void do_Series_LineWidth( void )
{
  skip_ws();
  if ( at_eol() ) parse_err( "line width expected" );
  if ( !get_double( state.line_width ) ) parse_err( "malformed line width" );
  if ( state.line_width < 0 || state.line_width > 100 ) {
    parse_err( "line width out of range [0;100]", true );
  }
  expect_eol();
  if ( state.defining_series ) {
    state.series_list.back()->SetLineWidth( state.line_width );
  }
}

void do_Series_LineDash( void )
{
  state.line_dash = 0;
  skip_ws();
  if ( at_eol() ) parse_err( "line dash expected" );
  if ( !get_double( state.line_dash ) ) {
    parse_err( "malformed line dash" );
  }
  if ( state.line_dash < 0 || state.line_dash > 100 ) {
    parse_err( "line dash out of range [0;100]", true );
  }
  state.line_hole = state.line_dash;
  if ( !at_eol() ) {
    expect_ws();
    if ( !at_eol() ) {
      if ( !get_double( state.line_hole ) ) {
        parse_err( "malformed line hole" );
      }
      if ( state.line_hole < 0 || state.line_hole > 100 ) {
        parse_err( "line hole out of range [0;100]", true );
      }
    }
  }
  expect_eol();
  if ( state.defining_series ) {
    state.series_list.back()->SetLineDash( state.line_dash, state.line_hole );
  }
}

void do_Series_Lighten( void )
{
  skip_ws();
  if ( at_eol() ) parse_err( "lighten value expected" );
  if ( !get_double( state.lighten ) ) {
    parse_err( "malformed lighten value" );
  }
  if ( state.lighten < -1.0 || state.lighten > +1.0 ) {
    parse_err( "lighten value out of range [-1.0;1.0]", true );
  }
  expect_eol();
  if ( state.defining_series ) {
    state.series_list.back()->LineColor()->Lighten( state.lighten );
    state.series_list.back()->FillColor()->Lighten( state.lighten );
  }
}

void do_Series_FillTransparency( void )
{
  skip_ws();
  if ( at_eol() ) parse_err( "transparency value expected" );
  if ( !get_double( state.fill_transparency ) ) {
    parse_err( "malformed transparency value" );
  }
  if ( state.fill_transparency < 0.0 || state.fill_transparency > 1.0 ) {
    parse_err( "transparency value out of range [-1.0;1.0]", true );
  }
  expect_eol();
  if ( state.defining_series ) {
    state.series_list.back()->FillColor()->SetTransparency( state.fill_transparency );
  }
}

void do_Series_LineColor( void )
{
  if ( !state.defining_series ) {
    parse_err( "LineColor outside defining series" );
  }
  do_Color( state.series_list.back()->LineColor() );
  state.series_list.back()->LineColor()->Lighten( state.lighten );
}

void do_Series_FillColor( void )
{
  if ( !state.defining_series ) {
    parse_err( "FillColor outside defining series" );
  }
  do_Color( state.series_list.back()->FillColor() );
  state.series_list.back()->FillColor()->Lighten( state.lighten );
  if ( state.fill_transparency >= 0 ) {
    state.series_list.back()->FillColor()->SetTransparency( state.fill_transparency );
  }
}

//------------------------------------------------------------------------------

void do_Series_Tag( void )
{
  do_Switch( state.tag_enable );
  expect_eol();
  if ( state.defining_series ) {
    state.series_list.back()->SetTagEnable( state.tag_enable );
  }
}

void do_Series_TagPos( void )
{
  skip_ws();
  do_Pos( state.tag_pos );
  expect_eol();
  if ( state.defining_series ) {
    state.series_list.back()->SetTagPos( state.tag_pos );
  }
}

void do_Series_TagSize( void )
{
  skip_ws();
  if ( at_eol() ) parse_err( "tag size value expected" );
  if ( !get_double( state.tag_size ) ) {
    parse_err( "malformed tag size value" );
  }
  if ( state.tag_size < 0.01 || state.tag_size > 100 ) {
    parse_err( "tag size value out of range", true );
  }
  expect_eol();
  if ( state.defining_series ) {
    state.series_list.back()->SetTagSize( state.tag_size );
  }
}

void do_Series_TagBox( void )
{
  do_Switch( state.tag_box );
  expect_eol();
  if ( state.defining_series ) {
    state.series_list.back()->SetTagBox( state.tag_box );
  }
}

void do_Series_TagTextColor( void )
{
  do_Color( &state.tag_text_color );
  if ( state.defining_series ) {
    state.series_list.back()->TagTextColor()->Set( &state.tag_text_color );
  }
}

void do_Series_TagFillColor( void )
{
  do_Color( &state.tag_fill_color );
  if ( state.defining_series ) {
    state.series_list.back()->TagFillColor()->Set( &state.tag_fill_color );
  }
}

void do_Series_TagLineColor( void )
{
  do_Color( &state.tag_line_color );
  if ( state.defining_series ) {
    state.series_list.back()->TagLineColor()->Set( &state.tag_line_color );
  }
}

//------------------------------------------------------------------------------

void parse_series_data( bool anonymous_snap = false )
{
  state.defining_series = false;

  uint32_t y_values = 0;
  uint32_t rows = 0;
  bool no_x_value = false;

  // Do a pre-scan of all the data.
  {
    save_line_pos();
    cur_col = 0;
    bool x_is_text = false;
    while ( !at_eof() ) {
      skip_ws( true );
      if ( at_eol() ) break;
      double d;
      bool got_number = false;
      if ( !x_is_text ) {
        if ( get_double( d ) ) {
          got_number = at_eol() || at_ws();
        }
        if ( !got_number ) cur_col = id_col;
      }
      if ( !got_number ) {
        std::string t;
        bool quoted;
        if ( !get_category( t, quoted ) ) {
          parse_err( "unmatched quote", true );
        }
        if ( !quoted && !t.empty() && id_col == 0 ) {
          if ( t.back() == ':' ) break;
          auto save_col = cur_col;
          skip_ws();
          if ( get_char() == ':' ) break;
          cur_col = save_col;
        }
        x_is_text = true;
      }
      if ( !at_eol() && !at_ws() ) {
        parse_err( "syntax error" );
      }
      ++rows;
      uint32_t n = 0;
      while ( at_ws() ) {
        skip_ws();
        if ( at_eol() ) break;
        while ( !at_eol() && !at_ws() ) cur_col++;
        ++n;
      }
      y_values = std::max( y_values, n );
      expect_eol();
    }
    if ( rows == 0 ) {
      cur_col = 0;
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
    restore_line_pos();
  }

  // Auto-add new series if needed.
  for ( uint32_t i = 0; i < y_values; i++ ) {
    if (
      state.series_list.size() == i ||
      state.series_list[ state.series_list.size() - i - 1 ]->Size() > 0
    )
      AddSeries( "", anonymous_snap );
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
    parse_err( "cannot mix XY/Scatter series types with other series types" );
  }

  std::string category;
  std::string_view tag_x;
  while ( rows-- ) {
    skip_ws( true );
    id_col = cur_col;
    if ( at_eol() ) parse_err( "X-value expected" );
    double x;
    if ( x_is_txt ) {
      if ( !no_x_value ) {
        bool quoted;
        if ( !get_category( category, quoted ) ) {
          parse_err( "unmatched quote", true );
        }
      }
      CurChart()->AddCategory( category );
      x = state.category_idx;
      state.category_idx++;
    } else {
      if ( !get_double( x, true ) ) parse_err( "malformed X-value" );
    }
    if ( !no_x_value && !at_eol() && !at_ws() ) {
      parse_err( "syntax error" );
    }
    tag_x =
      std::string_view(
        cur_line->line
      ).substr( id_col, cur_col - id_col );
    for ( uint32_t n = 0; n < y_values; ++n ) {
      uint32_t series_idx = state.series_list.size() - y_values + n;
      skip_ws();
      double y;
      if ( at_eol() && x_is_txt ) {
        y = Chart::num_skip;
        state.series_list[ series_idx ]->Add( x, y );
      } else {
        if ( at_eol() ) parse_err( "Y-value expected" );
        if ( !get_double( y, true ) ) parse_err( "malformed Y-value" );
        if ( !at_eol() && !at_ws() ) parse_err( "syntax error" );
        state.series_list[ series_idx ]->Add(
          x, y,
          tag_x,
          std::string_view(
            cur_line->line
          ).substr( id_col, cur_col - id_col )
        );
      }
    }
    expect_eol();
  }

  return;
}

void do_Series_Data( void )
{
  expect_eol();
  next_line();
  parse_series_data();
}

////////////////////////////////////////////////////////////////////////////////

using ChartAction = std::function< void() >;

std::unordered_map< std::string, ChartAction > chart_actions = {
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
  { "New"                    , do_New                     },
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

std::unordered_map< std::string, AxisAction > axis_actions = {
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
  std::string key;
  if ( !get_key( key ) ) return false;

  bool ok = false;

  do {
    if ( key.substr( 0, 5 ) == "Axis." ) {
      size_t i = key.find( '.', 5 );
      if ( i == std::string::npos ) break;
      std::string axis_id = key.substr( 5, i - 5 );
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

  if ( !ok ) parse_err( "unknown KEY '" + key + "'", true );

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void parse_lines( void )
{
  cur_line = lines.begin();
  while ( cur_line != lines.end() && cur_line->macro ) {
    ++cur_line;
  }
  cur_col = 0;

  // Support delivering nothing but data (implicit Series.Data).
  parse_series_data( true );

  while ( parse_spec() ) {}
}

////////////////////////////////////////////////////////////////////////////////

std::string in_macro_name;

void process_line(
  const std::string& line, size_t line_number, uint32_t file_name_idx
)
{
  bool macro_def = false;
  bool macro_end = false;
  if ( line.size() >= 9 && line.compare( 0, 5, "Macro" ) == 0 ) {
    macro_def = line.compare( 5, 4, "Def:" ) == 0;
    macro_end = line.compare( 5, 4, "End:" ) == 0;
  }
  bool macro = macro_def || !in_macro_name.empty();;
  lines.push_back( { line, line_number, file_name_idx, macro, macro_end } );
  if ( macro_def || macro_end ) {
    cur_line = lines.end() - 1;
    cur_col = 9;
    skip_ws();
    std::string macro_name = get_identifier();
    expect_eol();
    if ( macro_name.empty() ) {
      parse_err( "macro name expected", true );
    }
    if ( macro_def ) {
      if ( !in_macro_name.empty() ) {
        cur_col = 0;
        parse_err( "nested macro definitions not allowed" );
      }
      if ( macros.count( macro_name ) ) {
        parse_err( "macro '" + macro_name + "' already defined", true );
      }
      macros[ macro_name ] = lines.size() - 1;
      in_macro_name = macro_name;
    } else {
      if ( in_macro_name.empty() ) {
        cur_col = 0;
        parse_err( "not defining macro" );
      }
      if ( macro_name != in_macro_name ) {
        parse_err( "macro name mismatch", true );
      }
      in_macro_name.clear();
    }
  }
}

void process_files( const std::vector< std::string >& file_list )
{
  for ( const auto& file_name : file_list ) {
    uint32_t file_name_idx = file_names.size();
    file_names.push_back( file_name );
    size_t line_number = 0;
    if ( file_name == "-" ) {
      std::string line;
      while ( std::getline( std::cin, line ) ) {
        trunc_nl( line );
        process_line( line, ++line_number, file_name_idx );
      }
    } else {
      std::ifstream file( file_name );
      if ( file ) {
        std::string line;
        while ( std::getline( file, line ) ) {
          trunc_nl( line );
          process_line( line, ++line_number, file_name_idx );
        }
        file.close();
      } else {
        ERR( "Unable to open file '" << file_name << "'" );
      }
    }
  }
  if ( !in_macro_name.empty() ) {
    cur_line = lines.end();
    cur_col = 0;
    parse_err( "macro '" + in_macro_name + "' not ended" );
  }
  parse_lines();
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
    ERR( "Floating point exception" );
  }
  signal( SIGFPE, sigfpe_handler );
  feenableexcept( FE_DIVBYZERO | FE_INVALID );

  std::vector< std::string > file_list;

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
      if ( a != "-" && a[ 0 ] == '-' ) {
        ERR( "Unrecognized option '" << a << "'; try --help" );
      }
    }
    file_list.push_back( a );
  }

/*
// TBD
  if ( 1 ) {
    Chart::Grid grid;
    grid.Test();
    return 0;
  }
*/

  if ( file_list.size() == 0 ) {
    file_list.push_back( "-" );
  }

  process_files( file_list );

  std::cout << ensemble.Build();

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
