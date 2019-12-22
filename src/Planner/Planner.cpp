#include "Planner.h"
#include "Utility.h"

#include <HardestGame.h>
#include <Movement.h>
#include <Draw.h>

#include <random>
#include <memory>
#include <queue>
#include <climits>
#include <cassert>
#include <algorithm>

Planner::Planner(int samples, int seed) :
    gen(seed), samples(samples), steps(startSteps), generation(1)
{
    players.reserve(samples);
    for (int i = 0; i < samples; ++i)
    {
        players.emplace_back(steps, gen);
    }
}

std::vector<PlayerInfo> Planner::Offspring(int nChildren)
{
    assert(!FoundSolution() && !ExhaustedSearch());

    for (int i = 0; i < (int)players.size(); ++i)
    {
        players[i].Mutate(gen);
    }

    std::sort(players.begin(), players.end(),
        [](const PlayerInfo& lhs, const PlayerInfo& rhs)
        {
            return lhs.GetFitness() > rhs.GetFitness();
        });

    std::vector<PlayerInfo> children;
    children.resize(nChildren);

    int crossMaxIdx = crossoverTopPercent * samples;
    assert(crossMaxIdx >= 5 && crossMaxIdx < samples);
    for (int i = 0; i < nChildren; ++i)
    {
        int lhs = std::uniform_int_distribution<int>(0, crossMaxIdx)(gen);
        int rhs = std::uniform_int_distribution<int>(0, crossMaxIdx)(gen);
        if (lhs == rhs)
        {
            lhs = 0;
            rhs = 1;
        }
        children[i] = players[lhs].Crossover(gen, players[rhs]);
    }

    return children;
}

void Planner::NextGen()
{
    assert(!FoundSolution() && !ExhaustedSearch());
    if (generation >= genPerStepInc)
    {
        for (PlayerInfo& player : players)
        {
            player.IncreaseStep(gen, incSteps);
        }

        steps += incSteps;
        generation = 1;
        return;
    }

    std::vector<PlayerInfo> children = Offspring(samples * crossoverGrowth);
    if ((int)players.size() > 2*samples) NatSelect(players, players.size()-samples);

    for (int i = 0; i < (int)children.size(); ++i)
    {
        assert(!children[i].HasNoPlan());
        players.push_back(std::move(children[i]));
    }

    ++generation;
}

bool Planner::SearchForSolution()
{
    if (FoundSolution() || ExhaustedSearch()) return true;

    NextGen();
    for (const PlayerInfo& player : players)
    {
        if (player.IsWinner())
        {
            solutionPlan = player.GetSolution();
            return true;
        }
    }

    return ExhaustedSearch();
}

bool Planner::FoundSolution() const
{
    return !solutionPlan.empty();
}

bool Planner::ExhaustedSearch() const
{
    return steps>maxSteps && !FoundSolution();
}

const std::vector<Direction>& Planner::GetSolution() const
{
    assert(FoundSolution());
    return solutionPlan;
}
