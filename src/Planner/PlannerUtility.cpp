#include "Planner.h"
#include <HardestGame.h>
#include <Movement.h>

#include <memory>
#include <queue>

Direction RandomDirection(PlayerInfo::URNG& gen)
{
    int shift = std::uniform_int_distribution<int>(0, 4)(gen);
    return Direction(1<<shift);
}

Direction RandomOtherDirection(PlayerInfo::URNG& gen, Direction notThis)
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

    int offset = std::uniform_int_distribution<int>(1, n-1)(gen);
    int resIdx = (idx+offset) % n;
    return order[resIdx];
    /* int moves[5]; */
    /* int n = 0; */
    /* for (int bit = 0; bit < 5; ++bit) */
    /* { */
    /*     if (!(notMask&(1<<bit))) */
    /*     { */
    /*         moves[n++] = (1<<bit); */
    /*     } */
    /* } */
    /* int offset = std::uniform_int_distribution<int>(1, n-1)(gen); */
    /* return Direction(moves[offset]); */
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
int DstToGoal(int x, int y)
{
    static std::unique_ptr<std::vector<int>[]> dst (CalculateDst());
    assert (x >= 0 && x < width && y>=0 && y<height);
    return dst[x][y];
}

// heuristic for determining how close points are. if two points are
// close, but there is a wall between them, they are not actually close.
// equivalently, two points may be equally close to the goal, yet very
// far apart. this is why we use both to arrivate at an approximation.
/* static int DstHeuristic(const IPoint& lhs, const IPoint& rhs) */
/* { */
/*     int manh = abs(lhs.x-rhs.x) + abs(lhs.y-rhs.y); */
/*     int ddst = abs(DstToGoal(lhs.x, lhs.y) - DstToGoal(rhs.x, rhs.y)); */
/*     return std::max(manh, ddst); */
/* } */
