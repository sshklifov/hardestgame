// Automatically generated code.
static LevelDscr LoadLevel()
{
LevelDscr lvl;
lvl.area.emplace_back(90, 269, 180, 494);
lvl.area.emplace_back(270, 359, 180, 224);
lvl.area.emplace_back(315, 674, 225, 449);
lvl.area.emplace_back(630, 719, 450, 494);
lvl.area.emplace_back(720, 899, 180, 494);
lvl.startIdx=0;
lvl.endIdx=4;
lvl.enemies.emplace_back(652, 247, 337, 652, Direction(4));
lvl.enemies.emplace_back(337, 292, 337, 652, Direction(8));
lvl.enemies.emplace_back(652, 337, 337, 652, Direction(4));
lvl.enemies.emplace_back(337, 382, 337, 652, Direction(8));
lvl.enemies.emplace_back(652, 427, 337, 652, Direction(4));
lvl.enemies.emplace_back(337, 427, 247, 427, Direction(2));
lvl.enemies.emplace_back(427, 247, 247, 427, Direction(1));
lvl.enemies.emplace_back(517, 427, 247, 427, Direction(2));
lvl.enemies.emplace_back(607, 247, 247, 427, Direction(1));
lvl.enemies.emplace_back(652, 472, 247, 472, Direction(2));
lvl.player=IBox(177-playerRad, 177+playerRad, 437-playerRad, 437+playerRad);
return lvl;
}
