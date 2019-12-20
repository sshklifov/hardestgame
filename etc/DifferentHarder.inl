// Automatically generated code.
static LevelDscr LoadLevel()
{
LevelDscr lvl;
lvl.area.emplace_back(180, 359, 495, 584);
lvl.area.emplace_back(315, 359, 180, 494);
lvl.area.emplace_back(360, 809, 495, 674);
lvl.area.emplace_back(810, 989, 630, 674);
lvl.area.emplace_back(990, 1124, 585, 674);
lvl.area.emplace_back(360, 1169, 180, 224);
lvl.area.emplace_back(1125, 1169, 225, 449);
lvl.area.emplace_back(945, 1124, 405, 449);
lvl.area.emplace_back(945, 989, 450, 539);
lvl.area.emplace_back(990, 1169, 495, 539);
lvl.area.emplace_back(1080, 1124, 540, 584);
lvl.startIdx=0;
lvl.endIdx=4;
lvl.enemies.emplace_back(787, 652, 787, 832, Direction(8));
lvl.player=IBox(224-playerRad, 224+playerRad, 554-playerRad, 554+playerRad);
return lvl;
}
