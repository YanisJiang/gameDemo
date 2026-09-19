# 暴虎冯河（控制台）

三关限时生存 + 中/英。空格蓄力冲刺；自动远离老虎；贴身/落水失败。

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
├── dist/            # 发布产物
│   └── baohu.exe
├── build.bat        # 编译发布版
├── test.bat         # 跑单元测试
└── README.md
```

## 玩

```bat
dist\baohu.exe
```

启动选 `1` 中文 / `2` English。

| 键 | 作用 |
|----|------|
| 空格 | 仅游戏中：蓄力 / 冲刺 |
| Enter / N | 本关通过后进入下一关 |
| R | 失败重开本关；本关通过后重玩本关；全通后从第 1 关重开 |
| Q | 退出 |

## 编译

在 **x64 Native Tools Command Prompt for VS** 中：

```bat
build.bat
```

测试：

```bat
test.bat
```
