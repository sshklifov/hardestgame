#ifndef PLANNER_INCLUDED
#define PLANNER_INCLUDED

class PlayerInfo;
class Planner;

#include <HardestGame.h>
#include <Draw.h>

#include <climits>
#include <random>
#include <memory>

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
    void GenChildren(std::vector<PlayerInfo>& res, int nChildren);
    void Select();

public:
    static constexpr const float minAliveFrac = 0.1f;

    static const int incSteps = 4;
    static const int startSteps = 2*incSteps;
    static const int maxSteps = 200;

    static const int genPerStepInc = 50;

    static const int nRepeatMove = std::max(2, 20 / playerSpeed);
    static const int pixelsPerMove = playerSpeed*nRepeatMove;

    static const int pruneDistance = playerSize/2;
    static_assert(pruneDistance < pixelsPerMove, "bad pruning");

private:
    mutable std::default_random_engine gen;
    std::vector<PlayerInfo> players;
    int samples;
    int steps;
    int generation;
    std::vector<Direction> res; // TODO
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
public:
    using URNG = std::default_random_engine;

public:
    PlayerInfo();
    PlayerInfo(int steps, URNG& gen);

    bool IsDead() const;
    bool IsAlive() const;
    bool IsWinner() const;
    bool HasNoPlan() const;

    const std::vector<Direction> GetSolution() const;
    const IBox& GetLastPos() const;

    PlayerInfo Mutate(URNG& gen) const;
    void IncreaseStep(int n, URNG& gen);

    int GetFitness() const;

private:
    void Simulation();
    PlayerInfo Inherit(std::vector<Direction> plan, int changeIdx) const;

public: // TODO (visualized in Main.cpp)
    IBox pos;
    std::vector<Direction> plan;
    std::vector<IPoint> lastpos;
    int dst;

public:
    static const int deadPenalty = std::numeric_limits<int>::max() / 2;
    static_assert(std::is_same<decltype(dst),int>::value, "kofti");
};

// circular include
#include "ParallelPlanner.h"

#endif
