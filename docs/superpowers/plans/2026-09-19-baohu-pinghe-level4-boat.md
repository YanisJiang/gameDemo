# Level 4 Boat Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add level 4 with a tide-following boat; recoil onto boat is a secret win; timer win remains but is tight.

**Architecture:** Extend `GameConfig`/`GameState` with boat fields; sync `boatX` from `riverX` each tick; `tryDash` checks boat before river death; draw boat in river slots; add `buildFerrySecret` as 4th level.

**Tech Stack:** C++17, MSVC `/MT`, existing console VT renderer.

## Global Constraints

- Hint text must NOT mention boat win
- Only recoil (post-dash) landing on boat → Win; walk into river → DeathRiver
- Timer survival still Win
- `/MT` build to `dist/baohu.exe`

---

### Task 1: Boat win / sync logic (TDD)

**Files:**
- Modify: `games/baohu-pinghe/public/game.hpp`
- Modify: `games/baohu-pinghe/private/game.cpp`
- Modify: `games/baohu-pinghe/private/test_game.cpp`

- [ ] Write failing tests: recoil onto boat → Win; walk into river at boat coords → DeathRiver; boatX tracks riverX under tide
- [ ] Add `boatEnabled`, `boatOffset`, `boatHalfWidth`; `state.boatX`; `tideRising` already exists
- [ ] Sync boat in `makeInitialState` + `tick`; in `tryDash` after move, if on boat → Win before `checkTerminal`
- [ ] Run `dist/test_game.exe` green

### Task 2: Level 4 + i18n + draw

**Files:**
- Modify: `games/baohu-pinghe/private/levels.cpp`
- Modify: `games/baohu-pinghe/private/i18n.cpp`
- Modify: `games/baohu-pinghe/private/main.cpp`
- Modify: `games/baohu-pinghe/README.md` (level count only if already lists levels)

- [ ] `buildFerrySecret()` + `levelCount()==4`
- [ ] Title/hint without boat spoilers
- [ ] Draw `Boat` cell in river band at `boatX`
- [ ] Rebuild `dist/baohu.exe`
