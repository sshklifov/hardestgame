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
    int n = samples*2;
    players.reserve(n);
    for (int i = 0; i < n; ++i)
    {
        players.push_back(PlayerInfo(steps, gen));
    }
}

void Planner::GenChildren(std::vector<PlayerInfo>& children, int nChildren)
{
    assert(!FoundSolution() && !Bricked());

    std::shuffle(players.begin(), players.end(), gen);
    children.clear();
    children.resize(nChildren);

    int idx = 0;
    for (int off = 0; off < nChildren; ++off)
    {
        int i = off % players.size();
        children[idx] = players[i].Mutate(gen);
        assert(!children[idx].HasNoPlan());

        if (children[idx].IsWinner()) res = children[idx].GetSolution(); // TODO
        ++idx;
    }

    assert(idx==(int)children.size());
}

void Planner::Select()
{
    // Could be improved to 2 partial sorts + only one erase
    // Commits up to 4341c43 did it this way. However, with small
    // sample sizes, this does not make a lot of sense.

    assert(!FoundSolution() && !Bricked());

    Prune(players, players.size() - samples);
    if ((int)players.size() <= samples) return;

    std::sort(players.begin(), players.end(),
        [](const PlayerInfo& lhs, const PlayerInfo& rhs)
        {
            return lhs.GetFitness() > rhs.GetFitness();
        });
    assert(std::is_partitioned(players.cbegin(), players.cend(),
        [](const PlayerInfo& player)
        {
            return player.IsAlive();
        }));
    auto it = std::partition_point(players.cbegin(), players.cend(),
        [](const PlayerInfo& player)
        {
            return player.IsAlive();
        });

    int nAlive = it - players.begin();
    int nDead = players.size()-nAlive;
    int toDelete = players.size()-samples;

    float rat = (float)samples / players.size();
    int delAlive = Clamp(nAlive * (1.f-rat), 0.f, (float)nAlive);
    if (nAlive-delAlive < minAliveFrac * players.size()) delAlive = 0;

    int delDead = Clamp(toDelete-delAlive, 0, nDead);
    assert(delAlive >= 0 && delDead >= 0);
    assert(delAlive <= nAlive && delDead <= nDead);

    players.erase(players.end()-delDead, players.end());

    it = players.begin() + nAlive;
    assert(it <= players.end());
    players.erase(it-delAlive, it);

    assert((int)players.size() >= samples-1 && (int)players.size() <= samples+1);
}

bool Planner::FoundSolution() const
{
    return !res.empty();
}

const std::vector<Direction>& Planner::SeeSolution() const
{
    assert(FoundSolution());
    return res;
}

bool Planner::Bricked() const
{
    return steps > maxSteps && !FoundSolution();
}

bool Planner::NextGen()
{
    if (FoundSolution() || Bricked()) return true;

    if (generation >= genPerStepInc)
    {
        for (PlayerInfo& player : players)
        {
            player.IncreaseStep(incSteps, gen);
            if (player.IsWinner()) res = player.GetSolution(); // TODO
        }

        steps += incSteps;
        generation = 1;
        return Bricked() || FoundSolution();
    }

    std::vector<PlayerInfo> children;
    GenChildren(children, samples);

    players.reserve(players.size()+children.size());
    for (int i = 0; i < (int)children.size(); ++i)
    {
        assert(!children[i].HasNoPlan());
        players.push_back(std::move(children[i]));
    }
    if (FoundSolution()) return FoundSolution();

    Select();
    
    ++generation;
    return FoundSolution();
}
