# 暴虎冯河 C++ 控制台 Demo Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 实现可编译运行的 C++ 控制台限时生存 demo：蓄力冲刺打虎、击退/反冲、贴身死/落水死/超时胜。

**Architecture:** 纯逻辑放在 `game.hpp`/`game.cpp`（可单测）；`main.cpp` 负责 Windows 控制台输入、定时循环与刷新显示。冲刺命中在同一 tick 瞬时结算。

**Tech Stack:** C++17、Windows 控制台、`<conio.h>`（`_kbhit`/`_getch`）、无第三方库。

## Global Constraints

- 只做第一关限时生存；不做第 2/3 关、图形、音效
- 布局一维：`x` 增大朝河；`riverX=20`，`playerX` 初值 `12`，`tigerX` 初值 `0`，贴身距离 `1.0`
- 按住空格蓄力、松开冲刺；无左右移动键
- 路径在 `games/baohu-pinghe/`

---

### Task 1: 纯逻辑 — 状态、蓄力、冲刺结算

**Files:**
- Create: `games/baohu-pinghe/game.hpp`
- Create: `games/baohu-pinghe/game.cpp`
- Create: `games/baohu-pinghe/test_game.cpp`

**Interfaces:**
- Produces:
  - `enum class Outcome { Playing, Win, DeathMelee, DeathRiver };`
  - `struct GameConfig` — 上述常量与 `chargeFullSec=1.2`、`surviveSec=30`、`tigerSpeed`、`dashRange`、`knockbackRange`、`recoilRange`、`hitRadius`
  - `struct GameState` — `playerX, tigerX, timeLeft, charge01, charging, outcome`
  - `GameState makeInitialState(const GameConfig&)`
  - `void setCharging(GameState&, bool down)` — 按下开始蓄力；松开若 `charge01>0` 则调用冲刺结算
  - `void tick(GameState&, const GameConfig&, double dt)` — 蓄力增长、虎靠近、胜负检查
  - `void tryDash(GameState&, const GameConfig&)` — 按 `p=charge01` 瞬时冲刺/击退/反冲，然后 `charge01=0`

- [ ] **Step 1: 写失败测试**

`test_game.cpp` 用 `assert`：

```cpp
#include "game.hpp"
#include <cassert>
#include <cmath>

int main() {
  GameConfig c = GameConfig::defaults();
  GameState s = makeInitialState(c);
  // 满蓄力命中：虎应更左（x 更小），人应更右（更靠近河）
  s.charge01 = 1.0;
  tryDash(s, c);
  assert(s.tigerX < 0.0 + 1e-6); // 被击退到初始以左，或至少 < 撞前
  assert(s.playerX > 12.0);
  // 落水
  s = makeInitialState(c);
  s.playerX = c.riverX + 0.1;
  tick(s, c, 0.01);
  assert(s.outcome == Outcome::DeathRiver);
  // 贴身
  s = makeInitialState(c);
  s.tigerX = s.playerX - 0.5;
  tick(s, c, 0.01);
  assert(s.outcome == Outcome::DeathMelee);
  // 超时胜利
  s = makeInitialState(c);
  s.timeLeft = 0.0;
  tick(s, c, 0.01);
  assert(s.outcome == Outcome::Win);
  return 0;
}
```

（实现时把「满蓄力击退」断言改成：记录 dash 前 `tigerX`，之后 `tigerX < before`。）

- [ ] **Step 2: 编译运行测试，确认失败**

```bash
g++ -std=c++17 -O0 -o test_game games/baohu-pinghe/test_game.cpp games/baohu-pinghe/game.cpp
```

Expected: 链接失败或 assert（尚无实现）

- [ ] **Step 3: 实现 `game.hpp` / `game.cpp`**

常量建议：`tigerSpeed=0.35`，`dashRange=10`，`knockbackRange=10`，`recoilRange=8`，`hitRadius=1.5`。

`tryDash`：`playerX -= p*dashRange`；若 `playerX-tigerX <= hitRadius` 则命中：`tigerX -= p*knockbackRange`，`playerX += p*recoilRange`；否则停在冲刺点。然后清零蓄力。

`tick`：若 `Playing`：`timeLeft-=dt`；若 `charging` 则 `charge01=min(1, charge01+dt/chargeFullSec)`；`tigerX += tigerSpeed*dt` 且不超过 `playerX`；检查贴身、落水、`timeLeft<=0`→Win。

- [ ] **Step 4: 测试通过**

```bash
g++ -std=c++17 -O2 -o test_game games/baohu-pinghe/test_game.cpp games/baohu-pinghe/game.cpp && ./test_game
```

Expected: exit 0

- [ ] **Step 5: Commit**

```bash
git add games/baohu-pinghe/game.hpp games/baohu-pinghe/game.cpp games/baohu-pinghe/test_game.cpp
git commit -m "feat(baohu): add pure game logic and assert tests"
```

---

### Task 2: 控制台主循环 — 输入、显示、构建说明

**Files:**
- Create: `games/baohu-pinghe/main.cpp`
- Create: `games/baohu-pinghe/README.md`

**Interfaces:**
- Consumes: Task 1 的 `GameState` / `tick` / `setCharging` / `GameConfig`
- Produces: 可执行文件 `baohu.exe`

- [ ] **Step 1: 实现 `main.cpp`**

- 开始打印操作说明（空格蓄力松手冲刺，撑过 30 秒）
- 循环：约 `dt=0.05`；用 `GetAsyncKeyState(VK_SPACE)` 或 `_kbhit`+空格边沿检测按住/松开，调用 `setCharging`
- 清屏（`system("cls")` 或 ANSI）后打印：剩余时间、蓄力条、一行 `[虎]..[人]..[~河~]`（把 x 映射到 0..width）
- `outcome != Playing` 时打印 `存活` / `贴身死` / `落水死`，提示按 R 重开或 Q 退出

- [ ] **Step 2: 编译运行烟测**

```bash
g++ -std=c++17 -O2 -o games/baohu-pinghe/baohu games/baohu-pinghe/main.cpp games/baohu-pinghe/game.cpp
games/baohu-pinghe/baohu.exe
```

Expected: 能看见场景；蓄力条变化；可胜可负。

- [ ] **Step 3: 写 `README.md`**（怎么编译、怎么玩、调参在 `GameConfig::defaults`）

- [ ] **Step 4: Commit**

```bash
git add games/baohu-pinghe/main.cpp games/baohu-pinghe/README.md
git commit -m "feat(baohu): add console loop and play instructions"
```

- [ ] **Step 5: 更新根 `README.md` 当前点子表为「可玩」**

```bash
git add README.md
git commit -m "docs: mark baohu-fenghe console demo playable"
```

---

## Spec coverage

| 规格项 | 任务 |
|--------|------|
| 限时生存 / 贴身 / 落水 / 胜利 | Task 1 |
| 蓄力冲刺、击退、反冲 | Task 1 |
| 空格按住/松开、文字显示 | Task 2 |
| C++17 控制台、无第三方 | Task 1–2 |
| 不做另外两关 | 遵守非目标 |
