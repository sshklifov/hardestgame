#include "Planner.h"
#include "Utility.h"
#include <Movement.h>
#include <cassert>

const float PlayerInfo::incStability;
const float PlayerInfo::minStability;
const float PlayerInfo::maxStability;

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

int PlayerInfo::LastMoveIdx() const
{
    return IsDead() ? lastpos.size() : plan.size()-1;
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

void PlayerInfo::Mutate(URNG& gen)
{
    assert(!HasNoPlan() && !IsWinner());
    assert(stability >= minStability && stability <= maxStability);

    const float offsetIdxProb = 0.4f;
    const float nMutationsProb = 0.3f;

    int lastIdx = LastMoveIdx();
    int mutIdx = lastIdx -
        std::geometric_distribution<int>(offsetIdxProb*stability)(gen) % (lastIdx+1);

    int endMutIdx = mutIdx +
        std::geometric_distribution<int>(nMutationsProb*stability)(gen) % (lastIdx-mutIdx+1);

    Direction newDir = RandomOtherDirection(gen, plan[mutIdx]);
    std::vector<Direction> mutPlan = plan;
    mutPlan[mutIdx] = newDir;
    for (int i = mutIdx + 1; i <= endMutIdx; ++i)
    {
        if (endMutIdx-mutIdx+1 > 2) mutPlan[i] = newDir;
        else mutPlan[i] = RandomDirection(gen);
    }

    PlayerInfo child;
    child.Inherit((*this), std::move(mutPlan), mutIdx);
    if (child.fitness > fitness)
    {
        (*this) = std::move(child);
        stability = std::min(stability+incStability, maxStability);
    }
    else
    {
        float rat = fabs((float)(child.fitness-fitness) / fitness);
        float prob = 1.f / exp(rat / (1.f-stability));
        assert(prob >= 0.f && prob <= 1.f);
        if (std::bernoulli_distribution(prob)(gen))
        {
            (*this) = std::move(child);
        }
    }
}

PlayerInfo PlayerInfo::Crossover(URNG& gen, const PlayerInfo& rhs)
{
    assert(&rhs != this);

    int maxIdx = std::min(LastMoveIdx(), rhs.LastMoveIdx());
    int minIdx = std::max(maxIdx-crossoverBacktrack, 0);
    int idx = std::uniform_int_distribution<int>(minIdx, maxIdx)(gen);

    std::vector<Direction> crossPlan(plan.begin(), plan.begin()+idx);
    assert(rhs.plan.begin()+idx < rhs.plan.end());
    crossPlan.insert(crossPlan.end(), rhs.plan.begin()+idx, rhs.plan.end());

    PlayerInfo child;
    child.Inherit((*this), std::move(crossPlan), idx);
    return child;
}

void PlayerInfo::Inherit(const PlayerInfo& parent,
        std::vector<Direction> mutPlan, int mutIdx) 
{
    assert(!parent.HasNoPlan());

    plan = std::move(mutPlan);
    stability = parent.stability;
    if (mutIdx!=0)
    {
        lastpos.reserve(parent.lastpos.size());
        lastpos.assign(parent.lastpos.begin(), parent.lastpos.begin()+mutIdx);
    }
    Simulation();
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
    else pos = LevelDscr::Get().player;

    std::vector<EnemyPath> enemies = LevelDscr::Get().enemies;
    for (EnemyPath& e : enemies)
    {
        AdvanceEnemy(e, Planner::nRepeatMove*idx);
    }

    int steps = plan.size();
    int rep = 0;
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
                IPoint where = GetCenter(pos);
                fitness = -DstToGoal(where.x, where.y) +
                    DstFromStart(where.x, where.y) + deadPenalty;
                return;
            }
        }

        if (PlayerWins(pos))
        {
            for (int j = idx+1; j < steps; ++j) plan[j] = NONE;
            while (lastpos.size() < plan.size()) lastpos.push_back(GetCenter(pos));
            fitness = INT_MAX;
            return;
        }

        ++rep;
        if (rep == Planner::nRepeatMove)
        {
            rep = 0;
            ++idx;
            lastpos.push_back(GetCenter(pos));
        }
    }

    IPoint where = GetCenter(pos);
    fitness = -DstToGoal(where.x, where.y) + DstFromStart(where.x, where.y);
}

PlayerInfo::PlayerInfo() : pos(-100, -100, -100, -100)
{
    // try to render -> assert fail
}

PlayerInfo::PlayerInfo(int steps, URNG& gen) : stability(minStability)
{
    IncreaseStep(gen, steps);
}

void PlayerInfo::IncreaseStep(URNG& gen, int n)
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
    return fitness;
}
