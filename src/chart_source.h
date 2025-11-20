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
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <chart_common.h>

namespace Chart {

class Source
{
public:

  Source() = default;

  void Quit( int code );
  void Err( const std::string& msg );
  void ParseErr( const std::string& msg, bool show_ref = false );

  uint32_t SavePos();
  void RestorePos( uint32_t context );

  void AddFile( std::string_view file_name );
  void ProcessSegment();
  void ReadStream( std::istream& input, std::string name );
  void ReadFiles();
  void LoadCurSegment();
  void LoadLine();
  void NextLine( bool stay = false );

  static bool IsLF( char c )
  {
    return c == '\n' || c == '\r';
  }
  static bool IsWS( char c )
  {
    return c == ' ' || c == '\t';
  }
  static bool IsSep( char c )
  {
    return IsWS( c ) || IsLF( c );
  }
  static bool IsLetter( char c )
  {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
  }
  static bool IsDigit( char c )
  {
    return (c >= '0' && c <= '9');
  }

  char CurChar( size_t ofs = 0 )
  {
    return cur_pos.loc.buf[ cur_pos.loc.char_idx + ofs ];
  }
  char GetChar()
  {
    return cur_pos.loc.buf[ cur_pos.loc.char_idx++ ];
  }

  bool AtWS() { return IsWS( CurChar() ); }
  bool AtSep() { return IsSep( CurChar() ); }
  bool AtLetter() { return IsLetter( CurChar() ); }

  bool AtEOF()  // At end of the last file.
  {
    return cur_pos.loc.seg_idx == segments.size();
  }
  bool AtSOL()
  {
    return
      cur_pos.loc.char_idx == 0 ||
      IsLF( cur_pos.loc.buf[ cur_pos.loc.char_idx - 1 ] );
  }
  bool AtEOL()
  {
    return AtEOF() || IsLF( CurChar() );
  }

  void ToSOL();
  void ToEOL();
  void PastEOL();
  void SkipWS( bool multi_line = false );
  void ExpectEOL();
  void ExpectWS( const std::string& err_msg_if_eol = "" );

  std::string_view GetKey( bool try_only = false );
  std::string_view GetIdentifier();
  bool GetInt64( int64_t& i, bool sep_after = true );

  bool GetDoubleFull(
    double& d,
    bool none_allowed, bool sep_after, bool fail_on_error
  );
  void GetDouble( double& d )
  {
    GetDoubleFull( d, false, true, true );
  }
  void GetDoubleOrNone( double& d )
  {
    GetDoubleFull( d, true, true, true );
  }
  bool TryGetDoubleOrNone( double& d )
  {
    return GetDoubleFull( d, true, true, false );
  }

  void GetCategory( std::string_view& cat, bool& quoted );
  void GetText( std::string& txt, bool multi_line );

  // Get datum from current position. Current position is left right after
  // the Y-value.
  void GetDatum(
    std::string_view& x,
    std::string_view& y,
    bool no_x, uint32_t y_idx
  );

  void GetColor( SVG::Color* color );
  void ParseGradientDirection(
    double& x1, double& y1, double& x2, double& y2
  );
  // Returns true if a gradient direction was specified.
  bool GetColorOrGradient(
    SVG::Color* color, std::vector< uint32_t >& base_stop_idx_list
  );
  bool GetColorOrGradient( SVG::Color* color );

  void GetSwitch( bool& flag );

  void GetLetterSpacing(
    double& width_adj,
    double& height_adj,
    double& baseline_adj
  );

  void GetAxis( int& axis_y_n );

//------------------------------------------------------------------------------

  // This is spawned as a new thread and is responsible for pre-loading segments
  // as needed.
  void LoaderThread();

  // Flag to stop LoaderThread().
  std::atomic<bool> stop_loader{ false };

  // Error message from LoaderThread().
  std::string loader_msg;

  std::thread loader_thread;
  std::mutex loader_mutex;
  std::condition_variable loader_cond;

  std::vector< std::string > file_list;

  static constexpr size_t buffer_size = 4 * 1024 * 1024;
  size_t max_buffers = 16;

  struct segment_t {
    std::string name;
    size_t byte_ofs = 0;
    size_t byte_cnt = 0;
    size_t line_ofs = 0;
    int32_t pool_id = 0;
    bool loaded = false;
    char* bufptr = nullptr;
  };

  std::vector< segment_t > segments;

  int32_t active_seg = -1;
  int32_t locked_seg = -1;

  // We have fixed buffers and dynamic buffers in the pool. The fixed buffers
  // are used for STDIN and use negative IDs, while the dynamic buffers use
  // non-negative IDs.
  struct pool_t {
    uint32_t fix_cnt = 0;
    uint32_t dyn_cnt = 0;
    std::unordered_map< int32_t, char* > id2buf;
    std::unordered_map< int32_t, int32_t > id2seg;

    void LRU_UseID( int32_t id );
    int32_t LRU_GetID();
    std::list< int32_t > lru_lst;
    std::unordered_map< int32_t, std::list< int32_t >::iterator > lru_map;
  };
  pool_t pool;

  struct location_t {
    size_t seg_idx = 0;
    size_t line_idx = 0;
    size_t char_idx = 0;
    std::string_view buf;

    bool operator==( const location_t& other ) const {
      return
        seg_idx == other.seg_idx &&
        line_idx == other.line_idx &&
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

  std::unordered_map< uint32_t, position_t > saved_pos;
};

}
