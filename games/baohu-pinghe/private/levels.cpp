#include "levels.hpp"

namespace {

GameConfig baseConfig() {
  GameConfig c = GameConfig::defaults();
  c.surviveSec = 12.0;
  c.bankSlipFactor = 0.0;
  return c;
}

LevelConfig buildTrial() {
  GameConfig g = baseConfig();
  g.tigerSpeed *= 0.9;
  g.riverX = 17.0;
  g.playerStartX = 8.5;
  g.tigerStartX = 3.5;
  return LevelConfig{"trial", "level.1.title", g, 6};
}

LevelConfig buildRising() {
  GameConfig g = baseConfig();
  g.riverX = 13.0;
  g.playerStartX = 7.0;
  g.tigerStartX = 3.5;
  g.walkSpeed *= 0.95;
  return LevelConfig{"rising", "level.2.title", g, 8};
}

LevelConfig buildBerserk() {
  GameConfig g = baseConfig();
  g.tigerSpeed *= 1.15;
  g.riverX = 15.0;
  g.playerStartX = 8.0;
  g.tigerStartX = 4.0;
  g.bankSlipFactor = 0.15;
  return LevelConfig{"berserk", "level.3.title", g, 6};
}

const LevelConfig kLevels[3] = {buildTrial(), buildRising(), buildBerserk()};

}  // namespace

int levelCount() { return 3; }

const LevelConfig& levelAt(int index) {
  if (index < 0) {
    index = 0;
  }
  if (index >= levelCount()) {
    index = levelCount() - 1;
  }
  return kLevels[index];
}
