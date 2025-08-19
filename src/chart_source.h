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

#pragma once

#include <unordered_map>

#include <chart_common.h>

namespace Chart {

class Source
{
public:

  Source() = default;

  void Err( const std::string& msg );
  void ParseErr( const std::string& msg, bool show_ref = false );

  void SavePos( uint32_t context = 0 );
  void RestorePos( uint32_t context = 0 );

  void AddFile( std::string_view file_name );
  void ProcessLine( std::string& line );
  void ReadFiles();
  void NextLine( bool stay = false );
  void LoadLine() { NextLine( true ); }

  static bool IsWS( char c )
  {
    return c == ' ' || c == '\t' || c == '\r';
  }
  static bool IsSep( char c )
  {
    return IsWS( c ) || c == '\n';
  }

  char CurChar()
  {
    return file_recs[ cur_pos.loc.file_num ].data[ cur_pos.loc.char_idx ];
  }
  char GetChar()
  {
    return file_recs[ cur_pos.loc.file_num ].data[ cur_pos.loc.char_idx++ ];
  }

  bool AtWS() { return IsWS( CurChar() ); }
  bool AtSep() { return IsSep( CurChar() ); }

  bool AtEOF()  // At end of the last file.
  {
    return cur_pos.loc.file_num == file_recs.size();
  }
  bool AtSOL()
  {
    return
      cur_pos.loc.char_idx == 0 ||
      file_recs[ cur_pos.loc.file_num ].data[ cur_pos.loc.char_idx - 1 ] == '\n';
  }
  bool AtEOL()
  {
    return CurChar() == '\n';
  }

  void ToSOL();
  void ToEOL();
  void SkipWS( bool multi_line = false );
  void ExpectEOL();
  void ExpectWS( const std::string& err_msg_if_eol = "" );

  std::string_view GetIdentifier( bool all_non_ws = false );
  bool GetInt64( int64_t& i );
  bool GetDouble( double& d, bool none_allowed = false );
  void GetCategory( std::string_view& cat, bool& quoted );
  void GetText( std::string& txt, bool multi_line );

  // Get string from current line.
  // TBD: Delete if not used.
  std::string_view GetStringView( size_t idx1, size_t idx2 );

  // Get datum from current position. Current position is left right after
  // the Y-value.
  void GetDatum(
    std::string_view& x,
    std::string_view& y,
    bool no_x, uint32_t y_idx
  );

//------------------------------------------------------------------------------

  struct file_rec_t {
    std::string name;
    std::string data;
  };

  std::vector< file_rec_t > file_recs;

  struct location_t {
    size_t file_num = 0;
    size_t line_num = 1;
    size_t char_idx = 0;

    bool operator==( const location_t& other ) const {
      return
        file_num == other.file_num &&
        line_num == other.line_num &&
        char_idx == other.char_idx;
    }
  };

  struct position_t {
    location_t loc;
    std::vector< location_t > macro_stack;
  };

  std::unordered_map< std::string, location_t > macros;
  std::string in_macro_name;

  size_t ref_idx;
  position_t cur_pos;

  position_t saved_pos[ 2 ];
};

}
