#include <HardestGame.h>
#include "Draw.h"

#include <cassert>
#include <cmath>

template <typename T>
T Clamp(T x, T minVal, T maxVal)
{
    return std::max(minVal, std::min(maxVal, x));
}

static float SmoothStep(float edge0, float edge1, float x)
{
    assert (edge1 > edge0 && fabs(edge1-edge0) > fabs(edge1)*1e-5);

    float t = Clamp((x-edge0) / (edge1-edge0), 0.f, 1.f);
    return t * t * (3.f - 2.f*t);
}

const Color pink{1.f, 0.75f, 0.75f};

static bool IsPink(const Color& c)
{
    return c.r==pink.r && c.g==pink.g && c.b==pink.b;
}

static void Clear(Color* fb)
{
    int index = 0;
    for (int i = 0; i < pixels; ++i)
    {
        fb[index++] = pink;
    }
}

static void Fill(Color* fb, const Color& c)
{
    int index = 0;
    for (int i = 0; i < pixels; ++i)
    {
        if (IsPink(fb[index]))
        {
            fb[index] = c;
        }
        ++index;
    }
}

template <typename Func>
void DrawRectangle(Color* fb, const IBox& b, bool forceBorder, const Func& Sample)
{
    const int borderSize = 2;
    const Color border {0.f, 0.f, 0.f};

    assert(b.xmin >= 0 && b.xmax < width);
    assert(b.ymin >= 0 && b.ymax < height);

    int dy = b.ymax - b.ymin;
    int dx = b.xmax - b.xmin;
    for (int yoff = -borderSize; yoff <= dy+borderSize; ++yoff)
    {
        int xoff = -borderSize;
        int index = width*(b.ymin+yoff) + b.xmin+xoff;
        for (; xoff <= dx+borderSize; ++xoff)
        {
            if (!(xoff < 0 || xoff > dx || yoff < 0 || yoff > dy))
            {
                fb[index] = Sample(xoff, yoff);
            }
            else if (forceBorder || IsPink(fb[index])) fb[index] = border;

            ++index;
        }
    }
}

static void DrawSafezone(Color* fb, const IBox& b)
{
    const Color green {0.7f, 1.f, 0.f};
    return DrawRectangle(fb, b, false, [&green](int, int){return green;});
}

static void DrawDangerzone(Color* fb, const IBox& b)
{
    return DrawRectangle(fb, b, false, [&b](int x, int y)
    {
        const Color pos1{1.f, 1.f, 1.f};
        const Color pos2{0.8, 0.8, 0.8};

        int bitx = ((x+b.xmin) / checkerLen) & 1;
        int bity = ((y+b.ymin) / checkerLen) & 1;
        if (bitx ^ bity) return pos2;
        else return pos1;
    });
}

void DrawPlayer(Color* fb, const IBox& player, float red)
{
    assert(red >= -1e-5f && red <= 1+1e-5f);

    Color color{red, 0.f, 0.f};
    return DrawRectangle(fb, player, true, [&color](int, int){return color;});
}

void DrawEnemy(Color* fb, IPoint p)
{
    const Color enemy {0.3f, 0.3f, 0.3f};
    const int rad = enemyRad;
    const int edge = enemyRad+1;
    assert(p.x >= rad && p.x < width-rad);
    assert(p.y >= rad && p.y < height-rad);

    const int rad2 = rad*rad;
    const int edge2 = edge*edge;
    for (int yoff = -edge; yoff <= edge; ++yoff)
    {
        int xoff = -edge;
        int index = (p.y+yoff)*width + (p.x+xoff);
        for (; xoff <= edge; ++xoff)
        {
            int dist2 = yoff*yoff + xoff*xoff;
            if (dist2 < rad2)
            {
                fb[index] = enemy;
            }
            else if (dist2 < edge2)
            {
                assert(index>=0 && index<pixels);

                float t = SmoothStep(rad2, edge2, dist2);
                fb[index].r = fb[index].r*t + enemy.r*(1.f-t);
                fb[index].g = fb[index].g*t + enemy.g*(1.f-t);
                fb[index].b = fb[index].b*t + enemy.b*(1.f-t);
            }
            index++;
        }
    }
}

void DrawLevel(Color* fb, const LevelDscr& lvl)
{
    Clear(fb);

    for (int i = 0; i < (int)lvl.area.size(); ++i)
    {
        if (i != lvl.startIdx && i != lvl.endIdx)
            DrawDangerzone(fb, lvl.area[i]);
    }
    if (lvl.startIdx>=0 && lvl.startIdx<(int)lvl.area.size())
        DrawSafezone(fb, lvl.area[lvl.startIdx]);
    if (lvl.endIdx>=0 && lvl.endIdx<(int)lvl.area.size())
        DrawSafezone(fb, lvl.area[lvl.endIdx]);

    Fill(fb, Color{0.5, 0.5f, 0.5f});
}
