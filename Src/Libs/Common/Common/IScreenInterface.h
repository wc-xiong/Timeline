#pragma once
#include "CommonGlobal.h"
#include "ICopyInterface.h"
#include "Common/Common/Maths/Point.h"
#include "Common/Common/ICustomDataInterface.h"

#include <QRect>
#include <QUuid>
#include <memory>
#include <vector>
#include <map>
#include <QString>
#include <rapidjson/document.h>

struct STestChartData;

namespace CusMappingRenderData
{
	struct ICustomRenderFactor;
}

enum ESpliteDataFlag
{
	ESpliteData_None = 0,
	ESpliteData_Pre = 1 << 0,
	ESpliteData_Out = 1 << 1,
	ESpliteData_All = ESpliteData_Pre | ESpliteData_Out,
};

class IScreenItem: public ICopyInterface, public IManipulableObject
{
public:
	virtual ~IScreenItem() {}

	virtual QUuid GetID()const = 0;
	virtual void SetID(const QUuid& id) = 0;

	virtual QRect GetRect(bool bInput)const = 0;
	virtual void SetRect(bool bInput, const QRect& rc) = 0;

	virtual quint32  GetPhysicalID()const = 0;
	virtual void SetPhysicalID(quint32 PhysicalID) = 0;

	virtual int GetPhysicalX()const = 0;
	virtual int GetPhysicalY()const = 0;
	virtual int GetPhysicalWidth()const = 0;
	virtual int GetPhysicalHeight()const = 0;
	virtual QRect GetPhysicalRect()const = 0;

	virtual void SetPhysicalX(int x) = 0;
	virtual void SetPhysicalY(int y) = 0;
	virtual void SetPhysicalWidth(int width) = 0;
	virtual void SetPhysicalHeight(int height) = 0;
	virtual void SetPhysicalRect(const QRect& rc) = 0;

	virtual QString GetPhysicalName()const = 0;
	virtual void SetPhysicalName(const QString& name) = 0;

	virtual float GetFPS()const = 0;
	virtual void SetFPS(float FPS) = 0;

	virtual bool IsPrimary()const = 0;
	virtual void SetPrimary(bool bPrimary) = 0;

	virtual bool IsOpened() const = 0;
	virtual void SetOpened(bool bUsed) = 0;

	virtual bool IsUseful() const = 0;
	virtual void SetUseful(bool bUseful) = 0;

	virtual bool IsAdded()const = 0;
	virtual void SetAdded(bool bAdded) = 0;

	virtual bool IsVirtual()const = 0;
	virtual void SetVirtual(bool b) = 0;

	virtual QString GetName() const = 0;
	virtual void SetName(const QString& name) = 0;

	virtual int GetIndex() = 0;
	virtual void SetIndex(int nIndex) = 0;

	virtual int GetDXGIindex() = 0;
	virtual void SetDXGIIndex(int nIndex) = 0;

	// DXGI Adapter 枚举序号（与 DeviceResource_GetAdapterId 对齐）；未探测到为 -1
	virtual int GetAdapterIndex() const = 0;
	virtual void SetAdapterIndex(int nIndex) = 0;

	virtual void SaveDataToRapidJson(bool bSaveReciver, rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator, int nSpliteDataFlag = ESpliteData_None) = 0;
	virtual bool LoadDataFromRapidJson(bool bSaveReciver, const rapidjson::Value& root, int nSpliteDataFlag = ESpliteData_None) = 0;

	virtual QRectF GetWrapRect(bool bInput) = 0;

	struct SLayoutReceiver
	{
		unsigned int        prcIndex = 0;       // 发送器序号
		unsigned int        portIndex = 0;      // 网口序号
		unsigned int        rcvIndex = 0;       // 在此网口下接收卡序号

		// 接收卡位置
		int                 x = 0;              // 位置x
		int                 y = 0;              // 位置y
		int                 width = 0;          // 宽
		int                 height = 0;         // 高

		QString				filePath;			//连接文件路径

		SLayoutReceiver(){}
		SLayoutReceiver(unsigned int PrcIndex, unsigned int PortIndex,
			unsigned int RcvIndex, int X, int Y, int Width, int Height, const QString& FilePath)
		{
			prcIndex = PrcIndex;
			portIndex = PortIndex;
			rcvIndex = RcvIndex;

			x = X;
			y = Y;
			width = Width;
			height = Height;
			filePath = FilePath;
		}
	};

	virtual bool HasLayoutReceiver() = 0;
	virtual void SetLayoutReceiver(bool bAdd, const std::vector<SLayoutReceiver>& layoutReceivers) = 0;
	virtual bool GetLayoutReceiver(std::vector<SLayoutReceiver>& layoutReceivers, bool bLocal = false) = 0;

	virtual std::vector<CPointD> GetDrawRect(bool bInput) = 0;
	virtual std::vector<CPointD> GetDrawSizeRect(bool bInput) = 0;

	virtual void MapDataTo(std::shared_ptr<IScreenItem> pdata) = 0;
	virtual void CopyScreenPty(std::shared_ptr<IScreenItem> pOther) = 0;
	virtual void CopyScreenPhysicalPty(std::shared_ptr<IScreenItem> pOther) = 0;

	virtual void SetSpliteSliceData(bool bPre, std::shared_ptr<CusMappingRenderData::ICustomRenderFactor> ptr) = 0;
	virtual std::shared_ptr<CusMappingRenderData::ICustomRenderFactor> GetSpliteSliceData(bool bPre) = 0;

	virtual void SetConnTestChartFile(const QString& file) = 0;
	virtual QString GetConnTestChartFile() = 0;

	virtual bool HasConnTestChartData() const = 0;
	virtual const STestChartData& GetConnTestChartData() const = 0;
	virtual void SetConnTestChartData(const STestChartData& data) = 0;
	virtual void ClearConnTestChartData() = 0;

};

struct SAudioOutputDevice
{
	QString deviceKey;
	QString displayName;
	QString sdlName;
	bool isDefault = false;
};

class IDeviceItem :public ICopyInterface
{
public:
	virtual ~IDeviceItem() = default;

	virtual QUuid GetDeviceId()const = 0;
	virtual void SetDeviceId(const QUuid& id) = 0;

	virtual bool IsVirtual()const = 0;
	virtual void SetVirtual(bool b) = 0;

	// 获取设备添加屏幕数量
	virtual int GetAddScreenCount() = 0;

	virtual QString GetName() const = 0;
	virtual void SetName(const QString& name) = 0;

	virtual int GetMaxIndex() const = 0;
	virtual void SetMaxIndex(int index) = 0;

	virtual void AddScreen(std::shared_ptr <IScreenItem> p) = 0;
	virtual void DeleteScreen(const QUuid& id) = 0;
	virtual std::weak_ptr <IScreenItem> GetScreen(const QUuid& id) = 0;

	virtual int GetScreenCount() = 0;
	virtual std::vector<QUuid> GetAllScreenIDs() = 0;

	virtual std::weak_ptr <IScreenItem> GetScreen(quint32 physicalID) = 0;

	virtual bool HasScreenOpen() = 0;

	virtual QString GetScreenName(quint32 nPhysicalID) = 0;

	virtual void SetIsUseful(bool bValue) = 0;
	virtual bool IsUseful() = 0;

	virtual void SetPeerId(const QString& strPeerId) = 0;
	virtual QString GetPeerId() = 0;

	virtual std::vector<SAudioOutputDevice> GetAudioOutputDevices() const = 0;
	virtual void SetAudioOutputDevices(const std::vector<SAudioOutputDevice>& devices) = 0;
	virtual bool FindAudioOutputDevice(const QString& deviceKey, SAudioOutputDevice& outDevice) const = 0;

	virtual void SaveDataToRapidJson(bool bSaveReciver, rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator, int nSpliteDataFlag = ESpliteData_None) = 0;
	virtual bool LoadDataFromRapidJson(bool bSaveReciver, const rapidjson::Value& root, bool bClearAll, int nSpliteDataFlag = ESpliteData_None) = 0;
};

struct SCustomScreen;

class IDeviceModel :public ICopyInterface
{
public:
	virtual ~IDeviceModel() = default;

	virtual void AddDevice(std::shared_ptr <IDeviceItem> p) = 0;
	virtual void DeleteDevice(const QUuid& id) = 0;
	virtual void DeleteDeviceByPeerId(const QString& strPeerId) = 0;
	virtual std::weak_ptr <IDeviceItem> GetDevice(const QUuid& id) = 0;

	virtual std::weak_ptr <IDeviceItem> GetDeviceByPeerid(const QString& strPeerId) = 0;

	virtual bool AddScreen(const QUuid &deviceId, std::shared_ptr <IScreenItem> p) = 0;
	virtual void DeleteScreen(const QUuid& id) = 0;

	virtual std::weak_ptr<IScreenItem> GetScreen(const QUuid& screenId, QUuid &deviceId) = 0;
	virtual std::weak_ptr <IScreenItem> GetScreen(const QUuid& id) = 0;
	virtual std::weak_ptr<IScreenItem> GetScreen(const SCustomScreen& screen) = 0;
	virtual std::weak_ptr<IScreenItem> GetScreen(quint32 physicalId) = 0;

	virtual std::vector<QUuid> GetDeviceIDs(bool bWithVirtrual = true) = 0;

	virtual std::vector<IScreenItem::SLayoutReceiver> GetAllScreenReceiver() = 0;

	virtual int GetAddScreenCount() = 0;

	// 获取本地上一次保存的数据
	virtual std::shared_ptr<IScreenItem> GetLocalScreenData1(const quint32& PhysicalID) = 0;
	virtual std::weak_ptr <IDeviceItem> GetLocalDevice(const QUuid& id) = 0;

	virtual bool HasScreenOpen() = 0;

	virtual std::vector<QUuid> GetAllScreenIds(bool bWithVirtual = true) = 0;

	virtual void SaveLocalScreenData() = 0;
	virtual bool LoadLocalScreenData() = 0;

	virtual void ConnTestChart(bool bConn) = 0;
	virtual bool IsConnTestChart() = 0;

	// FTB（Fade To Black）运行时状态，不随工程保存
	virtual void SetFTB(bool bOn) = 0;
	virtual bool IsFTB() const = 0;
};
