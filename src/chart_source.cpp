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

void Source::Quit( int code )
{
  stop_loader = true;
  loader_cond.notify_one();
  if ( loader_thread.joinable() ) loader_thread.join();
  exit( code );
}

void Source::Err( const std::string& msg )
{
  std::cerr << "*** ERROR: " << msg << std::endl;
  Quit( 1 );
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

  size_t idx = show_ref ? ref_idx : cur_pos.loc.char_idx;

  if ( AtEOF() ) {
    std::cerr << "at EOF";
  } else {
    ToSOL();
    const char* p = cur_pos.loc.buf.data() + cur_pos.loc.char_idx;
    size_t col = idx - cur_pos.loc.char_idx;
    show_loc( cur_pos.loc, col );
    idx = cur_pos.loc.char_idx;
    ToEOL();
    std::cerr << std::string_view( p, cur_pos.loc.char_idx - idx ) << '\n';
    std::string indent;
    bool show_caret = true;
    for ( size_t i = 0; i < col; ++i ) {
      if ( p[ i ] & 0x80 ) {
        show_caret = false;
        break;
      }
      indent += (p[ i ] == '\t') ? '\t' : ' ';
    }
    if ( show_caret ) {
      std::cerr << indent << '^';
    }
  }
  std::cerr << std::endl;

  Quit( 1 );
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

void Source::pool_t::LRU_UseID( int32_t id )
{
  auto it = lru_map.find( id );
  if ( it != lru_map.end() ) {
    lru_lst.splice( lru_lst.begin(), lru_lst, it->second );
  } else {
    lru_lst.push_front( id );
    lru_map[ id ] = lru_lst.begin();
  }
}

int32_t Source::pool_t::LRU_GetID()
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
  cur_pos.loc.buf = std::string_view( segment.bufptr, segment.byte_cnt );
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
      cur_pos.loc.char_idx = sol_loc.char_idx;
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
        size_t max = max_buffers + file_list.size();
        if ( pool.dyn_cnt < 2 || pool.dyn_cnt < max ) {
          pool_id = +pool.dyn_cnt;
          pool.dyn_cnt++;
        } else {
          pool_id = pool.LRU_GetID();
          segments[ pool.id2seg[ pool_id ] ].loaded = false;
          segments[ pool.id2seg[ pool_id ] ].bufptr = nullptr;
        }
      }
      segments.back().pool_id = pool_id;
      if ( pool.id2buf.find( pool_id ) == pool.id2buf.end() ) {
        pool.id2buf[ pool_id ] =
          static_cast< char* >( malloc( buffer_size + 16 ) );
      }
      pool.id2seg[ pool_id ] = segments.size() - 1;
      if ( pool_id >= 0 ) pool.LRU_UseID( pool_id );
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
      segment.bufptr = pool.id2buf[ segment.pool_id ];
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
      char c = ptr[ segments.back().byte_cnt - 1 - to_move ];
      if ( c == '\n' ) break;
      if ( c == '\r' && to_move > 0 ) break;
      ++to_move;
      if ( to_move == segments.back().byte_cnt ) {
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
  if ( !in_macro_name.empty() ) {
    ParseErr( "macro '" + in_macro_name + "' not ended" );
  }

  {
    std::lock_guard< std::mutex > lk( loader_mutex );
  }
  loader_thread = std::thread( &Source::LoaderThread, this );

  cur_pos = {};
  LoadCurSegment();
}

////////////////////////////////////////////////////////////////////////////////

void Source::LoaderThread()
{
  int32_t my_active_seg = -1;

  auto err = [&]( std::string msg )
    {
      std::lock_guard< std::mutex > lk( loader_mutex );
      loader_msg = msg;
      loader_cond.notify_one();
    };

  auto load_segment = [&]( int32_t seg_idx )
    {
      int32_t pool_id = pool.LRU_GetID();
      {
        std::lock_guard< std::mutex > lk( loader_mutex );
        my_active_seg = active_seg;
        if ( seg_idx == locked_seg ) return false;
        if ( pool.id2seg[ pool_id ] == locked_seg ) {
          return false;
        }
        segments[ pool.id2seg[ pool_id ] ].loaded = false;
        segments[ pool.id2seg[ pool_id ] ].bufptr = nullptr;
      }
      pool.id2seg[ pool_id ] = seg_idx;
      pool.LRU_UseID( pool_id );

      std::ifstream file( segments[ seg_idx ].name, std::ios::binary );
      if ( !file ) {
        err( "failed to open file '" + segments[ seg_idx ].name + "'" );
        return false;
      }
      file.seekg( segments[ seg_idx ].byte_ofs, std::ios::beg );
      if ( !file ) {
        err( "seek failed in '" + segments[ seg_idx ].name + "'" );
        return false;
      }
      std::streamsize bytes_to_read = segments[ seg_idx ].byte_cnt;
      file.read( pool.id2buf[ pool_id ], bytes_to_read );
      std::streamsize bytes_read = file.gcount();
      if (
        bytes_read != bytes_to_read ||
        file.bad() || (file.fail() && !file.eof())
      ) {
        err( "error while reading '" + segments[ seg_idx ].name + "'" );
        return false;
      }

      {
        std::lock_guard< std::mutex > lk( loader_mutex );
        segments[ seg_idx ].pool_id = pool_id;
        segments[ seg_idx ].loaded = true;
        segments[ seg_idx ].bufptr = pool.id2buf[ pool_id ];
        my_active_seg = active_seg;
      }
      loader_cond.notify_one();

      return true;
    };

  while ( !stop_loader ) {

    // Make sure my_active_seg is loaded.
    while ( my_active_seg >= 0 && !segments[ my_active_seg ].loaded ) {
      if ( stop_loader ) return;
      if ( !load_segment( my_active_seg ) ) break;
    }
    if ( !loader_msg.empty() ) return;

    // Pre-load more segments.
    while ( my_active_seg >= 0 && segments[ my_active_seg ].loaded ) {
      if ( stop_loader ) return;
      int32_t seg_idx = my_active_seg;
      while ( true ) {
        if ( stop_loader ) return;
        seg_idx = (seg_idx + 1) % segments.size();
        if ( seg_idx == my_active_seg ) break;
        if ( segments[ seg_idx ].pool_id < 0 ) continue;
        if ( !segments[ seg_idx ].loaded ) break;
      }
      if ( seg_idx == my_active_seg ) break;
      if ( !segments[ seg_idx ].loaded ) {
        if ( !load_segment( seg_idx ) ) break;
      }
    }
    if ( !loader_msg.empty() ) return;

    // Wait for more work:
    {
      std::unique_lock< std::mutex > lk( loader_mutex );
      if ( my_active_seg >= 0 && segments[ my_active_seg ].loaded ) {
        loader_cond.wait(
          lk, [&]{ return stop_loader || active_seg != my_active_seg; }
        );
      }
      my_active_seg = active_seg;
    }

  }

  return;
}

void Source::LoadCurSegment()
{
  {
    std::unique_lock< std::mutex > lk( loader_mutex );
    active_seg = cur_pos.loc.seg_idx;
    locked_seg = -1;
    loader_cond.notify_one();
    loader_cond.wait(
      lk, [&]{ return !loader_msg.empty() || segments[ active_seg ].loaded; }
    );
    locked_seg = active_seg;
    cur_pos.loc.buf =
      std::string_view(
        segments[ active_seg ].bufptr,
        segments[ active_seg ].byte_cnt
      );
  }
  if ( !loader_msg.empty() ) {
    Err( loader_msg );
  }
}

void Source::LoadLine() {
  LoadCurSegment();
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
      LoadCurSegment();
    }
    if ( AtEOF() ) break;
    if ( CurChar() == '#' ) continue;

    segment_t& segment = segments[ cur_pos.loc.seg_idx ];
    const char* ptr = cur_pos.loc.buf.data() + cur_pos.loc.char_idx;
    size_t len = segment.byte_cnt - cur_pos.loc.char_idx;

    if ( len >= 5 && memcmp( ptr, "Macro", 5 ) == 0 ) {
      auto sol_loc = cur_pos.loc;
      bool macro_def  = len >= 8 && memcmp( ptr, "MacroDef", 8 ) == 0;
      bool macro_end  = len >= 8 && memcmp( ptr, "MacroEnd", 8 ) == 0;
      bool macro_call = !macro_def && !macro_end;
      cur_pos.loc.char_idx += macro_call ? 5 : 8;
      SkipWS();
      if ( GetChar() == ':' ) {
        SkipWS();
        std::string macro_name{ GetIdentifier() };
        if ( macro_name.empty() ) ParseErr( "macro name expected", true );
        ExpectEOL();
        cur_pos.loc.char_idx = sol_loc.char_idx;
        if ( in_macro_name.empty() ) {
          if ( macro_def ) {
            in_macro_name = macro_name;
          } else
          if ( macro_end ) {
            if ( cur_pos.macro_stack.empty() ) ParseErr( "not defining macro" );
            cur_pos.loc = cur_pos.macro_stack.back();
            LoadCurSegment();
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
            LoadCurSegment();
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
      cur_pos.loc.char_idx = sol_loc.char_idx;
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

std::string_view Source::GetKey()
{
  if ( !AtSOL() ) ParseErr( "KEY must be unindented" );
  ref_idx = cur_pos.loc.char_idx;
  const char* cur = cur_pos.loc.buf.data() + cur_pos.loc.char_idx;
  const char* ptr = cur;
  while ( true ) {
    char c = *ptr;
    if (
      (c >= 'a' && c <= 'z') ||
      (c >= 'A' && c <= 'Z') ||
      (c >= '0' && c <= '9') ||
      c == '_' || c == '.' || c == '@'
    ) {
      ++ptr;
    } else {
      break;
    }
  }
  cur_pos.loc.char_idx += ptr - cur;
  if ( ptr == cur ) ParseErr( "KEY expected", true );
  SkipWS();
  if ( CurChar() != ':' ) ParseErr( "':' expected" );
  cur_pos.loc.char_idx++;
  return std::string_view( cur, ptr - cur );
}

std::string_view Source::GetIdentifier()
{
  ref_idx = cur_pos.loc.char_idx;
  const char* cur = cur_pos.loc.buf.data() + cur_pos.loc.char_idx;
  const char* ptr = cur;
  while ( true ) {
    char c = *ptr;
    if (
      (c >= 'a' && c <= 'z') ||
      (c >= 'A' && c <= 'Z') ||
      (c >= '0' && c <= '9') ||
      c == '_'
    ) {
      ++ptr;
    } else {
      break;
    }
  }
  cur_pos.loc.char_idx += ptr - cur;
  return std::string_view( cur, ptr - cur );
}

bool Source::GetInt64( int64_t& i, bool sep_after )
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

  if ( ec != std::errc() || (sep_after && !IsSep( *ptr )) ) return false;

  cur_pos.loc.char_idx += ptr - cur;
  i = result;
  return true;
}

bool Source::GetDoubleFull(
  double& d,
  bool none_allowed, bool sep_after, bool fail_on_error
)
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

  if ( ec != std::errc() || (sep_after && !IsSep( *ptr )) ) {
    if ( fail_on_error ) {
      ParseErr( "invalid number", true );
    }
    return false;
  }

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

void Source::GetColor( SVG::Color* color, double& transparency )
{
  SkipWS();
  std::string color_id{ GetIdentifier() };
  if ( color_id.empty() ) ParseErr( "color expected" );

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
      uint8_t r =
        static_cast<uint8_t>( std::stoi( color_id.substr(1, 2), nullptr, 16 ) );
      uint8_t g =
        static_cast<uint8_t>( std::stoi( color_id.substr(3, 2), nullptr, 16 ) );
      uint8_t b =
        static_cast<uint8_t>( std::stoi( color_id.substr(5, 2), nullptr, 16 ) );
      color->Set( r, g, b );
    } else {
      color_ok = color->Set( color_id ) == color;
    }
  }

  if ( !color_ok ) {
    ParseErr( "invalid color", true );
  }

  if ( !color->IsClear() ) {
    if ( !AtEOL() ) {
      double lighten = 0.0;
      ExpectWS();
      if ( !AtEOL() ) {
        GetDouble( lighten );
        if ( lighten < -1.0 || lighten > 1.0 ) {
          ParseErr( "lighten value out of range [-1.0;1.0]", true );
        }
        if ( lighten < 0 )
          color->Darken( -lighten );
        else
          color->Lighten( lighten );
      }
    }
    if ( !AtEOL() ) {
      ExpectWS();
      if ( !AtEOL() ) {
        GetDouble( transparency );
        if ( transparency < 0.0 || transparency > 1.0 ) {
          ParseErr( "transparency value out of range [0.0;1.0]", true );
        }
        color->SetTransparency( transparency );
      }
    }
  }

  ExpectEOL();
}

void Source::GetColor( SVG::Color* color )
{
  double transparency;
  GetColor( color, transparency );
}

////////////////////////////////////////////////////////////////////////////////

void Source::GetSwitch( bool& flag )
{
  SkipWS();
  std::string_view id = GetIdentifier();
  if ( id == "On"  ) flag = true ; else
  if ( id == "Off" ) flag = false; else
  if ( id == "Yes" ) flag = true ; else
  if ( id == "No"  ) flag = false; else
  if ( id == "" ) ParseErr( "On/Off (Yes/No) expected" ); else
  ParseErr(
    "On/Off (Yes/No) expected, saw '" + std::string( id ) + "'", true
  );
}

////////////////////////////////////////////////////////////////////////////////

void Source::GetLetterSpacing(
  double& width_adj,
  double& height_adj,
  double& baseline_adj
)
{
  width_adj    = 1.0;
  height_adj   = 1.0;
  baseline_adj = 1.0;

  SkipWS();
  if ( AtEOL() ) ParseErr( "width adjustment expected" );
  GetDouble( width_adj );
  if ( width_adj < 0 || width_adj > 100 ) {
    ParseErr( "width adjustment out of range [0;100]", true );
  }

  if ( !AtEOL() ) {
    ExpectWS();
    if ( !AtEOL() ) {
      GetDouble( height_adj );
      if ( height_adj < 0 || height_adj > 100 ) {
        ParseErr( "height adjustment out of range [0;100]", true );
      }
    }
  }

  if ( !AtEOL() ) {
    ExpectWS();
    if ( !AtEOL() ) {
      GetDouble( baseline_adj );
      if ( baseline_adj < 0 || baseline_adj > 100 ) {
        ParseErr( "baseline adjustment out of range [0;100]", true );
      }
    }
  }

  ExpectEOL();
}

////////////////////////////////////////////////////////////////////////////////

void Source::GetAxis( int& axis_y_n )
{
  std::string_view id = GetIdentifier();
  if ( id == "Primary"   ) axis_y_n = 0; else
  if ( id == "Y1"        ) axis_y_n = 0; else
  if ( id == "Secondary" ) axis_y_n = 1; else
  if ( id == "Y2"        ) axis_y_n = 1; else
  if ( id == "" ) ParseErr( "Primary/Y1 or Secondary/Y2 expected" ); else
  ParseErr( "unknown Y-axis '" + std::string( id ) + "'", true );
}

////////////////////////////////////////////////////////////////////////////////
