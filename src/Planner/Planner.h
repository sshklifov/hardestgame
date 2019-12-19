#ifndef PLANNER_INCLUDED
#define PLANNER_INCLUDED

class PlayerInfo;
class Planner;

#include <HardestGame.h>
#include <Draw.h>

#include <climits>
#include <random>

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
    void StripBad();

public:
    static constexpr const float playerMinAlive = 0.1f;

    static const int incSteps = 4;
    static const int startSteps = 2*incSteps;
    static const int maxSteps = 200;

    static const int genPerStepInc = 30;

    static const int nRepeatMove = std::max(2, 20 / playerSpeed);
    static const int pixelsPerMove = playerSpeed*nRepeatMove;

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
    mutable float stability = 1.0; // TODO
    int locMin;
    int staleFactor;
};

// circular include
#include "ParallelPlanner.h"

#endif
