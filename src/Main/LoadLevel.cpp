#include <HardestGame.h>
#include "../../etc/Default.inl"

const LevelDscr& LevelDscr::Get()
{
    static LevelDscr lvl = LoadLevel();
    return lvl;
}
