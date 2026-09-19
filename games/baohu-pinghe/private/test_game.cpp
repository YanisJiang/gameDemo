#include "game.hpp"
#include "i18n.hpp"
#include "levels.hpp"

#include <cassert>
#include <cmath>
#include <cstring>

int main() {
  // i18n
  {
    setLocale(Locale::Zh);
    assert(std::strcmp(tr("end.melee"), "end.melee") != 0);
    setLocale(Locale::En);
    const char* en = tr("end.melee");
    assert(std::strstr(en, "Mauled") != nullptr);
    assert(std::strcmp(tr("missing.key.xyz"), "missing.key.xyz") == 0 ||
           tr("missing.key.xyz") != nullptr);
  }

  setLocale(Locale::Zh);
  assert(levelCount() == 6);
  assert(levelAt(0).game.surviveSec == 11.0);
  assert(levelAt(1).game.surviveSec == 12.0);  // L2 保持
  assert(levelAt(2).game.bankSlipFactor > 0.0);
  assert(levelAt(3).game.boatEnabled);
  assert(levelAt(4).game.madSlipEnabled);
  assert(levelAt(5).game.puppetTigerEnabled);

  const GameConfig c = GameConfig::defaults();

  {
    GameState s = makeInitialState(c);
    const double tigerBefore = s.tigerX;
    s.charge01 = 1.0;
    tryDash(s, c);
    assert(s.tigerX <= tigerBefore);
    assert(s.playerX > s.tigerX);
  }

  // Bank slip increases recoil distance
  {
    GameConfig noSlip = c;
    noSlip.bankSlipFactor = 0.0;
    noSlip.dashRange = 10.0;
    GameConfig slip = noSlip;
    slip.bankSlipFactor = 0.15;

    GameState a = makeInitialState(noSlip);
    a.tigerX = 2.0;
    a.playerX = 8.0;
    a.charge01 = 1.0;
    tryDash(a, noSlip);

    GameState b = makeInitialState(slip);
    b.tigerX = 2.0;
    b.playerX = 8.0;
    b.charge01 = 1.0;
    tryDash(b, slip);

    assert(a.impactHit);
    assert(b.impactHit);
    assert(b.playerX > a.playerX + 0.05);
  }

  {
    GameState s = makeInitialState(c);
    s.tigerX = 10.0;
    s.playerX = 14.0;
    s.charge01 = 1.0;
    tryDash(s, c);
    assert(s.playerX >= s.tigerX + c.meleeDist - 1e-9);
  }

  {
    GameState s = makeInitialState(c);
    const double before = s.playerX;
    tick(s, c, 0.5);
    assert(s.playerX > before);
  }

  {
    GameState s = makeInitialState(c);
    s.playerX = c.riverX - 0.05;
    s.riverX = c.riverX;
    s.tigerX = 0.0;
    const double need = 0.2 / (c.walkSpeed > 0.01 ? c.walkSpeed : 0.01);
    tick(s, c, need + 0.05);
    assert(s.outcome == Outcome::DeathRiver);
  }

  {
    GameState s = makeInitialState(c);
    s.tigerX = s.playerX - 0.5;
    tick(s, c, 0.01);
    assert(s.outcome == Outcome::DeathMelee);
  }

  {
    GameState s = makeInitialState(c);
    s.timeLeft = 0.0;
    tick(s, c, 0.01);
    assert(s.outcome == Outcome::Win);
  }

  // Feint enters windup after interval
  {
    GameConfig f = c;
    f.feintEnabled = true;
    f.feintInterval = 0.2;
    f.feintWindupSec = 0.5;
    f.tideEnabled = false;
    GameState s = makeInitialState(f);
    tick(s, f, 0.25);
    assert(s.feintPhase == FeintPhase::Windup);
  }

  // Tide floods then ebbs in a loop
  {
    GameConfig t = c;
    t.tideEnabled = true;
    t.tideSpeed = 1.0;
    t.riverX = 16.0;
    t.tideMinRiverX = 14.0;
    t.walkSpeed = 0.0;
    t.tigerSpeed = 0.0;
    t.surviveSec = 30.0;
    GameState s = makeInitialState(t);
    const double before = s.riverX;
    tick(s, t, 0.3);
    assert(s.riverX < before);
    assert(s.tideRising);
    tick(s, t, 3.0);  // 涨满
    assert(s.riverX <= t.tideMinRiverX + 1e-9);
    assert(!s.tideRising);
    tick(s, t, 3.0);  // 退潮
    assert(s.riverX >= t.riverX - 1e-9);
    assert(s.tideRising);
  }

  // Charge oscillates low→high→low while held
  {
    GameConfig g = c;
    g.chargeFullSec = 1.0;
    GameState s = makeInitialState(g);
    setCharging(s, g, true);
    tick(s, g, 0.5);
    assert(s.charge01 > 0.4 && s.charge01 < 0.6);
    tick(s, g, 0.5);
    assert(s.charge01 > 0.95);
    assert(!s.chargeRising);
    tick(s, g, 0.5);
    assert(s.charge01 > 0.4 && s.charge01 < 0.6);
    tick(s, g, 0.5);
    assert(s.charge01 < 0.05);
    assert(s.chargeRising);
  }

  // Recoil onto boat → Win; walk into river (even at boat) → DeathRiver
  {
    GameConfig g = c;
    g.boatEnabled = true;
    g.boatOffset = 0.9;
    g.boatHalfWidth = 0.65;
    g.bankSlipFactor = 0.0;
    g.dashRange = 10.0;
    g.knockbackRange = 0.0;
    g.hitRadius = 1.0;
    g.recoilRange = 3.0;
    g.recoilBaselineCharge = 0.5;
    g.recoilFullFactor = 1.0;
    g.riverX = 10.0;
    g.playerStartX = 8.0;
    g.tigerStartX = 6.5;
    g.meleeDist = 0.5;
    g.tideEnabled = false;

    GameState win = makeInitialState(g);
    // contactX = 7.5; recoil ~3 → land ~10.5 = river+0.5, boat at 10.9
    // Need land near boat: boat at 10.9, half 0.65 → [10.25, 11.55]
    win.charge01 = 1.0;
    win.playerX = 8.0;
    win.tigerX = 6.5;
    tryDash(win, g);
    assert(win.outcome == Outcome::Win);

    // 落在河里、但离舟太远 → 落水，不能过关
    GameConfig tight = g;
    tight.boatHalfWidth = 0.2;
    GameState miss = makeInitialState(tight);
    miss.charge01 = 1.0;
    miss.playerX = 8.0;
    miss.tigerX = 6.5;
    tryDash(miss, tight);
    assert(miss.outcome == Outcome::DeathRiver);

    GameState drown = makeInitialState(g);
    drown.playerX = g.riverX + g.boatOffset;  // 站在船坐标上走路不算
    drown.tigerX = 0.0;
    drown.boatX = g.riverX + g.boatOffset;
    tick(drown, g, 0.01);
    assert(drown.outcome == Outcome::DeathRiver);
  }

  // Boat tracks tide
  {
    GameConfig g = c;
    g.boatEnabled = true;
    g.boatOffset = 1.0;
    g.tideEnabled = true;
    g.tideSpeed = 1.0;
    g.riverX = 16.0;
    g.tideMinRiverX = 14.0;
    g.walkSpeed = 0.0;
    g.tigerSpeed = 0.0;
    g.surviveSec = 30.0;
    GameState s = makeInitialState(g);
    assert(std::abs(s.boatX - (s.riverX + g.boatOffset)) < 1e-9);
    tick(s, g, 0.5);
    assert(std::abs(s.boatX - (s.riverX + g.boatOffset)) < 1e-9);
    assert(s.boatX < 16.0 + g.boatOffset - 0.1);
  }

  // Mad slip: first peak (100%) force-dashes
  {
    GameConfig g = c;
    g.madSlipEnabled = true;
    g.madSlipOnPeak = true;
    g.madSlipHoldSec = 0.0;
    g.chargeFullSec = 0.5;
    g.dashRange = 4.0;
    g.knockbackRange = 0.0;
    g.hitRadius = 0.5;
    g.riverX = 20.0;
    g.playerStartX = 10.0;
    g.tigerStartX = 2.0;
    g.meleeDist = 0.5;
    g.walkSpeed = 0.0;
    g.tigerSpeed = 0.0;
    g.surviveSec = 30.0;
    GameState s = makeInitialState(g);
    setCharging(s, g, true);
    s.spaceHeld = true;
    const double x0 = s.playerX;
    tick(s, g, 0.4);
    assert(s.charge01 > 0.5);
    assert(s.charge01 < 1.0);
    tick(s, g, 0.2);  // reach peak → slip
    assert(s.chargeHoldSec == 0.0 || s.charge01 == 0.0);
    assert(s.playerX < x0 || s.animSec > 0.0);
  }

  // Puppet: hold walks person into tiger; release lunges tiger; no tiger retreat
  {
    GameConfig g = c;
    g.puppetTigerEnabled = true;
    g.puppetLungeRange = 3.0;
    g.puppetChargeRushSpeed = 2.0;
    g.walkSpeed = 1.0;
    g.tigerSpeed = 0.0;
    g.meleeDist = 1.0;
    g.riverX = 20.0;
    g.playerStartX = 10.0;
    g.tigerStartX = 4.0;
    g.surviveSec = 30.0;

    GameState rush = makeInitialState(g);
    rush.spaceHeld = true;
    rush.charging = true;
    const double px0 = rush.playerX;
    const double tx0 = rush.tigerX;
    tick(rush, g, 0.5);
    assert(rush.playerX < px0);  // 往虎走
    assert(std::abs(rush.tigerX - tx0) < 1e-9);

    GameState s = makeInitialState(g);
    setCharging(s, g, true);
    s.spaceHeld = true;
    s.charge01 = 1.0;
    const double tigerBefore = s.tigerX;
    setCharging(s, g, false);
    assert(s.tigerX > tigerBefore);
    // 低蓄也有保底扑距
    GameState low = makeInitialState(g);
    g.puppetLungeMin = 1.2;
    low = makeInitialState(g);
    setCharging(low, g, true);
    low.spaceHeld = true;
    low.charge01 = 0.05;
    const double tLow = low.tigerX;
    setCharging(low, g, false);
    assert(low.tigerX >= tLow + 1.2 - 1e-6);

    GameState die = makeInitialState(g);
    die.spaceHeld = true;
    die.charging = true;
    die.playerX = 6.0;
    die.tigerX = 4.5;
    tick(die, g, 2.0);
    assert(die.outcome == Outcome::DeathMelee);
  }

  // Puppet L6: one full charge-cycle hold then light release stays alive
  {
    const LevelConfig& lv = levelAt(5);
    GameConfig g = lv.game;
    GameState s = makeInitialState(g);
    setCharging(s, g, true);
    s.spaceHeld = true;
    // hold through rise+most of fall (~1.5s)
    for (int i = 0; i < 60; ++i) {
      tick(s, g, 0.025);
      assert(s.outcome == Outcome::Playing);
    }
    assert(s.charge01 < 0.55);  // 已在下降或低位
    setCharging(s, g, false);
    assert(s.outcome == Outcome::Playing);
  }

  return 0;
}
