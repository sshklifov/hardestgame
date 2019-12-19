#include <HardestGame.h>
#include "../../etc/MultiPathEasier.inl"

const LevelDscr& LevelDscr::Get()
{
    static LevelDscr lvl = LoadLevel();
    return lvl;
}
