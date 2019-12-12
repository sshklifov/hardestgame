#include "Planner.h"
#include <Movement.h>

// PlannerUtility.cpp
extern Direction RandomDirection(PlayerInfo::URNG& gen);
extern Direction RandomOtherDirection(PlayerInfo::URNG& gen, Direction notThis);
extern std::vector<Direction> RandomPlan(PlayerInfo::URNG& gen, int steps);
extern IPoint GetCenter(const IBox& b);
extern int DstToGoal(int x, int y);

bool PlayerInfo::IsDead() const
{
    assert(!HasNoPlan());
    return lastpos.size() < plan.size();
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

bool PlayerInfo::InsideLocalMin() const
{
    assert(!HasNoPlan());
    return staleFactor >= staleThreshold;
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
    // USE LOCAL STABILITY
    float stability = std::min((float)staleFactor / staleThreshold, 1.f);
    const int tooFresh = 5;
    if (staleFactor<=tooFresh) stability = 1.f;

    // TODO
    stability = 0.9f;
    this->stability = stability;

    int lastIdx;
    if (IsDead())
    {
        lastIdx = lastpos.size();
    }
    else
    {
        lastIdx = plan.size()-1;
    }

    int mutIdx;
    int n;
    /* int n; */
    /* if (staleFactor <= 0.1) */
    /* { */
    /*     stability = 1.0f; */
    /*     n = -1; */
    /* } */
    /* else */
    /* { */

    /* } */
    /* else */
    {
        const float offsetScale = 0.3f;
        mutIdx = lastIdx - std::geometric_distribution<int>(stability*offsetScale)(gen)%(lastIdx+1);
        const float nScale = 0.4f;
        n = mutIdx + std::geometric_distribution<int>(stability*nScale)(gen) % Planner::incSteps;
    }
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
    chld.locMin = locMin;
    chld.staleFactor = staleFactor+1;
    chld.stability = stability; // TODO

    if (mutIdx==0)
    {
        chld.Simulation();
        return chld;
    }

    chld.lastpos.reserve(lastpos.size());
    chld.lastpos.assign(lastpos.begin(), lastpos.begin()+mutIdx);

    chld.Simulation();
    return chld;
}

void PlayerInfo::Simulation()
{
    assert(!HasNoPlan());
    //! requires only lastpos information
    //! and stale
    //! and loc min

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
    if (dst<locMin-playerRad || (locMin/playerRad<4 && dst<locMin))
    {
        /* locMin = std::min(dst, locMin); */
        locMin = dst;
        staleFactor=0;
    }
    else
    {
        /* printf("%d loc min\n", dst); */
        ++staleFactor;
    }
}

PlayerInfo::PlayerInfo() : pos(-100, -100, -100, -100), locMin(INT_MAX)
{
    // try to render -> assert fail
}

PlayerInfo::PlayerInfo(int steps, URNG& gen) : PlayerInfo() // TODO
{
    /* IncreaseStep(steps, gen); */
    plan = RandomPlan(gen, steps);
    /* pos = LevelDscr::Get().player; */
    Simulation();
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

    /* const IPoint& p = lastpos.back(); */
    /* pos = IBox(p.x-playerRad, p.x+playerRad, */
    /*         p.y-playerRad, p.y+playerRad); */
    /* Simulation(players[i], players[i].lastpos.size()); */
    Simulation();
    /* assert(plan.size()>0); */
}
