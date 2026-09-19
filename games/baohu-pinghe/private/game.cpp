#include "game.hpp"

#include <algorithm>
#include <cmath>

static void enforceTigerLeftOfPlayer(GameState& state, const GameConfig& config) {
  if (state.tigerX < config.worldMinX) {
    state.tigerX = config.worldMinX;
  }
  // 只禁止穿模到虎左侧，不要把人“顶”出攻击范围（否则会先贴上再判定，画面跟不上）
  if (state.playerX <= state.tigerX) {
    state.playerX = state.tigerX + 0.01;
  }
}

static void checkTerminal(GameState& state, const GameConfig& config) {
  if (state.outcome != Outcome::Playing) {
    return;
  }
  if (state.playerX >= config.riverX) {
    state.outcome = Outcome::DeathRiver;
    state.animSec = 0.0;
    return;
  }
  if (state.playerX - state.tigerX <= config.meleeDist + 1e-9) {
    state.outcome = Outcome::DeathMelee;
    state.animSec = 0.0;
    // 定格在贴身位置，避免插值还停在“没贴上”
    state.playerX = state.tigerX + config.meleeDist;
    return;
  }
  if (state.timeLeft <= 0.0) {
    state.timeLeft = 0.0;
    state.outcome = Outcome::Win;
    state.animSec = 0.0;
  }
}

GameState makeInitialState(const GameConfig& config) {
  GameState s;
  s.playerX = config.playerStartX;
  s.tigerX = config.tigerStartX;
  s.timeLeft = config.surviveSec;
  s.charge01 = 0.0;
  s.charging = false;
  s.walkingAway = false;
  s.outcome = Outcome::Playing;
  s.animSec = 0.0;
  s.animDur = 0.0;
  s.animPlayerFrom = s.playerX;
  s.animPlayerTo = s.playerX;
  s.animTigerFrom = s.tigerX;
  s.animTigerTo = s.tigerX;
  s.impactHit = false;
  enforceTigerLeftOfPlayer(s, config);
  return s;
}

static void beginAnim(GameState& state, const GameConfig& config, double fromP,
                      double toP, double fromT, double toT, bool hit) {
  state.animDur = config.impactAnimSec;
  state.animSec = config.impactAnimSec;
  state.animPlayerFrom = fromP;
  state.animPlayerTo = toP;
  state.animTigerFrom = fromT;
  state.animTigerTo = toT;
  state.impactHit = hit;
}

static double recoilMultiplier(double charge01, const GameConfig& config) {
  const double baseAt = std::clamp(config.recoilBaselineCharge, 0.05, 0.99);
  const double p = std::clamp(charge01, 0.0, 1.0);
  if (p <= baseAt) {
    return p / baseAt;  // 75% → 1.0
  }
  const double t = (p - baseAt) / (1.0 - baseAt);
  return 1.0 + t * (config.recoilFullFactor - 1.0);  // 100% → fullFactor
}

void tryDash(GameState& state, const GameConfig& config) {
  if (state.outcome != Outcome::Playing) {
    return;
  }

  const double p = std::clamp(state.charge01, 0.0, 1.0);
  state.charge01 = 0.0;
  state.charging = false;

  if (p <= 0.0) {
    return;
  }

  const double playerBefore = state.playerX;
  const double tigerBefore = state.tigerX;
  const double contactX = state.tigerX + config.hitRadius;
  const double dashDist = p * config.dashRange;
  const double reachNeeded = state.playerX - contactX;
  const double recoil =
      recoilMultiplier(p, config) * config.recoilRange;

  if (reachNeeded > 0.0 && dashDist >= reachNeeded) {
    // Hit: stop at contact, knock tiger left (clamped), recoil player right
    state.playerX = contactX;
    state.tigerX -= p * config.knockbackRange;
    if (state.tigerX < config.worldMinX) {
      state.tigerX = config.worldMinX;
    }
    state.playerX += recoil;
    if (config.bankSlipFactor > 0.0) {
      state.playerX += recoil * config.bankSlipFactor;
    }
    enforceTigerLeftOfPlayer(state, config);
    beginAnim(state, config, playerBefore, state.playerX, tigerBefore,
              state.tigerX, true);
  } else {
    // Miss: dash left but never cross to tiger's left
    state.playerX -= dashDist;
    enforceTigerLeftOfPlayer(state, config);
    beginAnim(state, config, playerBefore, state.playerX, tigerBefore,
              state.tigerX, false);
  }

  checkTerminal(state, config);
}

void setCharging(GameState& state, const GameConfig& config, bool down) {
  if (state.outcome != Outcome::Playing) {
    return;
  }

  if (down) {
    if (!state.charging) {
      state.charging = true;
      state.charge01 = 0.0;
    }
    return;
  }

  if (state.charging) {
    tryDash(state, config);
  }
}

void setWalkingAway(GameState& state, bool down) {
  if (state.outcome != Outcome::Playing) {
    state.walkingAway = false;
    return;
  }
  state.walkingAway = down;
}

void tick(GameState& state, const GameConfig& config, double dt) {
  if (state.outcome != Outcome::Playing) {
    return;
  }
  if (dt < 0.0) {
    dt = 0.0;
  }

  state.timeLeft -= dt;

  if (state.animSec > 0.0) {
    state.animSec = std::max(0.0, state.animSec - dt);
  }

  if (state.charging) {
    state.charge01 =
        std::min(1.0, state.charge01 + dt / config.chargeFullSec);
  }

  // 向河漂：蓄力时也继续向右，但减速；撞击动画期间暂停
  if (state.animSec <= 0.0) {
    const double speed = state.charging
                             ? config.walkSpeed * config.chargeWalkFactor
                             : config.walkSpeed;
    state.playerX += speed * dt;
  }

  // 蓄力时老虎加速；人冲刺动画期间老虎减速
  double tigerSpd = config.tigerSpeed;
  if (state.animSec > 0.0) {
    tigerSpd *= config.dashTigerSpeedFactor;
  } else if (state.charging) {
    tigerSpd *= config.chargeTigerSpeedFactor;
  }
  state.tigerX += tigerSpd * dt;
  enforceTigerLeftOfPlayer(state, config);

  checkTerminal(state, config);
}

double displayPlayerX(const GameState& state) {
  if (state.outcome != Outcome::Playing || state.animSec <= 0.0 ||
      state.animDur <= 0.0) {
    return state.playerX;
  }
  const double t = 1.0 - (state.animSec / state.animDur);
  const double u = t < 0.45 ? (t / 0.45) * 0.55
                            : 0.55 + ((t - 0.45) / 0.55) * 0.45;
  return state.animPlayerFrom + (state.animPlayerTo - state.animPlayerFrom) * u;
}

double displayTigerX(const GameState& state) {
  if (state.outcome != Outcome::Playing || state.animSec <= 0.0 ||
      state.animDur <= 0.0) {
    return state.tigerX;
  }
  const double t = 1.0 - (state.animSec / state.animDur);
  const double u = t < 0.45 ? 0.0 : (t - 0.45) / 0.55;
  return state.animTigerFrom + (state.animTigerTo - state.animTigerFrom) * u;
}
