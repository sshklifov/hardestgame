#ifndef PLANNER_INCLUDED
#define PLANNER_INCLUDED

// circular include
class Planner;
#include "ParallelPlanner.h"

#include <HardestGame.h>
#include <Draw.h>
#include <CArray.h>

#include <climits>
#include <random>

struct PlayerInfo;

class Planner
{
    friend class ParallelPlanner;

public:
    Planner(int samples, int seed);

    bool FoundSol() const;
    const std::vector<Direction>& SeeSol() const;
    bool Bricked() const;

    bool NextGen();

    template <typename Func>
    void ForEachPlayer(const Func& f) const;

private:
    int GetUniformRandom(int a, int b) const;
    Direction RandomDirection() const;
    Direction RandomOtherDirection(Direction notThis) const;
    std::vector<Direction> RandomPlan(int steps) const;

private:
    bool Simulate();

    void Mutate(const PlayerInfo& player, PlayerInfo& mut, int lastIdx) const;
    static int EvalMatch(const PlayerInfo& lhs, const PlayerInfo& rhs, int& crossIdx);
    void CrossOver(const PlayerInfo& dead, const PlayerInfo& alive, int crossIdx, PlayerInfo& res) const;
    int GenChildren(std::vector<PlayerInfo>& res, int nChildren);

    void StripBad();

public:
    static const int incSteps = 4;
    static const int maxSteps = 100;
    static const int repeatGeneration = 30;
    static const int percentPruneProtectOverride = 30;

    static const int lastMovesSize = 2*incSteps;

    static const int pixelsPerMove = 30;
    static const int repeatMoves = pixelsPerMove / playerSpeed;
    static_assert(playerSpeed* repeatMoves == pixelsPerMove, "precision loss");

private:
    mutable std::default_random_engine gen;
    std::vector<PlayerInfo> players;
    int samples;
    int steps;
    int generation;
    int solIdx;
};

template <typename Func>
void Planner::ForEachPlayer(const Func& f) const
{
    for (const PlayerInfo& player : players)
    {
        f(player);
    }
}

struct PlayerInfo
{
    PlayerInfo() :
        pos(LevelDscr::Get().player), dieIdx(0), dst(INT_MAX), pruneProtect(0) {}

    IBox pos;
    std::vector<Direction> plan;
    CArray<IPoint,Planner::lastMovesSize> lastpos;
    int dieIdx;
    int dst;
    bool pruneProtect;
};

#endif
