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

class PlayerInfo;

class Planner
{
    friend class ParallelPlanner;

public:
    Planner(int samples, int seed);

    bool FoundSolution() const;
    const std::vector<Direction>& SeeSolution() const;
    bool Bricked() const;

    bool NextGen();

    template <typename Func>
    void ForEachPlayer(const Func& f) const;

private:
    /* int GetUniformRandom(int a, int b) const; */
    /* Direction RandomDirection() const; */
    /* Direction RandomOtherDirection(Direction notThis) const; */
    /* std::vector<Direction> RandomPlan(int steps) const; */

private:
    // TODO void somehow? (need pWinner) tho
    static bool Simulation(PlayerInfo& player, int idx);
    // below: u too buddy (and put ocnst)
    static bool Inherit(const PlayerInfo& prnt, PlayerInfo& chld, int idx);
    bool Mutate(const PlayerInfo& player, PlayerInfo& mut);
    /* static void ReverseEng(PlayerInfo& player, int idx, int n); */

private:
    /* bool Simulate(); */
    /* bool Simulate(PlayerInfo& player, int idx); */

    static int EvalMatch(const PlayerInfo& lhs, const PlayerInfo& rhs, int& crossIdx);
    void CrossOver(const PlayerInfo& dead, const PlayerInfo& alive, int crossIdx, PlayerInfo& res) const;
    void GenChildren(std::vector<PlayerInfo>& res, int nChildren);

    void StripOld();
    void StripBad();

public:
    static constexpr const float playerMinAlive = 0.1f;

    static const int incSteps = 4;
    static const int startSteps = 2*incSteps;
    static const int maxSteps = 200;
    static const int repeat = 30;
    static const int percentPruneProtectOverride = 25;
    static const int pruneManhThreshold = playerSize/2;

    static const int genPerStepInc = 30;
    static const int lastMovesSize = 2*incSteps; // TODO

    static const int nRepeatMove = std::max(2, 20 / playerSpeed);
    static const int pixelsPerMove = playerSpeed*nRepeatMove;

    static_assert(pruneManhThreshold < nRepeatMove*pixelsPerMove, "no point in pruning");

private:
    mutable std::default_random_engine gen;
    std::vector<PlayerInfo> players;
    int samples;
    int steps;
    int generation;
    /* const PlayerInfo* pWinner; */
    std::vector<Direction> res;
    /* int lastLocMinCheck = 0; */
};

template <typename Func>
void Planner::ForEachPlayer(const Func& f) const
{
    for (const PlayerInfo& player : players)
    {
        f(player);
    }
}

class PlayerInfo
{
    /* friend class Planner; // TODO REMOVE */

public:
    using URNG = std::default_random_engine;

public:
    PlayerInfo();
    PlayerInfo(int steps, URNG& gen);

public:
    PlayerInfo(PlayerInfo&&) = default;
    PlayerInfo& operator=(PlayerInfo&&) = default;

public:
    PlayerInfo(const PlayerInfo&) = default;
    PlayerInfo& operator=(const PlayerInfo&) = default;

public:
    bool IsDead() const;
    bool IsWinner() const;
    bool HasNoPlan() const;
    bool InsideLocalMin() const;

    const std::vector<Direction> GetSolution() const;
    const IBox& GetLastPos() const;

    PlayerInfo Mutate(URNG& gen) const;
    void IncreaseStep(int n, URNG& gen);

public:
    static const int staleThreshold = 2*Planner::genPerStepInc;
    static const int performLocMinCheck = 2*staleThreshold;

private:
    void Simulation();
    PlayerInfo Inherit(std::vector<Direction> plan, int changeIdx) const;

public: // TODO
    IBox pos;
    std::vector<Direction> plan;
    std::vector<IPoint> lastpos;
    int dst;
    mutable float stability = 1.0; // TODO!
    int locMin;
    int staleFactor;
    /* int protection = 0; // TODO AS well */
    /* bool IsAlive() const */
    /* { */
    /*     return plan.size() != lastpos.size(); */
    /* } */
};

#endif
