#pragma once
#include "UICommonGlobal.h"
#include <QSize>
#include <QString>
// 此头文件仅用于定义界面公共数据大小，例如：界面、控件宽高等
UICOMMON_EXPORT void InitMainScreenDpi();

UICOMMON_EXPORT float GetSystemDpi();

UICOMMON_EXPORT QString ProcessDpiStyleSheet(const QString& rawQss);

UICOMMON_EXPORT void InitSystemDpi(const QString& qssPath = QStringLiteral(":/style/dark/stylesheet/style.css"));

//富文本的tooltip生成函数
UICOMMON_EXPORT QString getCustomToolTip(const QString& tipText, const QString& shortcutText);

#define SysDpiZoom(nSize) int(floor((nSize) * GetSystemDpi()))

#define SysDpiZoomEx(nSize) int(round((nSize) * GetSystemDpi()))

#define UICommon_CtrlHeight SysDpiZoom(24)
#define UICommon_ImageBtnWidth SysDpiZoom(24)
#define UICommon_LabelHeight SysDpiZoom(16)

#define UICommon_SmallBtnWidth SysDpiZoom(72)
#define UICommon_MidBtnWidth SysDpiZoom(96)

#define UICommon_DevideHeight SysDpiZoom(1)
#define UICommon_DevideWidth SysDpiZoom(1)

#define UICommon_SmallBtnSize QSize(UICommon_SmallBtnWidth, UICommon_CtrlHeight)
#define UICommon_ImageBtnSize QSize(UICommon_ImageBtnWidth, UICommon_CtrlHeight)

// DockWidget最小高度
#define DockWidgetMinWidth  SysDpiZoom(340)
#define DockWidgetMinHeight  SysDpiZoom(150)

// 此部分定义各界面各种间距
#define nSizeDefine_1 SysDpiZoom(1)
#define nSizeDefine_2 SysDpiZoom(2)
#define nSizeDefine_3 SysDpiZoom(3)
#define nSizeDefine_4 SysDpiZoom(4)
#define nSizeDefine_5 SysDpiZoom(5)
#define nSizeDefine_6 SysDpiZoom(6)
#define nSizeDefine_7 SysDpiZoom(7)
#define nSizeDefine_8 SysDpiZoom(8)
#define nSizeDefine_9 SysDpiZoom(9)
#define nSizeDefine_10 SysDpiZoom(10)
#define nSizeDefine_11 SysDpiZoom(11)
#define nSizeDefine_12 SysDpiZoom(12)
#define nSizeDefine_13 SysDpiZoom(13)
#define nSizeDefine_14 SysDpiZoom(14)
#define nSizeDefine_15 SysDpiZoom(15)
#define nSizeDefine_16 SysDpiZoom(16)
#define nSizeDefine_18 SysDpiZoom(18)
#define nSizeDefine_19 SysDpiZoom(19)
#define nSizeDefine_22 SysDpiZoom(22)
#define nSizeDefine_23 SysDpiZoom(23)
#define nSizeDefine_24 SysDpiZoom(24)
#define nSizeDefine_26 SysDpiZoom(26)
#define nSizeDefine_20 SysDpiZoom(20)
#define nSizeDefine_25 SysDpiZoom(25)
#define nSizeDefine_28 SysDpiZoom(28)
#define nSizeDefine_30 SysDpiZoom(30)
#define nSizeDefine_32 SysDpiZoom(32)
#define nSizeDefine_35 SysDpiZoom(35)
#define nSizeDefine_36 SysDpiZoom(36)
#define nSizeDefine_38 SysDpiZoom(38)
#define nSizeDefine_39 SysDpiZoom(39)
#define nSizeDefine_40 SysDpiZoom(40)
#define nSizeDefine_42 SysDpiZoom(42)
#define nSizeDefine_44 SysDpiZoom(44)
#define nSizeDefine_45 SysDpiZoom(45)
#define nSizeDefine_46 SysDpiZoom(46)
#define nSizeDefine_48 SysDpiZoom(48)
#define nSizeDefine_50 SysDpiZoom(50)
#define nSizeDefine_52 SysDpiZoom(52)
#define nSizeDefine_54 SysDpiZoom(54)
#define nSizeDefine_55 SysDpiZoom(55)
#define nSizeDefine_56 SysDpiZoom(56)
#define nSizeDefine_58 SysDpiZoom(58)
#define nSizeDefine_60 SysDpiZoom(60)
#define nSizeDefine_61 SysDpiZoom(61)
#define nSizeDefine_62 SysDpiZoom(62)
#define nSizeDefine_63 SysDpiZoom(63)
#define nSizeDefine_64 SysDpiZoom(64)
#define nSizeDefine_65 SysDpiZoom(65)
#define nSizeDefine_66 SysDpiZoom(66)
#define nSizeDefine_68 SysDpiZoom(68)
#define nSizeDefine_70 SysDpiZoom(70)
#define nSizeDefine_72 SysDpiZoom(72)
#define nSizeDefine_74 SysDpiZoom(74)
#define nSizeDefine_76 SysDpiZoom(76)
#define nSizeDefine_80 SysDpiZoom(80)
#define nSizeDefine_82 SysDpiZoom(82)
#define nSizeDefine_83 SysDpiZoom(83)
#define nSizeDefine_84 SysDpiZoom(84)
#define nSizeDefine_88 SysDpiZoom(88)
#define nSizeDefine_92 SysDpiZoom(92)
#define nSizeDefine_90 SysDpiZoom(90)
#define nSizeDefine_96 SysDpiZoom(96)
#define nSizeDefine_100 SysDpiZoom(100)
#define nSizeDefine_104 SysDpiZoom(104)
#define nSizeDefine_108 SysDpiZoom(108)
#define nSizeDefine_110 SysDpiZoom(110)
#define nSizeDefine_112 SysDpiZoom(112)
#define nSizeDefine_118 SysDpiZoom(118)
#define nSizeDefine_120 SysDpiZoom(120)
#define nSizeDefine_122 SysDpiZoom(122)
#define nSizeDefine_124 SysDpiZoom(124)
#define nSizeDefine_128 SysDpiZoom(128)
#define nSizeDefine_129 SysDpiZoom(129)
#define nSizeDefine_130 SysDpiZoom(130)
#define nSizeDefine_140 SysDpiZoom(140)
#define nSizeDefine_144 SysDpiZoom(144)
#define nSizeDefine_148 SysDpiZoom(148)
#define nSizeDefine_152 SysDpiZoom(152)
#define nSizeDefine_160 SysDpiZoom(160)
#define nSizeDefine_162 SysDpiZoom(162)
#define nSizeDefine_164 SysDpiZoom(164)
#define nSizeDefine_170 SysDpiZoom(170)
#define nSizeDefine_178 SysDpiZoom(178)
#define nSizeDefine_180 SysDpiZoom(180)
#define nSizeDefine_186 SysDpiZoom(186)
#define nSizeDefine_190 SysDpiZoom(190)
#define nSizeDefine_200 SysDpiZoom(200)
#define nSizeDefine_210 SysDpiZoom(210)
#define nSizeDefine_215 SysDpiZoom(215)
#define nSizeDefine_217 SysDpiZoom(217)
#define nSizeDefine_230 SysDpiZoom(230)
#define nSizeDefine_240 SysDpiZoom(240)
#define nSizeDefine_241 SysDpiZoom(241)
#define nSizeDefine_246 SysDpiZoom(246)
#define nSizeDefine_259 SysDpiZoom(259)
#define nSizeDefine_278 SysDpiZoom(278)
#define nSizeDefine_286 SysDpiZoom(286)
#define nSizeDefine_320 SysDpiZoom(320)
#define nSizeDefine_324 SysDpiZoom(324)
#define nSizeDefine_364 SysDpiZoom(364)
#define nSizeDefine_360 SysDpiZoom(360)
#define nSizeDefine_400 SysDpiZoom(400)
#define nSizeDefine_411 SysDpiZoom(411)
#define nSizeDefine_440 SysDpiZoom(440)
#define nSizeDefine_441 SysDpiZoom(441)
#define nSizeDefine_510 SysDpiZoom(510)
#define nSizeDefine_724 SysDpiZoom(724)
#define nSizeDefine_1080 SysDpiZoom(1080)

// 素材库
#define Res_ThumbnailWidth 320
#define Res_ThumbnailHeight 180

#define Res_ThumbnailUIShowWidth SysDpiZoom(42)
#define Res_ThumbnailUIShowHeight SysDpiZoom(24)

#define Res_IconSize SysDpiZoom(16)

//环境信息
#define CMFCardSettingsDialog_Width SysDpiZoom(960)
#define CMFCardSettingsDialog_Height SysDpiZoom(640)

//屏幕分窗
#define CDivideRect_Width SysDpiZoom(774)
#define CDivideRect_Height SysDpiZoom(500)
#define CDivideRectScreen_Width SysDpiZoom(511)
#define CDivideRectView_Width SysDpiZoom(490)
#define CDivideRectView_Height SysDpiZoom(390)

//分辨率计算器
#define CalculatorDialog_Width SysDpiZoom(262)
#define CalculatorDialog_Height SysDpiZoom(368)


//幻彩字
#define UICommon_TextureBtnWidth SysDpiZoom(48)
#define UICommon_ImportTextureImageWidth SysDpiZoom(44)