#ifndef UTILITY_INCLUDED
#define UTILITY_INCLUDED

#include <Planner.h>

extern Direction RandomDirection(PlayerInfo::URNG& gen);
extern Direction RandomOtherDirection(PlayerInfo::URNG& gen, Direction notThis);
extern std::vector<Direction> RandomPlan(PlayerInfo::URNG& gen, int steps);
extern IPoint GetCenter(const IBox& b);

extern int DstToGoal(int x, int y);
extern int DstFromStart(int x, int y);
extern void Prune(std::vector<PlayerInfo>& players);

#endif
