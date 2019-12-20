#include "Planner.h"
#include "Utility.h"
#include <Movement.h>
#include <cassert>

bool PlayerInfo::IsDead() const
{
    assert(!HasNoPlan());
    return lastpos.size() < plan.size();
}

bool PlayerInfo::IsAlive() const
{
    assert(!HasNoPlan());
    return !IsDead();
}

bool PlayerInfo::IsWinner() const
{
    assert(!HasNoPlan());
    return PlayerWins(pos);
}

bool PlayerInfo::HasNoPlan() const
{
    return plan.size()==0;
}

const std::vector<Direction> PlayerInfo::GetSolution() const
{
    assert(IsWinner());
    return plan;
}

const IBox& PlayerInfo::GetLastPos() const
{
    assert(!HasNoPlan());
    return pos;
}

PlayerInfo PlayerInfo::Mutate(URNG& gen) const
{
    assert(!HasNoPlan());

    const float offsetIdxProb = 0.3f;
    const float nMutationsProb = 0.4f;

    int lastIdx = IsDead() ? lastpos.size() : plan.size()-1;
    int mutIdx = lastIdx -
        std::geometric_distribution<int>(offsetIdxProb)(gen) % (lastIdx+1);

    int n = mutIdx +
        std::geometric_distribution<int>(nMutationsProb)(gen) % Planner::incSteps;
    n = std::min((int)plan.size(), n);

    Direction newDir = RandomOtherDirection(gen, plan[mutIdx]);
    std::vector<Direction> mutPlan = plan;
    mutPlan[mutIdx] = newDir;
    for (int i = mutIdx + 1; i < n; ++i)
    {
        mutPlan[i] = RandomDirection(gen);
    }

    return Inherit(std::move(mutPlan), mutIdx);
}

PlayerInfo PlayerInfo::Inherit(std::vector<Direction> mutPlan, int mutIdx) const
{
    assert(!HasNoPlan());

    PlayerInfo chld;
    chld.dst = INT_MAX;
    chld.plan = std::move(mutPlan);

    if (mutIdx!=0)
    {
        chld.lastpos.reserve(lastpos.size());
        chld.lastpos.assign(lastpos.begin(), lastpos.begin()+mutIdx);
    }
    chld.Simulation();
    return chld;
}

void PlayerInfo::Simulation()
{
    assert(!HasNoPlan());
    assert(lastpos.size() < plan.size());

    int idx = lastpos.size();

    if (!lastpos.empty())
    {
        const IPoint& p = lastpos.back();
        pos = IBox(p.x-playerRad, p.x+playerRad, p.y-playerRad, p.y+playerRad);
    }
    else
    {
        pos = LevelDscr::Get().player;
    }

    std::vector<EnemyPath> enemies = LevelDscr::Get().enemies;
    for (EnemyPath& e : enemies)
    {
        AdvanceEnemy(e, Planner::nRepeatMove*idx);
    }

    int steps = plan.size();
    int rep = 0;
    bool died = false;
    while (idx < steps)
    {
        AdvancePlayer(pos, plan[idx]);
        for (EnemyPath& e : enemies)
        {
            AdvanceEnemy(e);
        }

        for (const EnemyPath& e : enemies)
        {
            if (PlayerDies(pos, e.pos))
            {
                died = true;
                break;
            }
        }
        if (died) break;

        if (PlayerWins(pos))
        {
            for (int j = idx+1; j < steps; ++j) plan[j] = NONE;
            break;
        }

        ++rep;
        if (rep == Planner::nRepeatMove)
        {
            rep = 0;
            ++idx;
            lastpos.push_back(GetCenter(pos));
        }
    }

    // either dead, winner, or just alive
    IPoint where = GetCenter(pos);
    dst = DstToGoal(where.x, where.y);
}

PlayerInfo::PlayerInfo() : pos(-100, -100, -100, -100), dst(INT_MAX)
{
    // try to render -> assert fail
}

PlayerInfo::PlayerInfo(int steps, URNG& gen) : PlayerInfo()
{
    IncreaseStep(steps, gen);
}

void PlayerInfo::IncreaseStep(int n, URNG& gen)
{
    int steps = plan.size();
    int newSteps = steps+n;
    plan.resize(newSteps);

    for (int j = steps; j < newSteps; ++j)
    {
        plan[j] = RandomDirection(gen);
    }

    Simulation();
}

int PlayerInfo::GetFitness() const
{
    return -dst -IsDead()*deadPenalty;
}
