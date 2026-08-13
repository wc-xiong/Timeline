# 08 附录：约定与索引

## 目录

- [1. 命名约定](#1-命名约定)
- [2. 全局单例速查](#2-全局单例速查)
- [3. 关键类与路径](#3-关键类与路径)
- [4. 播控 / 预览相关信号](#4-播控--预览相关信号)
- [5. 能力边界与遗留目录](#5-能力边界与遗留目录)
- [6. 文档地图](#6-文档地图)

## 1. 命名约定

| 前缀 / 形式 | 含义 |
|-------------|------|
| `C*` | 类（如 `CPlayerController`） |
| `I*` | 接口 |
| `g_p*` | 全局访问宏/函数（如 `g_pSignalCenter`） |
| `E*` / `En*` | 枚举 |
| `S*` | 结构体 |
| `signal*` | Qt 信号 |
| `*_STATIC_LIB` | 单体静态编入宏 |
| `*_EXPORT` | 导出宏（多 DLL 产品遗留习惯） |

注意：`EEDitMode_*` 为时间线编辑模式枚举名，与产品名无关。

## 2. 全局单例速查

| 符号 | 模块 |
|------|------|
| `g_pSignalCenter` | SignalCenter |
| `g_pPlayerController` | PlayerController |
| `g_pSelectController` | SelectController |
| `g_pTimeLineModel` | TimeLineModel |
| `g_pMediaModel` | MediaModel |
| `g_pResourceModel` | ResourceModel |
| `g_pConvertHelper` | ConvertHelper |
| `g_pProjectSetting` | PrjSettingData |
| `g_pContentViewData` | ContentViewData |
| `CPreviewGpuPipeline::Instance()` | 硬解管线 |
| `CPreviewFxPass::Instance()` | 特效 Pass |

## 3. 关键类与路径

### App / 预览

| 类 | 路径 |
|----|------|
| `CMainWindow` | `Src/App/Timeline/MainWindow.*` |
| `CMediaPreviewWnd` | `Src/App/Timeline/MediaPreviewWnd.*` |
| `CTimelinePreviewBridge` | `Src/App/Timeline/TimelinePreviewBridge.*` |
| `CPreviewGpuPipeline` | `Src/App/Timeline/PreviewGpuPipeline.*` |
| `CPreviewDecodeSession` | `Src/App/Timeline/PreviewDecodeSession.*` |
| `CPreviewFxPass` | `Src/App/Timeline/PreviewFxPass.*` |
| `CSimpleMediaPlayer` | `Src/App/Timeline/SimpleMediaPlayer.*` |
| `CPreviewPlayBar` | `Src/App/Timeline/PreviewPlayBar.*` |

### UI

| 类 | 路径 |
|----|------|
| `CTimeLineWnd` | `Src/Libs/UI/TimeLineWnd/` |
| `CResourceWnd` | `Src/Libs/UI/ResourceWnd/` |
| `CTimelinePropertyHost` | `Src/Libs/UI/PropertyWnd/` |

### Model

| 类 | 路径 |
|----|------|
| `CTimeLineModel` | `Src/Libs/Model/TimeLineModel/` |
| `CMediaModel` / `CMediaNode` | `Src/Libs/Model/MediaModel/` |
| `CResourceModel` | `Src/Libs/Model/ResourceModel/` |

### 播控

| 类 / 命名空间 | 路径 |
|---------------|------|
| `CPlayerController` | `Src/Libs/Controll/PlayerController/` + Stub |
| `PlayCommandCenter` | `Src/Libs/Controll/PlayCommandCenter/` + Stub |
| `PlayStateCenter` | `Src/Libs/Controll/PlayStateCenter/` + Stub |
| `LocalPlayTransport` | `Src/Libs/Stubs/LocalPlayTransport.cpp` |

### 解码 / 渲染

| 类 | 路径 |
|----|------|
| `CMMDecoder` | `Src/Libs/Play/PlayCore/MMDecoder.*` |
| `BasicEffect` | `Src/Libs/GSMedia/RenderBase2D/BasicEffect.*` |
| `CD3DEffectRender` | `Src/Libs/GSMedia/RenderItem/D3DEffectRender.*` |
| `VertexCalculator` | `Src/Libs/GSMedia/RenderItem/VertexCalculator.*` |

## 4. 播控 / 预览相关信号

| 信号 | 典型用途 |
|------|----------|
| `signalPlayPause` | 播放暂停 |
| `signalNoniusTimeChanged` | 游标时间 |
| `signalSeekTimelineAt` | 请求 Seek |
| `signalFinishSeekTimeline` | Seek 出帧收尾 |
| `signalUpdateMediaProperty` | 属性变更刷新预览 |
| `signalSyncClipFxActiveProperty` | ClipFx → 属性面板 |
| `signalClipFxPropertyKeyframeChanged` | 属性 → ClipFx Plot |
| `signalTimeLineFpsChange` | FPS |
| `signalRefreshWindow` | 通用刷新（若工程中有使用） |

完整声明：[`SignalCenter.h`](../Src/Libs/Common/SignalCenter/SignalCenter.h)。

## 5. 能力边界与遗留目录

**当前主路径具备：**

- 本地 Play/Pause/Seek + 预览同步
- ≤4 路硬解合成 + 软解回落
- ClipFx / TransFx（预览路径）+ 属性关键帧
- NSIS 打包

**未做或不完整：**

- 拖素材入轨、完整工程存盘
- 多屏 / 3D / Spout
- 网络播控、控制端/播放端分离
- 完整 TimeLinePlayer / AudioRender 产品栈

**磁盘常见但未链入 exe 的区域：**

- `Src/Libs/Net/`
- `Src/Libs/Play/` 下除 PlayCore 外的部分子树
- `Src/Libs/GSMedia/ThreadManager` 等
- 各 Lib 自带的独立 `CMakeLists.txt`（Src 级未 `ADD_SUBDIRECTORY(Libs)`）

## 6. 文档地图

返回总目录：[README.md](README.md)

| 分册 | 主题 |
|------|------|
| [01](01-system-overview.md) | 系统总览 |
| [02](02-build-and-packaging.md) | 构建打包 |
| [03](03-ui-and-interaction.md) | UI 交互 |
| [04](04-data-model.md) | 数据模型 |
| [05](05-play-control.md) | 播控 |
| [06](06-preview-pipeline.md) | 预览管线 |
| [07](07-render-and-effects.md) | 渲染特效 |
| [08](08-reference.md) | 本附录 |
