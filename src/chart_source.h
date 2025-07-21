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
  void ParseErr( const std::string& msg, bool revert_pos = false );

  void SavePos( uint32_t context = 0 );
  void RestorePos( uint32_t context = 0 );

  void AddFile( std::string_view file_name );
  void ProcessLine( const std::string& line );
  void ReadFiles();
  void NextLine();

  static bool IsWS( char c );

  bool AtEOF();         // At end of the last file.
  bool AtSOL();
  bool AtEOL();

  void ToSOL();
  void ToEOL();
  char CurChar();
  char GetChar();

  void SkipWS( bool multi_line = false );

  void ExpectEOL();

  std::string_view GetIdentifier( bool all_non_ws = false );
  bool GetKey( std::string_view& key );
  bool GetInt64( int64_t& i );
  bool GetDouble( double& d, bool none_allowed = false );
  void GetCategory( std::string_view& cat );

private:

  struct file_rec_t {
    std::string name;
    std::string data;
  };

  struct file_pos_t {
    size_t file_num = 0;
    size_t line_num = 0;
    size_t char_idx = 0;
  };

  std::vector< file_rec_t > file_recs;

  std::unordered_map< std::string, file_pos_t > macros;

  file_pos_t ref_pos;
  file_pos_t cur_pos;

  file_pos_t saved_pos[ 2 ];

};

}
