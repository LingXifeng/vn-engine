# Windows 构建指南

本文档介绍如何在 Windows 上构建 VN Engine（视觉小说引擎 + 编辑器 + 打包工具）。

## 目录

- [前提条件](#前提条件)
- [方法一：vcpkg + Visual Studio（推荐）](#方法一vcpkg--visual-studio推荐)
- [方法二：MinGW + MSYS2](#方法二mingw--msys2)
- [构建验证](#构建验证)
- [运行说明](#运行说明)
- [常见问题](#常见问题)

---

## 前提条件

| 工具 | 最低版本 | 说明 |
|------|---------|------|
| CMake | 3.16 | 构建系统 |
| C++ 编译器 | C++17 | MSVC 19.14+ 或 MinGW GCC 7+ |
| Git | 任意 | 获取 vcpkg / 源码 |

---

## 方法一：vcpkg + Visual Studio（推荐）

### 1. 安装 Visual Studio

安装 Visual Studio 2019 或 2022，勾选 **"使用 C++ 的桌面开发"** 工作负载。

### 2. 安装 vcpkg

```cmd
:: 克隆 vcpkg
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg

:: 编译并安装
bootstrap-vcpkg.bat
```

设置环境变量（可选，方便后续使用）：

```cmd
setx VCPKG_ROOT C:\vcpkg
setx PATH "%PATH%;C:\vcpkg"
```

### 3. 安装依赖库

```cmd
vcpkg install sdl2 sdl2-image sdl2-ttf sdl2-mixer lua --triplet x64-windows
```

> **注意**：如果使用 32 位构建，将 `x64-windows` 改为 `x86-windows`。

### 4. 配置 CMake

```cmd
:: 进入项目目录
cd path\to\vn_engine

:: 创建构建目录
mkdir build
cd build

:: 配置（使用 vcpkg 工具链文件）
cmake .. ^
    -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake ^
    -DVCPKG_TARGET_TRIPLET=x64-windows ^
    -G "Visual Studio 17 2022" ^
    -A x64
```

> **提示**：
> - Visual Studio 2019 使用 `-G "Visual Studio 16 2019"`
> - 如果 vcpkg 安装在其他路径，替换 `C:\vcpkg` 为实际路径

### 5. 构建

```cmd
:: 构建所有目标（Release 模式）
cmake --build . --config Release

:: 或只构建引擎
cmake --build . --config Release --target vn_engine

:: 或只构建编辑器
cmake --build . --config Release --target vn_editor
```

构建产物在 `build\Release\` 目录下：

```
build\Release\
    vn_engine.exe      :: 引擎主程序
    vn_editor.exe      :: 可视化编辑器
    pak_tool.exe       :: 资源打包工具
    *.dll              :: SDL2 等依赖 DLL（CMake 自动复制）
    scripts\           :: 脚本文件（CMake 自动复制）
```

### 6. 使用 IDE 构建

也可以直接用 Visual Studio 打开：

```cmd
:: 生成 .sln 文件后
start vn_engine.sln
```

在 Visual Studio 中选择 `Release` 配置和 `x64` 平台，右键 `ALL_BUILD` → 构建。

---

## 方法二：MinGW + MSYS2

### 1. 安装 MSYS2

从 https://www.msys2.org/ 下载并安装 MSYS2。

### 2. 安装工具链和依赖

打开 **MSYS2 MinGW 64-bit** 终端：

```bash
# 更新包管理器
pacman -Syu

# 安装编译工具链
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-make

# 安装依赖库
pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_image \
          mingw-w64-x86_64-SDL2_ttf mingw-w64-x86_64-SDL2_mixer \
          mingw-w64-x86_64-lua
```

### 3. 配置和构建

```bash
# 进入项目目录
cd /path/to/vn_engine

# 创建构建目录
mkdir build && cd build

# 配置（MinGW Makefiles 生成器）
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

# 构建
mingw32-make -j$(nproc)

# 或使用 cmake 命令
cmake --build .
```

构建产物在 `build\bin\` 目录下。

> **注意**：MinGW 构建需要确保 DLL 在 PATH 中或与可执行文件在同一目录。

---

## 构建验证

构建成功后，验证各组件：

### 引擎测试

```cmd
:: 运行内置测试
Release\vn_engine.exe --test

:: 运行指定脚本
Release\vn_engine.exe --script scripts\example_game.lua
```

预期输出：
```
========================================
  VN Engine - C++/SDL2/Lua
  Visual Novel Engine
========================================

Engine initialized successfully.
...
Engine shutdown.
Goodbye!
```

### 编辑器 CLI 测试

```cmd
:: 新建项目
Release\vn_editor.exe --cli --new --save my_project.lua

:: 导出游戏脚本
Release\vn_editor.exe --cli --open my_project.lua --export my_game.lua

:: 运行导出的游戏
Release\vn_engine.exe --script my_game.lua
```

### 打包工具测试

```cmd
:: 打包脚本目录
Release\pak_tool.exe pack scripts game_data.pak

:: 查看包内容
Release\pak_tool.exe list game_data.pak
```

---

## 运行说明

### 引擎命令行参数

| 参数 | 说明 |
|------|------|
| （无参数） | 运行默认脚本 `scripts/main.lua` |
| `--test` | 运行内置测试模式 |
| `--script <path>` | 运行指定 Lua 脚本 |

### 编辑器命令行参数

| 参数 | 说明 |
|------|------|
| （无参数） | 启动 GUI 编辑器（空项目） |
| `--new` | 新建项目 |
| `--open <file>` | 打开项目文件 |
| `--export <out>` | 导出 Lua 游戏脚本 |
| `--cli` | CLI 模式（无图形界面） |
| `--save <file>` | 保存项目文件（CLI 模式） |

### 资源路径

引擎按以下顺序查找资源文件：
1. 当前工作目录
2. 可执行文件所在目录
3. 已加载的 `.pak` 资源包

---

## 常见问题

### Q1: 找不到 SDL2（CMake 报错 "Could not find SDL2"）

**原因**：CMake 未找到 vcpkg 安装的库。

**解决**：确保使用了 `-DCMAKE_TOOLCHAIN_FILE` 参数指向 vcpkg 工具链文件：

```cmd
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

### Q2: 运行时缺少 DLL（如 SDL2.dll）

**原因**：DLL 不在可执行文件目录或 PATH 中。

**解决**：
- vcpkg + MSVC：CMake 会自动复制 DLL 到输出目录（构建时自动执行）
- MinGW：将 MSYS2 的 `mingw64\bin` 添加到 PATH，或手动复制 DLL

### Q3: 中文字体显示为方块

**原因**：缺少支持中文的 TTF 字体文件。

**解决**：将字体文件放到 `assets/fonts/` 目录下，或修改脚本中的字体路径。推荐使用 `NotoSansCJK` 或 `Microsoft YaHei`。

### Q4: 构建时编译器报错 `M_PI` 未定义

**原因**：MSVC 默认不定义 `M_PI`。

**解决**：已在代码中处理（`platform.h` 提供 `M_PI` 回退定义）。如果仍出现，确保包含了 `engine/platform.h`。

### Q5: CMake 生成器选择

| 场景 | 推荐生成器 |
|------|-----------|
| Visual Studio IDE | `-G "Visual Studio 17 2022" -A x64` |
| 命令行 MSVC | `-G "Ninja"` (需安装 Ninja) |
| MinGW | `-G "MinGW Makefiles"` |
| CLion | IDE 自动选择 |

### Q6: 如何调试构建问题

启用 CMake 详细输出：

```cmd
cmake .. -DCMAKE_TOOLCHAIN_FILE=... --debug-output
```

查看编译器包含路径和库路径：

```cmd
cmake .. -DCMAKE_TOOLCHAIN_FILE=... -DCMAKE_VERBOSE_MAKEFILE=ON
cmake --build . --config Release --verbose
```

---

## 项目结构

```
vn_engine/
├── CMakeLists.txt          :: CMake 构建配置（跨平台）
├── BUILD_WINDOWS.md        :: 本文件
├── README.md               :: 项目说明
├── src/
│   ├── main.cpp            :: 引擎主程序入口
│   ├── engine/             :: 引擎核心（窗口/渲染/音频/资源/平台）
│   │   └── platform.h      :: 跨平台工具头文件
│   ├── script/             :: Lua 脚本系统
│   ├── adv/                :: ADV 文字冒险系统
│   └── ui/                 :: UI 组件
├── tools/
│   ├── pak_tool.cpp        :: 资源打包工具
│   └── vn_editor/          :: 可视化编辑器
├── scripts/                :: Lua 游戏脚本
└── docs/
    └── api_reference.md    :: Lua API 文档
```

## 跨平台支持说明

本项目通过以下机制支持 Windows：

1. **CMake 平台检测**：自动检测 Windows/Unix，条件化依赖查找和链接
2. **platform.h**：提供跨平台工具函数（路径操作、M_PI 定义等）
3. **std::filesystem**：使用 C++17 标准文件系统替代平台特定的 `mkdir`/`stat`
4. **路径分隔符**：代码中使用 `/` 作为路径分隔符（Windows API 兼容）
5. **DLL 自动复制**：CMake POST_BUILD 命令自动复制 SDL2 DLL 到输出目录
