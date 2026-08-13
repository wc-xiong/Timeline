#pragma once
#define _USE_MATH_DEFINES		//解决qmath.h和math.h的头文件冲突
#include "CommonGlobal.h"
#include "ICopyInterface.h"

#include "Common/Common/Maths/Point.h"
#include "Common/Common/Maths/Matrix3x2.h"
#include "Common/Common/ICustomDataInterface.h"

#include <QUuid>
#include <QRectF>
#include <QString>
#include <QDomNode>
#include <QModelIndex>
#include <rapidjson/document.h>
#include "CommonRapidJson.h"

#include <set>
#include <vector>
#include <memory>

#include <DirectXMath.h>
#include <DirectXCollision.h>

#define ANGLE_MAX 360.0f
#define ANGLE_MIN 0.0f

#define BE_SYN_NUM (20) //顶点数大于BE_SYN_NUM时开启点编辑同步且不可以取消

#define MAX_VISIBLE_SLICE_NUM (40000)	//推荐显示的最大切片数量

enum ESliceNodeType
{
	ESNT_Root = -1,		// 根节点
	ESNT_Slice,			//分片节点
	ESNT_Group,			//组节点
	ESNT_DefaultGroup,	//默认分组节点
};

enum EQuadOptimizationMode
{
	NoneOpt, // 无优化
	Perspective, // 透视模式
	Stretching, // 拉伸模式

	ModeCount, // 模式数量计算
};

struct SSlicePty
{
	CPointD ptCenter;				//旋转中心点
	double fAngle = 0;				//旋转角度
	bool bFlipLR = false;				//左右翻转
	bool bFlipUD = false;				//上下翻转

	void Reset()
	{
		fAngle = 0;
		bFlipLR = false;
		bFlipUD = false;
	}


#define xml_SliceModel_SliceNodeCenter "Center"
#define xml_SliceModel_SliceNodeRotateAngle "RotationAngle"
#define xml_SliceModel_SliceNodeFlipLeftRight "FlipLR"
#define xml_SliceModel_SliceNodeFlipUpDown "FlipUD"
#define xml_SliceModel_SliceNodeCenterX "CenterX"
#define xml_SliceModel_SliceNodeCenterY "CenterY"
#define xml_SliceModel_SliceNodeGroupInCenterX "InputCenterX"
#define xml_SliceModel_SliceNodeGroupInCenterY "InputCenterY"
#define xml_SliceModel_SliceNodeGroupInRotationAngle "InputRotationAngle"
#define xml_SliceModel_SliceNodeGroupOutCenterX "OutputCenterX"
#define xml_SliceModel_SliceNodeGroupOutCenterY "OutputCenterY"
#define xml_SliceModel_SliceNodeGroupOutRotationAngle "OutputRotationAngle"

	void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) const
	{
		root.SetObject();
		rapidjson::Value center(rapidjson::kArrayType);
		center.PushBack(RapidJsonSafeDouble(ptCenter.x), allocator);
		center.PushBack(RapidJsonSafeDouble(ptCenter.y), allocator);
		root.AddMember(rapidjson::StringRef(xml_SliceModel_SliceNodeCenter), center, allocator);
		root.AddMember(rapidjson::StringRef(xml_SliceModel_SliceNodeRotateAngle), RapidJsonSafeDouble(fAngle), allocator);
		root.AddMember(rapidjson::StringRef(xml_SliceModel_SliceNodeFlipLeftRight), bFlipLR, allocator);
		root.AddMember(rapidjson::StringRef(xml_SliceModel_SliceNodeFlipUpDown), bFlipUD, allocator);
	}

	bool LoadDataFromRapidJson(const rapidjson::Value& object)
	{
		if (!object.IsObject() ||
			!object.HasMember(xml_SliceModel_SliceNodeCenter) ||
			!object[xml_SliceModel_SliceNodeCenter].IsArray())
		{
			return false;
		}
		const rapidjson::Value& center = object[xml_SliceModel_SliceNodeCenter];
		if (center.Size() != 2)
		{
			return false;
		}

		auto parseDouble = [](const rapidjson::Value& value, double& outValue)->bool
		{
			if (value.IsNumber())
			{
				outValue = value.GetDouble();
				return true;
			}

			if (value.IsString())
			{
				bool bOk = false;
				outValue = QString(value.GetString()).toDouble(&bOk);
				return bOk;
			}

			return false;
		};
		if (!parseDouble(center[0], ptCenter.x) || !parseDouble(center[1], ptCenter.y))
		{
			return false;
		}

		if (!object.HasMember(xml_SliceModel_SliceNodeRotateAngle))
		{
			return false;
		}
		if (!parseDouble(object[xml_SliceModel_SliceNodeRotateAngle], fAngle))
		{
			return false;
		}

		if (object.HasMember(xml_SliceModel_SliceNodeFlipLeftRight) &&
			object[xml_SliceModel_SliceNodeFlipLeftRight].IsBool())
		{
			bFlipLR = object[xml_SliceModel_SliceNodeFlipLeftRight].GetBool();
		}
		else
		{
			bFlipLR = false;
		}

		if (object.HasMember(xml_SliceModel_SliceNodeFlipUpDown) &&
			object[xml_SliceModel_SliceNodeFlipUpDown].IsBool())
		{
			bFlipUD = object[xml_SliceModel_SliceNodeFlipUpDown].GetBool();
		}
		else
		{
			bFlipUD = false;
		}

		return true;
	}

	bool LoadDataFromRapidJsonByGscdFileForGroupNode(const QDomNode & object, bool bInput)
	{
		const char* szCenterX = bInput ? xml_SliceModel_SliceNodeGroupInCenterX : xml_SliceModel_SliceNodeGroupOutCenterX;
		const char* szCenterY = bInput ? xml_SliceModel_SliceNodeGroupInCenterY : xml_SliceModel_SliceNodeGroupOutCenterY;
		const char* szRotationAngle = bInput ? xml_SliceModel_SliceNodeGroupInRotationAngle : xml_SliceModel_SliceNodeGroupOutRotationAngle;

		QDomNode Node;

		Node = object.namedItem(szCenterX);
		if (Node.isNull())
		{
			return false;
		}
		ptCenter.x = Node.firstChild().nodeValue().toDouble();

		Node = object.namedItem(szCenterY);
		if (Node.isNull())
		{
			return false;
		}
		ptCenter.y = Node.firstChild().nodeValue().toDouble();

		Node = object.namedItem(szRotationAngle);
		if (Node.isNull())
		{
			return false;
		}
		fAngle = Node.firstChild().nodeValue().toDouble();

		return true;
	}
};

class ISliceTree;
class ISliceTreeNode :public ICopyInterface,public IManipulableObject
{
public:
	virtual ~ISliceTreeNode() = default;

	virtual void			SetTree(std::shared_ptr<ISliceTree> pTree) = 0;

	virtual QUuid			GetID() = 0;
	virtual void			SetID(const QUuid& id) = 0;
	virtual QUuid			GetIdInModel() = 0;
	virtual void			SetIdInModel(const QUuid& id) = 0;

	// 获取节点所在树id
	virtual QUuid			GetTreeId() = 0;

	virtual QString			GetName() = 0;
	virtual void			SetName(const QString& name) = 0;

	virtual void			SetLock(bool b) = 0;
	virtual bool			IsLock() = 0;

	virtual void			SetVisible(bool b) = 0;
	virtual bool			IsVisible() = 0;

	virtual int				GetNodeType() = 0;

	virtual int				GetNameIndex() = 0;
	virtual void			SetNameIndex(int nIndex) = 0;

	virtual void			SetRotateCenter(const CPointD& ptCenter, bool bInput) = 0;

	virtual void			SetRotateAngle(double angle, bool bInput) = 0;

	virtual void	SetFlip(bool bInput, bool bLR) = 0;
	virtual void ResetPointPty(bool bInput) = 0;

	virtual D2D1_RECT_D		GetOutRect(bool bInput, bool bOrignal = false) = 0;
	virtual std::vector<CPointD> GetOutRectPts(bool bInput) = 0;
	virtual void			TransFormNode(const CMatrix3x2D& transform, double fOffsetAngle, bool bFlipUD, bool bFlipLR, double flipAngle, bool bInput) = 0;
	
	virtual HRESULT			SyncInOutPointData(bool bInput) = 0;

	virtual void			RefreshID() = 0;

	virtual bool LoadDataFromRapidJson(const rapidjson::Value &root) = 0;
	virtual void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) = 0;
};

struct SSegmentUnitData;
struct SGeometryPoint;

namespace CusMappingRenderData
{
	struct CSplitPoints;
};


enum ESliceType
{	
	EST_Model  = 0,		// 模型切片
	EST_Custom,			// 自定义切片	
};

class ISliceTreeSliceNode
{
public:
	virtual ~ISliceTreeSliceNode() = default;

	virtual void SetSliceType(int nType) = 0;
	virtual int GetSliceType() = 0;

	virtual void SetSliceValid(bool bValue) = 0;
	virtual bool IsSliceValid() = 0;	// 切片是否有输入切片
	virtual void SetSliceSplit(bool bValue) = 0;
	virtual bool IsSliceSplit() = 0;	// 切片分割是否成功

	virtual void SetPixmapSlice(bool bValue) = 0;
	virtual bool IsPixmapSlice() = 0;

	virtual int GetVertexCount() = 0;

	virtual void GetPoints(bool bInput, std::vector<CPointD>& pPts, bool bClear = true) = 0;
	virtual void SetPoints(std::vector<CPointD>& pPts, bool bInput) = 0;

	virtual void MovePoint(const CPointD& ptCurMode, int nIndex, bool bInput) = 0;
	virtual void MoveAllPoints(const CMatrix3x2D& transform, bool bInput) = 0;

	virtual bool IsSliceIntersect(bool bInput, std::vector<CPointD>& arPts) = 0;						//切片是否相交 
	virtual bool HasSlicePeakInRect(bool bInput, std::vector<CPointD>& arrBoxRectPt) = 0;			//切片是否相包含 

	virtual bool InsertPoint(int nIndex, const SGeometryPoint& pt) = 0;
	virtual bool RemovePoint(int nIndex, bool bInput, bool bCalcuCross = true) = 0;
	virtual bool IsSliceCross(int nIndex, bool bInput) = 0;

	virtual void CopyPointPty(bool bInput, std::shared_ptr<ISliceTreeSliceNode> pOther) = 0;
	

	virtual void SetSliceData(const SSegmentUnitData& data) = 0;
	virtual void GetSliceData(SSegmentUnitData& data) = 0;
	virtual void SetModelColor(const DirectX::XMFLOAT4 &clr) = 0;

	virtual bool DoSplit(int type,const QUuid &screenId, const D2D1_RECT_D& rcInScreen, const D2D1_RECT_D& rcOutScreen) = 0;
	virtual bool GetSliceSplitData(const QUuid& screenId, CusMappingRenderData::CSplitPoints& sSlicePoint) = 0;

	// 切片细分保存节点之间父子结构
	virtual void SetMeshParentId(const QUuid& id) = 0;
	virtual QUuid GetMeshParentId() = 0;

	virtual void SetMeshChildlIds(const std::vector<QUuid>& vec) = 0;
	virtual std::vector<QUuid> GetMeshChildlIds() = 0;

	virtual void RefreshRotateCenter(bool bInput) = 0;
};



enum ESliceTreeType
{
	ESTT_Preview = 0,	// 模型对应的预览单元切片
	ESTT_Output,	// 模型对应输出切片
};

class CCommonTreeViewNode;
struct GeometryData;
struct SGeometryRenderData;

enum EGeodataUpdateType
{
	EGUT_All = 0,
	EGUT_SamplePos,
	EGUT_Color,
};

class ISliceTree :public ICopyInterface
{
public:
	virtual ~ISliceTree() = default;

	virtual CCommonTreeViewNode* GetTreeRoot() = 0;												// 获取多叉树根节点							
	virtual std::weak_ptr<ISliceTreeNode>	GetSliceTreeNode(const QUuid& nodeID) = 0;				// 根据QUuid获取节点数据,需要作非空判断
	virtual int				GetChildIndex(const QUuid& childID) = 0;
	virtual std::shared_ptr<ISliceTree> CopySliceTree(const QUuid &id, std::map<QUuid, QUuid> &idCor) = 0;
	 

	virtual void SetSliceTreeType(int nType) = 0;
	virtual int GetSliceTreeType() = 0;

	virtual int GetExpansionType() = 0;
	virtual void SetExpansionType(int nValue) = 0;


	virtual QUuid 			GetRootID() = 0;
	virtual bool			IsExist(const QUuid& nodeID) = 0;
	virtual int				GetAllSliceCount() = 0;
	virtual void			GetAllLeafIDs(std::vector<QUuid>& arSliceID) = 0;						// 获取所有的SliceID
	virtual QUuid			GetParentID(const QUuid& nodeID) = 0;									// 通过切片（组）QUuid获取父Node
	virtual QUuid			GetTopMostGroupQUuid(const QUuid& nodeID) = 0;							// 通过切片（组）QUuid获取最上层组Node
	virtual QUuid			GetOptimizeGroupID() = 0;
	virtual QUuid			GetCustomGroupID() = 0;

	virtual void			GetLeafIDs(const QUuid& grpID, std::vector<QUuid>& arSlice) = 0;		// 通过组QUuid获取切片QUuid
	virtual void			GetNodeIDs(const QUuid& grpID, std::vector<QUuid>& arSlice) = 0;		// 通过组QUuid获取所有切片QUuid，包括组节点
	virtual void			GetChildIDs(const QUuid& grpID, std::vector<QUuid>& arSlice) = 0;		// 通过组QUuid获取直接子节点QUuid
	virtual void			GetAllLeafIDsWithOrder(std::vector<QUuid>& vecLeafIDs) = 0;
	virtual std::vector<QUuid>		SortInPreorder(std::set<QUuid>& setLeafIDs) = 0;
	virtual int 			GetLeafsCount(const QUuid& grpID) = 0;

	virtual void            SetName(const QUuid& id, const QString& name) = 0;
	virtual void			MoveNode(int eNodeMove, const QUuid& NodeID) = 0;
	virtual void			MoveNode(const QUuid& NodeID, int newIndex) = 0;
	virtual void			MoveNode(int eNodeMove, const std::vector<QUuid>& vecNodeIDs) = 0;
	virtual void			MoveNode(const std::vector<QUuid>& vecNodeIDs, int newIndex) = 0;

	virtual void			AddNode(const QUuid& parentID, std::shared_ptr<ISliceTreeNode> childData) = 0;
	virtual void			AddNode(const QUuid& parentID, const std::vector<std::shared_ptr<ISliceTreeNode>>& vecChildData) = 0;
	virtual std::weak_ptr<ISliceTreeNode>			AddNewSlice(CPointD& centerPt, double sizeHalfWidth = 128.0) = 0;

	virtual void			RemoveAllNodes() = 0;
	virtual void			RemoveNode(const QUuid& nodeID) = 0;
	virtual void			RemoveNodes(const std::vector<QUuid>& nodeIDs) = 0;
	virtual void			ChangeParentOfNodes(const std::set<QUuid>& nodeIDs, const QUuid& newParentID) = 0;

	virtual D2D1_RECT_D	GetOutRect(bool bInput) = 0;
	//Model数据保存与加载
	virtual bool LoadDataFromRapidJson(const rapidjson::Value &root, bool bInitDeaultGroup = false) = 0;
	virtual void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) = 0;

	virtual void SetModelID(const QUuid& ModelId) = 0;
	virtual QUuid GetModelID() = 0;

	// 该节点被细分
	virtual void MeshNode(const QUuid& id, std::vector<SSegmentUnitData>& vecSlices, const SRectD& rcScreen) = 0;

	// 该子节点被取消细分
	virtual std::vector<QUuid> CancelMeshNode(const QUuid& childId) = 0;
	// 获取取消分割时涉及的所有点映射切片 id（去重由调用方自行处理）
	virtual std::vector<QUuid> HasPixmapInCancel(const QUuid& childId) = 0;

	virtual void ResetNodeMeshData() = 0;

	virtual void InitSliceTreeFromModelData(const GeometryData& geoData, const SRectD& rcScreen) = 0;

	virtual bool GetModelData(SGeometryRenderData& geoData, int nUpdateType) = 0;

	virtual void AddHideSliceNode(const SSegmentUnitData& node) = 0;
	virtual void RemoveAllHideSliceNode() = 0;

	virtual bool IsHasPixmapSliceNode() = 0;
	virtual bool RemovePixmapSliceNode(const std::set<QUuid>& selectedIds) = 0;
	virtual bool AddPixmapSlices(const std::set<QUuid>& selectedIds) = 0;

	virtual void AllSliceApplyRect(bool bUpdateInput, const SRectD &rcInMinMax, const SRectD& rcBounding) = 0;
};

enum EScreenSliceType
{
	ESST_PreUnit = 0,	// 纯预览单元切片
	ESST_Output,	// 输出切片
};

class ISliceModel:public ICopyInterface
{
public:
	virtual ~ISliceModel() = default;

	virtual std::shared_ptr<ISliceTree> CreateSliceTree(const QUuid& treeId) = 0;
	virtual void AddSliceTree(std::shared_ptr<ISliceTree> pSliceTree) = 0;
	virtual void DeleteSliceTree(const QUuid& treeId) = 0;
	virtual void ClearSliceTree() = 0;

	virtual std::vector<QUuid> GetAllSliceTree() = 0;
	virtual std::weak_ptr<ISliceTree> GetSliceTree(const QUuid& id) = 0;
	virtual std::weak_ptr<ISliceTreeNode> GetSliceTreeNode(const QUuid& SliceId) = 0;
	
	// Model数据保存与加载
	virtual void LoadSliceModelFromRapidJson(const rapidjson::Value &root) = 0;
	virtual void SaveSliceModelToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) = 0;
};