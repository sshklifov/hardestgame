#include "Utility.h"
#include <Movement.h>

#include <memory>
#include <queue>
#include <cassert>
#include <algorithm>

Direction RandomDirection(PlayerInfo::URNG& gen)
{
    int shift = std::uniform_int_distribution<int>(0, 4)(gen);
    return Direction(1<<shift);
}

Direction RandomOtherDirection(PlayerInfo::URNG& gen, Direction notThis)
{
    const Direction order[] = {LEFT, RIGHT, UP, DOWN, NONE};
    const int n = std::extent<decltype(order)>::value;

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

    int offset = std::uniform_int_distribution<int>(1, n-1)(gen);
    int resIdx = (idx+offset) % n;
    return order[resIdx];
}

std::vector<Direction> RandomPlan(PlayerInfo::URNG& gen, int steps)
{
    std::vector<Direction> res;
    res.reserve(steps);
    for (int i = 0; i < steps; ++i)
    {
        res.push_back(RandomDirection(gen));
    }

    return res;
}

IPoint GetCenter(const IBox& b)
{
    return IPoint{(b.xmin+b.xmax) / 2, (b.ymin+b.ymax) / 2};
}

static std::vector<int>* CalculateDst(int boxIdx)
{
    std::unique_ptr<std::vector<int>[]> dst;
    dst.reset(new std::vector<int>[width]);
    for (int i = 0; i < width; ++i)
    {
        dst[i].resize(height, INT_MAX);
    }

    std::queue<IPoint> q;
    const IBox& box = LevelDscr::Get().area[boxIdx];
    for (int y = box.ymin; y <= box.ymax; ++y)
    {
        for (int x = box.xmin; x <= box.xmax; ++x)
        {
            if (x==box.xmin || x==box.xmax || y==box.ymin || y==box.ymax)
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

int DstToGoal(int x, int y)
{
    static std::unique_ptr<std::vector<int>[]> dst
        (CalculateDst(LevelDscr::Get().endIdx));

    assert (x >= 0 && x < width && y>=0 && y<height);
    return dst[x][y];
}

void Prune(std::vector<PlayerInfo>& players, int maxPruned)
{
    if (maxPruned < 0) return;
     
    std::sort(players.begin(), players.end(),
        [](const PlayerInfo& lhs, const PlayerInfo& rhs)
        {
            return lhs.GetFitness() > rhs.GetFitness();
        });

    // for a few samples n^2 is as good as a sliding window solution
    // also, points are not guaranteed to be sparse.

    int eraseAfter = players.size()-1;
    for (int i = players.size()-1; i >= 1; --i)
    {
        bool shouldRemove = false;
        for (int j = 0; j < i; ++j)
        {
            IPoint lhs = GetCenter(players[i].GetLastPos());
            IPoint rhs = GetCenter(players[j].GetLastPos());
            int manh = abs(lhs.x-rhs.x) + abs(lhs.y-rhs.y);

            if (manh < Planner::pruneDistance)
            {
                shouldRemove = true;
                break;
            }
        }
        if (shouldRemove)
        {
            std::swap(players[i], players[eraseAfter]);
            --eraseAfter;

            --maxPruned;
            if (maxPruned <= 0) break;
        }
    }

    players.erase(players.begin()+eraseAfter+1, players.end());
}
