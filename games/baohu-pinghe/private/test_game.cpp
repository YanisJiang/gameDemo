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
  assert(levelCount() == 3);
  assert(levelAt(0).game.surviveSec == 12.0);
  assert(levelAt(2).game.bankSlipFactor > 0.0);

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

  return 0;
}
