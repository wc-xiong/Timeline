#pragma once
#include "ICopyInterface.h"

#include <QUuid>
#include <QFont>
#include <QSize>
#include <QPixmap>
#include <QDomNode>
#include <rapidjson/document.h>

#include <memory>

////////////////////////////////////素材Model接口///////////////////////////////
class IResourceBase;
class CResourceBase;
class IResourceModel :public ICopyInterface
{
public:
	virtual ~IResourceModel() {}

	//添加素材资源
	virtual	std::weak_ptr<IResourceBase> CreateResource(int nType, const QUuid &id = QUuid::createUuid(), bool bNew = true) = 0;
	//删除素材资源
	virtual	void DeleteResource(const QUuid &id) = 0;
	//清空素材资源
	virtual	void ClearResources() = 0;
	//获取素材
	virtual std::weak_ptr<IResourceBase> GetResourceBaseNode(const QUuid &id) = 0;

	virtual std::list<QUuid> GetAllResNode() = 0;
	// 判断当前素材素材是否添加过
	virtual bool IsResNodeExist(const QString &strPath) = 0;
	// 重命名
	virtual void RenameResource(const QUuid& resId, const QString& name) = 0;
	//获取素材使用次数
	virtual int GetResUseCount(const QUuid &resId) = 0;
	//Model数据保存与加载
	virtual void LoadResourceModelFromRapidJson(const rapidjson::Value &root) = 0;
	virtual void SaveResourceModelToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) = 0;
};

////////////////////////////////////素材基类接口///////////////////////////////
class IResourceBase : public ICopyInterface
{
public:
	virtual ~IResourceBase() {}

	//设置、获取素材QUuid
	virtual void SetId(const QUuid &uuid) = 0;
	virtual QUuid GetId()const = 0;

	//设置、获取文件名称
	virtual void SetName(const QString &strName) = 0;
	virtual QString GetName() const = 0;

	//设置、获取文件名称(包含后缀)
	virtual void SetFullName(const QString &strName) = 0;
	virtual QString GetFullName() const = 0;

	//设置、获取文件类型
	virtual void SetType(int eType) = 0;
	virtual int GetType() = 0;

	//设置、获取素材分辨率
	virtual void SetResolution(QSize sz) = 0;
	virtual QSize GetResolution() = 0;

	//设置、获取文件播放时长
	virtual void SetDuration(__int64 nLenght) = 0;
	virtual __int64 GetDuration() const = 0;

	//设置、获取文件路径
	virtual void SetPath(const QString &strPath) = 0;
	virtual QString GetPath() const = 0;

	//设置、获取文件缩略图
	virtual void SetThumbnail(const QPixmap &pImage) = 0;
	virtual QPixmap GetThumbnail() const = 0;
	virtual QPixmap GetThumbnail(bool bUIImage, bool bNeedUsedSigned) const = 0;

	// 设置文件大小
	virtual void SetFilesize(long long nFileSize) = 0;
	virtual long long GetFilesize() const = 0;

	// 设置声道数目
	virtual void SetAudioChannels(int nAudioChannels) = 0;
	virtual int GetAudioChannels() const = 0;

	//是否支持硬解码
	virtual void SetSupportHWDEC(bool bSupportHWDEC) = 0;
	virtual bool GetbSupportHWDEC() const = 0;

	//是否是硬解码
	virtual void SetHWDEC(bool bHWDEC) = 0;
	virtual bool GetbHWDEC() const = 0;

	// 解码偏好（EDecoding：自动 / 软解 / 硬解）
	virtual void SetDecodingMode(int mode) = 0;
	virtual int GetDecodingMode() const = 0;

	// 设置视频帧率
	virtual void SetVideoFrameRate(double nVeidoFrameRate) = 0;
	virtual double GetVideoFrameRate() const = 0;

	// 设置视频格式
	virtual void SetVideoFormat(QString strVideoFormat) = 0;
	virtual QString GetVideoFormat() const = 0;

	// 设置视频码率
	virtual void SetVideoBitRate(__int64 nVeidoBitRate) = 0;
	virtual __int64 GetVideoBitRate() const = 0;

	// 设置音频格式
	virtual void SetAudioFormat(QString strAudioFormat) = 0;
	virtual QString GetAudioFormat() const = 0;

	// 设置音频采样率
	virtual void SetAudioSampleRate(__int64 nAudioSampleRate) = 0;
	virtual __int64 GetAudioSampleRate() const = 0;

	virtual void SetResourceFormat(const QString &strResourceFormat) = 0;
	virtual QString GetResourceFormat() const = 0;

	// 更新素材使用数目
	virtual void UpdateResourceUseCount(int nCount) = 0;
	virtual void SetResourceUseCount(int nUseCount) = 0;
	virtual int GetResourceUseCount() = 0;

	virtual void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) = 0;
	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root) = 0;

	virtual void SetResDemuxered(bool bValue) = 0;
	virtual bool GetResDemuxered() = 0;

	virtual void SetDemuxerSuccessed(bool bValue) = 0;
	virtual bool GetDemuxerSuccessed() = 0;

	// 是否支持特效设置
	virtual bool IsSupportFx() = 0;

	// 页面尺寸
	virtual void SetPageSize(QSize pageSize) = 0;
	virtual QSize GetPageSize() = 0;

	// 是否持续打开
	virtual void SetKeepOpen(bool bIsKeepOpen) = 0;
	virtual bool IsKeepOpen() = 0;
};

class IFolderResource
{
public:
	virtual ~IFolderResource() {}

	//设置获取文件夹展开状态
	virtual void SetFolderExpand(bool bExpand) = 0;
	virtual bool GetFolderExpand() = 0;

	//设置获取子文件夹类型（虚拟、真实）
	virtual void SetFolderType(bool bVirtual) = 0;
	virtual bool GetFolderType() = 0;

	//根目录
	virtual void SetRootFolder() = 0;
	virtual bool IsRootFolder() = 0;
};


//////////////////////////////////////////////////////////////////////////
class IVideoResource
{
public:
	virtual ~IVideoResource() {}

	// 颜色范围
	virtual int GetColorRange() const = 0;
	virtual void SetColorRange(int colorRange) = 0;

	// HDR
	virtual bool GetSupportHDR() const = 0;
	virtual void SetSupportHDR(bool b) = 0;
};

//////////////////////////////////////////////////////////////////////////
class IPictureResource
{
public:
	virtual ~IPictureResource() {}

	// 图片优化
	virtual int GetOptType() const = 0;
	virtual void SetOptType(int type) = 0;

	virtual void SetOptFilePaths(const std::map<int, QString>& map) = 0;
	virtual int GetMinOptType() const = 0;

	virtual int GetExifOrientation() const = 0;
	virtual void SetExifOrientation(int nType) = 0;

	virtual QString GetOriginalPath() const = 0;
};

//////////////////////////////////////////////////////////////////////////
class IVideoCaptureResource
{
public:
	virtual ~IVideoCaptureResource() {};

	virtual void SetFps(const float& fVal) = 0;
	virtual float GetFps() = 0;

	virtual void SetColorFmt(const QString& color) = 0;
	virtual QString GetColotFmt() = 0;

	virtual void SetDeviceId(const QString &strId) = 0;
	virtual QString GetDeviceId() = 0;
};

//////////////////////////////////////////////////////////////////////////

class IAudioResource
{
public:
	virtual ~IAudioResource() {};

	virtual void SetSampleSize(int nVal) = 0;
	virtual int GetSampleSize() = 0;

	virtual void SetSampleRate(int nVal) = 0;
	virtual int  GetSampleRate() = 0;

	virtual void SetChannels(int nVal) = 0;
	virtual int GetChannels() = 0;
};

class IAudioCaptureResource
{
public:
	virtual ~IAudioCaptureResource() {};

	virtual void SetSampleSize(int nVal) = 0;
	virtual int GetSampleSize() = 0;

	virtual void SetSampleRate(int nVal) = 0;
	virtual int  GetSampleRate() = 0;

	virtual void SetChannels(int nVal) = 0;
	virtual int GetChannels() = 0;

	virtual void SetDeviceId(const QString &strId) = 0;
	virtual QString GetDeviceId() = 0;
};

struct SAudioStreamFormat;
//////////////////////////////////////////////////////////////////////////
class IWebMediaResource
{
public:
	virtual ~IWebMediaResource() {};

	virtual void SetCacheTime(int nTime) = 0;
	virtual int GetCacheTime() const = 0;

	virtual void SetFPS(float fValue) = 0;
	virtual float GetFPS() const = 0;
};

//////////////////////////////////////////////////////////////////////////
struct SMediaScrollParam;
struct SColorTextFillParam;
struct SColorTextOutlineParam;
struct SColorTextShadeParam;
struct SColorTextBackgroundParam;

class ITextResource
{
public:
	virtual ~ITextResource() {}

	//读取
	virtual QString		GetText() = 0;							//文本内容
	virtual bool		IsHasBackGround() = 0;					//是否有背景颜色
	virtual	QColor		GetBackColor() = 0;						//获取背景颜色
	virtual QFont		GetTextFont() = 0;
	virtual QColor		GetTextColor() = 0;
	virtual int			GetAlignMent() = 0;
	virtual int			GetColorTextType() = 0;
	virtual int			GetShowType() = 0;								//获取文本素材类型
	//设置
	virtual void		SetText(const QString& Text) = 0;				//文本内容
	virtual void		SetHasBackGround(bool b) = 0;					//设置是否需要背景
	virtual	void		SetBackColor(const QColor& backColor) = 0;		//背景颜色
	virtual void		SetTextFont(QFont font) = 0;
	virtual void		SetTextColor(QColor clr) = 0;
	virtual void		SetAlignMent(int align) = 0;
	virtual void		SetColorTextType(int nType) = 0;
	virtual void		SetShowType(int nTextType) = 0;				//设置文本素材类型


	virtual SColorTextFillParam GetSColorTextFillParam() = 0;
	virtual SColorTextOutlineParam GetSColorTextOutlineParam() = 0;
	virtual SColorTextShadeParam GetSColorTextShadeParam() = 0;
	virtual SColorTextBackgroundParam GetSColorTextBackgroundParam() = 0;

	virtual void SetSColorTextFillParam(const SColorTextFillParam &node) = 0;
	virtual void SetSColorTextOutlineParam(const SColorTextOutlineParam& node) = 0;
	virtual void SetSColorTextShadeParam(const SColorTextShadeParam& node) = 0;
	virtual void SetSColorTextBackgroundParam(const SColorTextBackgroundParam& node) = 0;

	virtual bool GetSColorTextFillParamEnable() = 0;
	virtual bool GetSColorTextOutlineParamEnable() = 0;
	virtual bool GetSColorTextShadeParamEnable() = 0;
	virtual bool GetSColorTextBackgroundParamEnable() = 0;

	virtual int	GetBackgroundType() = 0;
	virtual bool IsAnimationText() = 0;

	//幻彩字-SetColorTextFillParam-SetCTFParam
	virtual void SetCTFParamEnable(bool bEnable) = 0;								//填充参数是否可用
	virtual void SetCTFParamFillType(int bType) = 0;
	virtual void SetCTFParamFillColor(QColor Color) = 0;
	virtual void SetCTFParamOpacity(double dOpacity) = 0;

	virtual void SetCTFParamGradientColorNum(int nValue) = 0;
	virtual void SetCTFParamGradientColor(QColor Color, int nValue) = 0;
	virtual void SetCTFParamGradientPosition(int nValue, double dPosition) = 0;

	virtual void SetCTFParamFillTexturechange(QString strPath) = 0;
	virtual void SetCTFParamImportTexturechange(QVector<QString>& vecStr) = 0;

	virtual void SetCTFParamGradientDirection(int nType) = 0;
	virtual void SetCTFParamTextureFillType(int nType) = 0;
	virtual void SetCTFParamGradientScrollType(int nValue) = 0;
	virtual void SetCTFParamTextureScrollType(int nValue) = 0;
	virtual void SetCTFParamScrollSpeed(int nValue) = 0;

	//幻彩字-SetColorTextOutlineParam-SetCTOParam
	virtual void SetCTOParamEnable(bool bEnable) = 0;
	virtual void SetCTOParamOutlineColor(QColor Color) = 0;
	virtual void SetCTOParamThickness(int nValue) = 0;
	virtual void SetCTOParamOpacity(double dValue) = 0;

	//幻彩字-SetColorTextShadeParam-SetCTSParam
	virtual void SetCTSParamEnable(bool bEnable) = 0;
	virtual void SetCTSParamShadeColor(QColor Color) = 0;
	virtual void SetCTSParamAngle(int nValue) = 0;
	virtual void SetCTSParamDistance(int nValue) = 0;
	virtual void SetCTSParamAmbiguity(double dValue) = 0;

	//幻彩字-SetColorTextBackgroundParam-SetCTBParam
	virtual void SetCTBParamEnable(bool bEnable) = 0;
	virtual void SetCTSParamBackgroundType(int bType) = 0;
	virtual void SetCTBParamBackgroundColor(QColor Color) = 0;
	virtual void SetCTBParamOpacity(double dValue) = 0;
	virtual void SetCTBParamFillet(double dValue) = 0;
	virtual void SetCTBParamWidth(double dValue) = 0;
	virtual void SetCTBParamHeight(double dValue) = 0;
};

struct SBaseInfo;
struct SColorModeSelect;
struct SOrder;
struct SSplineLine;
struct SCheek;
struct SCircle;
struct SDiagonal;
struct SResolution;
struct SBoxNum;
struct SLogo;
struct STextInfo;
struct STestChartData;

class ITestChartResource
{
public:
	virtual ~ITestChartResource() {}

	virtual QImage GetTestChartImage() = 0;//获取测试图     	
	virtual void SetTestChartImage(const QImage & testChartImage) = 0;//设置测试图

	//读取
	virtual SBaseInfo GetBaseInfo() = 0;                  //基本信息
	virtual SColorModeSelect GetColorModeSelect() = 0;    //颜色模式
	virtual SOrder GetOrder() = 0;                        //序号
	virtual SSplineLine GetSplineLine() = 0;              //分隔线
	virtual SCheek GetCheek() = 0;                        //边框
	virtual SCircle GetCircle() = 0;                      //圆
	virtual SDiagonal GetDiagonal() = 0;                  //对角线
	virtual SResolution GetResolutionInfo() = 0;          //获取分辨率
	virtual SBoxNum GetBoxNum() = 0;                      //获取箱体数
	virtual SLogo GetLogo() = 0;                          //获取logo
	virtual STextInfo GetTextInfo() = 0;                  //获取文字信息            

	virtual STestChartData GetTestChartParam() = 0;     //获取测试图参数
	//设置
	virtual void SetBaseInfo(const SBaseInfo & baseInfo) = 0;
	virtual void SetColorModeSelect(const SColorModeSelect & colorMode) = 0;
	virtual void SetOrder(const SOrder & order) = 0;
	virtual void SetSplineLine(const SSplineLine & splineLine) = 0;
	virtual void SetCheek(const SCheek & cheek) = 0;
	virtual void SetCircle(const SCircle & circle) = 0;
	virtual void SetDiagonal(const SDiagonal & diagonal) = 0;
	virtual void SetResolutionInfo(const SResolution & resolution) = 0;
	virtual void SetBoxNum(const SBoxNum & boxNum) = 0;
	virtual void SetLogo(const SLogo & logo) = 0;
	virtual void SetTextInfo(const STextInfo & textInfo) = 0;
	virtual void SetTestChartParam(const STestChartData & testChartParam) = 0;
	virtual void SetColorMode(const int nType) = 0;
	// 仅更新分辨率与 baseInfo 宽高，不重算 Logo/分割线等（类型切换恢复用）
	virtual void SetResolutionKeepProps(QSize sz) = 0;

};


//////////////////////////////////////////////////////////////////////////
class INdiStreamResource
{
public:
	virtual ~INdiStreamResource() {};

	virtual QString GetIp() = 0;
	virtual void SetIp(QString strIp) = 0;

	// video format
	virtual void SetFps(float fps) = 0;
	virtual void SetColor(QString Color) = 0;
	virtual float GetFps() = 0;
	virtual QString GetColor() = 0;

	// audio format
	virtual void SetChannel(int channel) = 0;
	virtual void SetBitSample(int bitSample) = 0;
	virtual void SetSampleRate(int sampleRate) = 0;
	virtual int GetChannel() = 0;
	virtual int GetBitSample() = 0;
	virtual int GetSampleRate() = 0;
};

struct SSubtitleDataNode;
// 字幕数据
class ISubtitleResource
{
public:
	virtual ~ISubtitleResource() {}

	// 设置、获取字幕数据
	virtual void SetSubtitleDatas(std::vector<SSubtitleDataNode> vecDatas) = 0;
	virtual std::vector<SSubtitleDataNode> GetSubtitleDatas() = 0;
	virtual bool GetSubTitleEmpty() = 0;
	virtual SSubtitleDataNode GetIndexSubtitleNode(int nIndex) = 0;

	// 字幕显示方向
	virtual void SetSubtitleDir(bool bHor) = 0;
	virtual bool GetSubtitleDir() = 0;

	// 设置显示字体
	virtual void SetShowFont(QFont font) = 0;
	virtual QFont GetShowFont() = 0;

	// 字体颜色
	virtual void SetFontColor(QColor clr) = 0;
	virtual QColor GetFontColor() = 0;

	// 背景颜色
	virtual void SetTextBkClrEnable(bool bValue) = 0;
	virtual bool GetTextBkClrEnable() = 0;

	virtual void SetTextBkClr(QColor bValue) = 0;
	virtual QColor GetTextBkClr() = 0;

	virtual int GetPlayTextIndexByTime(__int64 nMs) = 0;
};

////////////////////////////////////提词器素材//////////////////////////////////////
struct STeleprompterItem;
class CTeleprompterModel;
class ITeleprompterResource
{
public:
	virtual void AddDefaultTeleprompterItem(int index) = 0;

	virtual bool DeleteTeleprompterItem(int index) = 0;
	virtual void SetTeleprompterItemDur(int index, long long duration) = 0;
	virtual void SetTeleprompterItemText(int index, const QString &text) = 0;
	virtual int GetTeleprompterItemCount() = 0;

	virtual void SetTeleprompterData(std::vector <STeleprompterItem> itemList) = 0;
	virtual std::vector <STeleprompterItem> GetTeleprompterData() = 0;

	virtual long long GetTeleprompterItemDur(int index) = 0;
	virtual QString GetTeleprompterItemText(int index) = 0;

	// 字幕显示方向
	virtual void SetShowDir(bool bHor) = 0;
	virtual bool GetShowDir() = 0;

	// 设置显示字体
	virtual void SetShowFont(QFont font) = 0;
	virtual QFont GetShowFont() = 0;

	// 字体颜色
	virtual void SetFontColor(QColor clr) = 0;
	virtual QColor GetFontColor() = 0;

	// 背景颜色
	virtual void SetTextBkClrEnable(bool bValue) = 0;
	virtual bool GetTextBkClrEnable() = 0;

	virtual void SetTextBkClr(QColor bValue) = 0;
	virtual QColor GetTextBkClr() = 0;

	virtual void SetAlignment(int nAlignment) = 0;
	virtual int GetAlignment() = 0;
};

class ITurnPageResource
{
public:
	virtual ~ITurnPageResource() {}

	virtual void SetCurPage(int nPage) = 0;
	virtual int  GetCurPage() = 0;
};