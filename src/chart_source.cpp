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
#include <filesystem>

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
    segment_t& segment = segments[ loc.seg_idx ];
    std::cerr
      << segment.name << " ("
      << (segment.line_ofs + loc.line_idx + 1) << ','
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
    const char* p = cur_pos.loc.buf.data() + cur_pos.loc.char_idx;
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
  LoadLine();
  ref_idx = cur_pos.loc.char_idx;
}

////////////////////////////////////////////////////////////////////////////////

void Source::pool_t::UseID( int32_t id )
{
  auto it = lru_map.find( id );
  if ( it != lru_map.end() ) {
    lru_lst.splice( lru_lst.begin(), lru_lst, it->second );
  } else {
    lru_lst.push_front( id );
    lru_map[ id ] = lru_lst.begin();
  }
}

int32_t Source::pool_t::GetID()
{
  return lru_lst.back();
}

////////////////////////////////////////////////////////////////////////////////

void Source::AddFile( std::string_view file_name )
{
  file_list.emplace_back( file_name );
}

void Source::ProcessSegment()
{
  segment_t& segment = segments[ cur_pos.loc.seg_idx ];
  cur_pos.loc.buf =
    std::string_view( pool.id2buf[ segment.pool_id ], segment.byte_cnt );
  while ( true ) {
    size_t num = segment.byte_cnt - cur_pos.loc.char_idx;
    if ( num == 0 ) break;
    const char* ptr = cur_pos.loc.buf.data() + cur_pos.loc.char_idx;
    if ( num >= 9 && strncmp( ptr, "Macro", 5 ) == 0 ) {
      bool macro_def = strncmp( ptr + 5, "Def", 3 ) == 0;
      bool macro_end = strncmp( ptr + 5, "End", 3 ) == 0;
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
    PastEOL();
  }
}

void Source::ReadStream( std::istream& input, std::string name )
{
  auto add_segment =
    [&]() {
      segments.emplace_back();
      segments.back().name = name;
      int32_t pool_id;
      if ( name == "-" ) {
        pool.fix_cnt++;
        pool_id = -pool.fix_cnt;
      } else {
        if ( pool.dyn_cnt < 2 || pool.fix_cnt + pool.dyn_cnt < max_buffers ) {
          pool_id = +pool.dyn_cnt;
          pool.dyn_cnt++;
        } else {
          pool_id = pool.GetID();
          segments[ pool.id2seg[ pool_id ] ].loaded = false;
        }
      }
      segments.back().pool_id = pool_id;
      if ( pool.id2buf.find( pool_id ) == pool.id2buf.end() ) {
        pool.id2buf[ pool_id ] =
          static_cast< char* >( malloc( buffer_size + 16 ) );
      }
      pool.id2seg[ pool_id ] = segments.size() - 1;
      if ( pool_id >= 0 ) pool.UseID( pool_id );
    };

  size_t byte_ofs = 0;
  size_t line_ofs = 0;

  auto do_segment =
    [&]( size_t seg_idx ) {
      segment_t& segment = segments[ seg_idx ];
      if (
        segment.byte_cnt > 0 &&
        !IsLF( pool.id2buf[ segment.pool_id ][ segment.byte_cnt - 1 ] )
      ) {
        pool.id2buf[ segment.pool_id ][ segment.byte_cnt++ ] = '\n';
      }
      segment.loaded = true;
      segment.byte_ofs = byte_ofs;
      segment.line_ofs = line_ofs;
      ProcessSegment();
      byte_ofs += cur_pos.loc.buf.size();
      line_ofs += cur_pos.loc.line_idx;
      cur_pos.loc.seg_idx++;
      cur_pos.loc.line_idx = 0;
      cur_pos.loc.char_idx = 0;
      cur_pos.loc.buf = std::string_view();
    };

  add_segment();
  while ( true ) {
    std::streamsize bytes_to_read = buffer_size - segments.back().byte_cnt;
    input.read(
      pool.id2buf[ segments.back().pool_id ] + segments.back().byte_cnt,
      bytes_to_read
    );
    std::streamsize bytes_read = input.gcount();
    segments.back().byte_cnt += bytes_read;
    if ( bytes_read == 0 ) {
      do_segment( segments.size() - 1 );
      break;
    }
    if ( bytes_read < bytes_to_read ) continue;

    size_t to_move = 0;
    while ( true ) {
      auto ptr = pool.id2buf[ segments.back().pool_id ];
      char c = ptr[ buffer_size - 1 - to_move ];
      if ( c == '\n' ) break;
      if ( c == '\r' && to_move > 0 ) break;
      ++to_move;
      if ( to_move == buffer_size ) {
        Err( "line too long while reading '" + name + "'" );
      }
    }

    add_segment();
    if ( to_move > 0 ) {
      size_t src = segments.size() - 2;
      size_t dst = segments.size() - 1;
      auto src_ptr = pool.id2buf[ segments[ src ].pool_id ];
      auto dst_ptr = pool.id2buf[ segments[ dst ].pool_id ];
      memcpy( dst_ptr, src_ptr + segments[ src ].byte_cnt - to_move, to_move );
      segments[ src ].byte_cnt -= to_move;
      segments[ dst ].byte_cnt += to_move;
    }

    do_segment( segments.size() - 2 );
  }

  if ( input.bad() || (input.fail() && !input.eof()) ) {
    Err( "error while reading '" + name + "'" );
  }

  return;
}

void Source::ReadFiles()
{
  if ( file_list.empty() ) AddFile( "-" );

  for ( const auto& file_name : file_list ) {
    if ( file_name == "-" ) {
      ReadStream( std::cin, file_name );
    } else {
      std::ifstream file( file_name, std::ios::binary );
      if ( !file ) {
        Err( "failed to open file '" + file_name + "'" );
      }
      ReadStream( file, file_name );
    }
  }

  cur_pos = {};
}

////////////////////////////////////////////////////////////////////////////////

void Source::LoadSegment()
{
  active_seg = cur_pos.loc.seg_idx;
  if ( !segments[ active_seg ].loaded ) {
    int32_t pool_id = pool.GetID();
    segments[ pool.id2seg[ pool_id ] ].loaded = false;
    pool.id2seg[ pool_id ] = active_seg;
    segments[ active_seg ].pool_id = pool_id;
    {
      std::ifstream file( segments[ active_seg ].name, std::ios::binary );
      if ( !file ) {
        Err( "failed to open file '" + segments[ active_seg ].name + "'" );
      }
      file.seekg( segments[ active_seg ].byte_ofs, std::ios::beg );
      if ( !file ) {
        Err( "seek failed in '" + segments[active_seg].name + "'" );
      }
      std::streamsize bytes_to_read = segments[ active_seg ].byte_cnt;
      file.read( pool.id2buf[ pool_id ], bytes_to_read );
      std::streamsize bytes_read = file.gcount();
      if (
        bytes_read != bytes_to_read ||
        file.bad() || (file.fail() && !file.eof())
      ) {
        Err( "error while reading '" + segments[ active_seg ].name + "'" );
      }
    }
    segments[ active_seg ].loaded = true;
    pool.UseID( pool_id );
  }
}

void Source::LoadLine() {
  LoadSegment();
  cur_pos.loc.buf =
    std::string_view(
      pool.id2buf[ segments[ cur_pos.loc.seg_idx ].pool_id ],
      segments[ cur_pos.loc.seg_idx ].byte_cnt
    );
  NextLine( true );
}

void Source::NextLine( bool stay )
{
  while ( !AtEOF() ) {
    if ( !stay ) {
      PastEOL();
    }
    stay = false;
    while ( cur_pos.loc.char_idx == segments[ cur_pos.loc.seg_idx ].byte_cnt ) {
      cur_pos.loc.seg_idx++;
      cur_pos.loc.line_idx = 0;
      cur_pos.loc.char_idx = 0;
      cur_pos.loc.buf = std::string_view();
      if ( AtEOF() )  break;
      LoadSegment();
      cur_pos.loc.buf =
        std::string_view(
          pool.id2buf[ segments[ cur_pos.loc.seg_idx ].pool_id ],
          segments[ cur_pos.loc.seg_idx ].byte_cnt
        );
    }
    if ( AtEOF() ) break;
    if ( CurChar() == '#' ) continue;

    segment_t& segment = segments[ cur_pos.loc.seg_idx ];
    const char* ptr = cur_pos.loc.buf.data() + cur_pos.loc.char_idx;
    size_t len = segment.byte_cnt - cur_pos.loc.char_idx;

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
            LoadSegment();
            cur_pos.loc.buf =
              std::string_view(
                pool.id2buf[ segments[ cur_pos.loc.seg_idx ].pool_id ],
                segments[ cur_pos.loc.seg_idx ].byte_cnt
              );
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
            LoadSegment();
            cur_pos.loc.buf =
              std::string_view(
                pool.id2buf[ segments[ cur_pos.loc.seg_idx ].pool_id ],
                segments[ cur_pos.loc.seg_idx ].byte_cnt
              );
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

void Source::PastEOL()
{
  while ( !IsLF( CurChar() ) ) ++cur_pos.loc.char_idx;
  if ( CurChar() == '\n' ) {
    ++cur_pos.loc.char_idx;
  } else {
    ++cur_pos.loc.char_idx;
    if (
      cur_pos.loc.char_idx < segments[ cur_pos.loc.seg_idx ].byte_cnt &&
      CurChar() == '\n'
    ) {
      ++cur_pos.loc.char_idx;
    }
  }
  cur_pos.loc.line_idx += 1;
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
  const char* cur = cur_pos.loc.buf.data() + cur_pos.loc.char_idx;
  const char* ptr = cur;
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

  const char* cur = cur_pos.loc.buf.data() + cur_pos.loc.char_idx;
  const char* end = cur_pos.loc.buf.data() + cur_pos.loc.buf.size();

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

  const char* cur = cur_pos.loc.buf.data() + cur_pos.loc.char_idx;
  const char* end = cur_pos.loc.buf.data() + cur_pos.loc.buf.size();

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
  const char* cur = cur_pos.loc.buf.data() + cur_pos.loc.char_idx;
  quoted = *cur == '"';
  const char* beg = cur + (quoted ? 1 : 0);
  const char* ptr = beg;
  while ( *ptr != '"' && !IsLF( *ptr ) && (quoted || !IsWS( *ptr ) ) ) {
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
  return cur_pos.loc.buf.substr( idx1, idx2 - idx1 );
}

void Source::GetDatum(
  std::string_view& x,
  std::string_view& y,
  bool no_x, uint32_t y_idx
)
{
  const char* b = cur_pos.loc.buf.data() + cur_pos.loc.char_idx;
  const char* p = b;
  const char* q;

  while ( IsWS( *p ) ) ++p;
  if ( no_x ) {
    x = std::string_view{};
  } else {
    if ( *p == '"' ) {
      ++p;
      q = p;
      while ( *p != '"' ) ++p;
      x = std::string_view( q, p - q );
      ++p;
    } else {
      q = p;
      while ( !IsSep( *p ) ) ++p;
      if ( *q == '-' && p - q == 1 ) q = p;
      x = std::string_view( q, p - q );
    }
  }

  do {
    while ( IsWS( *p ) ) ++p;
    q = p;
    while ( !IsSep( *p ) ) ++p;
  } while ( y_idx-- > 0 );
  y = std::string_view( q, p - q );

  cur_pos.loc.char_idx += p - b;
  return;
}

////////////////////////////////////////////////////////////////////////////////
