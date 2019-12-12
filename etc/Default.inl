// Automatically generated code.
static LevelDscr LoadLevel()
{
LevelDscr lvl;
lvl.area.emplace_back(135, 269, 180, 494);
lvl.area.emplace_back(270, 359, 180, 224);
lvl.area.emplace_back(315, 674, 225, 449);
lvl.area.emplace_back(630, 719, 450, 494);
lvl.area.emplace_back(720, 854, 180, 494);
lvl.startIdx=0;
lvl.endIdx=4;
lvl.enemies.emplace_back(652, 247, 337, 652, Direction(4));
lvl.enemies.emplace_back(337, 292, 337, 652, Direction(8));
lvl.enemies.emplace_back(652, 337, 337, 652, Direction(4));
lvl.enemies.emplace_back(337, 382, 337, 652, Direction(8));
lvl.enemies.emplace_back(652, 427, 337, 652, Direction(4));
lvl.player=IBox(185, 209, 394, 418);
return lvl;
}
