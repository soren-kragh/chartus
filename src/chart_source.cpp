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

#include <chart_source.h>

using namespace Chart;

////////////////////////////////////////////////////////////////////////////////

void Source::ParseErr( const std::string& msg, bool revert_pos )
{
  auto show_pos = [&]( file_pos_t pos )
  {
    cur_pos = pos;
    ToSOL();
    auto col = pos.char_idx - cur_pos.char_idx;
    file_rec_t& file_rec = file_recs[ pos.file_rec_idx ];
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

void Source::AddFile( std::string_view file_name )
{
  file_rec_t file_rec;

  file_rec.name = file_name;

  file_recs.push_back( file_rec );
}

void Source::ProcessLine( const std::string& line )
{
  file_rec_t& file_rec = file_recs[ cur_pos.file_rec_idx ];
  file_rec.data += line;
  file_rec.data += '\n';
  if ( line.size() >= 9 && line.compare( 0, 9, "MacroDef:" ) == 0 ) {
    cur_pos.char_idx += 9;
    SkipWS();
    std::string macro_name = GetIdentifier();
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
}

void Source::NextLine()
{
  if ( AtEOF() ) return;
  ToEOL();
  cur_pos.line_num++;
}

////////////////////////////////////////////////////////////////////////////////

bool Source::IsWS( char c )
{
  return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

////////////////////////////////////////////////////////////////////////////////

bool Source::AtEOF()
{
  file_rec_t& file_rec = file_recs[ cur_pos.file_rec_idx ];
  return cur_pos.char_idx == file_rec.data.size();
}

bool Source::AtSOL()
{
  file_rec_t& file_rec = file_recs[ cur_pos.file_rec_idx ];
  if ( cur_pos.char_idx == 0 ) return true;
  if ( file_rec.data[ cur_pos.char_idx - 1 ] == '\n' ) return true;
  return false;
}

bool Source::AtEOL()
{
  file_rec_t& file_rec = file_recs[ cur_pos.file_rec_idx ];
  if ( AtEOF() ) return true;
  if ( file_rec.data[ cur_pos.char_idx ] == '\n' ) return true;
  if ( AtSOL() && file_rec.data[ cur_pos.char_idx ] == '#' ) return true;
  return false;
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
  if ( AtEOL() ) return '\n';
  file_rec_t& file_rec = file_recs[ cur_pos.file_rec_idx ];
  char c = file_rec.data[ cur_pos.char_idx ];
  return c;
}

char Source::GetChar( bool adv )
{
  char c = CurChar();
  if ( adv && c != '\n' ) cur_pos.char_idx++;
  return c;
}

void Source::SkipWS( bool multi_line )
{
  while ( !AtEOF() ) {
    while ( !AtEOL() ) {
      if ( !IsWS( CurChar() ) ) return;
      cur_pos.char_idx++;
    }
    if ( !multi_line ) break;
    NextLine();
  }
}

void Source::ExpectEOL()
{
  SkipWS();
  if ( !AtEOL() ) ParseErr( "garbage at EOL" );
}

////////////////////////////////////////////////////////////////////////////////

std::string Source::GetIdentifier( bool all_non_ws )
{
  ref_pos = cur_pos;
  std::string id = "";
  while ( !AtEOL() ) {
    char c = CurChar();
    if (
      (all_non_ws && !IsWS( c )) ||
      (c >= 'a' && c <= 'z') ||
      (c >= 'A' && c <= 'Z') ||
      (c >= '0' && c <= '9') ||
      (c == '.' || c == '-' || c == '+' || c == '_')
    ) {
      id.push_back( c );
      cur_pos.char_idx++;
    } else {
      break;
    }
  }
  return id;
}

////////////////////////////////////////////////////////////////////////////////
