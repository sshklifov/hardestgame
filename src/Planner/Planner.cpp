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

void Planner::StripBad()
{
    assert(!FoundSolution() && !Bricked());

    if ((int)players.size() <= samples) return;

    auto it = std::partition(players.begin(), players.end(),
        [](const PlayerInfo& info)
        {
            return !info.IsDead();
        });
    int nAlive = it - players.begin();

    int nDead = players.size()-nAlive;
    int toDelete = players.size()-samples;

    float r = (float)samples / players.size();
    int delAlive = Clamp(nAlive - r*nAlive, 0.f, (float)toDelete);
    /* if (nAlive-delAlive < minAliveFrac * players.size()) delAlive = 0; */

    int delDead = Clamp(toDelete-delAlive, 0, nDead);
    assert(delAlive >= 0 && delDead >= 0);
    assert(delAlive <= nAlive && delDead <= nDead);

    auto rstart = players.rend()-nAlive;
    std::partial_sort(rstart, rstart+delAlive, rstart+nAlive,
        [](const PlayerInfo& lhs, const PlayerInfo& rhs)
        {
            return lhs.dst > rhs.dst;
        });
    auto start = players.begin()+nAlive;
    std::partial_sort(start, start+delDead, start+nDead,
        [](const PlayerInfo& lhs, const PlayerInfo& rhs)
        {
            return lhs.dst > rhs.dst;
        });

    assert(start-delAlive >= players.begin());
    assert(start+delDead-1 <= players.end()-1);
    players.erase(start-delAlive, start+delDead);
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
    if (players.size()==0) return false; // TODO
    if (FoundSolution() || Bricked()) return true;

    /* static int genChanges = 0; */

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

    Prune(players);
    StripBad();

    std::vector<PlayerInfo> children;
    GenChildren(children, samples);

    players.reserve(players.size()+children.size());
    for (int i = 0; i < (int)children.size(); ++i)
    {
        assert(!children[i].HasNoPlan());
        players.push_back(std::move(children[i]));
    }
    
    ++generation;
    return FoundSolution();
}
