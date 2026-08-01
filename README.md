# VN Engine - C++/SDL2/Lua 视觉小说引擎

一个参考 ESCUDE 引擎架构设计的视觉小说引擎，使用 C++ + SDL2 + Lua 构建。

## 架构

```
┌─────────────────────────────────┐
│     Lua 脚本层（游戏内容）       │  ← 剧本、角色、场景、UI 布局
├─────────────────────────────────┤
│     C++ 引擎层（机制）           │  ← 渲染、音频、输入、存档
├─────────────────────────────────┤
│     SDL2 / SDL_mixer / SDL_ttf  │  ← 底层库
└─────────────────────────────────┘
```

## 模块

| 模块 | 目录 | 功能 |
|------|------|------|
| Engine Core | src/engine/ | 窗口、事件循环、状态机 |
| Renderer | src/engine/ | SDL2 纹理渲染、图层管理 |
| Audio | src/engine/ | BGM/SE/Voice 播放 |
| ResourceManager | src/engine/ | 纹理/音频缓存 |
| ScriptEngine | src/script/ | Lua 解释器、C++ API 绑定 |
| ADV System | src/adv/ | 文字框、立绘、图层、场景 |
| UI System | src/ui/ | 按钮、菜单、分支选择 |
| Save/Load | src/system/ | 序列化、存档槽 |
| Tween | src/tween/ | 缓动函数、插值动画 |

## 依赖

- C++17
- SDL2, SDL2_image, SDL2_ttf, SDL2_mixer
- Lua 5.3+
- CMake 3.16+

## 构建

### Linux / macOS

```bash
mkdir build && cd build
cmake .. && make
./bin/vn_engine
```

### Windows

详细说明见 [BUILD_WINDOWS.md](BUILD_WINDOWS.md)。

快速步骤（vcpkg + MSVC）：

```cmd
:: 安装依赖
vcpkg install sdl2 sdl2-image sdl2-ttf sdl2-mixer lua

:: 构建
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release

:: 运行
Release\vn_engine.exe
```

## 工具

| 工具 | 说明 |
|------|------|
| `vn_engine` | 视觉小说引擎主程序 |
| `vn_editor` | 可视化脚本编辑器（GUI + CLI 模式） |
| `pak_tool` | 资源打包工具（`.pak` 格式） |


