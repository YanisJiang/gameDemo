# gameDemo

小点子游戏合集：每个点子一个目录，互不耦合。

## 目录

```text
gameDemo/
├── games/                      # 各小游戏源码
│   └── baohu-pinghe/
│       ├── public/             # 头文件
│       ├── private/            # 源码
│       └── dist/               # 发布 exe
├── docs/superpowers/
│   ├── specs/                  # 设计规格
│   └── plans/                  # 实现计划
├── README.md
└── .gitignore
```

## 当前游戏

| 目录 | 说明 | 怎么玩 |
|------|------|--------|
| [`games/baohu-pinghe/`](games/baohu-pinghe/) | 三关限时生存 + 中/英；蓄力撞虎 / 反冲落水 | 运行 `games/baohu-pinghe/dist/baohu.exe` |

## 文档

| 文件 | 内容 |
|------|------|
| `docs/superpowers/specs/2026-09-19-baohu-fenghe-console-demo-design.md` | 初版单关设计（早期文件名用了 fenghe） |
| `docs/superpowers/specs/2026-09-19-baohu-pinghe-levels-i18n-design.md` | 三关 + 多语言设计 |
| `docs/superpowers/plans/` | 对应实现计划 |

## Git

构建产物（`*.exe` / `*.obj`）已在 `.gitignore` 中忽略。分发时拷贝 `games/<name>/dist/` 下的 exe。
