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

#include <fstream>
#include <cstdint>
#include <string>
#include <cstring>
#include <charconv>

#include <chart_source.h>

using namespace Chart;

////////////////////////////////////////////////////////////////////////////////

void Source::Err( const std::string& msg )
{
  std::cerr << "*** ERROR: " << msg << std::endl;
  exit( 1 );
}

void Source::ParseErr( const std::string& msg, bool show_ref )
{
  auto show_loc = [&]( location_t loc, size_t col, bool stack = false )
  {
    file_rec_t& file_rec = file_recs[ loc.file_num ];
    std::cerr
      << file_rec.name << " ("
      << loc.line_num << ','
      << col << ')'
      << (stack ? '>' : ':')
      << '\n';
  };

  std::cerr << "*** PARSE ERROR: " << msg << "\n";

  for ( auto& loc : cur_pos.macro_stack ) {
    show_loc( loc, 0, true );
  }

  auto idx = show_ref ? ref_idx : cur_pos.loc.char_idx;

  if ( AtEOF() ) {
    std::cerr << "at EOF";
  } else {
    ToSOL();
    char* p =
      file_recs[ cur_pos.loc.file_num ].data.data() + cur_pos.loc.char_idx;
    auto col = idx - cur_pos.loc.char_idx;
    show_loc( cur_pos.loc, col );
    idx = cur_pos.loc.char_idx;
    ToEOL();
    std::cerr << std::string_view( p, cur_pos.loc.char_idx - idx ) << '\n';
    std::cerr << std::string( col, ' ' ) << '^';
  }
  std::cerr << std::endl;

  exit( 1 );
}

////////////////////////////////////////////////////////////////////////////////

void Source::SavePos( uint32_t context )
{
  saved_pos[ context ] = cur_pos;
}

void Source::RestorePos( uint32_t context )
{
  cur_pos = saved_pos[ context ];
  ref_idx = cur_pos.loc.char_idx;
}

////////////////////////////////////////////////////////////////////////////////

void Source::AddFile( std::string_view file_name )
{
  file_rec_t file_rec;

  file_rec.name = file_name;

  file_recs.push_back( file_rec );
}

void Source::ProcessLine( std::string& line )
{
  while ( line.length() > 0 ) {
    if ( line.back() != '\r' && line.back() != '\n' ) break;
    line.pop_back();
  }
  file_rec_t& file_rec = file_recs[ cur_pos.loc.file_num ];
  file_rec.data += line;
  file_rec.data += '\n';
  if ( line.size() >= 8 && line.compare( 0, 5, "Macro" ) == 0 ) {
    bool macro_def = line.compare( 5, 3, "Def" ) == 0;
    bool macro_end = line.compare( 5, 3, "End" ) == 0;
    auto sol_loc = cur_pos.loc;
    cur_pos.loc.char_idx += 8;
    SkipWS();
    if ( GetChar() == ':' ) {
      SkipWS();
      std::string macro_name{ GetIdentifier() };
      if ( macro_name.empty() ) ParseErr( "macro name expected", true );
      ExpectEOL();
      if ( macro_def ) {
        if ( !in_macro_name.empty() ) ParseErr( "nested MacroDef not allowed" );
        if ( macros.count( macro_name ) ) {
          ParseErr( "macro '" + macro_name + "' already defined", true );
        }
        macros[ macro_name ] = sol_loc;
        in_macro_name = macro_name;
      } else
      if ( macro_end ) {
        if ( in_macro_name.empty() ) ParseErr( "not defining macro" );
        if ( macro_name != in_macro_name ) {
          ParseErr( "unmatched macro name", true );
        }
        in_macro_name.clear();
      }
    }
    cur_pos.loc = sol_loc;
  }
  cur_pos.loc.line_num += 1;
  cur_pos.loc.char_idx += line.size() + 1;
}

void Source::ReadFiles()
{
  for ( auto& file_rec : file_recs ) {
    if ( file_rec.name == "-" ) {
      std::string line;
      while ( std::getline( std::cin, line ) ) {
        ProcessLine( line );
      }
    } else {
      std::ifstream file( file_rec.name );
      if ( file ) {
        std::string line;
        while ( std::getline( file, line ) ) {
          ProcessLine( line );
        }
        file.close();
      } else {
        Err( "Unable to open file '" + file_rec.name + "'" );
      }
    }
    cur_pos.loc.file_num++;
    cur_pos.loc.line_num = 1;
    cur_pos.loc.char_idx = 0;
  }
  cur_pos = {};
}

void Source::NextLine( bool stay )
{
  while ( !AtEOF() ) {
    if ( !stay ) {
      while ( GetChar() != '\n' ) {}
      cur_pos.loc.line_num++;
    }
    stay = false;
    while (
      cur_pos.loc.char_idx == file_recs[ cur_pos.loc.file_num ].data.size()
    ) {
      cur_pos.loc.file_num++;
      cur_pos.loc.line_num = 1;
      cur_pos.loc.char_idx = 0;
      if ( AtEOF() ) break;
    }
    if ( AtEOF() ) break;
    if ( CurChar() == '#' ) continue;

    file_rec_t& file_rec = file_recs[ cur_pos.loc.file_num ];
    const char* ptr = file_rec.data.data() + cur_pos.loc.char_idx;
    size_t len = file_rec.data.size() - cur_pos.loc.char_idx;

    if ( len >= 5 && memcmp( ptr, "Macro", 5 ) == 0 ) {
      auto sol_loc = cur_pos.loc;
      std::string_view key = GetIdentifier();
      if ( key == "MacroDef" || key == "MacroEnd" || key == "Macro" ) {
        SkipWS();
        if ( CurChar() != ':' ) ParseErr( "':' expected" );
        GetChar();
        SkipWS();
        std::string macro_name{ GetIdentifier() };
        if ( macro_name.empty() ) ParseErr( "macro name expected", true );
        ExpectEOL();
        cur_pos.loc = sol_loc;
        bool macro_def  = key == "MacroDef";
        bool macro_end  = key == "MacroEnd";
        bool macro_call = key == "Macro";
        if ( in_macro_name.empty() ) {
          if ( macro_def ) {
            in_macro_name = macro_name;
          } else
          if ( macro_end ) {
            if ( cur_pos.macro_stack.empty() ) ParseErr( "not defining macro" );
            cur_pos.loc = cur_pos.macro_stack.back();
            cur_pos.macro_stack.pop_back();
          } else
          if ( macro_call ) {
            if ( macros.count( macro_name ) == 0 ) {
              ParseErr( "undefined macro", true );
            }
            for ( auto loc : cur_pos.macro_stack ) {
              if ( cur_pos.loc == loc ) ParseErr( "circular macro call", true );
            }
            cur_pos.macro_stack.push_back( cur_pos.loc );
            cur_pos.loc = macros[ macro_name ];
          }
        } else {
          if ( macro_def ) ParseErr( "nested MacroDef not allowed" );
          if ( macro_end ) {
            if ( macro_name != in_macro_name ) {
              ParseErr( "unmatched macro name", true );
            }
            in_macro_name.clear();
          }
        }
        continue;
      }
      cur_pos.loc = sol_loc;
    }

    if ( in_macro_name.empty() ) break;
  }

  if ( !in_macro_name.empty() ) {
    ParseErr( "macro '" + in_macro_name + "' not ended" );
  }

  ref_idx = cur_pos.loc.char_idx;
  return;
}

////////////////////////////////////////////////////////////////////////////////

void Source::ToSOL()
{
  while ( !AtSOL() ) cur_pos.loc.char_idx--;
}

void Source::ToEOL()
{
  while ( !AtEOL() ) cur_pos.loc.char_idx++;
}

void Source::SkipWS( bool multi_line )
{
  while ( !AtEOF() ) {
    while ( !AtEOL() ) {
      if ( !AtWS() ) return;
      cur_pos.loc.char_idx++;
    }
    if ( !multi_line ) break;
    NextLine();
  }
  return;
}

void Source::ExpectEOL()
{
  SkipWS();
  if ( !AtEOL() ) ParseErr( "garbage at EOL" );
}

void Source::ExpectWS( const std::string& err_msg_if_eol )
{
  auto old_idx = cur_pos.loc.char_idx;
  SkipWS();
  if ( cur_pos.loc.char_idx > old_idx && !AtEOL() ) return;
  if ( AtEOL() && !err_msg_if_eol.empty() ) ParseErr( err_msg_if_eol );
  if ( cur_pos.loc.char_idx == old_idx ) ParseErr( "whitespace expected" );
}

////////////////////////////////////////////////////////////////////////////////

std::string_view Source::GetIdentifier( bool all_non_ws )
{
  ref_idx = cur_pos.loc.char_idx;
  char* cur = file_recs[ cur_pos.loc.file_num ].data.data() + cur_pos.loc.char_idx;
  char* ptr = cur;
  while ( true ) {
    char c = *ptr;
    if (
      (all_non_ws && !IsSep( c )) ||
      (c >= 'a' && c <= 'z') ||
      (c >= 'A' && c <= 'Z') ||
      (c >= '0' && c <= '9') ||
      (c == '.' || c == '-' || c == '+' || c == '_')
    ) {
      ++ptr;
    } else {
      break;
    }
  }
  cur_pos.loc.char_idx += ptr - cur;
  return std::string_view( cur, ptr - cur );
}

bool Source::GetInt64( int64_t& i )
{
  ref_idx = cur_pos.loc.char_idx;
  file_rec_t& file_rec = file_recs[ cur_pos.loc.file_num ];

  const char* cur = file_rec.data.data() + cur_pos.loc.char_idx;
  const char* end = file_rec.data.data() + file_rec.data.size();

  int64_t result;
  const char* p = cur;
  if ( *p == '+' ) {
    ++p;
    if ( *p < '0' || *p > '9' ) --p;
  }
  auto [ptr, ec] = std::from_chars( p, end, result );

  if ( ec != std::errc() || !IsSep( *ptr ) ) return false;

  cur_pos.loc.char_idx += ptr - cur;
  i = result;
  return true;
}

bool Source::GetDouble( double& d, bool none_allowed )
{
  ref_idx = cur_pos.loc.char_idx;
  file_rec_t& file_rec = file_recs[ cur_pos.loc.file_num ];

  const char* cur = file_rec.data.data() + cur_pos.loc.char_idx;
  const char* end = file_rec.data.data() + file_rec.data.size();

  if ( none_allowed && (*cur == '!' || *cur == '-') && IsSep( *(cur + 1) ) ) {
    d = (*cur == '!') ? Chart::num_invalid : Chart::num_skip;
    cur_pos.loc.char_idx++;
    return true;
  }

  double result;
  const char* p = cur;
  if ( *p == '+' ) {
    ++p;
    if ( *p != '.' && (*p < '0' || *p > '9') ) --p;
  }
  auto [ptr, ec] = std::from_chars( p, end, result );

  if ( ec != std::errc() || !IsSep( *ptr ) ) return false;

  if ( std::abs( result ) > Chart::num_hi ) {
    ParseErr( "number too big", true );
  }

  cur_pos.loc.char_idx += ptr - cur;
  d = result;
  return true;
}

void Source::GetCategory( std::string_view& cat, bool& quoted )
{
  ref_idx = cur_pos.loc.char_idx;
  char* cur = file_recs[ cur_pos.loc.file_num ].data.data() + cur_pos.loc.char_idx;
  quoted = *cur == '"';
  char* beg = cur + (quoted ? 1 : 0);
  char* ptr = beg;
  while ( *ptr != '"' && *ptr != '\n' && (quoted || !IsWS( *ptr ) ) ) {
    ++ptr;
  }
  size_t len = ptr - beg;
  if ( quoted ) {
    if ( *ptr++ != '"' ) {
      ParseErr( "unmatched quote", true );
    }
  } else {
    if ( len == 1 && *beg == '-' ) len = 0;
  }
  if ( !IsSep( *ptr ) ) {
    ParseErr( "malformed category", true );
  }
  cat = std::string_view( beg, len );
  cur_pos.loc.char_idx += ptr - cur;
}

void Source::GetText( std::string& txt, bool multi_line )
{
  txt.clear();
  SkipWS();
  while ( !AtEOL() ) txt += GetChar();
  while ( !txt.empty() && IsWS( txt.back() ) ) txt.pop_back();
  if ( !txt.empty() || !multi_line ) return;

  NextLine();
  SavePos();
  size_t min_indent = 0;
  while ( !AtEOF() ) {
    size_t i = 0;
    while ( AtWS() ) {
      cur_pos.loc.char_idx++;
      i++;
    }
    if ( !AtEOL() ) {
      if ( i == 0 ) break;
      if ( min_indent == 0 || i < min_indent ) min_indent = i;
    }
    NextLine();
  }
  RestorePos();

  while ( !AtEOF() ) {
    if ( !txt.empty() ) txt += '\n';
    size_t i = 0;
    while ( AtWS() && i < min_indent ) {
      cur_pos.loc.char_idx++;
      i++;
    }
    if ( i == 0 && !AtEOL() ) break;
    while ( !AtEOL() ) txt += GetChar();
    while ( !txt.empty() && IsWS( txt.back() ) ) txt.pop_back();
    NextLine();
  }

  while ( !txt.empty() && IsSep( txt.back() ) ) txt.pop_back();
  return;
}

std::string_view Source::GetStringView( size_t idx1, size_t idx2 )
{
  char* p = file_recs[ cur_pos.loc.file_num ].data.data() + idx1;
  return std::string_view( p, idx2 - idx1 );
}

////////////////////////////////////////////////////////////////////////////////
