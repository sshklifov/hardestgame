#include "HardestGame.h"
#include <Draw.h>
#include <Setup.h>
#include <Movement.h>

#include <GLFW/glfw3.h>

#include <climits>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cerrno>

#define INVALID -100

static LevelDscr lvl {{}, -1, -1, {}, IBox(INVALID, INVALID, INVALID, INVALID)};

static int xmin=INT_MAX, xmax=INT_MIN, ymin=INT_MAX, ymax=INT_MIN;
static void CursorPosCallback(GLFWwindow*, double xpos, double ypos)
{
    int x = xpos;
    int y = height-ypos;

    if (x>=checkerLen && x<width-checkerLen && y>=checkerLen && y<height-checkerLen)
    {
        xmin = std::min(xmin, x);
        xmax = std::max(xmax, x);
        ymin = std::min(ymin, y);
        ymax = std::max(ymax, y);
    }
}

static std::vector<void(*)()> undo;
static void UndoPlayingField()
{
    if (lvl.area.empty()) return;

    lvl.area.pop_back();
    if (lvl.startIdx >= (int)lvl.area.size())
    {
        lvl.startIdx = -1;
    }
    if (lvl.endIdx >= (int)lvl.area.size())
    {
        lvl.endIdx = -1;
    }
}
static void UndoEnemy()
{
    if (lvl.enemies.empty()) return;
    lvl.enemies.pop_back();
}

static int makeMultipleOfCheckerLen(int x)
{
    return round((float)x/checkerLen)*checkerLen;
}
static int makeCenterAtCheckerLen(int x)
{
    return (x/checkerLen)*checkerLen + checkerLen/2;
}

static bool mouseMode=false;
static bool usingStart=true;
EnemyPath e (INVALID, INVALID, INVALID, INVALID, NONE);

static void KeyCallback(GLFWwindow* window, int key, int, int action, int mods)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        mouseMode^=1;
        if (mouseMode==0)
        {
            int xminAlign = makeMultipleOfCheckerLen(xmin);
            int xmaxAlign = makeMultipleOfCheckerLen(xmax)-1;
            int yminAlign = makeMultipleOfCheckerLen(ymin);
            int ymaxAlign = makeMultipleOfCheckerLen(ymax)-1;
            lvl.area.emplace_back(xminAlign, xmaxAlign, yminAlign, ymaxAlign);

            undo.push_back(UndoPlayingField);

        }
        else
        {
            xmin=INT_MAX, xmax=INT_MIN, ymin=INT_MAX, ymax=INT_MIN;
        }
    }
    if (key == GLFW_KEY_U && action == GLFW_PRESS)
    {
        if (!undo.empty())
        {
            undo.back()();
            undo.pop_back();
        }
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        if (lvl.startIdx<0) lvl.startIdx=lvl.area.size()-1;
        usingStart=true;
    }
    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        if (lvl.endIdx<0) lvl.endIdx=lvl.area.size()-1;
        usingStart=false;
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
    {
        int& idx = usingStart ? lvl.startIdx : lvl.endIdx;
        if (idx>0) --idx;
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
    {
        int& idx = usingStart ? lvl.startIdx : lvl.endIdx;
        if (idx<(int)lvl.area.size()-1) ++idx;
    }

    if (key == GLFW_KEY_H && mods==0 && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        int x=xpos, y=height-ypos;
        if (e.dir == NONE)
        {
            e.pos.x = makeCenterAtCheckerLen(x);
            e.pos.y = makeCenterAtCheckerLen(y);
            e.dir = Direction(LEFT | RIGHT);
        }
        else
        {
            if (x<e.pos.x)
            {
                e.from = makeCenterAtCheckerLen(x);
                e.to = e.pos.x;
                e.dir = LEFT;
            }
            else
            {
                e.from = e.pos.x;
                e.to = makeCenterAtCheckerLen(x);
                e.dir = RIGHT;
            }
            lvl.enemies.push_back(e);
            undo.push_back(UndoEnemy);
            e.dir = NONE;
        }
    }
    if (key == GLFW_KEY_V && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        int x=xpos, y=height-ypos;
        if (e.dir == NONE)
        {
            e.pos.x = makeCenterAtCheckerLen(x);
            e.pos.y = makeCenterAtCheckerLen(y);
            e.dir = Direction(UP | DOWN);
        }
        else
        {
            if (y<e.pos.y)
            {
                e.from = makeCenterAtCheckerLen(y);
                e.to = e.pos.y;
                e.dir = DOWN;
            }
            else
            {
                e.from = e.pos.y;
                e.to = makeCenterAtCheckerLen(y);
                e.dir = UP;
            }
            lvl.enemies.push_back(e);
            undo.push_back(UndoEnemy);
            e.dir = NONE;
        }
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        int x=xpos, y=height-ypos;
        lvl.player= IBox(x-playerRad, x+playerRad, y-playerRad, y+playerRad);
    }

    if (key == GLFW_KEY_H && mods == GLFW_MOD_SHIFT && action == GLFW_PRESS)
    {
        if (lvl.area.empty()) return;
        lvl.area.back().xmax -= checkerLen;
    }
    if (key == GLFW_KEY_H && mods == GLFW_MOD_CONTROL && action == GLFW_PRESS)
    {
        if (lvl.area.empty()) return;
        lvl.area.back().xmin -= checkerLen;
        lvl.area.back().xmax -= checkerLen;
    }

    if (key == GLFW_KEY_L && mods == GLFW_MOD_SHIFT && action == GLFW_PRESS)
    {
        if (lvl.area.empty()) return;
        lvl.area.back().xmax += checkerLen;
    }
    if (key == GLFW_KEY_L && mods == GLFW_MOD_CONTROL && action == GLFW_PRESS)
    {
        if (lvl.area.empty()) return;
        lvl.area.back().xmin += checkerLen;
        lvl.area.back().xmax += checkerLen;
    }

    if (key == GLFW_KEY_J && mods == GLFW_MOD_SHIFT && action == GLFW_PRESS)
    {
        if (lvl.area.empty()) return;
        lvl.area.back().ymax -= checkerLen;
    }
    if (key == GLFW_KEY_J && mods == GLFW_MOD_CONTROL && action == GLFW_PRESS)
    {
        if (lvl.area.empty()) return;
        lvl.area.back().ymin -= checkerLen;
        lvl.area.back().ymax -= checkerLen;
    }

    if (key == GLFW_KEY_K && mods == GLFW_MOD_SHIFT && action == GLFW_PRESS)
    {
        if (lvl.area.empty()) return;
        lvl.area.back().ymax += checkerLen;
    }
    if (key == GLFW_KEY_K && mods == GLFW_MOD_CONTROL && action == GLFW_PRESS)
    {
        if (lvl.area.empty()) return;
        lvl.area.back().ymin += checkerLen;
        lvl.area.back().ymax += checkerLen;
    }

    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        if (lvl.startIdx<0 || lvl.startIdx>=(int)lvl.area.size())
        {
            fputs("No startIdx. Exiting.\n", stdout);
            return;
        }
        if (lvl.endIdx<0 || lvl.endIdx>=(int)lvl.area.size())
        {
            fputs("No endIdx. Exiting.\n", stdout);
            return;
        }
        if (lvl.player.xmin==INVALID)
        {
            fputs("No player. Exiting.\n", stdout);
            return;
        }

        fputs("Name of level: ", stdout);
        char buf[32];
        scanf("%s", buf);

        char filename[64];
        snprintf(filename, 64, "../etc/%s.inl", buf);

        FILE* fp = fopen(filename, "w");
        if (!fp)
        {
            fprintf(stderr, "cannot open '%s'\n", buf);
            fprintf(stderr, "errno: %s\ntry again.\n", strerror(errno));
            return;
        }

        fputs("// Automatically generated code.\n", fp);
        fputs("static LevelDscr LoadLevel()\n{\n", fp);
        fputs("LevelDscr lvl;\n", fp);
        for (int i = 0; i < (int)lvl.area.size(); ++i)
        {
            fprintf(fp, "lvl.area.emplace_back(%d, %d, %d, %d);\n",
                    lvl.area[i].xmin, lvl.area[i].xmax,
                    lvl.area[i].ymin, lvl.area[i].ymax);
        }
        fprintf(fp, "lvl.startIdx=%d;\n", lvl.startIdx);
        fprintf(fp, "lvl.endIdx=%d;\n", lvl.endIdx);
        for (int i = 0; i < (int)lvl.enemies.size(); ++i)
        {
            fprintf(fp, "lvl.enemies.emplace_back(%d, %d, %d, %d, Direction(%d));\n",
                    lvl.enemies[i].pos.x, lvl.enemies[i].pos.y,
                    lvl.enemies[i].from, lvl.enemies[i].to,
                    lvl.enemies[i].dir);
        }

        int posx = (lvl.player.xmax+lvl.player.xmin) / 2;
        int posy = (lvl.player.ymax+lvl.player.ymin) / 2;
        fprintf(fp, "lvl.player=IBox(%d-playerRad, %d+playerRad, %d-playerRad, %d+playerRad);\n",
                posx, posx, posy, posy);
        fprintf(fp, "return lvl;\n}\n");
        fclose(fp);
    }
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
    {
        if (!undo.empty())
        {
            fprintf(stderr, "you have an unfinished level\n");
        }
        else lvl = LevelDscr::Get();
    }
}

void LevelEditor()
{
    GLFWwindow* window = InitializeGLFW(width, height);
    Color* fb = (Color*)glfwGetWindowUserPointer(window);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    InitializeGL();

    std::vector<EnemyPath> enemies;
    while (!glfwWindowShouldClose(window))
    {
        DrawLevel(fb, lvl);

        if (enemies.size() != lvl.enemies.size())
        {
            enemies = lvl.enemies;
        }
        for (EnemyPath& e : enemies)
        {
            AdvanceEnemy(e);
            DrawEnemy(fb, e.pos);
        }

        if (lvl.player.xmax-lvl.player.xmin+1 == playerSize)
        {
            DrawPlayer(fb, lvl.player);
        }

        SwapBuffers(window, (float*)fb);
        glfwPollEvents();
    }
}
