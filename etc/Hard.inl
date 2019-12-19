// Automatically generated code.
static LevelDscr LoadLevel()
{
LevelDscr lvl;
lvl.area.emplace_back(45, 224, 90, 224);
lvl.area.emplace_back(225, 494, 90, 179);
lvl.area.emplace_back(495, 629, 90, 179);
lvl.area.emplace_back(270, 359, 180, 629);
lvl.area.emplace_back(360, 629, 540, 629);
lvl.area.emplace_back(540, 629, 180, 539);
lvl.startIdx=0;
lvl.endIdx=2;
lvl.enemies.emplace_back(472, 157, 382, 472, Direction(4));
lvl.enemies.emplace_back(472, 112, 382, 472, Direction(4));
lvl.player=IBox(83, 107, 180, 204);
return lvl;
}
