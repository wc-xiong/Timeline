# 02 构建与打包

## 目录

- [1. 环境要求](#1-环境要求)
- [2. 日常构建](#2-日常构建)
- [3. 输出与目录约定](#3-输出与目录约定)
- [4. CMake 要点](#4-cmake-要点)
- [5. Visual Studio 工程](#5-visual-studio-工程)
- [6. 运行时依赖旁路文件](#6-运行时依赖旁路文件)
- [7. 打包链](#7-打包链)
- [8. 相关分册](#8-相关分册)

## 1. 环境要求

| 项 | 说明 |
|----|------|
| 编译器 | Visual Studio 2017 x64（与 bat 生成器一致） |
| 语言 | C++17（根 CMake `CMAKE_CXX_STANDARD 17`） |
| Qt | 环境变量 `QT_DIR` 指向 Qt 前缀（含 `bin/qmake.exe`），建议 5.12+ |
| 三方库 | 固定仓库内 [`3rdparty/`](../3rdparty)；bat/CMake 不采用外部环境中的 `E:\project\...\3rdparty` |
| 打包 | NSIS Unicode（`Installer/setup_nsis_link.bat` 或备用路径） |

`3rdparty` 常见内容：FFmpeg、SDL3、OpenCV、Boost、Rapidjson、DirectXTex、OpenSSL 等。

## 2. 日常构建

```bat
Debug.bat
Release.bat
```

典型流程：

1. 校验 `QT_DIR` / `THIRDPARTY_ROOT`
2. CMake 配置到 `build\Debug` 或 `build\Release`
3. 编译目标 `Timeline`
4. 将 `build\bin\Timeline.exe` 同步到仓库根 `bin\Timeline.exe`

禁止 in-source build：源码树与二进制目录相同会直接 FATAL。

## 3. 输出与目录约定

| 项 | 路径 |
|----|------|
| CMake 二进制目录 | `build\Debug` / `build\Release` |
| 编译输出 exe/lib | `build\bin\`、`build\lib\` |
| 运行入口（bat 同步后） | 根目录 `bin\Timeline.exe` |
| 废弃布局 | 旧根目录 `Debug\` 配置已废弃 |

根 [`CMakeLists.txt`](../CMakeLists.txt)：

```cmake
SET(BIN_DIR ${CMAKE_BINARY_DIR}/../bin)
SET(LIB_DIR ${CMAKE_BINARY_DIR}/../lib)
```

即 `-B build/Debug` 时，`BIN_DIR` 为 `build/bin`（与 Debug/Release 共享输出目录名约定）。

## 4. CMake 要点

- 模块路径：[`cmake/`](../cmake)（`wisConfig`、`wisFindQt`、`FindFFmpeg`、`FindSDL` 等）
- 包含路径含 `Src`、`Src/Libs`
- MSVC：`/utf-8`；强制包含 `MsvcQStringLiteralFix.h` 缓解中文 `QStringLiteral` 问题
- 应用目标：[`Src/App/Timeline/CMakeLists.txt`](../Src/App/Timeline/CMakeLists.txt)
  - GLOB UI / Model / Common / Stubs / Demuxer / PlayCore 等
  - 显式加入 RenderDevice / RenderCommon / RenderBase2D / RenderItem 选定源
  - Controll 中关键帧/滤镜辅助 cpp + Player/Select 头文件
  - POST_BUILD 拷贝 `RenderBase2D/HLSL` 到 exe 旁

## 5. Visual Studio 工程

- 打开：`build\Debug\Timeline.sln` 或 `build\Release\Timeline.sln`
- 筛选器：`SOURCE_GROUP(TREE "${CMAKE_SOURCE_DIR}/Src" PREFIX "Src" …)`，按真实目录树浏览

## 6. 运行时依赖旁路文件

成功运行通常需要（由部署脚本或手工保证）：

- Qt / FFmpeg / SDL / OpenCV 等 DLL
- `HLSL\` 着色器目录（与 `BasicEffect::InitAll` 加载路径一致）
- QRC 内嵌暗色样式与图标（`GSResource/timeline_resource.qrc`）

## 7. 打包链

详见 [Installer/README.md](../Installer/README.md)。摘要：

```text
1. Release.bat → 根 bin\Timeline.exe
2. Installer\Timeline\scripts\PrepareDeploy.bat（windeployqt 等）
3. MoveFilesToInstaller.bat → Installer\Timeline\
4. Pack.bat → NSIS → Timeline_Setup_V1.0.<BUILD>.exe
```

也可使用 FinalBuilder 工程 [`Timeline.bld`](../Timeline.bld)。

注意：安装包须用 **Release** 构建；Debug 依赖带 `d` 后缀 DLL 时，PrepareDeploy 会失败。

## 8. 相关分册

- 总览：[01-system-overview.md](01-system-overview.md)
- 渲染资源 HLSL：[07-render-and-effects.md](07-render-and-effects.md)
