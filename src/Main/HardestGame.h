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

// playerSpeed==2 is the bare minimum
// (playerSpeed==1 might as well not have a solution)
const int playerSpeed = 3;
const int enemySpeed = 6;

struct IBox
{
    IBox() = default;
    IBox(int xmin, int xmax, int ymin, int ymax) :
        xmin(xmin), xmax(xmax), ymin(ymin), ymax(ymax) {}

    int xmin, xmax;
    int ymin, ymax;
};

struct IPoint
{
    IPoint() = default;
    IPoint(int x, int y) : x(x), y(y) {}

    int x;
    int y;
};

enum Direction
{
    NONE =  0b0000,
    UP =    0b0001,
    DOWN =  0b0010,
    LEFT =  0b0100,
    RIGHT = 0b1000
};

struct EnemyPath
{
    IPoint pos;
    int minx;
    int maxx;
    Direction dir;
};

struct LevelDscr
{
    std::vector<IBox> area;
    size_t startIdx;
    size_t endIdx;
    std::vector<EnemyPath> enemies;
    IBox player;

public:
    static const LevelDscr& Get();
};

#endif
