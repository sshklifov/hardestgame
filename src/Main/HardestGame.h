#ifndef HARDEST_GAME_INCLUDED
#define HARDEST_GAME_INCLUDED

#include <vector>

const int width = 1280;
const int height = 720;
const int pixels = width*height;

const int playerRad = 12;
const int playerSize = 2*playerRad+1;
const int enemyRad = 12;

const int checkerLen = 45;

const int playerSpeed = 3;
const int enemySpeed = 7;

struct IPoint
{
    IPoint() = default;
    IPoint(int x, int y) : x(x), y(y) {}

    int x;
    int y;
};

struct IBox
{
    IBox() = default;
    IBox(int xmin, int xmax, int ymin, int ymax) :
        xmin(xmin), xmax(xmax), ymin(ymin), ymax(ymax)
    {}
    IBox(IPoint pmin, IPoint pmax) :
        xmin(pmin.x), xmax(pmax.x), ymin(pmin.y), ymax(pmax.y)
    {}

    int xmin, xmax;
    int ymin, ymax;
};

enum Direction
{
    UP =    0b00001,
    DOWN =  0b00010,
    LEFT =  0b00100,
    RIGHT = 0b01000,
    NONE =  0b10000,
};

struct EnemyPath
{
    EnemyPath() = default;
    EnemyPath(int xpos, int ypos, int from, int to, Direction dir) :
        pos(xpos, ypos), from(from), to(to), dir(dir)
    {}

    IPoint pos;
    int from;
    int to;
    Direction dir;
};

struct LevelDscr
{
    std::vector<IBox> area;
    int startIdx;
    int endIdx;
    std::vector<EnemyPath> enemies;
    IBox player;

public:
    static const LevelDscr& Get();
};

#endif
