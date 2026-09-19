# gameDemo

小点子游戏合集仓库：每个点子一个独立小 demo，互不耦合，方便试玩法和留档。

## 结构

```text
gamesDemo/
├── games/          # 每个子目录 = 一个小游戏
│   └── <name>/
├── docs/           # 跨项目的设计/计划文档（可选）
├── README.md
└── .gitignore
```

约定：

- 新点子在 `games/<简短英文名>/` 下从零开始（自带 README、源码、本地构建说明）
- 设计规格可放在该游戏目录内，或放在 `docs/superpowers/specs/`
- 不引入统一引擎或公共框架，除非多个 demo 明确要复用

## 当前点子

| 目录 | 说明 | 状态 |
|------|------|------|
| （待建）`games/baohu-fenghe/` | 暴虎冯河：蓄力撞虎 / 反冲落水，C++ 控制台限时生存 | 规格已写，代码未开 |

规格：`docs/superpowers/specs/2026-09-19-baohu-fenghe-console-demo-design.md`

## 本地 Git

本目录已是 Git 仓库。日常：

```bash
git status
git add …
git commit -m "…"
```

若需要远端备份（GitHub / Cursor 等），再说一声即可配置。
