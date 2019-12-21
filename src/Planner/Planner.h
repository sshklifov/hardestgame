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
    bool ExhaustedSearch() const;
    const std::vector<Direction>& GetSolution() const;

    bool SearchForSolution();

    template <typename Func>
    void ForEachPlayer(const Func& f) const;

private:
    void NextGen();
    std::vector<PlayerInfo> Offspring(int nChildren);
    void NatSelect();

public:
    static const int incSteps = 4;
    static const int startSteps = 2*incSteps;
    static const int maxSteps = 300;

    static const int genPerStepInc = 50;

    static const int nRepeatMove = std::max(2, 20 / playerSpeed);
    static const int pixelsPerMove = playerSpeed*nRepeatMove;

    static constexpr const float crossoverTopPercent = 0.1f;
    static constexpr const float crossoverGrowth = 0.1f;

    static const int pruneDistance = playerSize/2;
    static_assert(pruneDistance < pixelsPerMove, "bad pruning");

private:
    std::default_random_engine gen;
    std::vector<PlayerInfo> players;
    int samples;
    int steps;
    int generation;
    std::vector<Direction> solutionPlan;
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

    void Mutate(URNG& gen);
    PlayerInfo Crossover(URNG& gen, const PlayerInfo& rhs);
    void IncreaseStep(URNG& gen, int n);

    int GetFitness() const;

private:
    int LastMoveIdx() const;
    void Inherit(const PlayerInfo& parent,
            std::vector<Direction> plan, int changeIdx);
    void Simulation();

private:
    IBox pos;
    std::vector<Direction> plan;
    std::vector<IPoint> lastpos;
    float stability;
    int fitness;

private:
    static constexpr const float incStability = 0.01f;
    static constexpr const float minStability = 0.01f;
    static constexpr const float maxStability = 0.99f;

    static const int deadPenalty = INT_MIN / 2;
    static const int crossoverBacktrack = 10;
};

#endif
