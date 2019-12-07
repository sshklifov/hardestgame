#ifndef MOVEMENT_INCLUDED
#define MOVEMENT_INCLUDED

#include <HardestGame.h>

extern bool AdvancePlayer(IBox& b, Direction d);
extern void AdvanceEnemy(EnemyPath& p);

extern bool InsideField(int x, int y);
extern bool PlayerWins(const IBox& player);
extern bool PlayerDies(const IBox& box, const IPoint& p);

template <typename T>
T Clamp(T x, T minVal, T maxVal)
{
    return std::max(minVal, std::min(maxVal, x));
}

#endif
