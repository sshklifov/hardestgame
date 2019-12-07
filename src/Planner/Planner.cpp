#include <HardestGame.h>
#include <Movement.h>
#include <Draw.h>
#include "Planner.h"
#include "Prune.h"

#include <random>
#include <memory>
#include <queue>
#include <climits>
#include <cassert>
#include <algorithm>

// used in function below
static std::vector<int>* CalculateDst()
{
    std::unique_ptr<std::vector<int>[]> dst;
    dst.reset(new std::vector<int>[width]);
    for (int i = 0; i < width; ++i)
    {
        dst[i].resize(height, INT_MAX);
    }

    std::queue<IPoint> q;
    const IBox& end = LevelDscr::Get().area[LevelDscr::Get().endIdx];
    for (int y = end.ymin; y <= end.ymax; ++y)
    {
        for (int x = end.xmin; x <= end.xmax; ++x)
        {
            if (x==end.xmin || x==end.xmax || y==end.ymin || y==end.ymax)
                q.emplace(x, y);
            dst[x][y] = 0;
        }
    }

    while (!q.empty())
    {
        int x = q.front().x;
        int y = q.front().y;
        q.pop();

        const int offsets[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (const int (&offset)[2] : offsets)
        {
            int currx = x+offset[0];
            int curry = y+offset[1];
            if (currx>=0 && currx<width && curry>=0 && curry<height &&
                    InsideField(currx, curry) && dst[currx][curry]==INT_MAX)
            {
                dst[currx][curry] = dst[x][y] + 1;
                q.push(IPoint{currx, curry});
            }
        }
    }

    return dst.release();
}

// returns the distance to the goal in pixels
static int DstToGoal(int x, int y)
{
    static std::unique_ptr<std::vector<int>[]> dst (CalculateDst());
    assert (x >= 0 && x < width && y>=0 && y<height);
    return dst[x][y];
}

// heuristic for determining how close points are. if two points are
// close, but there is a wall between them, they are not actually close.
// equivalently, two points may be equally close to the goal, yet very
// far apart. this is why we use both to arrivate at an approximation.
static int DstHeuristic(const IPoint& lhs, const IPoint& rhs)
{
    int manh = abs(lhs.x-rhs.x) + abs(lhs.y-rhs.y);
    int ddst = abs(DstToGoal(lhs.x, lhs.y) - DstToGoal(rhs.x, rhs.y));
    return std::max(manh, ddst);
}

int Planner::GetUniformRandom(int a, int b) const
{
    return std::uniform_int_distribution<int>(a, b)(gen);
}

// avoid Direction::NONE
Direction Planner::RandomDirection() const
{
    int shift = GetUniformRandom(0, 4);
    if (shift == 4)
    {
        shift = GetUniformRandom(0, 4);
        if (shift == 4) return NONE;
    }
    return Direction(1<<shift);
}

// Used in mutation, no point in changing something to itself
Direction Planner::RandomOtherDirection(Direction notThis) const
{
    const Direction order[] = {LEFT, RIGHT, UP, DOWN, NONE};
    int n = sizeof(order) / sizeof(Direction);

    int idx = -1;
    switch (notThis)
    {
    case LEFT:
        idx = 0;
        break;
    case RIGHT:
        idx = 1;
        break;
    case UP:
        idx = 2;
        break;
    case DOWN:
        idx = 3;
        break;
    case NONE:
        idx = 4;
        break;
    }

    int offset = GetUniformRandom(1, n-1);
    int resIdx = (idx+offset) % n;
    if (order[resIdx] == NONE)
    {
        offset = GetUniformRandom(1, n-1);
        resIdx = (idx+offset) % n;

    }
    return order[resIdx];
}

std::vector<Direction> Planner::RandomPlan(int steps) const
{
    std::vector<Direction> res;
    res.reserve(steps);
    for (int i = 0; i < steps; ++i)
    {
        res.push_back(RandomDirection());
    }

    return res;
}

Planner::Planner(int samples, int seed) :
    gen(seed), samples(samples), steps(lastMovesSize), generation(1), solIdx(-1)
{
    players.resize(samples*2);
    for (int i = 0; i < (int)players.size(); ++i)
    {
        players[i].plan = RandomPlan(steps);
    }
}

// -play out the players strategies
// -use rep to repeat every Direction
// -fill in PlayerInfo useful data
// -return true if either Bricked() or FoundSol()
bool Planner::Simulate()
{
    assert(!FoundSol() && !Bricked());

    std::vector<EnemyPath> enemies = LevelDscr::Get().enemies;

    int idx = 0;
    int rep = 0;
    while (idx < steps)
    {
        for (PlayerInfo& player : players)
        {
            if (player.dieIdx<0)
            {
                AdvancePlayer(player.pos, player.plan[idx]);
            }
        }
        for (EnemyPath& e : enemies)
        {
            AdvanceEnemy(e);
        }

        for (int i = 0; i < (int)players.size(); ++i)
        {
            if (players[i].dieIdx >= 0) continue;
            for (const EnemyPath& e : enemies)
            {
                if (PlayerDies(players[i].pos, e.pos))
                {
                    players[i].dieIdx = idx;
                    break;
                }
            }
            if (players[i].dieIdx<0 && PlayerWins(players[i].pos) && !FoundSol())
            {
                for (int j = idx+1; j < steps; ++j) players[i].plan[j] = NONE;
                solIdx = i;
            }
        }

        assert(repeatMoves>1);
        idx += (rep+1)/repeatMoves;
        rep = (rep+1)%repeatMoves;

        if (rep==0)
        {
            for (PlayerInfo& player : players)
            {
                if (player.dieIdx<0)
                {
                    player.lastpos.Enque(GetCenter(player.pos));
                }
            }
        }
    }

    for (PlayerInfo& player : players)
    {
        IPoint where = GetCenter(player.pos);
        player.dst = DstToGoal(where.x, where.y);
    }

    return FoundSol();
}

// evaluate how alike are the players
// PlayerInfo::lastpos contains the few last positions of the players. They can
// be used to determine if two players follow roughly the same strategy (a good 
// cross over pair).
int Planner::EvalMatch(const PlayerInfo& dead, const PlayerInfo& alive, int& crossIdx)
{
    assert(dead.dieIdx>=0 && alive.dieIdx<0);
    int steps = alive.plan.size();
    assert(steps == (int)dead.plan.size());

    int aliveFirstIdx = steps+1-lastMovesSize;
    int deadLastIdx = dead.dieIdx;
    if (aliveFirstIdx > deadLastIdx)
    {
        crossIdx = steps-1;
        return DstHeuristic(GetCenter(dead.pos), alive.lastpos[lastMovesSize-1]);
    }

    int aliveLastIdx = steps;
    int aliveOffset = aliveLastIdx-deadLastIdx;
    int cnt = deadLastIdx-aliveFirstIdx+1;
    assert(aliveOffset+cnt-1 == lastMovesSize-1);

    int diff = INT_MAX;
    for (int i = 0; i < cnt; ++i)
    {
        IPoint pointDead = dead.lastpos[i];
        IPoint pointAlive = alive.lastpos[i + aliveOffset];
        int dst = DstHeuristic(pointDead, pointAlive);
        if (dst < diff)
        {
            diff = dst;
            crossIdx = deadLastIdx-i;
            assert(crossIdx>= 0 && crossIdx < steps);
        }
    }

    return diff;
}

// cross over players. always cross over the dead with the living so the dead
// can learn from the living. do not consider indices before crossIdx as they would
// be suboptimal. eg: cross over at the last steps, when the player has died in the
// begining.
void Planner::CrossOver(const PlayerInfo& dead, const PlayerInfo& alive, int crossIdx, PlayerInfo& res) const
{
    assert(!FoundSol() && !Bricked());
    assert(dead.dieIdx>=0 && alive.dieIdx<0);

    int steps = alive.plan.size();
    assert(steps == (int)dead.plan.size());

    int from = GetUniformRandom(crossIdx, steps-1);
    int n = std::geometric_distribution<int>(0.5f)(gen) % (steps-from);
    int to = from+n;

    res.plan = dead.plan;
    for (int i = from; i <= to; ++i)
    {
        assert (i >= 0 && i < steps);
        res.plan[i] = alive.plan[i];
    }

    res.pruneProtect = true;
}

void Planner::Mutate(const PlayerInfo& player, PlayerInfo& mut, int lastIdx) const
{
    assert(!FoundSol() && !Bricked());

    mut.plan = player.plan;

    int mutIdx = lastIdx - std::geometric_distribution<int>(0.4f)(gen) % (lastIdx+1);
    Direction newdir = RandomOtherDirection(player.plan[mutIdx]);
    mut.plan[mutIdx] = newdir;

    mut.pruneProtect = true;
}

// key notes:
// maintain the ratio dead : alive, so as to give everyone an equal chance.
// both dead and alive players are valuable. Also give equal chance to everyone
// to have offspring. Otherwise, the players will behave the same.
int Planner::GenChildren(std::vector<PlayerInfo>& children, int nChildren)
{
    assert(!FoundSol() && !Bricked());

    std::shuffle(players.begin(), players.end(), gen);
    auto it = std::partition(players.begin(), players.end(),
        [](const PlayerInfo& info)
        {
            return info.dieIdx<0;
        });

    children.clear();
    children.resize(nChildren);

    int nAlive = it-players.begin();
    int nDead = players.size()-nAlive;

    double ratioAlive = (float)nAlive/players.size();
    int newAlive = ceil(ratioAlive*nChildren);
    int newDead = nChildren - newAlive;

    int idx = 0;
    for (int off = 0; off < newDead; ++off)
    {
        int bestAlive = -1;
        int crossIdx = -1;
        int diff = INT_MAX;

        int currDead = (off % nDead) + nAlive;
        for (int currAlive = 0; currAlive < nAlive; ++currAlive)
        {
            int tmpCrossIdx;
            int curr = EvalMatch(players[currDead], players[currAlive], tmpCrossIdx);
            if (curr < diff)
            {
                diff = curr;
                bestAlive = currAlive;
                crossIdx = tmpCrossIdx;
            }
        }

        const int threshold = 200;
        if (diff < threshold && crossIdx < steps-2) // avoid copying parent completely / only copy if relevant
            CrossOver(players[currDead], players[bestAlive], crossIdx, children[idx++]);
        else
            Mutate(players[currDead], children[idx++], players[currDead].dieIdx);
    }

    // crossover with self (simulate beam search)
    // leave the players which are alive to explore so they:
    // a: die and balance the alive : dead ratio
    // b: find a path that leads to a solution
    // some of these mutations are bound to work and they will supersede the majority of bad ones
    for (int off = 0; off < newAlive; ++off)
    {
        int i = off % nAlive;
        Mutate(players[i], children[idx++], steps-1);
    }

    assert(idx==(int)children.size());
    return nAlive;
}

void Planner::StripBad()
{
    // remove any exceess elements
    // do not prefer alive players. keep the ratio alive : dead fixed
    // this is done with a single vector::erase command

    assert(!FoundSol() && !Bricked());

    if ((int)players.size() <= samples) return;

    auto it = std::partition(players.begin(), players.end(),
        [](const PlayerInfo& info)
        {
            return info.dieIdx<0;
        });
    int nAlive = it - players.begin();

    int nDead = players.size()-nAlive;
    int toDelete = players.size()-samples;

    float r = (float)samples / players.size();
    int delAlive = std::min(nAlive - (int)floor(r*nAlive), toDelete);
    int delDead = toDelete-delAlive;
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

bool Planner::FoundSol() const
{
    return solIdx >= 0;
}

const std::vector<Direction>& Planner::SeeSol() const
{
    assert(FoundSol());
    return players[solIdx].plan;
}

bool Planner::Bricked() const
{
    return steps > maxSteps;
}

bool Planner::NextGen()
{
    // -player.size() is not constant, but is never > 2*samples
    // -if enough iterations are mode, increase the number of moves
    if (FoundSol() || Bricked()) return true;

    if (generation >= repeatGeneration)
    {
        int newSteps = steps+incSteps;
        for (int i = 0; i < (int)players.size(); ++i)
        {
            players[i].plan.resize(newSteps);
            for (int j = steps; j < newSteps; ++j)
            {
                players[i].plan[j] = RandomDirection();
            }
        }

        steps = newSteps;
        generation = 1;
        return false;
    }

    for (int i = 0; i < (int)players.size(); ++i)
    {
        players[i].pos = LevelDscr::Get().player;
        players[i].dieIdx = -1;
    }
    if (Simulate()) return true;

    Prune(players, gen);
    StripBad();
    for (int i = 0; i < (int)players.size(); ++i)
    {
        players[i].pruneProtect = false;
    }

    std::vector<PlayerInfo> children;
    GenChildren(children, samples);

    players.reserve(players.size()+children.size());
    for (int i = 0; i < (int)children.size(); ++i)
    {
        players.push_back(std::move(children[i]));
    }
    
    ++generation;
    return false;
}
