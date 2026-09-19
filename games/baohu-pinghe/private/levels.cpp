#include "levels.hpp"

namespace {

LevelConfig buildTutorial() {
  GameConfig g = GameConfig::defaults();
  // 仍能教学，但要有紧迫感
  g.surviveSec = 11.0;
  g.tigerSpeed = 1.55;
  g.walkSpeed = 1.7;
  g.riverX = 16.5;
  g.playerStartX = 8.5;
  g.tigerStartX = 3.2;
  g.meleeDist = 1.25;
  g.bankSlipFactor = 0.0;
  g.recoilFullFactor = 1.28;
  g.feintEnabled = false;
  g.tideEnabled = false;
  return LevelConfig{"tutorial", "level.1.title", "hud.hint.tutorial", g, 6};
}

LevelConfig buildPressure() {
  GameConfig g = GameConfig::defaults();
  g.surviveSec = 12.0;
  // 再 -5% 难度（相对上一版）——本关保持，不加压
  g.tigerSpeed = 2.30;
  g.walkSpeed = 1.88;
  g.riverX = 13.8;
  g.playerStartX = 7.2;
  g.tigerStartX = 3.2;
  g.meleeDist = 1.40;
  g.bankSlipFactor = 0.20;
  g.recoilRange = 2.89;
  g.recoilFullFactor = 1.49;
  g.chargeTigerSpeedFactor = 1.23;
  g.feintEnabled = false;
  g.tideEnabled = false;
  return LevelConfig{"pressure", "level.2.title", "hud.hint.pressure", g, 8};
}

LevelConfig buildFeintTide() {
  GameConfig g = GameConfig::defaults();
  // 假扑更密、涨潮更凶：不懂节奏很难耗时
  g.surviveSec = 14.0;
  g.tigerSpeed = 2.05;
  g.walkSpeed = 1.9;
  g.riverX = 14.8;
  g.playerStartX = 7.6;
  g.tigerStartX = 3.4;
  g.meleeDist = 1.38;
  g.bankSlipFactor = 0.14;
  g.recoilFullFactor = 1.42;
  g.feintEnabled = true;
  g.feintInterval = 2.8;
  g.feintWindupSec = 0.48;
  g.feintLungeSec = 0.42;
  g.feintRetreatSpeed = 0.35;
  g.feintLungeFactor = 2.9;
  g.tideEnabled = true;
  g.tideSpeed = 0.7;
  g.tideMinRiverX = 10.5;
  g.tideExtraRiverSlots = 6;
  return LevelConfig{"feint_tide", "level.3.title", "hud.hint.feint", g, 6};
}

LevelConfig buildFerrySecret() {
  GameConfig g = GameConfig::defaults();
  // 纯耗时近乎不可过；反冲上船才是活路（捷径发现后可以很轻松）
  g.surviveSec = 16.0;
  g.tigerSpeed = 2.35;
  g.walkSpeed = 2.0;
  g.riverX = 13.5;
  g.playerStartX = 7.0;
  g.tigerStartX = 3.4;
  g.meleeDist = 1.42;
  g.bankSlipFactor = 0.28;
  g.recoilRange = 3.35;
  g.recoilFullFactor = 1.55;
  g.chargeTigerSpeedFactor = 1.28;
  g.feintEnabled = true;
  g.feintInterval = 2.6;
  g.feintWindupSec = 0.42;
  g.feintLungeSec = 0.45;
  g.feintRetreatSpeed = 0.5;
  g.feintLungeFactor = 3.0;
  g.tideEnabled = true;
  g.tideSpeed = 0.75;
  g.tideMinRiverX = 10.0;
  g.tideExtraRiverSlots = 6;
  g.boatEnabled = true;
  g.boatOffset = 0.75;
  g.boatHalfWidth = 0.32;  // 只有落在舟这一格才算，河里旁边不算
  return LevelConfig{"ferry_secret", "level.4.title", "hud.hint.ferry", g, 6};
}

LevelConfig buildMadSlip() {
  GameConfig g = GameConfig::defaults();
  // 贪满蓄必手滑葬送；必须学会轻点——纯莽耗时很难
  g.surviveSec = 15.0;
  g.tigerSpeed = 2.4;
  g.walkSpeed = 1.95;
  g.riverX = 13.8;
  g.playerStartX = 7.2;
  g.tigerStartX = 3.4;
  g.meleeDist = 1.4;
  g.bankSlipFactor = 0.22;
  g.recoilRange = 3.0;
  g.recoilFullFactor = 1.5;
  g.chargeFullSec = 0.5;
  g.chargeTigerSpeedFactor = 1.3;
  g.feintEnabled = true;
  g.feintInterval = 2.7;
  g.feintWindupSec = 0.42;
  g.feintLungeSec = 0.4;
  g.feintRetreatSpeed = 0.4;
  g.feintLungeFactor = 2.85;
  g.tideEnabled = false;
  g.boatEnabled = false;
  g.madSlipEnabled = true;
  g.madSlipOnPeak = true;
  g.madSlipHoldSec = 0.0;
  g.madChargeWalkBoost = 1.35;
  return LevelConfig{"mad_slip", "level.5.title", "hud.hint.mad", g, 6};
}

LevelConfig buildTigerPuppet() {
  GameConfig g = GameConfig::defaults();
  // 当普通人猛冲/猛蓄会速死；等条降轻放仍可过，但河与虎都更紧
  g.surviveSec = 13.0;
  g.tigerSpeed = 0.42;
  g.walkSpeed = 1.75;              // 没按：更快落水压力
  g.riverX = 15.0;
  g.playerStartX = 8.8;
  g.tigerStartX = 3.0;
  g.meleeDist = 1.38;
  g.chargeFullSec = 0.9;
  g.feintEnabled = false;
  g.tideEnabled = false;
  g.boatEnabled = false;
  g.madSlipEnabled = false;
  g.puppetTigerEnabled = true;
  g.puppetLungeRange = 3.0;
  g.puppetLungeMin = 0.45;
  g.puppetChargeRushSpeed = 1.55;  // 仍能撑过一圈下降，但不能发呆
  return LevelConfig{"tiger_puppet", "level.6.title", "hud.hint.puppet", g, 6};
}

const LevelConfig kLevels[6] = {buildTutorial(), buildPressure(),
                                buildFeintTide(), buildFerrySecret(),
                                buildMadSlip(), buildTigerPuppet()};

}  // namespace

int levelCount() { return 6; }

const LevelConfig& levelAt(int index) {
  if (index < 0) {
    index = 0;
  }
  if (index >= levelCount()) {
    index = levelCount() - 1;
  }
  return kLevels[index];
}
