# gameDemo

小点子游戏合集：每个点子一个目录，互不耦合。

仓库：https://github.com/YanisJiang/gameDemo

## 目录

```text
gameDemo/
├── games/                         # 各小游戏源码
│   ├── README.md                  # games 布局约定
│   └── baohu-pinghe/              # 暴虎冯河
│       ├── public/                # C++ 头文件
│       ├── private/               # C++ 实现与入口
│       ├── web/                   # 浏览器 / itch HTML5
│       ├── dist/                  # 本地构建产物（gitignore）
│       ├── build.bat / test.bat
│       └── README.md
├── docs/superpowers/
│   ├── specs/                     # 设计规格
│   └── plans/                     # 实现计划
├── README.md
└── .gitignore
```

## 当前游戏

| 目录 | 说明 | 怎么玩 |
|------|------|--------|
| [`games/baohu-pinghe/`](games/baohu-pinghe/) | 六关侧视生存：蓄力撞虎、反冲避河；控制台 + HTML5 | 控制台：`games/baohu-pinghe/dist/baohu.exe`；网页：打开 `games/baohu-pinghe/web/index.html` |

各游戏的玩法、编译与分发细节见对应子目录 README。

## 文档

设计规格与实现计划在 `docs/superpowers/`。与「暴虎冯河」相关的主要文档：

| 文件 | 内容 |
|------|------|
| `specs/2026-09-19-baohu-fenghe-console-demo-design.md` | 初版单关控制台设计（早期文件名用了 fenghe） |
| `specs/2026-09-19-baohu-pinghe-levels-i18n-design.md` | 多关 + 中/英 |
| `specs/2026-09-19-baohu-pinghe-level-redesign.md` | 关卡机制再设计 |
| `specs/2026-09-19-baohu-pinghe-web-itch.md` | 浏览器 / itch 版 |
| `plans/` | 对应实现计划 |

## 约定

- 新点子放在 `games/<name>/`，参考 [`games/README.md`](games/README.md)。
- `dist/`、`*.exe` 等构建产物已在 `.gitignore` 中忽略；分发时从本地 `dist/` 拷贝。
