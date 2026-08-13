#include "Transform.h"
#include "pch.h"
#include "Transform.h"

#include <QDebug>
#include "Common/Common/CommonRapidJson.h"
using namespace DirectX;

#define TransformScale_Xml          QString("TransformScale")
#define TransformRotation_Xml     QString("TransformRotation")
#define TransformPosition_Xml      QString("TransformPosition")

Transform::Transform(const DirectX::XMFLOAT3& scale, const DirectX::XMFLOAT3& rotation, const DirectX::XMFLOAT3& position)
	: m_Scale(scale), m_Rotation(rotation), m_Position(position)
{
}

SimpleMath::Vector3 Transform::GetScale() const
{
	return m_Scale;
}

DirectX::XMVECTOR Transform::GetScaleXM() const
{
	return XMLoadFloat3(&m_Scale);
}

SimpleMath::Vector3 Transform::GetRotation() const
{
	return m_Rotation;
}

DirectX::XMVECTOR Transform::GetRotationXM() const
{
	return XMLoadFloat3(&m_Rotation);
}

SimpleMath::Vector3 Transform::GetPosition() const
{
	return m_Position;
}

DirectX::XMVECTOR Transform::GetPositionXM() const
{
	return XMLoadFloat3(&m_Position);
}

XMFLOAT3 Transform::GetRightAxis() const
{
	XMMATRIX R = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_Rotation));
	XMFLOAT3 right;
	XMStoreFloat3(&right, R.r[0]);
	return right;
}

DirectX::XMVECTOR Transform::GetRightAxisXM() const
{
	XMFLOAT3 right = GetRightAxis();
	return XMLoadFloat3(&right);
}

XMFLOAT3 Transform::GetUpAxis() const
{
	XMMATRIX R = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_Rotation));
	XMFLOAT3 up;
	XMStoreFloat3(&up, R.r[1]);
	return up;
}

DirectX::XMVECTOR Transform::GetUpAxisXM() const
{
	XMFLOAT3 up = GetUpAxis();
	return XMLoadFloat3(&up);
}

XMFLOAT3 Transform::GetForwardAxis() const
{
	XMMATRIX R = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_Rotation));
	XMFLOAT3 forward;
	XMStoreFloat3(&forward, R.r[2]);
	return forward;
}

DirectX::XMVECTOR Transform::GetForwardAxisXM() const
{
	XMFLOAT3 forward = GetForwardAxis();
	return XMLoadFloat3(&forward);
}

XMFLOAT4X4 Transform::GetLocalToWorldMatrix() const
{
	XMFLOAT4X4 res;
	XMStoreFloat4x4(&res, GetLocalToWorldMatrixXM());
	return res;
}

XMMATRIX Transform::GetLocalToWorldMatrixXM() const
{
	XMVECTOR scaleVec = XMLoadFloat3(&m_Scale);
	XMVECTOR rotationVec = XMLoadFloat3(&m_Rotation);
	XMVECTOR positionVec = XMLoadFloat3(&m_Position);
	XMMATRIX World = XMMatrixScalingFromVector(scaleVec) * XMMatrixRotationRollPitchYawFromVector(rotationVec) * XMMatrixTranslationFromVector(positionVec);
	return World;
}

XMFLOAT4X4 Transform::GetWorldToLocalMatrix() const
{
	XMFLOAT4X4 res;
	XMStoreFloat4x4(&res, GetWorldToLocalMatrixXM());
	return res;
}

XMMATRIX Transform::GetWorldToLocalMatrixXM() const
{
	XMMATRIX InvWorld = XMMatrixInverse(nullptr, GetLocalToWorldMatrixXM());
	return InvWorld;
}

void Transform::SetScale(const XMFLOAT3& scale)
{
	m_Scale = scale;
}

void Transform::SetScale(float x, float y, float z)
{
	m_Scale = XMFLOAT3(x, y, z);
}

void Transform::Scale(const DirectX::XMFLOAT3 & pos, const Transform & old, const DirectX::XMFLOAT3 & radio)
{
	//缩放
	auto scale = old.GetScale();
	m_Scale = old.GetScale() + radio;
	
	//缩放中心不为模型中心时还需要平移
	auto position = old.GetPosition();
	SimpleMath::Vector3 Direct(position - pos);
	if (Direct.Length() > FLT_MIN)
	{
		m_Position = pos + (position - pos) * m_Scale / scale;
	}
}

void Transform::SetRotation(const XMFLOAT3& eulerAnglesInRadian)
{
	m_Rotation = eulerAnglesInRadian;
}

void Transform::SetRotation(float x, float y, float z)
{
	m_Rotation = XMFLOAT3(x, y, z);
}

void Transform::SetPosition(const XMFLOAT3& position)
{
	m_Position = position;
}

void Transform::SetPosition(float x, float y, float z)
{
	m_Position = XMFLOAT3(x, y, z);
}

void Transform::Rotate(const XMFLOAT3& eulerAnglesInRadian)
{
	XMVECTOR newRotationVec = XMVectorAdd(XMLoadFloat3(&m_Rotation), XMLoadFloat3(&eulerAnglesInRadian));
	XMStoreFloat3(&m_Rotation, newRotationVec);
}

void Transform::RotateAxis(const XMFLOAT3& axis, float radian)
{
	XMVECTOR rotationVec = XMLoadFloat3(&m_Rotation);
	XMMATRIX R = XMMatrixRotationRollPitchYawFromVector(rotationVec) * 
		XMMatrixRotationAxis(XMLoadFloat3(&axis), radian);
	XMFLOAT4X4 rotMatrix;
	XMStoreFloat4x4(&rotMatrix, R);
	m_Rotation = GetEulerAnglesFromRotationMatrix(rotMatrix);
}

void Transform::RotateAround(const XMFLOAT3& point, const XMFLOAT3& axis, float radian)
{
	XMVECTOR rotationVec = XMLoadFloat3(&m_Rotation);
	XMVECTOR positionVec = XMLoadFloat3(&m_Position);
	XMVECTOR centerVec = XMLoadFloat3(&point);

	//把旋转中心移到坐标原点
	XMMATRIX RT = XMMatrixRotationRollPitchYawFromVector(rotationVec) * XMMatrixTranslationFromVector(positionVec - centerVec);

	// 绕指定轴执行旋转
	RT *= XMMatrixRotationAxis(XMLoadFloat3(&axis), radian);

	// 把物体移回原空间
	RT *= XMMatrixTranslationFromVector(centerVec);

	//更新旋转（欧拉角）
	XMFLOAT4X4 rotMatrix;
	XMStoreFloat4x4(&rotMatrix, RT);
	m_Rotation = GetEulerAnglesFromRotationMatrix(rotMatrix);
	
	//更新位置
	XMStoreFloat3(&m_Position, RT.r[3]);
}

void Transform::Translate(const XMFLOAT3& direction, float magnitude)
{
	XMVECTOR directionVec = XMVector3Normalize(XMLoadFloat3(&direction));
	XMVECTOR newPosition = XMVectorMultiplyAdd(XMVectorReplicate(magnitude), directionVec, XMLoadFloat3(&m_Position));
	XMStoreFloat3(&m_Position, newPosition);
}

void Transform::LookAt(const XMFLOAT3& target, const XMFLOAT3& up)
{
	XMMATRIX View = XMMatrixLookAtLH(XMLoadFloat3(&m_Position), XMLoadFloat3(&target), XMLoadFloat3(&up));
	XMMATRIX InvView = XMMatrixInverse(nullptr, View);
	XMFLOAT4X4 rotMatrix;
	XMStoreFloat4x4(&rotMatrix, InvView);
	m_Rotation = GetEulerAnglesFromRotationMatrix(rotMatrix);
}

void Transform::LookTo(const XMFLOAT3& direction, const XMFLOAT3& up)
{
	XMMATRIX View = XMMatrixLookToLH(XMLoadFloat3(&m_Position), XMLoadFloat3(&direction), XMLoadFloat3(&up));
	XMMATRIX InvView = XMMatrixInverse(nullptr, View);
	XMFLOAT4X4 rotMatrix;
	XMStoreFloat4x4(&rotMatrix, InvView);
	m_Rotation = GetEulerAnglesFromRotationMatrix(rotMatrix);
}

void Transform::SetRotationQuaternion(const DirectX::XMFLOAT4 & rotationQuaternion)
{
	m_RotationQuaternion = rotationQuaternion;;
}

DirectX::SimpleMath::Vector4 Transform::GetRotationQuaternion() const
{
	return m_RotationQuaternion;
}

XMFLOAT3 Transform::GetEulerAnglesFromRotationMatrix(const XMFLOAT4X4& rotationMatrix)
{
	// 通过旋转矩阵反求欧拉角
	float c = sqrtf(1.0f - rotationMatrix(2, 1) * rotationMatrix(2, 1));
	// 防止r[2][1]出现大于1的情况
	if (isnan(c))
		c = 0.0f;

	XMFLOAT3 rotation;
	rotation.z = atan2f(rotationMatrix(0, 1), rotationMatrix(1, 1));
	rotation.x = atan2f(-rotationMatrix(2, 1), c);
	rotation.y = atan2f(rotationMatrix(2, 0), rotationMatrix(2, 2));

	return rotation;
}

static bool LoadFloat3(DirectX::SimpleMath::Vector3& float3, QJsonArray& jsonArr)
{
	int num = jsonArr.size();
	if (jsonArr.size() != 3)
		return false;

	float3.x = jsonArr[0].toString().toDouble();
	float3.y = jsonArr[1].toString().toDouble();
	float3.z = jsonArr[2].toString().toDouble();

	return true;
};

static void SaveFloat3(const DirectX::SimpleMath::Vector3& float3, QJsonArray& jsonArr)
{
	jsonArr.append(QString::number(float3.x, 'f', 6));
	jsonArr.append(QString::number(float3.y, 'f', 6));
	jsonArr.append(QString::number(float3.z, 'f', 6));
};


QJsonObject Transform::SaveDataToJson() const
{
	QJsonObject root;

	QJsonArray scaleArr;
	SaveFloat3(m_Scale, scaleArr);
	root.insert(TransformScale_Xml, scaleArr);

	QJsonArray rotationArr;
	SaveFloat3(m_Rotation, rotationArr);
	root.insert(TransformRotation_Xml, rotationArr);

	QJsonArray positionArr;
	SaveFloat3(m_Position, positionArr);
	root.insert(TransformPosition_Xml, positionArr);

	return root;
}

bool Transform::LoadDataFromJson(const QJsonObject & object)
{
	if (!object.contains(TransformScale_Xml) || !object.value(TransformScale_Xml).isArray()) return false;
	QJsonArray scaleArr = object.value(TransformScale_Xml).toArray();
	if (!LoadFloat3(m_Scale, scaleArr))
		return false;

	if (!object.contains(TransformRotation_Xml) || !object.value(TransformRotation_Xml).isArray()) return false;
	QJsonArray rotationArr = object.value(TransformRotation_Xml).toArray();
	if (!LoadFloat3(m_Rotation, rotationArr))
		return false;

	if (!object.contains(TransformPosition_Xml) || !object.value(TransformPosition_Xml).isArray()) return false;
	QJsonArray positionArr = object.value(TransformPosition_Xml).toArray();
	if (!LoadFloat3(m_Position, positionArr))
		return false;

	return true;
}

static bool LoadFloat3RapidJson(DirectX::SimpleMath::Vector3& float3, const rapidjson::Value& jsonArr)
{
	if (!jsonArr.IsArray() || jsonArr.Size() != 3)
		return false;

	double x = 0.0, y = 0.0, z = 0.0;
	if (!RapidJsonValueToDouble(jsonArr[0], x) || !RapidJsonValueToDouble(jsonArr[1], y) || !RapidJsonValueToDouble(jsonArr[2], z))
	{
		return false;
	}

	float3.x = static_cast<float>(x);
	float3.y = static_cast<float>(y);
	float3.z = static_cast<float>(z);
	return true;
}

static void SaveFloat3RapidJson(const DirectX::SimpleMath::Vector3& float3, rapidjson::Value& jsonArr, rapidjson::Document::AllocatorType& allocator)
{
	jsonArr.SetArray();
	jsonArr.PushBack(static_cast<double>(float3.x), allocator);
	jsonArr.PushBack(static_cast<double>(float3.y), allocator);
	jsonArr.PushBack(static_cast<double>(float3.z), allocator);
}

void Transform::SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) const
{
	root.SetObject();

	QByteArray scaleKeyUtf8 = TransformScale_Xml.toUtf8();
	rapidjson::Value scaleArr(rapidjson::kArrayType);
	SaveFloat3RapidJson(m_Scale, scaleArr, allocator);
	root.AddMember(rapidjson::Value(scaleKeyUtf8.constData(), allocator).Move(), scaleArr, allocator);

	QByteArray rotKeyUtf8 = TransformRotation_Xml.toUtf8();
	rapidjson::Value rotArr(rapidjson::kArrayType);
	SaveFloat3RapidJson(m_Rotation, rotArr, allocator);
	root.AddMember(rapidjson::Value(rotKeyUtf8.constData(), allocator).Move(), rotArr, allocator);

	QByteArray posKeyUtf8 = TransformPosition_Xml.toUtf8();
	rapidjson::Value posArr(rapidjson::kArrayType);
	SaveFloat3RapidJson(m_Position, posArr, allocator);
	root.AddMember(rapidjson::Value(posKeyUtf8.constData(), allocator).Move(), posArr, allocator);
}

bool Transform::LoadDataFromRapidJson(const rapidjson::Value& object)
{
	if (!object.IsObject())
		return false;

	QByteArray scaleKeyUtf8 = TransformScale_Xml.toUtf8();
	if (!object.HasMember(scaleKeyUtf8.constData()) || !object[scaleKeyUtf8.constData()].IsArray())
		return false;
	if (!LoadFloat3RapidJson(m_Scale, object[scaleKeyUtf8.constData()]))
		return false;

	QByteArray rotKeyUtf8 = TransformRotation_Xml.toUtf8();
	if (!object.HasMember(rotKeyUtf8.constData()) || !object[rotKeyUtf8.constData()].IsArray())
		return false;
	if (!LoadFloat3RapidJson(m_Rotation, object[rotKeyUtf8.constData()]))
		return false;

	QByteArray posKeyUtf8 = TransformPosition_Xml.toUtf8();
	if (!object.HasMember(posKeyUtf8.constData()) || !object[posKeyUtf8.constData()].IsArray())
		return false;
	if (!LoadFloat3RapidJson(m_Position, object[posKeyUtf8.constData()]))
		return false;

	return true;
}
