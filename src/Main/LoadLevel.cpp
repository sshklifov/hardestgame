#include "HardestGame.h"

const float offx = checkerLen*2;
const float offy = checkerLen*3;

static void OffsetIBox(IBox& b)
{
    b.xmin += offx;
    b.xmax += offx;
    b.ymin += offy;
    b.ymax += offy;
}

static void OffsetEnemyPath(EnemyPath& e)
{
    e.minx += offx;
    e.maxx += offx;
    e.pos.x += offx;
    e.pos.y += offy;
};

static LevelDscr LoadLevel()
{
    LevelDscr lvl;

    lvl.area.emplace_back(0, 3*checkerLen-1, 0, 7*checkerLen-1);
    /* lvl.area.emplace_back(3*checkerLen, 5*checkerLen-1, 6*checkerLen, 7*checkerLen-1); */
    /* lvl.area.emplace_back(4*checkerLen, 5*checkerLen-1, 5*checkerLen, 6*checkerLen-1); */
    lvl.area.emplace_back(3*checkerLen, 5*checkerLen-1, 0, 1*checkerLen-1);
    lvl.area.emplace_back(4*checkerLen, 12*checkerLen-1, 1*checkerLen, 6*checkerLen-1);
    lvl.area.emplace_back(11*checkerLen, 13*checkerLen-1, 6*checkerLen, 7*checkerLen-1);
    lvl.area.emplace_back(13*checkerLen, 16*checkerLen-1, 0, 7*checkerLen-1);
    for (IBox& b : lvl.area)
    {
        OffsetIBox(b);
    }

    lvl.startIdx = 0;
    lvl.endIdx = lvl.area.size()-1;

    lvl.enemies.push_back({{11*checkerLen, int(1.5*checkerLen)},
            4*checkerLen+enemyRad, 12*checkerLen-enemyRad, LEFT});
    lvl.enemies.push_back({{5*checkerLen, int(2.5*checkerLen)},
            4*checkerLen+enemyRad, 12*checkerLen-enemyRad, RIGHT});
    lvl.enemies.push_back({{11*checkerLen, int(3.5*checkerLen)},
            4*checkerLen+enemyRad, 12*checkerLen-enemyRad, LEFT});
    lvl.enemies.push_back({{5*checkerLen, int(4.5*checkerLen)},
            4*checkerLen+enemyRad, 12*checkerLen-enemyRad, RIGHT});
    lvl.enemies.push_back({{11*checkerLen, int(5.5*checkerLen)},
            4*checkerLen+enemyRad, 12*checkerLen-enemyRad, LEFT});
    for (EnemyPath& p : lvl.enemies)
    {
        OffsetEnemyPath(p);
    }

    int startx = (lvl.area[lvl.startIdx].xmin+lvl.area[lvl.startIdx].xmax) / 2;
    int starty = (lvl.area[lvl.startIdx].ymin+lvl.area[lvl.startIdx].ymax) / 2;
    lvl.player.xmin = startx-playerRad;
    lvl.player.xmax = startx+playerRad;
    lvl.player.ymin = starty-playerRad;
    lvl.player.ymax = starty+playerRad;

    return lvl;
}

const LevelDscr& LevelDscr::Get()
{
    static LevelDscr lvl = LoadLevel();
    return lvl;
}
