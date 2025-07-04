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

#include <chart_label.h>

using namespace SVG;
using namespace Chart;

////////////////////////////////////////////////////////////////////////////////

Label::Label( void )
{
}

Label::~Label( void )
{
}

////////////////////////////////////////////////////////////////////////////////

SVG::Group* Label::CreateInDB(
  SVG::Group* g, const std::string& txt, SVG::U size, bool append
)
{
  return Label::Create( this, g, txt, size, append );
}

//------------------------------------------------------------------------------

// Static member.
SVG::Group* Label::CreateLabel(
  SVG::Group* g, const std::string& txt, SVG::U size
)
{
  return Label::Create( nullptr, g, txt, size, false );
}

//------------------------------------------------------------------------------

// Static member.
SVG::Group* Label::Create(
  Label* label_db,
  SVG::Group* g, const std::string& txt, SVG::U size,
  bool append
)
{
  U y = 0;
  Entry e{ nullptr, 0, 0 };
  if ( append ) {
    e.link = label_db->containers[ g ].link;
    if ( e.link ) {
      y = e.link->GetBB().min.y;
    }
  } else {
    g = g->AddNewGroup();
    if ( size > 0 ) {
      g->Attr()->TextFont()->SetSize( size );
    }
  }

  Attributes attr;
  if ( append && size > 0 ) {
    g->AddNewGroup();
    g->Last()->Attr()->TextFont()->SetSize( size );
    g->Last()->Attr()->Collect( attr );
    g->DeleteFront();
  } else {
    g->Attr()->Collect( attr );
  }
  U h = attr.TextFont()->GetHeight();
  U w = attr.TextFont()->GetWidth();

  std::string s;
  bool non_space_seen = false;
  auto it = txt.cbegin();
  char c = '\n';
  while ( true ) {
    if ( it != txt.cend() ) {
      c = *(it++);
      if ( c != '\n' ) {
        if ( c == ' ' ) {
          if ( non_space_seen ) {
            e.trailing_space += w;
          } else {
            e.leading_space += w;
          }
        } else {
          non_space_seen = true;
          e.trailing_space = 0;
        }
        s += c;
      }
    }
    if ( c == '\n' || it == txt.cend() ) {
      g->Add( new Text( 0, y, s ) );
      if ( append && size > 0 ) {
        g->Last()->Attr()->TextFont()->SetSize( size );
      }
      y -= h;
      if ( label_db ) label_db->entries[ g->Last() ] = e;
      e.link = g->Last();
      e.leading_space = 0;
      e.trailing_space = 0;
      non_space_seen = false;
      s.clear();
    }
    if ( it == txt.cend() ) break;
  }

  if ( label_db ) {
    Container c;
    c.link = e.link;
    c.bb = g->GetBB();
    label_db->containers[ g ] = c;
  }
  return g;
}

void Label::Update( SVG::Group* container )
{
  containers[ container ].bb = container->GetBB();
}

////////////////////////////////////////////////////////////////////////////////

void Label::Delete( SVG::Group* container )
{
  auto link = containers[ container ].link;
  containers.erase( container );
  while ( link ) {
    auto it = entries.find( link );
    link = it->second.link;
    entries.erase( it );
  }
  return;
}

////////////////////////////////////////////////////////////////////////////////

void Label::AddBackground(
  SVG::Group* bg_g, const SVG::BoundaryBox& area, bool partial_ok
)
{
  for ( const auto& [container, c] : containers ) {
    BoundaryBox bb = container->GetBB();
    U dx = bb.min.x - c.bb.min.x;
    U dy = bb.min.y - c.bb.min.y;
    auto link = c.link;
    while ( link ) {
      Entry e = entries[ link ];
      bb = link->GetBB();
      bb.min.x += dx;
      bb.min.y += dy;
      bb.max.x += dx;
      bb.max.y += dy;
      U r = (bb.max.y - bb.min.y) / 3;
      bb.min.x += e.leading_space;
      bb.max.x -= e.trailing_space;
      bb.min.x -= r/2;
      bb.max.x += r/2;
      bb.min.y -= r/5;
      bb.max.y += r/5;
      bool inside =
        bb.min.x > area.min.x && bb.max.x < area.max.x &&
        bb.min.y > area.min.y && bb.max.y < area.max.y;
      bool outside =
        bb.max.x < area.min.x || bb.min.x > area.max.x ||
        bb.max.y < area.min.y || bb.min.y > area.max.y;
      if ( inside || (partial_ok && !outside) ) {
        bg_g->Add( new Rect( bb.min, bb.max, r ) );
      }
      link = e.link;
    }
  }
  return;
}

////////////////////////////////////////////////////////////////////////////////
