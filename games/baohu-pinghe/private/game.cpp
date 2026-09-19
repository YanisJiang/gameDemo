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

static bool onBoat(const GameState& state, const GameConfig& config) {
  if (!config.boatEnabled) {
    return false;
  }
  return std::abs(state.playerX - state.boatX) <= config.boatHalfWidth + 1e-9;
}

static void syncBoat(GameState& state, const GameConfig& config) {
  if (config.boatEnabled) {
    state.boatX = state.riverX + config.boatOffset;
  }
}

static void checkTerminal(GameState& state, const GameConfig& config) {
  if (state.outcome != Outcome::Playing) {
    return;
  }
  if (state.playerX >= state.riverX) {
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
  s.riverX = config.riverX;
  s.tideRising = true;
  s.boatX = config.riverX + config.boatOffset;
  s.timeLeft = config.surviveSec;
  s.charge01 = 0.0;
  s.chargeRising = true;
  s.charging = false;
  s.walkingAway = false;
  s.spaceHeld = false;
  s.chargeHoldSec = 0.0;
  s.outcome = Outcome::Playing;
  s.animSec = 0.0;
  s.animDur = 0.0;
  s.animPlayerFrom = s.playerX;
  s.animPlayerTo = s.playerX;
  s.animTigerFrom = s.tigerX;
  s.animTigerTo = s.tigerX;
  s.impactHit = false;
  s.feintPhase = FeintPhase::Approach;
  s.feintTimer = config.feintEnabled ? config.feintInterval : 0.0;
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
  state.chargeHoldSec = 0.0;

  if (p <= 0.0) {
    return;
  }

  const double playerBefore = state.playerX;
  const double tigerBefore = state.tigerX;

  if (config.puppetTigerEnabled) {
    // 表象：人在冲刺；实际：虎扑向人（低蓄也有保底扑距）
    const double lunge =
        std::max(config.puppetLungeMin, p * config.puppetLungeRange);
    state.tigerX += lunge;
    const double contact = state.playerX - config.meleeDist;
    bool hit = false;
    if (state.tigerX >= contact - 1e-9) {
      state.tigerX = contact;
      hit = true;
    }
    if (state.tigerX < config.worldMinX) {
      state.tigerX = config.worldMinX;
    }
    beginAnim(state, config, playerBefore, state.playerX, tigerBefore,
              state.tigerX, hit);
    checkTerminal(state, config);
    return;
  }

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

  syncBoat(state, config);
  if (onBoat(state, config)) {
    state.outcome = Outcome::Win;
    state.animSec = 0.0;
    return;
  }

  checkTerminal(state, config);
}

void setCharging(GameState& state, const GameConfig& config, bool down) {
  if (state.outcome != Outcome::Playing) {
    return;
  }

  state.spaceHeld = down;

  if (down) {
    if (!state.charging) {
      state.charging = true;
      state.charge01 = 0.0;
      state.chargeRising = true;
      state.chargeHoldSec = 0.0;
    }
    return;
  }

  if (state.charging || state.charge01 > 0.0) {
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

  state.charging = state.spaceHeld;

  if (state.spaceHeld) {
    // 蓄力：0→1→0 循环；条=真实出手
    const double rate = dt / std::max(0.05, config.chargeFullSec);
    const double before = state.charge01;
    const bool wasRising = state.chargeRising;
    if (state.chargeRising) {
      state.charge01 += rate;
      if (state.charge01 >= 1.0) {
        state.charge01 = 1.0;
        state.chargeRising = false;
      }
    } else {
      state.charge01 -= rate;
      if (state.charge01 <= 0.0) {
        state.charge01 = 0.0;
        state.chargeRising = true;
      }
    }

    bool slip = false;
    if (config.madSlipEnabled) {
      // 第一次顶满：上升沿碰到 1.0 → 手滑（轻点永远碰不到）
      if (config.madSlipOnPeak && wasRising && before < 1.0 &&
          state.charge01 >= 1.0 - 1e-12) {
        slip = true;
      }
      if (config.madSlipHoldSec > 0.0) {
        state.chargeHoldSec += dt;
        if (state.chargeHoldSec >= config.madSlipHoldSec &&
            state.charge01 > 0.0) {
          slip = true;
        }
      }
    }

    if (slip) {
      tryDash(state, config);
      if (state.outcome == Outcome::Playing && state.spaceHeld) {
        state.chargeRising = true;
      }
    }
  } else {
    state.chargeHoldSec = 0.0;
  }

  // Tide: flood (left) then ebb (right), loop
  if (config.tideEnabled) {
    const double maxX = config.riverX;
    const double minX = config.tideMinRiverX;
    if (state.tideRising) {
      state.riverX -= config.tideSpeed * dt;
      if (state.riverX <= minX) {
        state.riverX = minX;
        state.tideRising = false;
      }
    } else {
      state.riverX += config.tideSpeed * dt;
      if (state.riverX >= maxX) {
        state.riverX = maxX;
        state.tideRising = true;
      }
    }
  }

  syncBoat(state, config);

  // Feint state machine
  if (config.feintEnabled) {
    state.feintTimer -= dt;
    if (state.feintTimer <= 0.0) {
      if (state.feintPhase == FeintPhase::Approach) {
        state.feintPhase = FeintPhase::Windup;
        state.feintTimer = config.feintWindupSec;
      } else if (state.feintPhase == FeintPhase::Windup) {
        state.feintPhase = FeintPhase::Lunge;
        state.feintTimer = config.feintLungeSec;
      } else {
        state.feintPhase = FeintPhase::Approach;
        state.feintTimer = config.feintInterval;
      }
    }
  }

  // Walk
  if (state.animSec <= 0.0) {
    if (config.puppetTigerEnabled) {
      // 没按：往河走；按住：往虎撞（像疯子）——在中间带点按才能活
      if (state.spaceHeld) {
        state.playerX -= config.puppetChargeRushSpeed * dt;
      } else {
        state.playerX += config.walkSpeed * dt;
      }
    } else {
      double speed = state.charging ? config.walkSpeed * config.chargeWalkFactor
                                    : config.walkSpeed;
      if (config.madSlipEnabled && state.charging) {
        speed *= config.madChargeWalkBoost;
      }
      state.playerX += speed * dt;
    }
  }

  // Tiger movement
  double tigerSpd = config.tigerSpeed;
  if (config.puppetTigerEnabled) {
    // 蓄力不改变虎步态；只有松手虎扑突变
    if (state.animSec > 0.0) {
      tigerSpd = 0.0;
    } else {
      tigerSpd = config.tigerSpeed;
    }
  } else if (config.feintEnabled) {
    if (state.feintPhase == FeintPhase::Windup) {
      tigerSpd = -config.feintRetreatSpeed;
    } else if (state.feintPhase == FeintPhase::Lunge) {
      tigerSpd = config.tigerSpeed * config.feintLungeFactor;
    } else if (state.charging) {
      tigerSpd *= config.chargeTigerSpeedFactor;
    }
  } else if (state.charging) {
    tigerSpd *= config.chargeTigerSpeedFactor;
  }

  if (!config.puppetTigerEnabled && state.animSec > 0.0) {
    tigerSpd *= config.dashTigerSpeedFactor;
  }

  state.tigerX += tigerSpd * dt;
  if (state.tigerX < config.worldMinX) {
    state.tigerX = config.worldMinX;
  }
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
