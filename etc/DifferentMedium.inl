// Automatically generated code.
static LevelDscr LoadLevel()
{
LevelDscr lvl;
lvl.area.emplace_back(495, 584, 495, 584);
lvl.area.emplace_back(225, 494, 495, 539);
lvl.area.emplace_back(585, 899, 495, 539);
lvl.area.emplace_back(225, 269, 225, 494);
lvl.area.emplace_back(270, 359, 180, 269);
lvl.area.emplace_back(855, 899, 360, 494);
lvl.area.emplace_back(315, 854, 405, 449);
lvl.area.emplace_back(315, 359, 360, 404);
lvl.area.emplace_back(315, 359, 315, 359);
lvl.area.emplace_back(360, 854, 315, 359);
lvl.area.emplace_back(810, 854, 270, 314);
lvl.area.emplace_back(810, 854, 225, 269);
lvl.area.emplace_back(540, 809, 225, 269);
lvl.area.emplace_back(540, 584, 135, 224);
lvl.area.emplace_back(405, 539, 135, 179);
lvl.area.emplace_back(405, 449, 180, 224);
lvl.area.emplace_back(360, 404, 180, 224);
lvl.startIdx=0;
lvl.endIdx=4;
lvl.enemies.emplace_back(247, 337, 292, 337, Direction(2));
lvl.player=IBox(533-playerRad, 533+playerRad, 560-playerRad, 560+playerRad);
return lvl;
}
