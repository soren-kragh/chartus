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
#include <charconv>

#include <chart_source.h>

using namespace Chart;

////////////////////////////////////////////////////////////////////////////////

void Source::Err( const std::string& msg )
{
  std::cerr << "*** ERROR: " << msg << std::endl;
  exit( 1 );
}

void Source::ParseErr( const std::string& msg, bool revert_pos )
{
  auto show_pos = [&]( file_pos_t pos )
  {
    cur_pos = pos;
    ToSOL();
    auto col = pos.char_idx - cur_pos.char_idx;
    file_rec_t& file_rec = file_recs[ pos.file_num ];
    std::cerr
      << file_rec.name << " ("
      << pos.line_num << ','
      << col << ')'
      << '\n';
  };

  file_pos_t pos = revert_pos ? ref_pos : cur_pos;
  std::cerr << "*** PARSE ERROR: " << msg << "\n";
  show_pos( pos );
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
}

////////////////////////////////////////////////////////////////////////////////

void Source::AddFile( std::string_view file_name )
{
  file_rec_t file_rec;

  file_rec.name = file_name;

  file_recs.push_back( file_rec );
}

void Source::ProcessLine( const std::string& line )
{
  file_rec_t& file_rec = file_recs[ cur_pos.file_num ];
  bool comment = line.size() >= 1 && line[ 0 ] == '#';
  if ( !comment ) file_rec.data += line;
  file_rec.data += '\n';
  if ( line.size() >= 9 && line.compare( 0, 9, "MacroDef:" ) == 0 ) {
    cur_pos.char_idx += 9;
    SkipWS();
    std::string macro_name{ GetIdentifier() };
    ExpectEOL();
    if ( macros.count( macro_name ) ) {
      ParseErr( "macro '" + macro_name + "' already defined", true );
    }
    macros[ macro_name ] = cur_pos;
  }
  cur_pos.line_num += 1;
  cur_pos.char_idx += line.size() + 1;
}

void Source::ReadFiles()
{
  auto trunc_nl = []( std::string& s )
  {
    while ( s.length() > 0 ) {
      if ( s.back() != '\r' && s.back() != '\n' ) break;
      s.pop_back();
    }
  };

  for ( auto& file_rec : file_recs ) {
    if ( file_rec.name == "-" ) {
      std::string line;
      while ( std::getline( std::cin, line ) ) {
        trunc_nl( line );
        ProcessLine( line );
      }
    } else {
      std::ifstream file( file_rec.name );
      if ( file ) {
        std::string line;
        while ( std::getline( file, line ) ) {
          trunc_nl( line );
          ProcessLine( line );
        }
        file.close();
      } else {
        Err( "Unable to open file '" + file_rec.name + "'" );
      }
    }
    cur_pos.file_num++;
  }
  cur_pos = {};
}

void Source::NextLine()
{
  if ( AtEOF() ) return;
  file_rec_t& file_rec = file_recs[ cur_pos.file_num ];
  while ( file_rec.data[ cur_pos.char_idx++ ] != '\n' ) {}
  cur_pos.line_num++;
  if ( cur_pos.char_idx == file_rec.data.size() ) {
    cur_pos.file_num++;
    cur_pos.char_idx = 0;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool Source::AtEOF()
{
  return cur_pos.file_num == file_recs.size();
}

bool Source::AtSOL()
{
  file_rec_t& file_rec = file_recs[ cur_pos.file_num ];
  return cur_pos.char_idx == 0 || file_rec.data[ cur_pos.char_idx - 1 ] == '\n';
}

bool Source::AtEOL()
{
  return CurChar() == '\n';
}

////////////////////////////////////////////////////////////////////////////////

void Source::ToSOL()
{
  while ( !AtSOL() ) cur_pos.char_idx--;
}

void Source::ToEOL()
{
  while ( !AtEOL() ) cur_pos.char_idx++;
}

char Source::CurChar()
{
  return file_recs[ cur_pos.file_num ].data[ cur_pos.char_idx ];
}

char Source::GetChar()
{
  char c = CurChar();
  if ( c != '\n' ) cur_pos.char_idx++;
  return c;
}

void Source::SkipWS( bool multi_line )
{
  while ( !AtEOF() ) {
    while ( !AtEOL() ) {
      if ( !AtWS() ) return;
      cur_pos.char_idx++;
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

////////////////////////////////////////////////////////////////////////////////

std::string_view Source::GetIdentifier( bool all_non_ws )
{
  ref_pos = cur_pos;
  char* cur = file_recs[ cur_pos.file_num ].data.data() + cur_pos.char_idx;
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
  cur_pos.char_idx += ptr - cur;
  return std::string_view( cur, ptr - cur );
}

bool Source::GetKey( std::string_view& key )
{
  SkipWS( true );
  if ( AtEOF() ) return false;
  if ( !AtSOL() ) ParseErr( "KEY must be unindented" );
  key = GetIdentifier();
  SkipWS();
  if ( key.empty() ) ParseErr( "KEY expected" );
  if ( CurChar() != ':' ) ParseErr( "':' expected" );
  cur_pos.char_idx++;
  return true;
}

bool Source::GetInt64( int64_t& i )
{
  ref_pos = cur_pos;
  file_rec_t& file_rec = file_recs[ cur_pos.file_num ];

  const char* cur = file_rec.data.data() + cur_pos.char_idx;
  const char* end = file_rec.data.data() + file_rec.data.size();

  int64_t result;
  auto [ptr, ec] = std::from_chars( cur, end, result );

  if ( ec != std::errc() || !IsSep( *ptr ) ) return false;

  cur_pos.char_idx += ptr - cur;
  i = result;
  return true;
}

bool Source::GetDouble( double& d, bool none_allowed )
{
  ref_pos = cur_pos;
  file_rec_t& file_rec = file_recs[ cur_pos.file_num ];

  const char* cur = file_rec.data.data() + cur_pos.char_idx;
  const char* end = file_rec.data.data() + file_rec.data.size();

  if ( none_allowed && (*cur == '!' || *cur == '-') && IsSep( *(cur + 1) ) ) {
    d = (*cur == '!') ? Chart::num_invalid : Chart::num_skip;
    cur_pos.char_idx++;
    return true;
  }

  double result;
  auto [ptr, ec] = std::from_chars( cur, end, result );

  if ( ec != std::errc() || !IsSep( *ptr ) ) return false;

  if ( std::abs( result ) > Chart::num_hi ) {
    ParseErr( "number too big", true );
  }

  cur_pos.char_idx += ptr - cur;
  d = result;
  return true;
}

void Source::GetCategory( std::string_view& cat )
{
  ref_pos = cur_pos;
  char* b = file_recs[ cur_pos.file_num ].data.data() + cur_pos.char_idx;
  bool quoted = *b == '"';
  if ( quoted ) ++b;
  char* p = b;
  while ( *p != '"' && *p != '\n' && (quoted || !IsWS( *p ) ) ) {
    ++p;
  }
  cat = std::string_view( b, (!quoted && p == b + 1 && *b == '-') ? 0 : p - b );
  if ( quoted && *p++ != '"' ) {
    ParseErr( "unmatched quote", true );
  }
  if ( !IsSep( *p ) ) {
    ParseErr( "malformed category", true );
  }
  cur_pos.char_idx += p - b;
}

void Source::GetText( std::string& txt, bool multi_line )
{
  txt.clear();
  SkipWS();
  while ( !AtEOL() ) txt += GetChar();
  while ( txt.length() > 0 && IsWS( txt.back() ) ) txt.pop_back();
  if ( !txt.empty() || !multi_line ) return;

  NextLine();
  SavePos();
  size_t min_indent = 0;
  while ( !AtEOF() ) {
    size_t i = 0;
    while ( AtWS() ) {
      cur_pos.char_idx++;
      i++;
    }
    if ( !AtEOL() ) {
      if ( i == 0 ) break;
      if ( min_indent == 0 || i < min_indent ) min_indent = i;
    }
    NextLine();
  }
  RestorePos();
  if ( min_indent < 1 ) return;

  while ( !AtEOF() ) {
    if ( !txt.empty() ) txt += '\n';
    size_t i = 0;
    while ( AtWS() && i < min_indent ) {
      cur_pos.char_idx++;
      i++;
    }
    if ( i == 0 && !AtEOL() ) break;
    while ( !AtEOL() ) txt += GetChar();
    while ( txt.length() > 0 && IsWS( txt.back() ) ) txt.pop_back();
    NextLine();
  }

  while ( txt.length() > 0 && IsSep( txt.back() ) ) txt.pop_back();
  return;
}

////////////////////////////////////////////////////////////////////////////////
