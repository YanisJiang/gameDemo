#pragma once

enum class Outcome { Playing, Win, DeathMelee, DeathRiver };

enum class FeintPhase { Approach, Windup, Lunge };

struct GameConfig {
  double worldMinX = 0.0;
  double riverX = 16.0;
  double riverWidth = 3.0;
  double playerStartX = 8.0;
  double tigerStartX = 4.0;
  double meleeDist = 1.35;
  double surviveSec = 12.0;
  double chargeFullSec = 0.83;
  double tigerSpeed = 2.16;
  double walkSpeed = 1.85;
  double chargeWalkFactor = 0.5;
  double chargeTigerSpeedFactor = 1.15;
  double dashTigerSpeedFactor = 0.9;
  double dashRange = 4.2;
  double knockbackRange = 3.85;
  double recoilRange = 2.8;
  double recoilBaselineCharge = 0.75;
  double recoilFullFactor = 1.4;
  double hitRadius = 1.7;
  double impactAnimSec = 0.28;
  double bankSlipFactor = 0.0;

  // Tiger feint (level 3)
  bool feintEnabled = false;
  double feintInterval = 4.0;       // Approach 持续多久后进入 Windup
  double feintWindupSec = 0.55;     // 停顿/微退
  double feintLungeSec = 0.4;       // 猛扑持续
  double feintRetreatSpeed = 0.35;  // Windup 微退（向左，减小 x）
  double feintLungeFactor = 2.4;    // Lunge 相对 tigerSpeed 倍率

  // Tide (level 3): 涨潮/退潮循环，河界在 max↔min 间摆动
  bool tideEnabled = false;
  double tideSpeed = 0.22;          // riverX 每秒变化量
  double tideMinRiverX = 0.0;       // 涨满下限（河界最左）
  int tideExtraRiverSlots = 4;      // 涨满时河面最多多占几格

  // Boat (level 4): 贴河界漂，随涨退潮
  bool boatEnabled = false;
  double boatOffset = 0.9;          // boatX = riverX + offset
  double boatHalfWidth = 0.65;

  // Mad slip (level 5): 操作照旧；条第一次顶满强制出手
  bool madSlipEnabled = false;
  bool madSlipOnPeak = true;    // 蓄力首次到 100% 必手滑
  double madSlipHoldSec = 0.0;  // >0 时：按住超过此时长也会手滑（备用）
  double madChargeWalkBoost = 1.15;

  // Puppet tiger (level 6): 以为控人；蓄力人往虎撞；松手虎扑
  bool puppetTigerEnabled = false;
  double puppetLungeRange = 3.2;          // 松手虎扑 ∝ 蓄力
  double puppetLungeMin = 0.85;           // 再低的蓄也会扑一截（防「等到0再放」白嫖）
  double puppetChargeRushSpeed = 1.85;    // 按住时人向左撞虎（像疯子）

  static GameConfig defaults() { return GameConfig{}; }
};

struct GameState {
  double playerX = 0.0;
  double tigerX = 0.0;
  double riverX = 0.0;  // 动态河界（涨退潮会改）
  bool tideRising = true;  // true=涨潮(河界左移)，false=退潮
  double boatX = 0.0;      // 船心（随河界）
  double timeLeft = 0.0;
  double charge01 = 0.0;
  bool chargeRising = true;  // 蓄力高低循环方向
  bool charging = false;
  bool walkingAway = false;
  bool spaceHeld = false;
  double chargeHoldSec = 0.0;  // 本次按住蓄力已持续多久
  Outcome outcome = Outcome::Playing;

  double animSec = 0.0;
  double animDur = 0.0;
  double animPlayerFrom = 0.0;
  double animPlayerTo = 0.0;
  double animTigerFrom = 0.0;
  double animTigerTo = 0.0;
  bool impactHit = false;

  FeintPhase feintPhase = FeintPhase::Approach;
  double feintTimer = 0.0;  // 当前阶段剩余时间
};

GameState makeInitialState(const GameConfig& config);
void setCharging(GameState& state, const GameConfig& config, bool down);
void setWalkingAway(GameState& state, bool down);
void tryDash(GameState& state, const GameConfig& config);
void tick(GameState& state, const GameConfig& config, double dt);

double displayPlayerX(const GameState& state);
double displayTigerX(const GameState& state);
