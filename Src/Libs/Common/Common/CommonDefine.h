#pragma once
#include <QUuid>
#include <QJsonObject>
#include <QString>
#include "LanguageDefine.h"
#include <winerror.h>

const QString ResMimeType = "ResourceItem";	//素材库内部拖拽
const QString SysMimeType = "text/uri-list";	//直接从系统中拖拽文件进入素材库
const QString ModelObjectIDMimeType = "ModelObjectID";	//对象列表场景模型实例 ID（跨面板拖拽）

#define APP_CODE_REBOOT 10086	// 软件重启号

static const std::string g_strPriateKey = "-----BEGIN PRIVATE KEY-----\
\nMIICdgIBADANBgkqhkiG9w0BAQEFAASCAmAwggJcAgEAAoGBANT75ANKLC9A3iV0\
\nSgaGLla/R2QllizX/1qQfEquJ7rTNxddopwsQ9iBaasIVTc1EX3bL4L1Xc2GQ82i\
\njtiD+jTZndjhg6R+fUSc4SxPSPnJ3az22hnIzrm75giK0LpDqlxV6+r+Bqq92J5o\
\nlJS/hCNrFfgBFD4XcCdjdKANN5vVAgMBAAECgYBeNLx6KAi7LfoUnwdPM3KztA+j\
\n4Gw4kaHFMTuI3Nd974bisuwGIB4yWauflyP6M2L1REoTpuxCyCydfFWqt1Wn1BHY\
\nFxD8y/OW4ueumrsCaJWi+fGusMRf7GY8/sKpXqu13L9M1O7wzd1steTg8hRoX5Pv\
\nMFv6EdNYetIwY1CegQJBAPKdwVd0WISZ/QLii2/mCDe1y9v2CixBOqIN+MTwqWOy\
\nB3EVFabo0gDjqko2U6eBs+QMzLOKn5O8T8s8HeH5PMkCQQDgu6qEt1a98BzoLnHw\
\nk1Qqni+9+vs1GBsEvGPNXf+Ny23LQO+Dl7zcDvcglGHiceZKRi+uOreUSvEml5Mu\
\nqUitAkAhJ6XQSbn/BPwqdMMRW6Ir2tAtEwZ43cEhfD9OyoSbdyzFK1h2IsLwFCyl\
\nT59G+NaYh5dBwePk0Ism/uQmiWThAkATXFeOY3UXm+4Bmq66ORcRLNuUFGE76724\
\nJLq8YJK5loL49bksrw/YLXng1qewKHvcfjtxWSGjpukrl/dYF8FpAkEA2w3HabzR\
\nix6wBBLdcUEm001TpKbdJjsbOAnV7xUzfRjd/mw6Ln0l5HxUl8j4Mu8Q6GdwjkWB\
\nRaZq24YqNgp6Vw==\
\n-----END PRIVATE KEY-----";

static const std::string g_strPublicKey = "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDU++QDSiwvQN4ldEoGhi5Wv0dk\
\nJZYs1/9akHxKrie60zcXXaKcLEPYgWmrCFU3NRF92y+C9V3NhkPNoo7Yg/o02Z3Y\
\n4YOkfn1EnOEsT0j5yd2s9toZyM65u+YIitC6Q6pcVevq/gaqvdieaJSUv4QjaxX4\
\nARQ+F3AnY3SgDTeb1QIDAQAB";

#define Xml_Project_Data QString("ProjectData")
#define Xml_Project_Version QString("ProjectVersion")
#define Xml_Project_Id QString("ProjectId")
#define Xml_Project_Type QString("ProjectType")

#define Xml_Project_APPStage QString("APPStage")
#define Xml_Project_GlobalData QString("GlobalData")
#define Xml_Project_GlobalData_QuadOptType QString("QuadOptType")

#define Xml_Project_ModelData_Resource QString("ResourceModelData")
#define Xml_Project_ModelData_Media QString("MediaModelData")
#define Xml_Project_ModelData_Slice QString("SliceModelData")
#define Xml_Project_ModelData_Model QString("ModelModelData")
#define Json_Project_ModelData_TimeLine QString("TimeLineModelData")
#define Json_Project_ModelData_VirtualScreen QString("VirtualScreenData")
#define Json_Project_ModelData_TimeCodeRecv QString("TimeCodeRecvData")

#define ModelModel_Xml QString("ModelModel")
#define ModelDataModel_Xml QString("ModelDataModel")

#define ZoomLevelCountMin	1
#define ZoomLevelCountMax	400
#define WNDRADIO(x) ((x)/100.0f)

#define ZoomLevelCount	12
const int nRatio[ZoomLevelCount] = {1, 5, 10, 25,50,75,100,125,150,200,300,400 };	//缩放比率

//软件各窗口类型
enum EDockWidgetType
{
	EDWT_ResourceWnd = 0, 
	EDWT_SliceManageWnd,
	EDWT_PropertyWnd,
	EDWT_GridWnd,
	EDWT_ModelWnd,
	EDWT_ScreenMgrWnd,
	EDWT_TimeLineWnd,
	EDWT_InputSliceViewWnd,
	EDWT_OutputSliceViewWnd,		//这里的顺序会影响软件启动时，堆叠窗口的顺序
	EDWT_ResPropertyWnd,

	EDWT_NetworkStatusWnd,
	EDWT_CueListWnd,				// CueList 窗口
	EDWT_CameraRecordWnd,			// 相机录制（默认浮动、默认隐藏）
	EDWT_TimeCodeRecvWnd,			// 时间码接收

	EDWT_ModelViewWnd,				// 主界面固定窗口要作为最后一个窗口，若添加新窗口，请添加在此前面

	EDWT_Count,
	EDWT_DefaultWindowsMgr,	// 默认窗口布局
	EDWT_SaveCurLayout,
};

enum EDockAreaWidgetType //停靠区域类型
{
	EDAT_Normal = 0,			//正常，正常停靠
	EDAT_Fixed					//固定，禁止一切停靠,也禁止移动拉伸
};

/////////////////////////////////////枚举状态/////////////////////////////////////
//播放模式（考虑修改或删除）
enum EnPlayMode
{
	EPM_INVALID = 0x00,				// 未知状态

	EPM_NORMAL = 0x10,				// 窗口播放
	EPM_NORMAL_EXIT,				// 退出播放
};


enum EnPlayState
{
	EPS_Invalid = -1,		// 无效值
	EPS_PLAYING,			// 正在播放
	EPS_PAUSE,				// 暂停
	EPS_PAUSE_SECTION_END,	// Section末尾播放导致的暂停
	EPS_FadeOut,			// 淡出中
	EPS_FadeOutFinish,		// 淡出结束
};

enum EnPauseType
{
	EPT_Normal = 0,			// 正常停止
	EPT_SectionEnd,			// Section末尾播放停止
};

enum EFPS
{
	E_30FPS = 0,
	E_50FPS,
	E_60FPS,
	E_120FPS,
	E_240FPS,
};

enum EDecoding
{
	ED_Auto = 0,
	ED_Sort,
	ED_Hard,
};

enum HDRDeviceIndex
{
	EDI_HDR = 0,
	EDI_SDR,
};

//字体操作
enum ETextBtHandleType
{
	EMHT_Bold = 0,
	EMHT_Italic,
	EMHT_UnderLine,
	EMHT_AlignLeft,
	EMHT_AlignHcenter,
	EMHT_AlignRight,
	EMHT_AlignJustify,
	EMHT_AlignTop,
	EMHT_AlignVcenter,
	EMHT_AlignBottom,

	EMHT_Count,
};

//动态幻彩字预设类型-12种  presupposition types
enum EDynamicColorTextPreType
{
	EDCPT_Forbidden = 0,
	EDCPT_DynamicBackground1,		//动态背景
	EDCPT_DynamicBackground2,
	EDCPT_DynamicBackground3,
	EDCPT_DynamicBackground4,
	EDCPT_DynamicBackground5,
	EDCPT_DynamicBackground6,
	EDCPT_WhiteCBlackB,				//White character and black box 白字黑框 下同
	EDCPT_WhiteCRedB,
	EDCPT_BlueCWhiteB,
	EDCPT_BlackCWhiteBg,				//黑字白底
	EDCPT_BlackCYellowBg,
	EDCPT_WhiteCCyanS,				//Black character with cyan shade  黑字青色阴影，下同
	EDCPT_YellowCOrangeS,

	EDCPT_Count,
};

//静态幻彩字预设类型-12种  presupposition types
enum EStaticColorTextPreType
{
	ESCPT_Forbidden = 0,
	ESCPT_WhiteCBlackB,		//White character and black box 白字黑框 下同
	ESCPT_BlackCWhiteB,
	ESCPT_BlueCWhiteB,
	ESCPT_WhiteCRedB,
	ESCPT_YellowTBlackB,
	ESCPT_OrangeTWhiteB,
	ESCPT_WhiteCBlackBg,		//white character with black background 白字黑底，下同
	ESCPT_BlackCWhiteBg,
	ESCPT_BlackCYellowBg,
	ESCPT_WhiteCRedBg,
	ESCPT_BlackCCyanS,		//Black character with cyan shade  黑字青色阴影，下同
	ESCPT_YellowCOrangeS,
	ESCPT_WhiteCBlueS,

	ESCPT_Count,
};


enum ETimerDisplayAccuracy
{
	ETDA_Day = 0,
	ETDA_Hour,
	ETDA_Minute,
	ETDA_Second,

	ETDA_Count,
};

// 操作日志操作端
enum EControllerType
{
	ECT_PC = 0,
	ECT_APP,
	ECT_CenterControl,
	ECT_CloudControl
};
////////////////////////////图片exif信息 旋转角度定义////////////////////////////////////
#define MIN_PIC_OPT_WIDTH		(1920)
#define MIN_PIC_OPT_HEIGHT		(1080)

#define AUTO_PIC_OPT_WIDTH		(3840)
#define AUTO_PIC_OPT_HEIGHT		(2160)

// 软件三种操作模式：预览、调试、播控
enum EAppOpType
{
	ESL_Preview = 0,
	ESL_Debug,
	ESL_PlayControll,
	ESL_Count,
};

//四边形优化
enum EQuadOpt
{
	EQO_NoQuadOpt = 0,
	EQO_PerspectQuadOpt,
	EQO_StrechQuadOpt,
	EQO_Count,
};

//模型展开类型
enum EUnwrapType
{
	EUT_None = -1,						// 
	EUT_Common,							// 内置展开
	EUT_Camera_Perspective,				// 透视展开
	EUT_Camera_Orthogonal,				// 正交视角
	EUT_FullView_21VR,					// 2:1VR全景展开
	EUT_FullView_SingleFisheye,			// 单鱼眼全景展开
	EUT_FullView_DualFisheye,			// 双鱼眼全景展开
	EUT_FullView_Skybox,				// 天空盒全景展开
};

//vr session 状态类型
enum EVRSessionState
{
	EVRSS_UNKNOWN = 0,
	EVRSS_IDLE = 1,
	EVRSS_READY = 2,
	EVRSS_SYNCHRONIZED = 3,
	EVRSS_VISIBLE = 4,
	EVRSS_FOCUSED = 5,
	EVRSS_STOPPING = 6,
	EVRSS_LOSS_PENDING = 7,
	EVRSS_EXITING = 8,
	EVRSS_MAX_ENUM = 0x7FFFFFFF
};

enum ECameraMovementState
{
	ECMS_Horizon = 0,
	ECMS_WithPitch = 1,
};

enum EVRCameraEye
{
	EVRCE_LeftEye = 0,
	EVRCE_RightEye,
	EVRCE_Count
};

enum EPageTurnType
{
	EPTT_Previous = 0,
	EPTT_Next,
	EPTT_First,
	EPTT_Last,
};

enum EToastType
{
	ETT_Success = 0,
	ETT_Fail,
	ETT_Waring,
};

enum ESectionPlayMode						//Section播放模式
{
	ECPM_Invalid = -1,
	ECPM_Play = 0,							//正常继续播放，播放下一个Section
	ECPM_EndOfSection = 1,					//播放到Section末尾
	ECPM_LoopSection = 2,					//循环Section
};

enum ESwitchPlayMode						//Section跳转播放模式
{
	ESPM_Invalid = -1,
	ESPM_NormalPlay = 0,					//正在Section中间播放
	ESPM_ReadyFinish = 1,					//快要播放到Section尾部
	ESPM_FinishPlayToSectionEnd = 2,		//播放到Section尾部
	ESPM_FinishPlayToSectionLoop = 3,		//播放到Section尾部循环跳转
};

enum EPreviewModelAnchorType
{
	EPMAT_OriginalCenter = 0, // 原始中心 (0,0,0)
	EPMAT_GeometryCenter,     // 整体中心 (W/2,H/2,D/2)
	EPMAT_TopCenter,          // 顶部中心
	EPMAT_BottomCenter,       // 底部中心
	EPMAT_LeftCenter,         // 左侧中心
	EPMAT_RightCenter,        // 右侧中心
	EPMAT_FrontCenter,        // 前侧中心
	EPMAT_BackCenter,         // 后侧中心
	EPMAT_Count
};

///////////////////////////// 基础定义  /////////////////////////////////////
//指针空判断
#define POINT_VOID_RETURN_NULL(p)			{	/*ASSERT(p != nullptr);*/ if(p == nullptr) return nullptr;		}
#define POINT_VOID_RETURN_ret(p, ret)		{	/*ASSERT(p != nullptr);*/ if(p == nullptr) return ret;			}
#define POINT_VOID_RETURN_Void(p)			{	/*ASSERT(p != nullptr);*/ if(p == nullptr) return;				}
#define POINT_VOID_Break(p)					{	/*ASSERT(p != nullptr);*/ if(p == nullptr) break;				}
#define POINT_VOID_Continue(p)				{	/*ASSERT(p != nullptr);*/ if(p == nullptr) continue;			}

// assert
#define ASSERT_HANDLER(expresstion, handler) \
if(!(expresstion)) \
{ \
	assert(0); \
	handler; \
}

#define FAILED_RETURN(hr)				{	if(FAILED(hr)) return hr;	}
#define FAILED_RETURN_ret(hr, ret)		{	if(FAILED(hr)) return ret;	}
#define FAILED_RETURN_void(hr)			{	if(FAILED(hr)) return;		}
#define FAILED_BREAK(hr)				{	if(FAILED(hr)) break;		}
#define FAILED_CONTINUE(hr)				{	if(FAILED(hr)) continue;	}

#define SUCCEEDED_RETURN(hr)			{	if(SUCCEEDED(hr)) return hr;	}
#define SUCCEEDED_RETURN_ret(hr, ret)	{	if(SUCCEEDED(hr)) return ret;	}
#define SUCCEEDED_RETURN_void(hr)		{	if(SUCCEEDED(hr)) return;		}
#define SUCCEEDED_BREAK(hr)				{	if(SUCCEEDED(hr)) break;		}
#define SUCCEEDED_CONTINUE(hr)			{	if(SUCCEEDED(hr)) continue;	}


#define FAILED_RETURN_hr				FAILED_RETURN(hr)
#define FAILED_RETURN_ret_hr(ret)		FAILED_RETURN_ret(hr, ret)
#define FAILED_RETURN_void_hr			FAILED_RETURN_void(hr)
#define FAILED_BREAK_hr					FAILED_BREAK(hr)
#define FAILED_CONTINUE_hr				FAILED_CONTINUE(hr)
#define FAILED_GOTO(hr, label)			if (FAILED(hr)) goto label;

#define SUCCEEDED_RETURN_hr				SUCCEEDED_RETURN(hr)
#define SUCCEEDED_RETURN_ret_hr(ret)	SUCCEEDED_RETURN_ret(hr, ret)
#define SUCCEEDED_RETURN_void_hr		SUCCEEDED_RETURN_void(hr)
#define SUCCEEDED_BREAK_hr				SUCCEEDED_BREAK(hr)
#define SUCCEEDED_CONTINUE_hr			SUCCEEDED_CONTINUE(hr)
#define SUCCEEDED_GOTO(hr, label)		if (SUCCEEDED(hr)) goto label;

#define FAIL_RET(x)						do{ if(FAILED(hr=(x))) return hr;}while(0)

#define IF_FALSE_Continue(Flag)			{if(!(Flag))	continue;}		
#define IF_FALSE_Break(Flag)			{if(!(Flag))	break;}		
#define IF_FALSE_RETURN_Null(Flag)		{if(!(Flag))	return nullptr;}		
#define IF_FALSE_RETURN_Ret(Flag,ret)	{if(!(Flag))	return ret;}		
#define IF_FALSE_RETURN_Void(Flag)		{if(!(Flag))	return;}

#define IF_TRUE_Continue(Flag)			{if(Flag)	continue;}		
#define IF_TRUE_Break(Flag)				{if(Flag)	break;}		
#define IF_TRUE_RETURN_Null(Flag)		{if(Flag)	return nullptr;}		
#define IF_TRUE_RETURN_Ret(Flag,ret)	{if(Flag)	return ret;}		
#define IF_TRUE_RETURN_Void(Flag)		{if(Flag)	return;}




///////////////////////////// 基础定义  /////////////////////////////////////
