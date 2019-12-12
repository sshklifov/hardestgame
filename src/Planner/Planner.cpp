#include "Planner.h"
#include "Prune.h"

#include <HardestGame.h>
#include <Movement.h>
#include <Draw.h>

#include <random>
#include <memory>
#include <queue>
#include <climits>
#include <cassert>
#include <algorithm>


// TODO WTF;
constexpr const float Planner::playerMinAlive;

/* int Planner::GetUniformRandom(int a, int b) const */
/* { */
/*     return std::uniform_int_distribution<int>(a, b)(gen); */
/* } */

// avoid Direction::NONE
/* Direction Planner::RandomDirection() const */
/* { */
/*     int shift = GetUniformRandom(0, 4); */
    /* if (shift == 4) */
    /* { */
    /*     shift = GetUniformRandom(0, 4); */
    /*     if (shift == 4) return NONE; */
    /* } */
    /* return Direction(1<<shift); */
/* } */

// Used in mutation, no point in changing something to itself

/* std::vector<Direction> Planner::RandomPlan(int steps) const */
/* { */
/*     std::vector<Direction> res; */
/*     res.reserve(steps); */
/*     for (int i = 0; i < steps; ++i) */
/*     { */
/*         res.push_back(RandomDirection()); */
/*     } */

/*     return res; */
/* } */

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

// key notes:
// maintain the ratio dead : alive, so as to give everyone an equal chance.
// both dead and alive players are valuable. Also give equal chance to everyone
// to have offspring. Otherwise, the players will behave the same.
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
        assert (!children[idx].HasNoPlan());
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
            /* return info.dieIdx<0; */
            return !info.IsDead();
            // TODO
        });
    int nAlive = it - players.begin();

    int nDead = players.size()-nAlive;
    int toDelete = players.size()-samples;

    /* const float minAliveFrac = 0.1; */
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
            /* return lhs.protection<rhs.protection || (lhs.protection==rhs.protection&&lhs.dst > rhs.dst); */
            return lhs.dst > rhs.dst;
        });
    auto start = players.begin()+nAlive;
    std::partial_sort(start, start+delDead, start+nDead,
        [](const PlayerInfo& lhs, const PlayerInfo& rhs)
        {
            return lhs.dst > rhs.dst;
            /* return lhs.protection<rhs.protection || (lhs.protection==rhs.protection&&lhs.dst > rhs.dst); */
            /* return false; // TODO jk */
        });

    assert(start-delAlive >= players.begin());
    assert(start+delDead-1 <= players.end()-1);
    players.erase(start-delAlive, start+delDead);
    /* printf("removed elems: %d\n", delDead+delAlive); */
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
    if (players.size()==0) return false;
    // -player.size() is not constant, but is never > 2*samples
    // -if enough iterations are mode, increase the number of moves
    if (FoundSolution() || Bricked()) return true;

    /* for (PlayerInfo& player : players) */
    /* { */
    /*     if(player.protection) player.protection--; */
    /* } */

    static int genChanges = 0;
    /* int totalGens = () */
    /* if (generation + genChanges*genPerStepInc >= PlayerInfo::performLocMinCheck) */
    if (false)
    {
        auto it = std::partition(players.begin(), players.end(),
                [](const PlayerInfo& info)
                {
                    return !info.InsideLocalMin();
                });
        for (auto i = it; i != players.end(); ++i)
        {
            assert(i->InsideLocalMin());
        }
        printf("performed loc min check\n");
        printf("players erased: %ld\n", players.end()-it);
        players.erase(it, players.end());
        genChanges = 0;

        /* int n = players.end()-it; */
        /* for (int i = 0; i < n; ++i) */
        /* { */
        /*     players.push_back(PlayerInfo(samples, gen)); */
        /*     players.back().protection = 30; */
        /* } */

        return false;
    }

    if (generation >= genPerStepInc) // TODO set plan size 0 in ctor
    {
        int newSteps = steps+incSteps;
        for (PlayerInfo& player : players)
        {
            player.IncreaseStep(Planner::incSteps, gen);
            if (player.IsWinner()) res = player.GetSolution(); // TODO
        }


        /* for (int i = 0; i < (int)players.size(); ++i) */
        /* { */
        /*     players[i].plan.resize(newSteps); */
        /*     for (int j = steps; j < newSteps; ++j) */
        /*     { */
        /*         players[i].plan[j] = RandomDirection(); */
        /*     } */
        /*     // TODO naistina na funkciq */
        /*     const IPoint& p = players[i].lastpos.back(); */
        /*     players[i].pos = IBox(p.x-playerRad, p.x+playerRad, */
        /*             p.y-playerRad, p.y+playerRad); */
        /*     Simulation(players[i], players[i].lastpos.size()); */
        /* } */

        steps = newSteps;
        generation = 1;
        // TODO
        genChanges++;
        // TODO
        return Bricked() || FoundSolution();
    }

    /* for (int i = 0; i < (int)players.size(); ++i) */
    /* { */
    /*     /1* players[i].pos = LevelDscr::Get().player; *1/ */
    /*     /1* players[i].lastpos.clear(); *1/ */
    /*     /1* players[i].dieIdx = -1; *1/ */
    /* } */
    /* for (PlayerInfo& player : players) */
    /* { */
    /*     if (Simulation(player, 0)) */
    /*     { */
    /*         pWinner = &player; */
    /*         return true; */
    /*     } */
    /* } */


    Prune(players, gen);
    /* StripOld(); */
    StripBad();
    /* for (int i = 0; i < (int)players.size(); ++i) */
    /* { */
    /*     players[i].pruneProtect = false; */
    /* } */

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
