// Automatically generated code.
static LevelDscr LoadLevel()
{
LevelDscr lvl;
lvl.area.emplace_back(135, 269, 135, 539);
lvl.area.emplace_back(270, 359, 135, 179);
lvl.area.emplace_back(45, 134, 585, 674);
lvl.area.emplace_back(135, 269, 630, 674);
lvl.area.emplace_back(225, 269, 540, 629);
lvl.area.emplace_back(270, 449, 495, 539);
lvl.area.emplace_back(405, 854, 135, 494);
lvl.area.emplace_back(810, 1034, 495, 539);
lvl.area.emplace_back(945, 1169, 135, 494);
lvl.area.emplace_back(360, 404, 45, 179);
lvl.startIdx=2;
lvl.endIdx=8;
lvl.enemies.emplace_back(697, 472, 427, 472, Direction(2));
lvl.enemies.emplace_back(697, 427, 382, 427, Direction(2));
lvl.enemies.emplace_back(697, 382, 337, 382, Direction(2));
lvl.enemies.emplace_back(697, 337, 652, 697, Direction(4));
lvl.enemies.emplace_back(652, 337, 607, 652, Direction(4));
lvl.enemies.emplace_back(607, 337, 562, 607, Direction(4));
lvl.enemies.emplace_back(562, 337, 517, 562, Direction(4));
lvl.enemies.emplace_back(517, 337, 472, 517, Direction(4));
lvl.enemies.emplace_back(472, 337, 427, 472, Direction(4));
lvl.player=IBox(87-playerRad, 87+playerRad, 632-playerRad, 632+playerRad);
return lvl;
}
