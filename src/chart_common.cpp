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

#include <chart_common.h>

using namespace SVG;

///////////////////////////////////////////////////////////////////////////////

Object* Chart::Collides(
  SVG::Object* obj, const std::vector< SVG::Object* >& objects,
  SVG::U margin_x, SVG::U margin_y
)
{
  margin_x -= epsilon;
  margin_y -= epsilon;
  if ( obj == nullptr || obj->Empty() ) return nullptr;
  for ( auto object : objects ) {
    if ( object->Empty() ) continue;
    if ( SVG::Collides( obj, object, margin_x, margin_y ) ) {
      return object;
    }
  }
  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

// Move objects so as to avoid collisions with other objects.
void Chart::MoveObjs(
  Dir dir,
  const std::vector< SVG::Object* >& move_objs,
  const std::vector< SVG::Object* >& avoid_objs,
  SVG::U margin_x, SVG::U margin_y
)
{
  while ( true ) {
    U dx = 0;
    U dy = 0;
    for ( auto obj : move_objs ) {
      Object* col = Collides( obj, avoid_objs, margin_x, margin_y );
      if ( col != nullptr ) {
        BoundaryBox col_bb = col->GetBB();
        BoundaryBox obj_bb = obj->GetBB();
        switch ( dir ) {
          case Dir::Right : dx = col_bb.max.x - obj_bb.min.x + margin_x; break;
          case Dir::Left  : dx = col_bb.min.x - obj_bb.max.x - margin_x; break;
          case Dir::Up    : dy = col_bb.max.y - obj_bb.min.y + margin_y; break;
          case Dir::Down  : dy = col_bb.min.y - obj_bb.max.y - margin_y; break;
        }
        break;
      }
    }
    if ( std::abs( dx ) < epsilon && std::abs( dy ) < epsilon ) break;
    for ( auto obj : move_objs ) {
      obj->Move( dx, dy );
    }
  }
}

void Chart::MoveObj(
  Dir dir,
  SVG::Object* obj,
  const std::vector< SVG::Object* >& avoid_objs,
  SVG::U margin_x, SVG::U margin_y
)
{
  std::vector< SVG::Object* > move_objs;
  move_objs.push_back( obj );
  MoveObjs( dir, move_objs, avoid_objs, margin_x, margin_y );
}

///////////////////////////////////////////////////////////////////////////////

/*
// To be used for debug.
void Chart::ShowObjBB( SVG::Object* obj )
{
  SVG::BoundaryBox bb = obj->GetBB();
  SVG_DBG(
    std::fixed << std::setprecision( 3 ) <<
    "            " << std::setw( 12 ) << bb.min.x << std::setw( 12 ) << bb.max.x << '\n' <<
     std::setw( 12 ) << bb.max.y << "        +----------+" << '\n' <<
     std::setw( 12 ) << bb.min.y << "        +----------+"
  );
}
*/

///////////////////////////////////////////////////////////////////////////////

void Chart::MakeColorVisible(
  Color* color, Color* bg_color, float min_visibility
)
{
  if ( bg_color->IsClear() ) return;
  if ( color->IsClear() || Color::Diff( color, bg_color ) < min_visibility ) {
    color->Set( ColorName::black );
    float b_diff = Color::Diff( color, bg_color );
    color->Set( ColorName::white );
    float w_diff = Color::Diff( color, bg_color );
    if ( b_diff > w_diff ) {
      color->Set( ColorName::black );
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

bool Chart::NormalWidthUTF8( const std::string_view s )
{
  for ( const auto c : s ) {
    // Detect if 3- or 4-byte UTF-8 codes are present.
    if ( uint8_t( c ) >= 0xE0 ) return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
