# 03 UI 与交互

## 目录

- [1. 主窗布局](#1-主窗布局)
- [2. App 壳](#2-app-壳)
- [3. 素材库 ResourceWnd](#3-素材库-resourcewnd)
- [4. 预览区](#4-预览区)
- [5. 属性区 PropertyWnd](#5-属性区-propertywnd)
- [6. 时间线 TimeLineWnd](#6-时间线-timelinewnd)
- [7. 交互入口索引](#7-交互入口索引)
- [8. 相关分册](#8-相关分册)

## 1. 主窗布局

[`CMainWindow`](../Src/App/Timeline/MainWindow.cpp) 使用嵌套 `QSplitter`：

```text
Root (Vertical)
├── Top (Horizontal)
│   ├── CResourceWnd          # 素材库
│   ├── PreviewPanel
│   │   ├── CMediaPreviewWnd  # 预览画面
│   │   └── CPreviewPlayBar   # 预览播控条
│   └── CTimelinePropertyHost # 属性
└── CTimeLineWnd              # 时间线
```

默认拉伸：预览区最宽；素材/属性有最小宽度约束。主窗 `MainWindow=true` 以匹配暗色 QSS。

## 2. App 壳

| 单元 | 路径 | 职责 |
|------|------|------|
| `main.cpp` | `Src/App/Timeline/` | QApplication、暗色 QSS、GPU 设备初始化 |
| `CMainWindow` | 同上 | 布局、单例 Init、Bridge 挂接 |
| `StyleHelper` | 同上 | 加载 QSS，处理 `@N$` DPI 占位 |
| resources | `App/Timeline/resources/` | 图标 / rc |

## 3. 素材库 ResourceWnd

路径：[`Src/Libs/UI/ResourceWnd/`](../Src/Libs/UI/ResourceWnd/)

| 类型 | 职责 |
|------|------|
| `CResourceWnd` | Dock 容器，初始化模型 |
| `CResContentWidget` | 内容区 |
| `CResourceTreeList` | 树列表、导入、双击预览 |

导入依赖 Demuxer 探测媒体信息；数据落入 `CResourceModel`（见 [04-data-model.md](04-data-model.md)）。

典型信号：`signalPreviewMedia(path)` → 预览窗 `OpenFile`。

## 4. 预览区

| 类型 | 职责 |
|------|------|
| `CMediaPreviewWnd` | D3D11 预览表面：多层 Present / 软解回落 |
| `CPreviewPlayBar` | 预览侧 Play/Pause 等 |
| `CTimelinePreviewBridge` | 时间线时钟 → 预览同步（逻辑在 06，非纯 UI） |

本册只描述「窗在哪、按钮在哪」；解码与 Present 见 [06](06-preview-pipeline.md) / [07](07-render-and-effects.md)。

## 5. 属性区 PropertyWnd

路径：[`Src/Libs/UI/PropertyWnd/`](../Src/Libs/UI/PropertyWnd/)

| 类型 | 职责 |
|------|------|
| `CTimelinePropertyHost` | 按选中切换面板 |
| `CMediaPropertyFrame` | 媒体属性（音量、颜色、特效参数等） |
| `CTimeLinePropertyFrame` | 时间线级属性 |
| `CTimeLineTransFxPropertyFrame` | 转场类型 / 时长 |
| `MediaKeyframeHelper` 等 | 关键帧写回 |

选中源：`CSelectController`（图层 / 片段 / Tag / TransFx …）。编辑后通过 `signalUpdateMediaProperty`、`signalRefreshWindow`、ClipFx 专用同步信号刷新预览与 Plot。

## 6. 时间线 TimeLineWnd

路径：[`Src/Libs/UI/TimeLineWnd/`](../Src/Libs/UI/TimeLineWnd/)

| 类型 | 职责 |
|------|------|
| `CTimeLineWnd` | 时间线根窗 |
| `CContentWidget` | 内容区编排 |
| `CTopWidget` / `CBarWidget` / `CToolWidget` | 顶栏、工具条 |
| `CLayersViewWidget` / `CLayerLabelsWidget` | 图层视图与标签 |
| `CNoniusViewWidget` | 游标绘制与拖拽 |
| `CScaleWidget` | 时间刻度 |
| `CClipWidget` / `CObjectWidget` | 片段 / 对象 |
| `CClipFxWidget` / Plot / PropertyRow | ClipFx 关键帧 UI |
| `CClipTransFxWidget` | 转场半段编辑 |
| `SContentViewData` (`g_pContentViewData`) | 缩放、帧编辑模式、视觉游标 |

重叠自动创建转场等逻辑在 OverlapHelper 等辅助类中。

## 7. 交互入口索引

| 用户动作 | 进入点 | 后续（他册） |
|----------|--------|--------------|
| 导入素材 | ResourceWnd | Model Resource + Demuxer |
| 双击素材预览 | `signalPreviewMedia` | 06 OpenFile |
| 时间线 Play/Pause | ToolWidget / PlayBar | 05 PlayerController |
| 拖游标 Seek | NoniusViewWidget | 05 Seek + 06 出帧 |
| 选中图层/片段/转场 | SelectController | PropertyHost 刷新 |
| 拖属性滑条 | MediaPropertyFrame | 04 写 MediaNode → 07 刷新 Fx |
| 编辑 ClipFx Plot | ClipFxWidget | 信号同步属性 / 关键帧 |

播控内部实现不在本册展开，见 [05-play-control.md](05-play-control.md)。

## 8. 相关分册

- 模型：[04-data-model.md](04-data-model.md)
- 播控：[05-play-control.md](05-play-control.md)
- 预览 / 渲染：[06-preview-pipeline.md](06-preview-pipeline.md)、[07-render-and-effects.md](07-render-and-effects.md)
