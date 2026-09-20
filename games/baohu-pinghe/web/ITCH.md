# 浏览器版发布（itch / 国内）

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

## 打包

将本目录下这三个文件打成 zip（**zip 根目录就要有 `index.html`**）：

- `index.html`
- `style.css`
- `game.js`

本地产物示例：`../dist/baohu-web-itch.zip`。

## itch.io（海外）

1. 新建项目 → Kind: **HTML**
2. Upload 上述 zip
3. Embed options 建议：
   - Viewport：**960 × 540**（或 900×500；不要用过高的 1280×720，下面会空一大块）
   - 勾选 **Mobile friendly**（有触屏蓄力按钮）
4. 简介里写上**国内可访问地址**（itch 在国内常打不开）
5. 若只改过 viewport，保存即可，不必重传 zip

示例页：`https://yanisjiang.itch.io/baohu-pinghe`

## 国内托管（推荐：自有域名 + 阿里云）

游戏是纯静态页，无需数据库。

1. **内地 ECS**：域名先完成 [ICP 备案](https://beian.aliyun.com/)，再解析到公网 IP；安全组放行 80/443。  
   **香港 / 海外机器**：一般无需备案，直接解析即可。
2. 安装 Nginx，站点根目录放入上述三个文件，例如 `/var/www/baohu/`。
3. `server_name` 填你的域名；建议再配 HTTPS（阿里云免费 DV 证书即可）。
4. 以后改游戏：覆盖这三个文件即可，不必重启机器。

**不要用 Gitee Pages**：官方已下线该功能。

可选：网盘直接发 zip；[indienova](https://indienova.com/) 作品页外链；[4399 开放平台](https://open.4399.cn/) 看 H5/小游戏联运（审核更严）。

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

游戏内只展示操作与过关/失败条件，不剧透各关机制。不需要额外 DLL，也不依赖桌面版 `baohu.exe`。
