# Timeline 打包说明

FinalBuilder + Installer 打包流程（已去除全部 SVN 相关步骤与脚本）。

## 目录结构

| 路径 | 作用 |
|------|------|
| [Timeline.bld](../Timeline.bld) | FinalBuilder 工程（无 SVN 组） |
| [Pack.bat](../Pack.bat) | 一键打包入口 |
| [MoveFilesToInstaller.bat](../MoveFilesToInstaller.bat) | Pack\bin → Installer\Timeline |
| [pack.bat](../pack.bat) | 调用 NSIS 生成安装包（版本号用 git count 或日期） |
| [Installer/Timeline/scripts/](Timeline/scripts/) | Release 编译、windeployqt |
| [Installer/installerScripts/](installerScripts/) | setup.nsi / app.nsh（MUI2，无牛牛皮肤依赖） |
| [Installer/setup_nsis_link.bat](setup_nsis_link.bat) | 联接 NSIS Unicode 工具链 |

## 环境要求

- `QT_DIR`：Qt 5.12+（含 `windeployqt`）
- 三方库：工程内 `3rdparty`（构建脚本固定使用，不依赖外部环境 `THIRDPARTY_ROOT`）
- Visual Studio 2017 x64（与 `Release.bat` 相同）
- NSIS Unicode：`Installer\setup_nsis_link.bat` 指向可用的 `Installer\NSIS`

## 打包流程

```text
1. 配置环境变量 QT_DIR
2. （首次）运行 Installer\setup_nsis_link.bat
3. 任选其一：
   A. 一键：根目录 Pack.bat
   B. FinalBuilder 打开 Timeline.bld 按步骤执行
   C. 分步：
      Release.bat
      Installer\Timeline\scripts\PrepareDeploy.bat
      MoveFilesToInstaller.bat
      pack.bat
4. 根目录得到 Timeline_Setup_V1.0.<BUILD>.exe
```

## 安装包内容

- `Timeline.exe` 及 Qt / FFmpeg / SDL 等 DLL  
- `platforms` 等 Qt 插件  
- `HLSL` 着色器  
- 开始菜单 / 桌面快捷方式与卸载程序  

## 注意

- **安装包必须使用 Release 构建**。若 `bin\Timeline.exe` 是 Debug（依赖 `opencv_world450d.dll` / `Qt5Cored.dll`），`PrepareDeploy.bat` 会直接失败。
- 请先能成功 `Release.bat` 生成根目录 `bin\Timeline.exe`（由 `build\bin` 同步），再打包。
- 签名步骤未纳入；需要时可自行追加到 `Timeline.bld`
- Staging 目录 `Installer\Timeline\` 下的 exe/dll 已被 `.gitignore` 忽略
