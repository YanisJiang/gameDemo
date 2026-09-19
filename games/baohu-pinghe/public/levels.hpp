#pragma once

#include "game.hpp"

struct LevelConfig {
  const char* id;
  const char* titleKey;
  const char* hintKey;
  GameConfig game;
  int riverDisplaySlots;
};

int levelCount();
const LevelConfig& levelAt(int index);
