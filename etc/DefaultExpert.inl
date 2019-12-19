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
lvl.enemies.emplace_back(652, 427, 472, 652, Direction(4));
lvl.enemies.emplace_back(337, 427, 337, 562, Direction(8));
lvl.enemies.emplace_back(562, 382, 337, 562, Direction(4));
lvl.enemies.emplace_back(427, 382, 427, 652, Direction(8));
lvl.enemies.emplace_back(337, 337, 337, 562, Direction(8));
lvl.enemies.emplace_back(652, 337, 427, 652, Direction(4));
lvl.enemies.emplace_back(562, 292, 337, 562, Direction(4));
lvl.enemies.emplace_back(427, 292, 427, 652, Direction(8));
lvl.enemies.emplace_back(337, 247, 337, 562, Direction(8));
lvl.enemies.emplace_back(652, 247, 427, 652, Direction(4));
lvl.player=IBox(177-playerRad, 177+playerRad, 437-playerRad, 437+playerRad);
return lvl;
}
