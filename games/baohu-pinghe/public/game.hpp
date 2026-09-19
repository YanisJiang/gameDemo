#pragma once

enum class Outcome { Playing, Win, DeathMelee, DeathRiver };

struct GameConfig {
  double worldMinX = 0.0;
  double riverX = 16.0;
  double riverWidth = 3.0;
  double playerStartX = 8.0;
  double tigerStartX = 4.0;
  double meleeDist = 1.35;  // -10% 攻击范围
  double surviveSec = 15.0;
  double chargeFullSec = 0.83;
  double tigerSpeed = 2.16;      // -10%
  double walkSpeed = 1.85;       // +10% 更容易拉开
  double chargeWalkFactor = 0.5;
  double chargeTigerSpeedFactor = 1.15;  // 蓄力时虎加速略减
  double dashTigerSpeedFactor = 0.9;
  double dashRange = 4.2;        // +5% 更好打到
  double knockbackRange = 3.85;  // +10% 击退
  double recoilRange = 2.8;
  double recoilBaselineCharge = 0.75;
  double recoilFullFactor = 1.4; // 满蓄反冲略减，少落水
  double hitRadius = 1.7;
  double impactAnimSec = 0.28;
  double bankSlipFactor = 0.0;  // 命中反冲后再滑一段（第3关）

  static GameConfig defaults() { return GameConfig{}; }
};

struct GameState {
  double playerX = 0.0;
  double tigerX = 0.0;
  double timeLeft = 0.0;
  double charge01 = 0.0;
  bool charging = false;
  bool walkingAway = false;
  Outcome outcome = Outcome::Playing;

  // Visual-only impact / dash feedback
  double animSec = 0.0;
  double animDur = 0.0;
  double animPlayerFrom = 0.0;
  double animPlayerTo = 0.0;
  double animTigerFrom = 0.0;
  double animTigerTo = 0.0;
  bool impactHit = false;
};

GameState makeInitialState(const GameConfig& config);
void setCharging(GameState& state, const GameConfig& config, bool down);
void setWalkingAway(GameState& state, bool down);
void tryDash(GameState& state, const GameConfig& config);
void tick(GameState& state, const GameConfig& config, double dt);

// Display helpers: positions including in-progress dash/impact lerp
double displayPlayerX(const GameState& state);
double displayTigerX(const GameState& state);
