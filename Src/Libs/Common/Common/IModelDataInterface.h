#pragma once
#include "CommonGlobal.h"
#include "ICopyInterface.h"
#include "CustomScreen.h"
#include "GSMedia/RenderCommon/SimpleMath.h"
#include "GSMedia/RenderCommon/LightHelper.h"
#include "Common/Common/Maths/Point.h"
#include "Common/Common/Maths/Matrix3x2.h"
#include "Common/Common/CommonDefine.h"
#include <array>
#include <vector>
#include <deque>
#include <chrono>
#include <QUuid>
#include <QString>
#include <QPointF>
#include <QtXml>
#include <QDomDocument>
#include <DirectXCollision.h>
#include <rapidjson/document.h>

#define MaxLightNum 48

enum EModelTreeNodeType
{
	EMTNT_None = -1,
	EMTNT_Root = 0,
	EMTNT_CustomFolder,
	EMTNT_Custom,
	EMTNT_SightFolder,
	EMTNT_Sight,
	EMTNT_DefaultFolder,
	EMTNT_Sphere,
	EMTNT_IrregularPrism, 
	EMTNT_ImmersiveSpace,
	EMTNT_RotationSolid,
	EMTNT_Plane,
	EMTNT_AuxiliaryFolder,
	EMTNT_Camera,
	EMTNT_DirectionalLight,
	EMTNT_PointLight,
	EMTNT_SpotLight,
	EMTNT_VRCamera,

	EMTNT_ModuleInference,
	EMTNT_SightImage,
	EMTNT_SliceOnly,
	EMTNT_AIPanorama,
	//新模型类型请添加到这行注释的上一行, 后面几个类型相关公共接口也需要检查一下
	EMTNT_num
};

static bool IsEditableModel(int nType)
{
	switch (nType)
	{
	case EMTNT_Custom:
	case EMTNT_Plane:
	case EMTNT_Sphere:
	case EMTNT_RotationSolid:
	case EMTNT_IrregularPrism:
	case EMTNT_ImmersiveSpace:
	case EMTNT_ModuleInference:
		return true;
	default:
		break;
	}
	return false;
}

static bool IsCameraModel(int nType)
{
	return nType == EMTNT_Camera || nType == EMTNT_VRCamera;
}

static bool IsModelEditableInStage(int nModelType, int eAppStage)
{
	if (eAppStage == ESL_Preview)
		return true;
	if (eAppStage == ESL_PlayControll)
		return false;
	if (eAppStage == ESL_Debug)
		return IsCameraModel(nModelType);
	return true;
}

static bool IsAuxiliaryModel(int nType)
{
	switch (nType)
	{
	case EMTNT_AuxiliaryFolder:
	case EMTNT_Camera:
	case EMTNT_VRCamera:
	case EMTNT_DirectionalLight:
	case EMTNT_PointLight:
	case EMTNT_SpotLight:
		return true;
	default:
		break;
	}
	return false;
}

static bool IsCopyableModel(int nType)
{
	switch (nType)
	{
	case EMTNT_Sight:
	case EMTNT_SightImage:
	case EMTNT_AIPanorama:
	case EMTNT_Camera:
	case EMTNT_DirectionalLight:
	case EMTNT_PointLight:
	case EMTNT_SpotLight:
		return true;
	default:
		break;
	}
	return false;
}

static bool IsCanExpandModel(int nType)
{
	switch (nType)
	{
	case EMTNT_Custom:
	case EMTNT_Sphere:
	case EMTNT_IrregularPrism:
	case EMTNT_ImmersiveSpace:
	case EMTNT_RotationSolid:
	case EMTNT_Plane:
	case EMTNT_ModuleInference:
	case EMTNT_SliceOnly:
		return true;
	default:
		break;
	}
	return false;
}

static bool IsSupportShowOnly(int nType)
{
	switch (nType)
	{
	case EMTNT_Custom:
	case EMTNT_Sight:
	case EMTNT_SightImage:
	case EMTNT_AIPanorama:
	case EMTNT_Sphere:
	case EMTNT_IrregularPrism:
	case EMTNT_ImmersiveSpace:
	case EMTNT_RotationSolid:
	case EMTNT_Plane:
	case EMTNT_ModuleInference:
		return true;
	default:
		break;
	}
	return false;
}

static bool IsShowOnlyExcludedModelType(int nType)
{
	switch (nType)
	{
	case EMTNT_Camera:
	case EMTNT_VRCamera:
	case EMTNT_DirectionalLight:
	case EMTNT_PointLight:
	case EMTNT_SpotLight:
		return true;
	default:
		break;
	}
	return false;
}

static bool IsSupportLightTrack(int nType)
{
	switch (nType)
	{
	case EMTNT_Custom:
	case EMTNT_Sight:
	case EMTNT_SightImage:
	case EMTNT_AIPanorama:
	case EMTNT_Sphere:
	case EMTNT_IrregularPrism:
	case EMTNT_ImmersiveSpace:
	case EMTNT_RotationSolid:
	case EMTNT_Plane:
	case EMTNT_ModuleInference:
		return true;
	default:
		break;
	}
	return false;
}

enum EClipboardTransformKind
{
	ECTK_None = 0,
	ECTK_Position = 1,
	ECTK_Rotation = 2,
	ECTK_Scale = 4,
};

/** 按模型类型返回可编辑的变换分量（与属性面板一致）。 */
static int GetModelTransformCapabilities(int modelType)
{
	int caps = ECTK_None;
	switch (modelType)
	{
	case EMTNT_Camera:
	case EMTNT_SpotLight:
		caps = ECTK_Position | ECTK_Rotation;
		break;
	case EMTNT_VRCamera:
		break;
	case EMTNT_DirectionalLight:
		caps = ECTK_Rotation;
		break;
	case EMTNT_PointLight:
		caps = ECTK_Position;
		break;
	default:
		caps = ECTK_Position | ECTK_Rotation | ECTK_Scale;
		break;
	}
	return caps;
}

enum EModelDataTreeNodeType
{
	EMDTNT_Root,
	EMDTNT_Folder,
	EMDTNT_Node,
	EMDTNT_Null
};

enum EModelAnchorType
{
	EMAT_OriginalCenter,
	EMAT_BottomCenter,
	EMAT_GeometryCenter,
	EMAT_Num
};

class IModelNode :public ICopyInterface
{
public:
	virtual ~IModelNode() = default;
	virtual QUuid GetID() = 0;
	virtual void SetID(const QUuid& id) = 0;
	virtual int GetType() = 0;
	virtual QString GetName() = 0;
	virtual void SetName(const QString& name) = 0;
	virtual bool LoadDataFromRapidJson(const rapidjson::Value &root) = 0;
	virtual void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) = 0;
	static QUuid CreateModelNodelID(int type)
	{
		QUuid id;
		switch (type)
		{
		case EMTNT_Root:
			id = QUuid::fromString(QString("5E3B2A5C-587B-43EF-A029-A18D2E110F25"));
			break;
		case EMTNT_CustomFolder:
			id = QUuid::fromString(QString("20C4212E-8A4F-4B83-AF96-447B64054AFD"));
			break;
		case EMTNT_DefaultFolder:
			id = QUuid::fromString(QString("F5E8FA2D-2B62-49EB-9BFF-C7A5C06A7126"));
			break;
		case EMTNT_SightFolder:
			id = QUuid::fromString(QString("1DD94947-650F-4C63-A876-C56435BCF1EE"));
			break;
		case EMTNT_Sphere:
			id = QUuid::fromString(QString("53AE5B50-1892-4121-928B-1C3156BAD128"));
			break;
		case EMTNT_IrregularPrism:
			id = QUuid::fromString(QString("9045858A-3448-7C8E-1C23-46522645CC21"));
			break;
		case EMTNT_RotationSolid:
			id = QUuid::fromString(QString("56C4253A-4557-2EC8-91C3-47833945AB42"));
			break;
		case EMTNT_ImmersiveSpace:
			id = QUuid::fromString(QString("C8F42E1A-6D3B-4F90-A1E2-7B5C9D0E3F41"));
			break;
		case EMTNT_Plane:
			id = QUuid::fromString(QString("1CD0BA83-E72A-4FA1-BCAD-1A308226582B"));
			break;
		case EMTNT_AuxiliaryFolder:
			id = QUuid::fromString(QString("A51F3D8E-485F-4E1E-AD2E-5F7F4A88999A"));
			break;
		case EMTNT_Camera:
			id = QUuid::fromString(QString("31219EE3-7A15-4907-AD0B-46BD1A18E5B3"));
			break;
		case EMTNT_DirectionalLight:
			id = QUuid::fromString(QString("0E02D7C5-82C2-4055-96C6-A13EDD2EDD37"));
			break;
		case EMTNT_PointLight:
			id = QUuid::fromString(QString("AF336D80-79AA-46E5-A57F-D4664CAA8E24"));
			break;
		case EMTNT_SpotLight:
			id = QUuid::fromString(QString("AC37E6AA-804B-4EB4-B00D-BB0DABC9A01E"));
			break;
		case EMTNT_ModuleInference:
			id = QUuid::fromString(QString("BB46E5B9-9E5D-5DC5-CE1C-CA1CBAD8BE2D"));
			break;
		case EMTNT_AIPanorama:
			id = QUuid::fromString(QString("A1F4A0B4-C0D3-4A1A-B2C3-D4E5F6071829"));
			break;
		default:
			id = QUuid::createUuid();
			break;
		}
		return id;
	}
};

class IModelResource
{
public:
	virtual ~IModelResource() = default;
	virtual QString GetFilePath() = 0;
	virtual void SetFilePath(const QString& path) = 0;
	virtual bool IsFileExist() = 0;
	virtual void SetFileExist(bool bExist) = 0;
	virtual int GetAnchorType() = 0;
	virtual void SetAnchorType(int type) = 0;
	virtual void IncreaseUseCount() = 0;
	virtual void DecreaseUseCount() = 0;
	virtual int GetUseCount() = 0;
};

class IModelImageSightResource
{
public:
	virtual ~IModelImageSightResource() = default;
	virtual int GetImageWidth() = 0;
	virtual void SetImageWidth(int width) = 0;
	virtual int GetImageHeight() = 0;
	virtual void SetImageHeight(int height) = 0;
	// false: 单面（默认）；true: 双面
	virtual bool GetImageDoubleSided() = 0;
	virtual void SetImageDoubleSided(bool doubleSided) = 0;
};

struct GeometryData;
class Material;
class Transform;
class ThirdPersonCamera;
class IBaseModelData :public ICopyInterface
{
public:
	virtual ~IBaseModelData() = default;
	virtual bool IsInit() = 0;
	virtual void Init(int nAnthorType = -1) = 0;
	virtual void SetInitState(bool b) = 0;
	virtual int GetModelType() = 0;
	virtual Transform& GetTransform() = 0;
	virtual float GetScaleSize() = 0;
	virtual QUuid GetID() = 0;
	virtual void SetID(const QUuid& id) = 0;
	virtual QUuid GetResourceID() = 0;
	virtual void SetResourceID(const QUuid& id) = 0;
	virtual QString GetName() = 0;
	virtual void SetName(const QString& name) = 0;
	virtual bool IsVisible() = 0;
	virtual void SetVisible(bool bVisible) = 0;
	virtual bool IsHideByShowOnly() = 0;
	virtual void SetHideByShowOnly(bool bHide) = 0;
	virtual bool IsLock() = 0;
	virtual void SetLock(bool bLock) = 0;
	virtual DirectX::XMMATRIX GetWorldMatrix() = 0;
	// 局部坐标系包围盒缓存（用于 Skybox 相关计算复用）
	virtual void SetLocalBoundingBox(const DirectX::BoundingBox& bbox) = 0;
	virtual void GetLocalBoundingBox(DirectX::BoundingBox& bbox) const = 0;
	virtual void BindObserveObjId(const QUuid& id, bool unique = false) = 0;
	virtual void UnBindObserveObjId(const QUuid& id) = 0;
	virtual void GetBindObserveObjId(std::vector<QUuid>& id) = 0;
	virtual void ClearBindObserveObjId() = 0;
	virtual void BindLightObserveObjId(const QUuid& id, bool unique = false) = 0;
	virtual void UnBindLightObserveObjId(const QUuid& id) = 0;
	virtual void GetBindLightObserveObjId(std::vector<QUuid>& id) = 0;
	virtual void ClearBindLightObserveObjId() = 0;
	virtual bool LoadDataFromRapidJson(const rapidjson::Value &root) = 0;
	virtual void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) = 0;
};

struct SPreUnitModelDatas;
struct SModelGeometryData;
class IModelData
{
public:
	virtual ~IModelData() = default;
	virtual QUuid GetPreUnitSliceTreeId() = 0;
	virtual void SetOutputSliceTreeId(const QUuid & id) = 0;
	virtual QUuid GetOutputSliceTreeId() = 0;
	// 获取当前操作切片树id，若是存在输出切片则返回输出切片，否则返回预览单元切片
	virtual QUuid GetCurOpSliceTreeId() = 0;
	virtual void SetScaleSize(float fsize) = 0;
	virtual int GetExpansionType(int nAppOpType) = 0;
	virtual void SetExpansionType(int nAppOpType, int nType) = 0;
	virtual QString GetCustomPath() = 0;
	virtual void SetCustomPath(QString& path) = 0;
	virtual void SetCamera(std::shared_ptr<ThirdPersonCamera> ptr) = 0;
	virtual std::shared_ptr<ThirdPersonCamera> GetCamera() = 0;
	virtual const Material& GetMaterials() = 0;
	virtual GeometryData& GetOriGeoData() = 0;
	virtual bool CreateFromDataEx(const GeometryData& geometryData) = 0;
	virtual void SetPreviewUnitVisible(bool b) = 0;
	virtual bool IsPreviewUnitVisible() = 0;
	virtual void GetDpiSize(int& width, int& height) = 0;
	virtual void SetAssociatedScreen(const SCustomScreen& screen) = 0;
	virtual void GetAssociatedScreen(SCustomScreen& screen) = 0;
};

class IModelScreen
{
public:
	virtual ~IModelScreen() = default;
	virtual void SetAnchor(int type, DirectX::SimpleMath::Vector3 &Anchor) = 0;

	virtual int GetExpandOptimizeType() const = 0;
	virtual void SetExpandOptimizeType(int nType) = 0;
	virtual int GetNormalCoplanarThresholdIndex() const = 0;
	virtual void SetNormalCoplanarThresholdIndex(int nIndex) = 0;
	virtual bool GetNonQuadFaceSeparate() const = 0;
	virtual void SetNonQuadFaceSeparate(bool bSeparate) = 0;
	virtual bool GetQuadSliceOptimize() const = 0;
	virtual void SetQuadSliceOptimize(bool bOptimize) = 0;
};

class IModelCamera
{
public:
	virtual ~IModelCamera() = default;
	virtual std::shared_ptr<ThirdPersonCamera> GetCamera() = 0;
	virtual void SetPerspective(bool bValue) = 0;
	virtual void SetFovY(float value) = 0;
	virtual void SetNearZ(float value) = 0;
	virtual void SetFarZ(float value) = 0;
	virtual void SetAspectRadio(float value) = 0;
	virtual void AutoLockObservation(const DirectX::XMVECTOR& objectModelCenter, const DirectX::XMFLOAT3& objectModelYAxisPosVector, const DirectX::XMFLOAT3& objectModelXAxisPosVector) = 0;
	virtual float GetFovY() = 0;
	virtual float GetNearZ() = 0;
	virtual float GetFarZ() = 0;
	virtual float GetAspectRadio() = 0;
	virtual float GetViewPortRadio() = 0;
	virtual void SetViewPortRaido(float value) = 0;
	virtual void SetTarget(const DirectX::XMFLOAT3& target) = 0;

	// 路径相机
	virtual bool IsPathCamera() const = 0;
	virtual void SetPathCamera(bool b) = 0;
	virtual int GetPathCameraClipCount() const = 0;
	virtual void IncrementPathCameraClipCount() = 0;
	virtual std::chrono::milliseconds GetPathPoseTime() const = 0;
	virtual void SetPathPoseTime(const std::chrono::milliseconds& ms) = 0;
	virtual void AddPathCameraClipId(const QUuid& clipId, const std::chrono::milliseconds& startTime) = 0;
	virtual void RemovePathCameraClipId(const QUuid& clipId) = 0;
	virtual void UpdatePathCameraClipStartTime(const QUuid& clipId, const std::chrono::milliseconds& newStartTime) = 0;
	virtual const std::vector<QUuid>& GetPathCameraClipIds() const = 0;
	/** 游标是否落在该相机任一路径动画片段内（DrawModels 每帧同步，不持久化） */
	virtual bool IsInPathCameraClip() const = 0;
	virtual void SetInPathCameraClip(bool b) = 0;

};

// VR眼镜只支持添加一个，当前固定VR相机Model Id
#define VRCameraId "193DD8DF-9F09-4219-AFA5-629351E158E6"
class IModelVRCamera
{
public:
	virtual ~IModelVRCamera() = default;
	virtual DirectX::XMMATRIX GetWorldMatrix() = 0;
	virtual void SetRotationQuaternion(const DirectX::XMFLOAT4& rotationQuaternion) = 0;
	virtual DirectX::SimpleMath::Vector4 GetRotationQuaternion() const = 0;

	// 基准相机id
	virtual QUuid GetBaseCameraId() = 0;
	virtual void SetBaseCameraId(const QUuid &id) = 0;
	virtual int GetCameraMoveState() = 0;
	virtual void SetCameraMoveState(int state) = 0;
	virtual DirectX::XMFLOAT3 GetInitialPosition() = 0;
	virtual void SetInitialPosition(const DirectX::XMFLOAT3& pos) = 0;
	virtual float GetSmoothPrecision() = 0;
	virtual void SetSmoothPrecision(float precision) = 0;
	virtual float GetXMinLimit() = 0;
	virtual float GetYMinLimit() = 0;
	virtual float GetZMinLimit() = 0;
	virtual float GetXMaxLimit() = 0;
	virtual float GetYMaxLimit() = 0;
	virtual float GetZMaxLimit() = 0;
	virtual bool IsXLimit() = 0;
	virtual bool IsYLimit() = 0;
	virtual bool IsZLimit() = 0;
	virtual void SetXMinLimit(float X) = 0;
	virtual void SetYMinLimit(float Y) = 0;
	virtual void SetZMinLimit(float Z) = 0;
	virtual void SetXMaxLimit(float X) = 0;
	virtual void SetYMaxLimit(float Y) = 0;
	virtual void SetZMaxLimit(float Z) = 0;
	virtual void SetXLimit(bool bLimit) = 0;
	virtual void SetYLimit(bool bLimit) = 0;
	virtual void SetZLimit(bool bLimit) = 0;
	virtual bool IsPosOutOfBound() = 0;
	virtual bool IsPosOutOfBound(const DirectX::XMFLOAT3& position) = 0;
	virtual void BoundPos(DirectX::XMFLOAT3& position) = 0;
	virtual void SetEyePos(const DirectX::XMFLOAT3& pos, EVRCameraEye eye) = 0;
	virtual DirectX::XMFLOAT3 GetEyePos(EVRCameraEye eye) = 0;
};

class IModelSight
{
public:
	virtual ~IModelSight() = default;
	virtual void SetScaleSize(float fsize) = 0;
	virtual int GetAnchorType() = 0;
	virtual void SetAnchorType(int type) = 0;
	virtual DirectX::XMFLOAT3 GetAnchor() = 0;
	virtual DirectX::XMFLOAT3 GetOriginalAnchor() = 0;
	virtual void SetAnchor(int type, DirectX::SimpleMath::Vector3 &Anchor) = 0;
};

// 场景图片静物接口：仅用于 EMTNT_SightImage 场景节点的数据访问
class IModelSightImage
{
public:
	virtual ~IModelSightImage() = default;
	// false: 单面（默认）；true: 双面
	virtual bool GetDoubleSided() = 0;
	virtual void SetDoubleSided(bool bDoubleSided) = 0;
};

class CCommonTreeViewNode;
class IModelDataModel :public ICopyInterface
{
public:
	virtual ~IModelDataModel() = default;
	virtual CCommonTreeViewNode* GetTreeRoot() = 0;
	virtual CCommonTreeViewNode* GetCopyTreeNode(const QUuid& Id) = 0;
	virtual std::shared_ptr<IBaseModelData> CreateModel(int type) = 0;
	virtual std::weak_ptr<IBaseModelData> GetModel(const QUuid& id) = 0;
	virtual void AddModel(std::shared_ptr<IBaseModelData> model) = 0;
	virtual bool IsModelExist(const QUuid& id) = 0;
	virtual void AddFolder(const QUuid& Id, const QString& name) = 0;
	virtual void DeleteChild(const QUuid& childId) = 0;
	virtual void ChangeParentOfChild(const QUuid& childId, const QUuid& newParentId) = 0;
	virtual void ChangeIndexOfChild(const QUuid& childId, int index) = 0;
	virtual void ClearModel() = 0;
	virtual void GetAllNodeIDs(std::vector<QUuid>& vecNodeIDs) = 0;
	virtual void GetLeafIDs(const QUuid& parentId, std::vector<QUuid>& vecLeafIDs) = 0;
	virtual void GetNodeIDs(const QUuid& parentId, std::vector<QUuid>& vecNodeIDs) = 0;
	virtual bool IsLock(const QUuid& id) = 0;
	virtual void SetLock(const QUuid& id, bool b) = 0;
	virtual bool IsVisible(const QUuid& id) = 0;
	virtual void SetVisible(const QUuid& id, bool b) = 0;
	virtual bool IsHideByShowOnly(const QUuid& id) = 0;
	virtual QString GetName(const QUuid& id) = 0;
	virtual void SetName(const QUuid& id, const QString& name) = 0;
	virtual int GetFolderIndex() = 0;
	virtual void SetFolderIndex(int index) = 0;
	virtual bool IsUpperLightLimit() = 0;
	virtual int GetLightNum() = 0;
	virtual int GetModelIndex(EModelTreeNodeType type) = 0;
	virtual void SetModelIndex(EModelTreeNodeType type, int index) = 0;
	virtual void SetMainCameraId(const QUuid& id) = 0;
	virtual QUuid GetMainCameraId() = 0;
	virtual void GetAllCameraId(std::vector<QUuid>& vecCameraIds) = 0;
	virtual void ShowOnlyModel(const QUuid& id) = 0;
	virtual void CancelShowOnlyModel() = 0;

	virtual QUuid GetShowOnlyModelId() = 0;
	virtual bool CanEnterShowOnlyMode(const QUuid& id) = 0;
	virtual bool CanEnterCancelShowOnlyMode(const QUuid& id) = 0;

	//Model数据保存与加载
	virtual bool LoadDataFromRapidJson(const rapidjson::Value &root) = 0;
	virtual void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) = 0;
};

class IBaseModellLight
{
public:
	virtual ~IBaseModellLight() = default;
	virtual float GetLightIntensity() = 0;
	virtual void SetLightIntensity(float fvalue) = 0;
	virtual int GetLightIndex() = 0;
	virtual void SetLightIndex(int nValue) = 0;
	virtual Light GetLightData() = 0;
	virtual DirectX::XMFLOAT3 GetColor() = 0;
	virtual void SetColor(DirectX::XMFLOAT3 Color) = 0;
	virtual void AutoLockObject(float x, float y, float z) = 0;
};

class IModelDirectionalLight
{
public:
	virtual ~IModelDirectionalLight() = default;
	virtual void SetCameraData(bool bPerspective, const DirectX::SimpleMath::Vector3& CameraParameter) = 0;
};

class IModelPointLight
{
public:
	virtual ~IModelPointLight() = default;
	virtual float GetRange() = 0;
	virtual void SetRange(float fsize) = 0;
	virtual DirectX::XMFLOAT3 GetAttenuation() = 0;
	virtual void SetAttenuation(DirectX::XMFLOAT3 att) = 0;
};

class IModelSpotLight
{
public:
	virtual ~IModelSpotLight() = default;
	virtual float GetSpot() = 0;
	virtual void SetSpot(float fsize) = 0;
	virtual float GetRange() = 0;
	virtual void SetRange(float fsize) = 0;
	virtual float GetAngle() = 0;
	virtual void SetAngle(float fsize) = 0;
	virtual DirectX::XMFLOAT3 GetAttenuation() = 0;
	virtual void SetAttenuation(DirectX::XMFLOAT3 att) = 0;
};


//沉浸式空间平面的六个面
enum ECubeFace
{ 
	ECF_Front = 0, 
	ECF_Back,
	ECF_Left,
	ECF_Right,
	ECF_Top,
	ECF_Bottom,
	ECF_Count
};

// 沉浸式空间单面四边裁剪索引
enum EImmersiveFaceEdgeCrop
{
	EFEC_Top = 0,		// 上边裁剪量（mm）
	EFEC_Left,			// 左边裁剪量（mm）
	EFEC_Bottom,		// 下边存 vSize - 裁剪量（mm）
	EFEC_Right,			// 右边存 uSize - 裁剪量（mm）
	EFEC_Count
};

// 沉浸式空间平面的 12 条棱
enum EImmersiveEdge
{
	EIE_TopLeft = 0,
	EIE_TopRight,
	EIE_TopFront,
	EIE_TopBack,
	EIE_BottomLeft,
	EIE_BottomRight,
	EIE_BottomFront,
	EIE_BottomBack,
	EIE_LeftFront,
	EIE_LeftBack,
	EIE_RightFront,
	EIE_RightBack,

	EIE_Count

};

struct SContourPath;
class IModelIrregularPrism
{
public:
	virtual ~IModelIrregularPrism() = default;
	virtual SContourPath GetContourPath() = 0;
	virtual bool SetContourPath(const SContourPath& contourPath) = 0;
	virtual bool IsImageInward() = 0;
	virtual bool SetImageInward(bool b) = 0;
	virtual void GetFacePresent(bool &bBottomface, bool &bTopface) = 0;
	virtual bool SetFacePresent(bool &bBottomface, bool &bTopface) = 0;
	virtual int GetHeight() = 0;
	virtual bool SetHeight(int h) = 0;

	/*
	功能：获取顶/底面 R 角圆弧的细分段数
	参数：无
	返回值：R 角圆弧细分段数
	*/
	virtual size_t GetBaseSubdivisionAmount() = 0;
	/*
	功能：设置顶/底面 R 角圆弧的细分段数
	参数：
	  n - R 角圆弧细分段数
	返回值：属性值发生变化返回 true，否则返回 false
	*/
	virtual bool SetBaseSubdivisionAmount(size_t n) = 0;

	/*
	功能：设置顶/底面 R 角半径
	参数：
	  n - R 角半径，单位 mm
	返回值：属性值发生变化返回 true，否则返回 false
	*/
	virtual bool SetCornerRadius(int n) = 0;
	/*
	功能：获取当前 R 角半径
	参数：无
	返回值：R 角半径，单位 mm
	*/
	virtual int GetCornerRadius() = 0;

	/*
	功能：获取顶/底面是否启用 R 角
	参数：
	  bBottomface - 输出，底面是否启用 R 角
	  bTopface    - 输出，顶面是否启用 R 角
	返回值：无
	*/
	virtual void GetFaceCorner(bool &bBottomface, bool &bTopface) = 0;
	/*
	功能：设置顶/底面是否启用 R 角
	参数：
	  bBottomface - 输入/输出，底面是否启用 R 角
	  bTopface    - 输入/输出，顶面是否启用 R 角
	返回值：属性值发生变化返回 true，否则返回 false
	*/
	virtual bool SetFaceCorner(bool &bBottomface, bool &bTopface) = 0;

	virtual int GetAnchorType() const = 0;
	virtual bool SetAnchorType(int anchorType) = 0;
};

class IModelImmersiveSpace
{
public:
	virtual ~IModelImmersiveSpace() = default;

	virtual bool SetNormalDirection(bool inward) = 0;
	virtual bool GetNormalDirection() const = 0;
	virtual bool SetFaceData(const std::array<bool, ECF_Count>& faces) = 0;
	virtual void GetFaceData(std::array<bool, ECF_Count>& faces) const = 0;
	virtual bool SetEdgeState(const std::array<bool, EIE_Count>& edges) = 0;
	virtual void GetEdgeState(std::array<bool, EIE_Count>& edges) const = 0;
	virtual bool SetEdgeSegments(const std::array<int, EIE_Count>& segments) = 0;
	virtual void GetEdgeSegments(std::array<int, EIE_Count>& segments) const = 0;
	virtual bool SetModelSize(float width, float height, float depth, size_t RWidth) = 0;
	virtual void GetModelSize(float& width, float& height, float& depth, size_t& RWidth) const = 0;
	virtual bool SetFaceEdgeCropData(const std::array<std::array<float, EFEC_Count>, ECF_Count>& faceEdgeCropData) = 0;
	virtual void GetFaceEdgeCropData(std::array<std::array<float, EFEC_Count>, ECF_Count>& faceEdgeCropData) const = 0;
	virtual int GetAnchorType() const = 0;
	virtual bool SetAnchorType(int anchorType) = 0;
};

class IModelSphere
{
public:
	virtual ~IModelSphere() = default;
	virtual double GetRadius() = 0;
	virtual void SetRadius(double v) = 0;
	virtual bool IsImageInward() = 0;
	virtual bool SetImageInward(bool b) = 0;
	virtual int GetRow() = 0;
	virtual void SetRow(int v) = 0;
	virtual int GetColumn() = 0;
	virtual void SetColumn(int v) = 0;
	virtual double GetLatitudeStart() = 0;
	virtual void SetLatitudeStart(double v) = 0;
	virtual double GetLatitudeEnd() = 0;
	virtual void SetLatitudeEnd(double v) = 0;
	virtual double GetLongitudeRange() = 0;
	virtual void SetLongitudeRange(double v) = 0;

	virtual int GetAnchorType() const = 0;
	virtual bool SetAnchorType(int anchorType) = 0;
};

class IModelPlane
{
public:
	virtual ~IModelPlane() = default;
	virtual double GetWidth() = 0;
	virtual void SetWidth(double v) = 0;
	virtual double GetHeight() = 0;
	virtual void SetHeight(double v) = 0;
	virtual double GetDotPitch() = 0;
	virtual void SetDotPitch(double v) = 0;
	virtual int GetRow() = 0;
	virtual void SetRow(int v) = 0;
	virtual int GetColumn() = 0;
	virtual void SetColumn(int v) = 0;
};

class IModelRotationSolid
{
public:
	virtual ~IModelRotationSolid() = default;
	virtual bool IsImageInward() = 0;
	virtual bool SetImageInward(bool b) = 0;
	virtual SContourPath GetContourPath() = 0;
	virtual bool SetContourPath(const SContourPath& contourPath) = 0;
	virtual int GetStep() = 0;
	virtual bool SetStep(int n) = 0;
	virtual int GetAngle() = 0;
	virtual bool SetAngle(int n) = 0;

	virtual int GetAnchorType() const = 0;
	virtual bool SetAnchorType(int anchorType) = 0;
};

struct SModuleNode;
struct SyntopyData;
class IModelModuleInference
{
public:
	virtual ~IModelModuleInference() = default;
	virtual int IsImageInward() = 0;
	virtual bool SetImageInward(int b) = 0;
	virtual float GetCurlValue() = 0;
	virtual bool SetCurlValue(float value) = 0;
	virtual std::vector<SModuleNode>& GetModuleData() = 0;
	virtual bool SetModuleData(std::vector<SModuleNode> vecModuleData) = 0;
	virtual QString GetMappingPath() = 0;
	virtual bool SetMappingPath(QString strPath) = 0;
	virtual std::vector<SyntopyData> GetSyntopyData() = 0;
	virtual bool SetSyntopyData(std::vector<SyntopyData> vecTopologyData) = 0;
	virtual void  SetGeometryData(GeometryData data) = 0;
	virtual GeometryData&  GetGeometryData() = 0;

	virtual int GetAnchorType() const = 0;
	virtual bool SetAnchorType(int anchorType) = 0;
};
