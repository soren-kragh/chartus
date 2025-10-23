
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

  std::this_thread::sleep_for(std::chrono::seconds(1));

  SVG_DBG( dbg_oss_main.str() );
  SVG_DBG( dbg_oss_thread.str() );

  SVG_DBG( "active_seg " << active_seg );
  SVG_DBG( "locked_seg " << locked_seg );
  SVG_DBG( "cur_pos.loc.seg_idx = " << cur_pos.loc.seg_idx );
  SVG_DBG(
    "cur_pos.loc.buf.data() = "
    << (reinterpret_cast< uint64_t>( cur_pos.loc.buf.data() ))
  );
  SVG_DBG( "------------------------------------------------------------" );
  SVG_DBG( ">>> " << cur_pos.loc.seg_idx );
  SVG_DBG( cur_pos.loc.buf );
  SVG_DBG( "------------------------------------------------------------" );

  {
    size_t i = 0;
    for ( auto& segment : segments ) {
      SVG_DBG( "Segment " << i );
      SVG_DBG( "  loaded    = " << (segment.loaded ? 'T' : 'F') );
      SVG_DBG( "  pool_id   = " << segment.pool_id );
      SVG_DBG( "  bufptr    = " << (reinterpret_cast< uint64_t>( segment.bufptr )) );
      SVG_DBG( "  assign_id = " << segment.assign_id );
      ++i;
    }
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

void Source::LoaderThread()
{
  int32_t my_active_seg = -1;
  int32_t my_locked_seg = -1;

  auto err = [&]( std::string msg )
    {
      std::lock_guard< std::mutex > lk( loader_mutex );
      loader_msg = msg;
      loader_cond.notify_one();
    };

  auto load_segment = [&]( int32_t seg_idx )
    {
      if ( seg_idx == my_locked_seg ) return false;
      int32_t pool_id = seg_idx % pool.dyn_cnt;
      if ( pool.id2seg[ pool_id ] == my_locked_seg ) {
        return false;
      }
      {
        std::lock_guard< std::mutex > lk( loader_mutex );
        int32_t i = pool.id2seg[ pool_id ];
        segments[ i ].loaded = false;
        segments[ i ].bufptr = nullptr;
        dbg_oss_thread
          << "# delete " << i
          << "  my_active_seg:" << my_active_seg
          << "  my_locked_seg:" << my_locked_seg
          << "\n";
      }
      pool.id2seg[ pool_id ] = seg_idx;

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
        segments[ seg_idx ].assign_id = ++assign_id;
        my_active_seg = active_seg;
        my_locked_seg = locked_seg;
        loader_cond.notify_one();
        dbg_oss_thread
          << "# " << assign_id
          << "  " << seg_idx
          << "  " << reinterpret_cast< uint64_t>( pool.id2buf[ pool_id ] )
          << "\n";
      }

      return true;
    };

  while ( !stop_loader ) {

    // Make sure my_active_seg is loaded.
    while ( my_active_seg >= 0 && !segments[ my_active_seg ].loaded ) {
      if ( stop_loader ) return;
      if ( !load_segment( my_active_seg ) ) break;
    }
    if ( !loader_msg.empty() ) return;

    if ( my_active_seg >= 0 && segments[ my_active_seg ].loaded ) {
      int32_t seg_idx = (my_active_seg + 1) % segments.size();
      if ( !segments[ seg_idx ].loaded ) {
        load_segment( seg_idx );
      }
    }

    if ( !loader_msg.empty() ) return;

    // Wait for more work:
    {
      std::unique_lock< std::mutex > lk( loader_mutex );
      if ( my_active_seg >= 0 && segments[ my_active_seg ].loaded ) {
        loader_cond.wait(
          lk,
          [&]{
            return
              stop_loader ||
              active_seg != my_active_seg ||
              locked_seg != my_locked_seg;
          }
        );
      }
      my_active_seg = active_seg;
      my_locked_seg = locked_seg;
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
    loader_cond.notify_one();
    cur_pos.loc.buf =
      std::string_view(
        segments[ active_seg ].bufptr,
        segments[ active_seg ].byte_cnt
      );
    dbg_oss_main
      << "% " << segments[ active_seg ].assign_id
      << "  " << cur_pos.loc.seg_idx
      << "  " << reinterpret_cast< uint64_t>( cur_pos.loc.buf.data() )
      << "\n";
  }
  if ( !loader_msg.empty() ) {
    Err( loader_msg );
  }
  if ( cur_pos.loc.buf.data() == nullptr ) {
    Err( "nullptr!!!" );
  }
}
