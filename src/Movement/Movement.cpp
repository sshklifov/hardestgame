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

void AdvancePlayer(IBox& b, Direction d, int times)
{
    assert(b.xmin >= 0 && b.xmax < width);
    assert(b.ymin >= 0 && b.ymax < height);

    int dx = 0, dy = 0;
    switch (d)
    {
    case UP:
        dy = std::min(MaxAdvance(b, d), playerSpeed*times);
        break;
    case DOWN:
        dy = -std::min(MaxAdvance(b, d), playerSpeed*times);
        break;
    case LEFT:
        dx = -std::min(MaxAdvance(b, d), playerSpeed*times);
        break;
    case RIGHT:
        dx = std::min(MaxAdvance(b, d), playerSpeed*times);
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
}

void AdvanceEnemy(EnemyPath& e, int times)
{
    if (times==0) return;

    int px = times*enemySpeed;
    int halfStates = e.to-e.from;
    int states = 2*halfStates;

    if (e.dir == LEFT)
    {
        assert(e.pos.x >= e.from && e.pos.x <= e.to);
        int offset = e.pos.x-e.from;
        int state = (-states + offset - px) % states;
        if (state > -halfStates)
        {
            e.dir = RIGHT;
            e.pos.x = e.from + (-state);
        }
        else
        {
            e.dir = LEFT;
            e.pos.x = e.to - (-halfStates-state);
        }
    }
    else if (e.dir == RIGHT)
    {
        assert(e.pos.x >= e.from && e.pos.x <= e.to);
        int offset = e.pos.x-e.from;
        int state = (offset + px) % states;
        if (state < halfStates)
        {
            e.dir = RIGHT;
            e.pos.x = e.from + state;
        }
        else
        {
            e.dir = LEFT;
            e.pos.x = e.to - (state-halfStates);
        }
    }
    else if (e.dir == DOWN)
    {
        assert(e.pos.y >= e.from && e.pos.y <= e.to);
        int offset = e.pos.y-e.from;
        int state = (-states + offset - px) % states;
        if (state >= -halfStates)
        {
            e.dir = UP;
            e.pos.y = e.from + (-state);
        }
        else
        {
            e.dir = DOWN;
            e.pos.y = e.to - (-halfStates-state);
        }
    }
    else if (e.dir == UP)
    {
        assert(e.pos.y >= e.from && e.pos.y <= e.to);
        int offset = e.pos.y-e.from;
        int state = (offset + px) % states;
        if (state < halfStates)
        {
            e.dir = UP;
            e.pos.y = e.from + state;
        }
        else
        {
            e.dir = DOWN;
            e.pos.y = e.to - (state-halfStates);
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
