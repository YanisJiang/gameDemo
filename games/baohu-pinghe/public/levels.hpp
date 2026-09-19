#pragma once

#include "game.hpp"

struct LevelConfig {
  const char* id;
  const char* titleKey;
  GameConfig game;
  int riverDisplaySlots;  // 右侧河面格子数
};

int levelCount();
const LevelConfig& levelAt(int index);
