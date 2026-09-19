#include "game.hpp"
#include "i18n.hpp"
#include "levels.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <conio.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>

namespace {

constexpr int kSlots = 36;
constexpr double kTickSec = 1.0 / 40.0;
constexpr int kContentRows = 10;

enum class CellKind { Road, Threat, Tiger, Player, Hit, River };
enum class AppMode { SelectLang, Playing, LevelClear, AllClear, Dead };

void enableAnsi() {
  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD mode = 0;
  if (GetConsoleMode(h, &mode)) {
    SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
  }
}

void hideCursor() {
  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_CURSOR_INFO info{1, FALSE};
  SetConsoleCursorInfo(h, &info);
}

void showCursor() {
  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_CURSOR_INFO info{1, TRUE};
  SetConsoleCursorInfo(h, &info);
}

void beginFrame() { std::fputs("\x1b[H", stdout); }

void endLine() { std::fputs("\x1b[0K\n", stdout); }

int consoleCols() {
  CONSOLE_SCREEN_BUFFER_INFO info;
  if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
    return 80;
  }
  return info.srWindow.Right - info.srWindow.Left + 1;
}

int consoleRows() {
  CONSOLE_SCREEN_BUFFER_INFO info;
  if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
    return 25;
  }
  return info.srWindow.Bottom - info.srWindow.Top + 1;
}

int displayWidth(const char* s) {
  int w = 0;
  for (const unsigned char* p = reinterpret_cast<const unsigned char*>(s); *p;) {
    if (*p < 0x80) {
      ++w;
      ++p;
    } else if ((*p & 0xE0) == 0xC0) {
      w += 2;
      p += 2;
    } else if ((*p & 0xF0) == 0xE0) {
      w += 2;
      p += 3;
    } else if ((*p & 0xF8) == 0xF0) {
      w += 2;
      p += 4;
    } else {
      ++p;
    }
  }
  return w;
}

void printPad(int contentWidth) {
  const int pad = (consoleCols() - contentWidth) / 2;
  if (pad > 0) {
    std::printf("%*s", pad, "");
  }
}

void printCenteredLine(const char* text) {
  printPad(displayWidth(text));
  std::fputs(text, stdout);
  endLine();
}

void printBlankLine() { endLine(); }

int worldToSlot(double x, double minX, double maxX, int slots) {
  const double span = std::max(1.0, maxX - minX);
  int s = static_cast<int>(std::lround((x - minX) / span * (slots - 1)));
  return std::clamp(s, 0, slots - 1);
}

std::string buildChargeBar(double charge01, int width = 8) {
  const int filled =
      static_cast<int>(std::lround(std::clamp(charge01, 0.0, 1.0) * width));
  std::string bar(width, ' ');
  for (int i = 0; i < filled; ++i) {
    bar[static_cast<size_t>(i)] = '=';
  }
  return bar;
}

void paint(CellKind kind, int row) {
  switch (kind) {
    case CellKind::Tiger:
      std::fputs("\x1b[48;5;208;38;5;16m虎\x1b[0m", stdout);
      break;
    case CellKind::Player:
      std::fputs("\x1b[48;5;48;38;5;16m人\x1b[0m", stdout);
      break;
    case CellKind::Hit:
      std::fputs("\x1b[48;5;196;38;5;231m撞\x1b[0m", stdout);
      break;
    case CellKind::Threat:
      if (row == 0) {
        std::fputs("\x1b[48;5;52;38;5;224m▀▀\x1b[0m", stdout);
      } else if (row == 2) {
        std::fputs("\x1b[48;5;52;38;5;224m▄▄\x1b[0m", stdout);
      } else {
        std::fputs("\x1b[48;5;52;38;5;224m··\x1b[0m", stdout);
      }
      break;
    case CellKind::River:
      std::fputs("\x1b[48;5;27;38;5;231m河\x1b[0m", stdout);
      break;
    case CellKind::Road:
    default:
      if (row == 0) {
        std::fputs("\x1b[48;5;94;38;5;230m▀▀\x1b[0m", stdout);
      } else if (row == 2) {
        std::fputs("\x1b[48;5;94;38;5;230m▄▄\x1b[0m", stdout);
      } else {
        std::fputs("\x1b[48;5;100;38;5;230m··\x1b[0m", stdout);
      }
      break;
  }
}

void drawScene(const GameState& state, const GameConfig& config,
               int riverSlots) {
  riverSlots = std::clamp(riverSlots, 2, kSlots - 8);
  const int trackSlots = kSlots - riverSlots;

  std::vector<CellKind> mid(static_cast<size_t>(kSlots), CellKind::Road);
  std::vector<char> inThreat(static_cast<size_t>(kSlots), 0);

  for (int i = trackSlots; i < kSlots; ++i) {
    mid[static_cast<size_t>(i)] = CellKind::River;
  }

  const bool lethal =
      (state.outcome == Outcome::DeathMelee) ||
      (state.playerX - state.tigerX <= config.meleeDist + 1e-9);
  double px = lethal ? state.playerX : displayPlayerX(state);
  double tx = lethal ? state.tigerX : displayTigerX(state);
  if (px < tx + 0.01) {
    px = tx + 0.01;
  }

  int playerSlot =
      worldToSlot(px, config.worldMinX, config.riverX, trackSlots);
  int tigerSlot =
      worldToSlot(tx, config.worldMinX, config.riverX, trackSlots);
  if (tigerSlot >= playerSlot) {
    tigerSlot = std::max(0, playerSlot - 1);
  }

  int threatEnd = worldToSlot(tx + config.meleeDist, config.worldMinX,
                              config.riverX, trackSlots);
  threatEnd = std::min(threatEnd, trackSlots - 1);

  if (lethal) {
    threatEnd = std::max(threatEnd, playerSlot);
    playerSlot = std::max(playerSlot, tigerSlot + 1);
    threatEnd = playerSlot;
  }

  for (int i = tigerSlot + 1; i <= threatEnd && i < trackSlots; ++i) {
    inThreat[static_cast<size_t>(i)] = 1;
    mid[static_cast<size_t>(i)] = CellKind::Threat;
  }

  if (state.animSec > 0.0 && state.impactHit && !lethal) {
    const int hitSlot = (tigerSlot + playerSlot) / 2;
    if (hitSlot != tigerSlot && hitSlot != playerSlot) {
      mid[static_cast<size_t>(hitSlot)] = CellKind::Hit;
    }
  }

  mid[static_cast<size_t>(tigerSlot)] = CellKind::Tiger;
  mid[static_cast<size_t>(playerSlot)] = CellKind::Player;

  for (int row = 0; row < 3; ++row) {
    printPad(kSlots * 2);
    for (int i = 0; i < kSlots; ++i) {
      CellKind kind = CellKind::Road;
      if (i >= trackSlots) {
        kind = CellKind::River;
      } else if (row == 1) {
        kind = mid[static_cast<size_t>(i)];
      } else if (inThreat[static_cast<size_t>(i)]) {
        kind = CellKind::Threat;
      }
      paint(kind, row);
    }
    endLine();
  }
}

void drawLangSelect() {
  beginFrame();
  const int topPad = (consoleRows() - 4) / 2;
  for (int i = 0; i < topPad; ++i) {
    printBlankLine();
  }
  printCenteredLine(tr("game.title"));
  printBlankLine();
  printCenteredLine(tr("lang.prompt"));
  for (int i = 0; i < 4; ++i) {
    printBlankLine();
  }
  std::fflush(stdout);
}

void drawGame(const GameState& state, const LevelConfig& level, int levelIndex,
              AppMode mode) {
  beginFrame();

  const int topPad = (consoleRows() - kContentRows) / 2;
  for (int i = 0; i < topPad; ++i) {
    printBlankLine();
  }

  printCenteredLine(tr("game.title"));

  char levelLine[160];
  std::snprintf(levelLine, sizeof(levelLine), "%s %d/%d  %s", tr("hud.level"),
                levelIndex + 1, levelCount(), tr(level.titleKey));
  printCenteredLine(levelLine);

  if (mode == AppMode::Playing) {
    printCenteredLine(tr("hud.hint"));
  } else if (mode == AppMode::LevelClear) {
    printCenteredLine(tr("end.level_clear"));
  } else if (mode == AppMode::AllClear) {
    printCenteredLine(tr("end.all_clear"));
  } else if (mode == AppMode::Dead && state.outcome == Outcome::DeathMelee) {
    printCenteredLine(tr("end.melee"));
  } else if (mode == AppMode::Dead && state.outcome == Outcome::DeathRiver) {
    printCenteredLine(tr("end.river"));
  }

  if (mode == AppMode::Playing) {
    char status[64];
    std::snprintf(status, sizeof(status), "%4.0fs   [%s] %3.0f%%",
                  state.timeLeft, buildChargeBar(state.charge01).c_str(),
                  state.charge01 * 100.0);
    printCenteredLine(status);
  } else if (mode == AppMode::LevelClear) {
    printCenteredLine(tr("end.level_clear_hint"));
  } else if (mode == AppMode::AllClear) {
    printCenteredLine(tr("end.retry_all"));
  } else {
    printCenteredLine(tr("end.retry_level"));
  }

  printBlankLine();
  drawScene(state, level.game, level.riverDisplaySlots);

  for (int i = 0; i < 4; ++i) {
    printBlankLine();
  }
  std::fflush(stdout);
}

bool spaceDown() {
  return (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
}

bool keyDown(int vk) {
  return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

bool menuKeysHeld() {
  return keyDown('R') || keyDown('N') || keyDown('Q') || keyDown(VK_RETURN);
}

void drainConsoleKeys() {
  while (_kbhit()) {
    (void)_getch();
  }
}

}  // namespace

int main() {
  SetConsoleOutputCP(65001);
  SetConsoleCP(65001);
  enableAnsi();
  hideCursor();

  AppMode mode = AppMode::SelectLang;
  int levelIndex = 0;
  LevelConfig level = levelAt(0);
  GameState state = makeInitialState(level.game);
  bool prevSpace = false;
  bool menuArmed = false;
  bool prevMenuR = false;
  bool prevMenuN = false;
  bool prevMenuEnter = false;
  bool prevMenuQ = false;

  auto armMenu = [&]() {
    menuArmed = false;
    prevMenuR = true;
    prevMenuN = true;
    prevMenuEnter = true;
    prevMenuQ = true;
    drainConsoleKeys();
  };

  auto startLevel = [&](int index) {
    levelIndex = index;
    level = levelAt(levelIndex);
    state = makeInitialState(level.game);
    prevSpace = spaceDown();
    mode = AppMode::Playing;
    drawGame(state, level, levelIndex, mode);
  };

  std::fputs("\x1b[2J", stdout);
  drawLangSelect();

  while (true) {
    if (mode == AppMode::SelectLang) {
      if (_kbhit()) {
        const int ch = _getch();
        if (ch == '2') {
          setLocale(Locale::En);
        } else {
          setLocale(Locale::Zh);
        }
        startLevel(0);
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }
      continue;
    }

    if (mode == AppMode::Playing) {
      const bool down = spaceDown();
      if (down && !prevSpace) {
        setCharging(state, level.game, true);
      } else if (!down && prevSpace) {
        setCharging(state, level.game, false);
      } else if (down) {
        state.charging = true;
      } else {
        state.charging = false;
      }
      prevSpace = down;

      tick(state, level.game, kTickSec);

      if (state.outcome == Outcome::Win) {
        if (levelIndex + 1 >= levelCount()) {
          mode = AppMode::AllClear;
        } else {
          mode = AppMode::LevelClear;
        }
        armMenu();
      } else if (state.outcome == Outcome::DeathMelee ||
                 state.outcome == Outcome::DeathRiver) {
        mode = AppMode::Dead;
        armMenu();
      }

      drawGame(state, level, levelIndex, mode);
      std::this_thread::sleep_for(std::chrono::duration<double>(kTickSec));
      continue;
    }

    // 结算 / 失败 / 全通：必须先松开按键，再按一次才生效（避免过关时空格连带跳关）
    drawGame(state, level, levelIndex, mode);

    if (!menuArmed) {
      if (!menuKeysHeld()) {
        menuArmed = true;
        prevMenuR = false;
        prevMenuN = false;
        prevMenuEnter = false;
        prevMenuQ = false;
        drainConsoleKeys();
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      continue;
    }

    const bool mR = keyDown('R');
    const bool mN = keyDown('N');
    const bool mEnter = keyDown(VK_RETURN);
    const bool mQ = keyDown('Q');

    const bool pressR = mR && !prevMenuR;
    const bool pressN = mN && !prevMenuN;
    const bool pressEnter = mEnter && !prevMenuEnter;
    const bool pressQ = mQ && !prevMenuQ;

    prevMenuR = mR;
    prevMenuN = mN;
    prevMenuEnter = mEnter;
    prevMenuQ = mQ;

    if (pressQ) {
      break;
    }

    if (mode == AppMode::LevelClear) {
      if (pressR) {
        startLevel(levelIndex);  // 重玩本关
      } else if (pressEnter || pressN) {
        startLevel(levelIndex + 1);  // 下一关（不用空格）
      }
    } else if (mode == AppMode::Dead) {
      if (pressR) {
        startLevel(levelIndex);
      }
    } else if (mode == AppMode::AllClear) {
      if (pressR) {
        startLevel(0);
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  showCursor();
  return 0;
}
