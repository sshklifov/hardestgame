#include <HardestGame.h>
#include "../../etc/DefaultExpert.inl"

const LevelDscr& LevelDscr::Get()
{
    static LevelDscr lvl = LoadLevel();
    return lvl;
}
