# 暴虎冯河（控制台）

有勇无度：在虎与河之间蓄力冲刺，撑过时间。

## 简介

「暴虎冯河」本意是空手搏虎、无舟渡河：有勇，却没有分寸。

这是一款侧视控制台小游戏。虎在左，人在中，河在右。人会自己往河边走；按住空格蓄力（力度由低到高再回到低），松手朝虎冲刺。打中会把虎击退，自己也被反冲推向河岸。撑过本关计时即过关。贴进虎的红区，或走进河里，都会死。

一共六关。操作始终是这一套，但每一关的压力和规则都会变——有的只是更狠，有的要你重新理解「你在控制什么」。先用普通打法硬撑，再去摸关里的例外。

## 浏览器版

源码目录：

```text
games/baohu-pinghe/web/
```

本地打开 `web/index.html`，或把 `index.html` / `style.css` / `game.js` 打成 zip（**根目录就要有 `index.html`**）。详细说明见 `web/ITCH.md`。本地打好的包：`dist/baohu-web-itch.zip`（若编过）。

游戏内文案只保留**操作**与**胜利/失败条件**，不剧透各关机制。

### 分发建议

| 渠道 | 用途 |
|------|------|
| itch.io（海外） | HTML 项目上传上述 zip；Embed 建议 **960×540**，勾选 Mobile friendly |
| 自有域名 + 阿里云 ECS/静态托管（国内） | 把三个静态文件放到 Nginx `root`；内地服务器需先完成 **ICP 备案** |
| 直接发 zip | 网盘 / 群文件；解压后打开 `index.html`（个别浏览器需本地静态服务） |

说明：

- itch 在国内常打不开或极慢，请在 itch 简介里写上**国内可访问地址**。
- **Gitee Pages 已下线**，不要再当作国内托管方案。
- 内地备案入口：[阿里云 ICP 代备案](https://beian.aliyun.com/)；通过后网站底部悬挂备案号，并在开通后 30 日内做公安联网备案。
- 可选曝光：[indienova](https://indienova.com/) 作品页外链到国内站；大众联运可再看 [4399 开放平台](https://open.4399.cn/)（审核与素材要求较多）。

## About

Courage without measure: charge and dash between the tiger and the river, and outlast the clock.

*Bao Hu Ping He* names the old warning against fighting a tiger bare-handed or fording a river with no boat — brave, and reckless.

A side-view console game. Tiger on the left, you in the middle, river on the right. You walk toward the water on your own. Hold Space to charge (the bar rises, then falls); release to dash at the tiger. A hit knocks the tiger back and recoils you toward the bank. Survive the timer to clear the stage. Step into the red zone, or into the river, and you die.

Six stages. The controls stay the same; the pressure and the rules do not. Some stages are simply harsher. Others ask you to rethink what you are actually controlling. Try to last the ordinary way first — then look for the exception.

## 目录

```text
baohu-pinghe/
├── public/          # 对外头文件（.hpp）
│   ├── game.hpp
│   ├── i18n.hpp
│   └── levels.hpp
├── private/         # 实现与入口（.cpp）
│   ├── main.cpp
│   ├── game.cpp
│   ├── i18n.cpp
│   ├── levels.cpp
│   └── test_game.cpp
├── web/             # 浏览器 / itch HTML5
│   ├── index.html
│   ├── style.css
│   ├── game.js
│   └── ITCH.md
├── dist/            # 发布产物（gitignore）
│   ├── baohu.exe
│   └── baohu-web-itch.zip
├── build.bat        # 编译发布版
├── test.bat         # 跑单元测试
└── README.md
```

## 玩

```bat
dist\baohu.exe
```

启动先显示封面，按任意键进入选关：`1`–`6` 进关，`Z` 中文 / `E` English，`Q` 退出。

| 键 | 作用 |
|----|------|
| 空格 | 仅游戏中：蓄力 / 冲刺 |
| Enter / N | 本关通过后进入下一关 |
| R | 失败重开本关；本关通过后重玩本关；全通后从第 1 关重开 |
| Z / E | 选关界面：中文 / English |
| Q | 结算时返回选关；选关界面退出游戏 |

## 快速调试某一关

跳过选语言，直接进指定关（1～6）：

```bat
dist\baohu.exe --level 2
dist\baohu.exe -l 6 --lang en
debug_level.bat 2
debug_level.bat 6 en
```

## 编译

在 **x64 Native Tools Command Prompt for VS** 中：

```bat
build.bat
```

测试：

```bat
test.bat
```
