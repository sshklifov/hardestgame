#ifndef DRAW_INCLUDED
#define DRAW_INCLUDED

#include <HardestGame.h>

struct Color
{
    float r;
    float g;
    float b;
};

extern void DrawPlayer(Color* fb, const IBox& player, float red = 1.f);
extern void DrawEnemy(Color* fb, IPoint p);
extern void DrawLevel(Color* fb, const LevelDscr& = LevelDscr::Get());

#endif
