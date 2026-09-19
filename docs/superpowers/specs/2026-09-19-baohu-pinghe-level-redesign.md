# 暴虎冯河 · 前三关重设计（教程 / 重损失 / 假动作+轻涨潮）

日期：2026-09-19  
状态：实现中  

## 目标

重做现有三关，拉开体验差距：

1. **第 1 关**：教操作与胜负条件（松、清晰）  
2. **第 2 关**：同一机制，数值拉大损失，逼取舍  
3. **第 3 关**：虎假动作（主）+ 轻涨潮（辅）

## 第 1 关 `tutorial`

- 虎慢、河远、危区标准；`surviveSec = 12`  
- `bankSlipFactor = 0`；无假动作；无涨潮  
- HUD 使用教学向提示 key（`hud.hint.tutorial`）强调操作与胜负  

## 第 2 关 `pressure`

- 仍无假动作、无涨潮  
- 虎更快、河更近、满蓄反冲/岸滑更重（`bankSlipFactor` 可 > 0）  
- 提示强调「乱满蓄代价大」（`hud.hint.pressure`）  

## 第 3 关 `feint_tide`

### 虎假动作（主）

状态机（挂在 `GameState` / 由 `GameConfig` 开关）：

```text
Approach → (计时) Windup(停顿/微退) → Lunge(加速扑近) → Approach …
```

- `feintEnabled = true`  
- Windup：约 0.5s，虎速 ≈ 0 或微退  
- Lunge：约 0.35s，虎速 × `lungeSpeedFactor`（如 2.2）  
- 两次假动作间隔随机或固定（如每 3.5～5s 一轮）  
- 显示：Windup 时危区可换色提示  

### 轻涨潮（辅）

- `tideEnabled = true`  
- 本关内河界 `riverX` 缓慢减小，总收缩约 1.0～1.5 世界单位  
- 或分 1～2 次小幅跳动；优先匀速慢缩，避免抢戏  

## 实现要点

- `GameConfig` 增加：`feintEnabled`, `tideEnabled`, 假动作/涨潮参数  
- `tick` 中更新假动作与涨潮；`tryDash` 等保持  
- `levels.cpp` 重写三关表与 title/hint keys  
- `i18n` 增加教学/压力/机制提示文案  
- 主循环 HUD 按关卡 `hintKey` 显示  

## 验收

1. 第 1 关明显最好过，提示教胜负  
2. 第 2 关满蓄更容易葬送局  
3. 第 3 关能感知停顿后猛扑，且河缓慢变近  
4. `/MT` 可编出 `dist/baohu.exe`  
