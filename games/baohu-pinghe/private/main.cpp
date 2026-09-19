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
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

namespace {

constexpr int kSlots = 36;
constexpr double kTickSec = 1.0 / 40.0;
constexpr int kContentRows = 10;

enum class CellKind { Road, Threat, Tiger, Player, Hit, River, Boat };
enum class AppMode { Cover, SelectLevel, Playing, LevelClear, AllClear, Dead };

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

void paint(CellKind kind, int row, bool windupThreat, bool madPlayer) {
  switch (kind) {
    case CellKind::Tiger:
      std::fputs("\x1b[48;5;208;38;5;16m虎\x1b[0m", stdout);
      break;
    case CellKind::Player:
      if (madPlayer) {
        std::fputs("\x1b[48;5;201;38;5;16m疯\x1b[0m", stdout);
      } else {
        std::fputs("\x1b[48;5;48;38;5;16m人\x1b[0m", stdout);
      }
      break;
    case CellKind::Hit:
      std::fputs("\x1b[48;5;196;38;5;231m撞\x1b[0m", stdout);
      break;
    case CellKind::Threat:
      // Windup：更亮的红，提示即将猛扑
      if (windupThreat) {
        if (row == 0) {
          std::fputs("\x1b[48;5;196;38;5;231m▀▀\x1b[0m", stdout);
        } else if (row == 2) {
          std::fputs("\x1b[48;5;196;38;5;231m▄▄\x1b[0m", stdout);
        } else {
          std::fputs("\x1b[48;5;196;38;5;231m!!\x1b[0m", stdout);
        }
      } else if (row == 0) {
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
    case CellKind::Boat:
      std::fputs("\x1b[48;5;94;38;5;231m舟\x1b[0m", stdout);
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

  // 涨潮：河面格子向左变多，一眼能看出“河在推进”
  int dynamicRiver = riverSlots;
  if (config.tideEnabled) {
    const double span =
        std::max(0.01, config.riverX - config.tideMinRiverX);
    const double progress =
        std::clamp((config.riverX - state.riverX) / span, 0.0, 1.0);
    dynamicRiver =
        riverSlots +
        static_cast<int>(std::lround(progress * config.tideExtraRiverSlots));
    dynamicRiver = std::clamp(dynamicRiver, riverSlots, kSlots - 6);
  }

  const int trackSlots = kSlots - dynamicRiver;

  std::vector<CellKind> mid(static_cast<size_t>(kSlots), CellKind::Road);
  std::vector<char> inThreat(static_cast<size_t>(kSlots), 0);

  for (int i = trackSlots; i < kSlots; ++i) {
    mid[static_cast<size_t>(i)] = CellKind::River;
  }

  int boatSlot = -1;
  const double riverSpan =
      std::max(1.6, (config.boatOffset + config.boatHalfWidth) * 3.0);
  auto toRiverSlot = [&](double x) {
    const double into = std::max(0.0, x - state.riverX);
    int idx = static_cast<int>(
        std::lround((into / riverSpan) * (dynamicRiver - 1)));
    idx = std::clamp(idx, 0, dynamicRiver - 1);
    return trackSlots + idx;
  };
  if (config.boatEnabled && dynamicRiver > 0) {
    boatSlot = toRiverSlot(state.boatX);
    mid[static_cast<size_t>(boatSlot)] = CellKind::Boat;
  }

  const bool lethal =
      (state.outcome == Outcome::DeathMelee) ||
      (state.playerX - state.tigerX <= config.meleeDist + 1e-9);
  double px = lethal ? state.playerX : displayPlayerX(state);
  double tx = lethal ? state.tigerX : displayTigerX(state);
  if (px < tx + 0.01) {
    px = tx + 0.01;
  }

  const double riverBound = state.riverX;
  int playerSlot = 0;
  if (px >= riverBound - 1e-9 && dynamicRiver > 0) {
    const bool aboard =
        config.boatEnabled &&
        std::abs(px - state.boatX) <= config.boatHalfWidth + 1e-9;
    if (aboard && boatSlot >= 0) {
      playerSlot = boatSlot;  // 过关时人必须站在舟上，不能画在旁边的河里
    } else {
      playerSlot = toRiverSlot(px);
    }
  } else {
    playerSlot =
        worldToSlot(px, config.worldMinX, riverBound, trackSlots);
  }
  int tigerSlot =
      worldToSlot(tx, config.worldMinX, riverBound, trackSlots);
  if (tigerSlot >= playerSlot && playerSlot < trackSlots) {
    tigerSlot = std::max(0, playerSlot - 1);
  }

  int threatEnd = worldToSlot(tx + config.meleeDist, config.worldMinX,
                              riverBound, trackSlots);
  threatEnd = std::min(threatEnd, trackSlots - 1);

  if (lethal && playerSlot < trackSlots) {
    threatEnd = std::max(threatEnd, playerSlot);
    playerSlot = std::max(playerSlot, tigerSlot + 1);
    threatEnd = playerSlot;
  }

  for (int i = tigerSlot + 1; i <= threatEnd && i < trackSlots; ++i) {
    inThreat[static_cast<size_t>(i)] = 1;
    mid[static_cast<size_t>(i)] = CellKind::Threat;
  }

  if (state.animSec > 0.0 && state.impactHit && !lethal) {
    const int hitSlot = (tigerSlot + std::min(playerSlot, trackSlots - 1)) / 2;
    if (hitSlot != tigerSlot && hitSlot != playerSlot && hitSlot < trackSlots) {
      mid[static_cast<size_t>(hitSlot)] = CellKind::Hit;
    }
  }

  if (tigerSlot >= 0 && tigerSlot < trackSlots) {
    mid[static_cast<size_t>(tigerSlot)] = CellKind::Tiger;
  }
  mid[static_cast<size_t>(playerSlot)] = CellKind::Player;
  if (boatSlot >= 0 && boatSlot != playerSlot &&
      mid[static_cast<size_t>(boatSlot)] != CellKind::Player) {
    mid[static_cast<size_t>(boatSlot)] = CellKind::Boat;
  }

  const bool windupThreat =
      config.feintEnabled && state.feintPhase == FeintPhase::Windup;

  for (int row = 0; row < 3; ++row) {
    printPad(kSlots * 2);
    for (int i = 0; i < kSlots; ++i) {
      CellKind kind = CellKind::Road;
      if (i >= trackSlots) {
        if (config.boatEnabled && i == boatSlot && row != 1) {
          kind = CellKind::Boat;
        } else if (row == 1) {
          kind = mid[static_cast<size_t>(i)];
        } else {
          kind = CellKind::River;
        }
      } else if (row == 1) {
        kind = mid[static_cast<size_t>(i)];
      } else if (inThreat[static_cast<size_t>(i)]) {
        kind = CellKind::Threat;
      }
      paint(kind, row, windupThreat && kind == CellKind::Threat,
            config.madSlipEnabled);
    }
    endLine();
  }
}

void drawCoverStrip() {
  const int tigerSlot = 8;
  const int playerSlot = 18;
  const int riverStart = 26;
  for (int row = 0; row < 3; ++row) {
    printPad(kSlots * 2);
    for (int i = 0; i < kSlots; ++i) {
      CellKind kind = CellKind::Road;
      if (i >= riverStart) {
        kind = CellKind::River;
      } else if (i == tigerSlot && row == 1) {
        kind = CellKind::Tiger;
      } else if (i == playerSlot && row == 1) {
        kind = CellKind::Player;
      } else if (i > tigerSlot && i < playerSlot) {
        kind = CellKind::Threat;
      }
      paint(kind, row, false, false);
    }
    endLine();
  }
}

void drawCover() {
  beginFrame();
  const int lines = 8;
  const int topPad = std::max(0, (consoleRows() - lines) / 2);
  for (int i = 0; i < topPad; ++i) {
    printBlankLine();
  }
  printCenteredLine(tr("game.title"));
  printBlankLine();
  printCenteredLine(tr("cover.tagline"));
  printBlankLine();
  drawCoverStrip();
  printBlankLine();
  printCenteredLine(tr("cover.prompt"));
  for (int i = 0; i < 2; ++i) {
    printBlankLine();
  }
  std::fflush(stdout);
}

void drawLevelSelect() {
  beginFrame();
  const int lines = 5 + levelCount();
  const int topPad = std::max(0, (consoleRows() - lines) / 2);
  for (int i = 0; i < topPad; ++i) {
    printBlankLine();
  }
  printCenteredLine(tr("game.title"));
  printBlankLine();
  printCenteredLine(tr("select.level"));
  printBlankLine();
  for (int i = 0; i < levelCount(); ++i) {
    char line[160];
    std::snprintf(line, sizeof(line), "%d  %s", i + 1, tr(levelAt(i).titleKey));
    printCenteredLine(line);
  }
  printBlankLine();
  printCenteredLine(tr("select.lang"));
  for (int i = 0; i < 2; ++i) {
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
    printCenteredLine(tr("hud.rules"));
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

// 调试：baohu.exe --level 2 [--lang zh|en]
// 关卡为 1-based；省略 --lang 时默认中文并跳过选语言
bool parseArgs(int argc, char** argv, int* outLevelIndex, Locale* outLocale,
               bool* outSkipLang) {
  *outLevelIndex = 0;
  *outLocale = Locale::Zh;
  *outSkipLang = false;
  for (int i = 1; i < argc; ++i) {
    if ((std::strcmp(argv[i], "--level") == 0 ||
         std::strcmp(argv[i], "-l") == 0) &&
        i + 1 < argc) {
      int n = std::atoi(argv[++i]);
      if (n < 1) {
        n = 1;
      }
      if (n > levelCount()) {
        n = levelCount();
      }
      *outLevelIndex = n - 1;
      *outSkipLang = true;
    } else if (std::strcmp(argv[i], "--lang") == 0 && i + 1 < argc) {
      ++i;
      if (std::strcmp(argv[i], "en") == 0 || std::strcmp(argv[i], "EN") == 0) {
        *outLocale = Locale::En;
      } else {
        *outLocale = Locale::Zh;
      }
      *outSkipLang = true;
    }
  }
  return true;
}

}  // namespace

int main(int argc, char** argv) {
  SetConsoleOutputCP(65001);
  SetConsoleCP(65001);
  enableAnsi();
  hideCursor();

  int bootLevel = 0;
  Locale bootLocale = Locale::Zh;
  bool skipLang = false;
  parseArgs(argc, argv, &bootLevel, &bootLocale, &skipLang);
  setLocale(bootLocale);

  AppMode mode = skipLang ? AppMode::Playing : AppMode::Cover;
  int levelIndex = bootLevel;
  LevelConfig level = levelAt(levelIndex);
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

  auto backToSelect = [&]() {
    mode = AppMode::SelectLevel;
    drainConsoleKeys();
    prevMenuQ = true;
    drawLevelSelect();
  };

  std::fputs("\x1b[2J", stdout);
  if (skipLang) {
    startLevel(bootLevel);
  } else {
    drawCover();
  }

  while (true) {
    if (mode == AppMode::Cover) {
      drawCover();
      if (_kbhit()) {
        (void)_getch();
        mode = AppMode::SelectLevel;
        drainConsoleKeys();
        drawLevelSelect();
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }
      continue;
    }

    if (mode == AppMode::SelectLevel) {
      drawLevelSelect();
      if (_kbhit()) {
        const int ch = _getch();
        if (ch == 'q' || ch == 'Q') {
          break;
        }
        if (ch == 'z' || ch == 'Z') {
          setLocale(Locale::Zh);
        } else if (ch == 'e' || ch == 'E') {
          setLocale(Locale::En);
        } else if (ch >= '1' && ch <= '0' + levelCount()) {
          startLevel(ch - '1');
        }
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }
      continue;
    }

    if (mode == AppMode::Playing) {
      const bool down = spaceDown();
      state.spaceHeld = down;
      if (down && !prevSpace) {
        setCharging(state, level.game, true);
      } else if (!down && prevSpace) {
        setCharging(state, level.game, false);
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
      backToSelect();
      continue;
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
