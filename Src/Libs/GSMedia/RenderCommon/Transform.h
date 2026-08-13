//***************************************************************************************
// Transform.h by X_Jun(MKXJun) (C) 2018-2020 All Rights Reserved.
// Licensed under the MIT License.
//
// 描述对象缩放、旋转(欧拉角为基础)、平移
// Provide 1st person(free view) and 3rd person cameras.
//***************************************************************************************

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <cmath>
#include "SimpleMath.h"
#include "RenderCommonGlobal.h"

#include <QJsonObject>
#include <QJsonArray>
#include <rapidjson/document.h>

class RENDERCOMMON_API Transform
{
public:
	Transform() = default;
	Transform(const DirectX::XMFLOAT3& scale, const DirectX::XMFLOAT3& rotation, const DirectX::XMFLOAT3& position);
	~Transform() = default;

	Transform(const Transform&) = default;
	Transform& operator=(const Transform&) = default;

	Transform(Transform&&) = default;
	Transform& operator=(Transform&&) = default;

	// 获取对象缩放比例
	DirectX::SimpleMath::Vector3 GetScale() const;
	// 获取对象缩放比例
	DirectX::XMVECTOR GetScaleXM() const;

	// 获取对象欧拉角(弧度制)
	// 对象以Z-X-Y轴顺序旋转
	DirectX::SimpleMath::Vector3 GetRotation() const;
	// 获取对象欧拉角(弧度制)
	// 对象以Z-X-Y轴顺序旋转
	DirectX::XMVECTOR GetRotationXM() const;

	// 获取对象位置
	DirectX::SimpleMath::Vector3 GetPosition() const;
	// 获取对象位置
	DirectX::XMVECTOR GetPositionXM() const;

	// 获取右方向轴
	DirectX::XMFLOAT3 GetRightAxis() const;
	// 获取右方向轴
	DirectX::XMVECTOR GetRightAxisXM() const;

	// 获取上方向轴
	DirectX::XMFLOAT3 GetUpAxis() const;
	// 获取上方向轴
	DirectX::XMVECTOR GetUpAxisXM() const;

	// 获取前方向轴
	DirectX::XMFLOAT3 GetForwardAxis() const;
	// 获取前方向轴
	DirectX::XMVECTOR GetForwardAxisXM() const;

	// 获取世界变换矩阵
	DirectX::XMFLOAT4X4 GetLocalToWorldMatrix() const;
	// 获取世界变换矩阵
	DirectX::XMMATRIX GetLocalToWorldMatrixXM() const;

	// 获取逆世界变换矩阵
	DirectX::XMFLOAT4X4 GetWorldToLocalMatrix() const;
	// 获取逆世界变换矩阵
	DirectX::XMMATRIX GetWorldToLocalMatrixXM() const;

	// 设置对象缩放比例
	void SetScale(const DirectX::XMFLOAT3& scale);
	// 设置对象缩放比例
	void SetScale(float x, float y, float z);
	// 以pos为中心，原始矩阵为基准，进行xyz三轴比例为radio的线性缩放
	void Scale(const DirectX::XMFLOAT3& pos, const Transform& old, const DirectX::XMFLOAT3& radio);

	// 设置对象欧拉角(弧度制)
	// 对象将以Z-X-Y轴顺序旋转
	void SetRotation(const DirectX::XMFLOAT3& eulerAnglesInRadian);
	// 设置对象欧拉角(弧度制)
	// 对象将以Z-X-Y轴顺序旋转
	void SetRotation(float x, float y, float z);

	// 设置对象位置
	void SetPosition(const DirectX::XMFLOAT3& position);
	// 设置对象位置
	void SetPosition(float x, float y, float z);
	
	// 指定欧拉角旋转对象
	void Rotate(const DirectX::XMFLOAT3& eulerAnglesInRadian);
	// 指定以原点为中心绕轴旋转
	void RotateAxis(const DirectX::XMFLOAT3& axis, float radian);
	// 指定以point为旋转中心绕轴旋转
	void RotateAround(const DirectX::XMFLOAT3& point, const DirectX::XMFLOAT3& axis, float radian);

	// 沿着某一方向平移
	void Translate(const DirectX::XMFLOAT3& direction, float magnitude);

	// 观察某一点
	void LookAt(const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up = { 0.0f, 1.0f, 0.0f });
	// 沿着某一方向观察
	void LookTo(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& up = { 0.0f, 1.0f, 0.0f });

	void SetRotationQuaternion(const DirectX::XMFLOAT4& rotationQuaternion);
	DirectX::SimpleMath::Vector4 GetRotationQuaternion() const;


	QJsonObject SaveDataToJson() const;
	bool LoadDataFromJson(const QJsonObject& object);

	// Rapidjson 保存/加载（用于替换 ModelModel 内部 QJson 中转）
	void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) const;
	bool LoadDataFromRapidJson(const rapidjson::Value& object);

private:
	// 从旋转矩阵获取旋转欧拉角
	DirectX::XMFLOAT3 GetEulerAnglesFromRotationMatrix(const DirectX::XMFLOAT4X4& rotationMatrix);

private:
	DirectX::SimpleMath::Vector3 m_Scale = { 1.0f, 1.0f, 1.0f };				// 缩放
	DirectX::SimpleMath::Vector3 m_Rotation = { 0.0f,0.0f,0.0f };				// 旋转欧拉角(弧度制)
	DirectX::SimpleMath::Vector3 m_Position = { 0.0f,0.0f,0.0f };				// 位置


	DirectX::SimpleMath::Vector4 m_RotationQuaternion = { 0.0f, 0.0f, 0.0f, 1.0f };	//与欧拉角两套数据，暂未同步修改
};

#endif


