#include <HardestGame.h>
#include "Movement.h"

#include <vector>
#include <algorithm> // std::min
#include <memory>
#include <cassert>

static std::vector<bool>* CalcValid()
{
    static std::unique_ptr<std::vector<bool>[]> valid;
    valid.reset(new std::vector<bool>[width]);
    for (int i = 0; i < width; ++i)
    {
        valid[i].resize(height, false);
    }
    for (const IBox& b : LevelDscr::Get().area)
    {
        for (int y = b.ymin; y <= b.ymax; ++y)
        {
            for (int x = b.xmin; x <= b.xmax; ++x)
            {
                valid[x][y] = true;
            }
        }
    }

    return valid.release();
}

bool InsideField(int x, int y)
{
    static std::unique_ptr<std::vector<bool>[]> valid(CalcValid());
    assert(x>=0 && x<width && y>=0 && y<height);
    return valid[x][y];
}

static std::vector<int>* CalcMaxDstLeft()
{
    std::unique_ptr<std::vector<int>[]> maxDstLt;
    maxDstLt.reset(new std::vector<int>[width]);
    for (int i = 0; i < width; ++i)
    {
        maxDstLt[i].resize(height, 0);
    }
    for (int y = 0; y < height; ++y)
    {
        maxDstLt[0][y] = 0;
        for (int x = 1; x < width; ++x)
        {
            if (InsideField(x, y) && InsideField(x-1, y)) maxDstLt[x][y] = maxDstLt[x-1][y]+1;
            else maxDstLt[x][y] = 0;
        }
    }

    return maxDstLt.release();
}

static int MaxDstLeft(int x, int y)
{
    static std::unique_ptr<std::vector<int>[]> maxDstLt(CalcMaxDstLeft());
    assert(x>=0 && x<width && y>=0 && y<height);
    return maxDstLt[x][y];
}

static std::vector<int>* CalcMaxDstRight()
{
    std::unique_ptr<std::vector<int>[]> maxDstRt;
    maxDstRt.reset(new std::vector<int>[width]);
    for (int i = 0; i < width; ++i)
    {
        maxDstRt[i].resize(height, 0);
    }
    for (int y = 0; y < height; ++y)
    {
        maxDstRt[width-1][y] = 0;
        for (int x = width-2; x >= 0; --x)
        {
            if (InsideField(x, y) && InsideField(x+1, y)) maxDstRt[x][y] = maxDstRt[x+1][y]+1;
            else maxDstRt[x][y] = 0;
        }
    }

    return maxDstRt.release();
}

static int MaxDstRight(int x, int y)
{
    static std::unique_ptr<std::vector<int>[]> maxDstRt(CalcMaxDstRight());
    assert(x>=0 && x<width && y>=0 && y<height);
    return maxDstRt[x][y];
}

static std::vector<int>* CalcMaxDstUp()
{
    std::unique_ptr<std::vector<int>[]> maxDstUp;
    maxDstUp.reset(new std::vector<int>[width]);
    for (int i = 0; i < width; ++i)
    {
        maxDstUp[i].resize(height, 0);
    }
    for (int x = 0; x < width; ++x)
    {
        maxDstUp[x][height-1] = 0;
        for (int y = height-2; y >= 0; --y)
        {
            if (InsideField(x, y) && InsideField(x, y+1)) maxDstUp[x][y] = maxDstUp[x][y+1]+1;
            else maxDstUp[x][y] = 0;
        }
    }

    return maxDstUp.release();
}

static int MaxDstUp(int x, int y)
{
    static std::unique_ptr<std::vector<int>[]> maxDstUp(CalcMaxDstUp());
    assert(x>=0 && x<width && y>=0 && y<height);
    return maxDstUp[x][y];
}

static std::vector<int>* CalcMaxDstDown()
{
    std::unique_ptr<std::vector<int>[]> maxDstDn;
    maxDstDn.reset(new std::vector<int>[width]);
    for (int i = 0; i < width; ++i)
    {
        maxDstDn[i].resize(height, 0);
    }
    for (int x = 0; x < width; ++x)
    {
        maxDstDn[x][0] = 0;
        for (int y = 1; y < height; ++y)
        {
            if (InsideField(x, y) && InsideField(x, y-1)) maxDstDn[x][y] = maxDstDn[x][y-1]+1;
            else maxDstDn[x][y] = 0;
        }
    }

    return maxDstDn.release();
}

static int MaxDstDown(int x, int y)
{
    static std::unique_ptr<std::vector<int>[]> maxDstDown(CalcMaxDstDown());
    assert(x>=0 && x<width && y>=0 && y<height);
    return maxDstDown[x][y];
}

static int MaxAdvance(const IBox& b, Direction d)
{
    switch (d)
    {
    case UP:
        return std::min(MaxDstUp(b.xmin, b.ymax), MaxDstUp(b.xmax, b.ymax));
    case DOWN:
        return std::min(MaxDstDown(b.xmin, b.ymin), MaxDstDown(b.xmax, b.ymin));
    case LEFT:
        return std::min(MaxDstLeft(b.xmin, b.ymin), MaxDstLeft(b.xmin, b.ymax));
    case RIGHT:
        return std::min(MaxDstRight(b.xmax, b.ymin), MaxDstRight(b.xmax, b.ymax));
    default:
        assert(false && "invalid direction");
    }
}

bool AdvancePlayer(IBox& b, Direction d)
{
    assert(b.xmin >= 0 && b.xmax < width);
    assert(b.ymin >= 0 && b.ymax < height);

    int dx = 0, dy = 0;
    switch (d)
    {
    case UP:
        dy = std::min(MaxAdvance(b, d), playerSpeed);
        break;
    case DOWN:
        dy = -std::min(MaxAdvance(b, d), playerSpeed);
        break;
    case LEFT:
        dx = -std::min(MaxAdvance(b, d), playerSpeed);
        break;
    case RIGHT:
        dx = std::min(MaxAdvance(b, d), playerSpeed);
        break;
    case NONE:
        break;
    default:
        assert(false && "invalid direction");
    }

    b.xmin += dx;
    b.xmax += dx;
    b.ymin += dy;
    b.ymax += dy;

    return dx==0 && dy==0;
}

void AdvanceEnemy(EnemyPath& p)
{
    assert (p.pos.x >= p.minx && p.pos.x <= p.maxx);

    if (p.dir == LEFT)
    {
        p.pos.x -= enemySpeed;
        if (p.pos.x < p.minx)
        {
            p.pos.x = p.minx + (p.minx-p.pos.x);
            p.dir = RIGHT;
        }
    }
    else // p.dir == RIGHT
    {
        p.pos.x += enemySpeed;
        if (p.pos.x > p.maxx)
        {
            p.pos.x = p.maxx - (p.pos.x-p.maxx);
            p.dir = LEFT;
        }
    }
}

bool PlayerWins(const IBox& player)
{
    const IBox& box = LevelDscr::Get().area[LevelDscr::Get().endIdx];
    return player.xmax >= box.xmin && player.xmin <= box.xmax &&
        player.ymax >= box.ymin && player.ymin <= box.ymax;
}

bool PlayerDies(const IBox& box, const IPoint& p)
{
    auto Sqr = [](int x) {return x*x;};
    const int r2 = Sqr(enemyRad);

    int dx = p.x - Clamp(p.x, box.xmin, box.xmax);
    int dy = p.y - Clamp(p.y, box.ymin, box.ymax);
    return Sqr(dx)+Sqr(dy) <= r2;
}
