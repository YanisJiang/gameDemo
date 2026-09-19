# 暴虎冯河 三关 + i18n Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 `games/baohu-pinghe` 增加启动语言选择、三关参数链（各 12s）、岸滑（第 3 关）、以及 zh/en 结局/HUD 文案。

**Architecture:** `i18n` 提供 `setLocale`/`tr`；`levels` 提供三关 `LevelConfig`；`GameConfig` 增加 `bankSlipFactor`，在 `tryDash` 命中反冲后附加滑移；`main` 管理 `SelectLang → Playing → LevelEnd/AllClear` 状态机。

**Tech Stack:** C++17、Windows 控制台、`/MT` 分发 `baohu.exe`。

## Global Constraints

- 核心操作不变；不做体力/得分
- 每关 12s；失败 R 重开本关；全通 R 从第 1 关重开
- 文案全部 `tr(key)`；缺 key 回退 zh 再回退 key
- 规格：`docs/superpowers/specs/2026-09-19-baohu-pinghe-levels-i18n-design.md`

---

### Task 1: i18n 字符串表

**Files:**
- Create: `games/baohu-pinghe/i18n.hpp`
- Create: `games/baohu-pinghe/i18n.cpp`
- Test: extend `test_game.cpp` 或单独 assert `tr` 回退

- [ ] **Step 1:** 实现 `enum class Locale { Zh, En }`、`void setLocale(Locale)`、`const char* tr(const char* key)`，内置 zh/en 表覆盖规格全部 key
- [ ] **Step 2:** 编译进测试，断言 `setLocale(En); tr("end.melee")` 非中文且非空
- [ ] **Step 3:** Commit（若用户未要求提交可跳过，与仓库习惯一致时再提交）

---

### Task 2: bankSlip + 三关配置

**Files:**
- Modify: `games/baohu-pinghe/game.hpp` — `bankSlipFactor`
- Modify: `games/baohu-pinghe/game.cpp` — 命中后 `playerX += recoil * bankSlipFactor`
- Create: `games/baohu-pinghe/levels.hpp` / `levels.cpp` — `LevelConfig` 数组 3 关
- Modify: `test_game.cpp` — 岸滑与关卡 surviveSec=12

- [ ] **Step 1:** 失败测试：`bankSlipFactor=0.15` 时满蓄命中后 `playerX` 大于 `bankSlipFactor=0`
- [ ] **Step 2:** 实现 slip + `levelCount()` / `levelAt(i)`
- [ ] **Step 3:** 测试通过

---

### Task 3: main 状态机 + HUD + `/MT` 发布

**Files:**
- Modify: `games/baohu-pinghe/main.cpp`
- Modify: `games/baohu-pinghe/README.md`

- [ ] **Step 1:** 启动选语言 → 载入关卡 0；HUD 显示 `1/3` + `tr(titleKey)`；Win 进下一关或全通；失败/全通文案用 `tr`；R/Q 行为符合规格
- [ ] **Step 2:** `cl ... /MT /Fe:baohu.exe main.cpp game.cpp i18n.cpp levels.cpp user32.lib`
- [ ] **Step 3:** 烟测：中英文切换、三关推进、第 3 关岸滑

---

## Spec coverage

| 规格项 | 任务 |
|--------|------|
| i18n keys / 回退 | Task 1 |
| 三关参数 + 岸滑 | Task 2 |
| 流程与 HUD | Task 3 |
| /MT baohu.exe | Task 3 |
