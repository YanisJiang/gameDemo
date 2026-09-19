# 上传 itch.io（HTML5）

## 本地试玩

用浏览器打开：

```text
games/baohu-pinghe/web/index.html
```

或在本目录起一个静态服务（推荐，避免个别浏览器限制本地脚本）：

```bat
cd games\baohu-pinghe\web
python -m http.server 8080
```

浏览器访问 `http://localhost:8080`。

## 打包上传

1. 将 `web` 目录下这三个文件打成 zip（**zip 根目录就要有 `index.html`**）：
   - `index.html`
   - `style.css`
   - `game.js`
2. itch 新建项目 → Kind: **HTML**
3. Upload 该 zip
4. Embed options 建议：
   - Viewport：**960 × 540**（或 900×500；不要用过高的 1280×720，下面会空一大块）
   - 勾选 **Mobile friendly**（有触屏蓄力按钮）
5. 若已上传过，改完 viewport 后保存即可，不必重传 zip（仅改尺寸时）

## 操作

| 输入 | 作用 |
|------|------|
| 任意键 / 点击 | 封面进入选关 |
| 1–6 / 关卡按钮 | 进关 |
| Z / E 或中英按钮 | 语言 |
| 空格或按住按钮 | 蓄力；松开冲刺 |
| N / 下一关 | 过关后下一关 |
| R / 重玩 | 重开本关 |
| Q / 选关 | 返回选关 |

不需要额外 DLL，也不依赖桌面版 `baohu.exe`。
