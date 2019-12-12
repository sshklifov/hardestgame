#ifndef MOVEMENT_INCLUDED
#define MOVEMENT_INCLUDED

#include <HardestGame.h>

extern void AdvancePlayer(IBox& b, Direction d, int times = 1);
extern void AdvanceEnemy(EnemyPath& p, int times = 1);

extern bool InsideField(int x, int y);
extern bool PlayerWins(const IBox& player);
extern bool PlayerDies(const IBox& box, const IPoint& p);

template <typename T>
T Clamp(T x, T minVal, T maxVal)
{
    return std::max(minVal, std::min(maxVal, x));
}

#endif
