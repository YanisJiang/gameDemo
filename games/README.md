# games

每个子目录是一个独立小点子 demo。

推荐布局：

```text
games/<name>/
  public/       # .hpp
  private/      # .cpp
  dist/         # 发布用 exe（本地构建，一般不入库）
  build.bat
  README.md
```

当前：

- `baohu-pinghe` — 暴虎冯河
