# 暴虎冯河 · 第四关：潮上暗船

日期：2026-09-19  
状态：已批准（方案 1 + 胜负 B + 计时 A）

## 目标

新增第 4 关：河上有船，随涨退潮移动。表面是更难的耗时关；**反冲落点在船上**为隐藏捷径过关。提示不剧透船上可赢。

## 胜负

| 条件 | 结果 |
|------|------|
| `timeLeft ≤ 0` | `Win`（数值压紧，刚可过） |
| **冲刺后坐力**落点落在船宽内 | `Win`（隐藏） |
| 走路 / 非反冲进入 `playerX ≥ riverX` | `DeathRiver`（即使坐标重叠船也不算） |
| 贴身 | `DeathMelee` |

## 船

- `boatEnabled`（仅第 4 关）
- 每帧：`boatX = riverX + boatOffset`（默认 `+0.9`）
- 半宽 `boatHalfWidth`（默认 `0.65`）——反冲后 `\|playerX - boatX\| ≤ boatHalfWidth` 即上船
- 画面：河区内画船格，不说明用途

## 关卡数值（相对第 3 关加压）

- 保留假扑 + 涨退潮
- 虎略快 / 河略近 / 岸滑略增 / 计时略紧，使纯耗时刚可过
- `hintKey` 只写假扑与涨潮，**不提船、不上船过关**
- 过关文案与计时共用，不单独剧透

## 实现要点

- `GameConfig` / `GameState` 增加船字段；`tick` 同步 `boatX`
- `tryDash` 结算位置后、`checkTerminal` 前：若船上 → `Win`
- `levels.cpp`：`levelCount = 4`，`buildFerrySecret()`
- `main.cpp`：绘制船；HUD 用本关 hint
- 测试：反冲上船赢；走路进河（含船坐标）仍死；船随 `riverX` 移动

## 验收

1. 第 4 关可见船随涨退潮移动  
2. 走路落水死；反冲上船赢  
3. 提示不提船赢  
4. 计时仍可过关；`dist/baohu.exe` 可编  
