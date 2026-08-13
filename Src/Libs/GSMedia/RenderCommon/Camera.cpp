#include "Camera.h"
#include <math.h>
#include "Common/Common/CommonRapidJson.h"

#define Camera_Xml					QString("Camera")
#define CameraTransform_Xml          QString("CameraTransform")
#define CameraNearZ_Xml          QString("CameraNearZ")
#define CameraFarZ_Xml          QString("CameraFarZ")
#define CameraAspect_Xml          QString("CameraAspect")
#define CameraFovY_Xml          QString("CameraFovY")
#define CameraViewPort_Xml          QString("CameraViewPort")
#define CameraViewPort_TopLeftX_Xml          QString("CameraViewPortTopLeftX")
#define CameraViewPort_TopLeftY_Xml          QString("CameraViewPortTopLeftY")
#define CameraViewPort_Width_Xml          QString("CameraViewPortWidth")
#define CameraViewPort_Height_Xml          QString("CameraViewPortHeight")
#define CameraViewPort_MinDepth_Xml          QString("CameraViewPortMinDepth")
#define CameraViewPort_MaxDepth_Xml          QString("CameraViewPortMaxDepth")

#define ThridCameraTarget_Xml          QString("ThridCameraTarget")
#define ThridCameraDistance_Xml     QString("ThridCameraDistance")
#define ThridCameraMinDist_Xml      QString("ThridCameraMinDist")
#define ThridCameraMaxDist_Xml   QString("ThridCameraMaxDist")
#define ThridCameraPerspective_Xml QString("ThridCameraPerspective")

static bool LoadFloat3(DirectX::XMFLOAT3& float3, QJsonArray& jsonArr)
{
	int num = jsonArr.size();
	if (jsonArr.size() != 3)
		return false;

	float3.x = jsonArr[0].toString().toDouble();
	float3.y = jsonArr[1].toString().toDouble();
	float3.z = jsonArr[2].toString().toDouble();

	return true;
};

static void SaveFloat3(const DirectX::XMFLOAT3& float3, QJsonArray& jsonArr)
{
	jsonArr.append(QString::number(float3.x, 'f', 6));
	jsonArr.append(QString::number(float3.y, 'f', 6));
	jsonArr.append(QString::number(float3.z, 'f', 6));
};

static bool LoadFloat3RapidJson(DirectX::XMFLOAT3& float3, const rapidjson::Value& jsonArr)
{
	if (!jsonArr.IsArray() || jsonArr.Size() != 3)
		return false;

	double x = 0.0, y = 0.0, z = 0.0;
	if (!RapidJsonValueToDouble(jsonArr[0], x) ||
		!RapidJsonValueToDouble(jsonArr[1], y) ||
		!RapidJsonValueToDouble(jsonArr[2], z))
		return false;

	float3.x = static_cast<float>(x);
	float3.y = static_cast<float>(y);
	float3.z = static_cast<float>(z);
	return true;
}

void Camera::SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) const
{
	root.SetObject();

	QByteArray transformKeyUtf8 = CameraTransform_Xml.toUtf8();
	rapidjson::Value transformObject(rapidjson::kObjectType);
	m_Transform.SaveDataToRapidJson(transformObject, allocator);
	root.AddMember(rapidjson::Value(transformKeyUtf8.constData(), allocator).Move(), transformObject, allocator);

	root.AddMember(
		rapidjson::Value(CameraNearZ_Xml.toUtf8().constData(), allocator).Move(),
		static_cast<double>(m_NearZ),
		allocator);
	root.AddMember(
		rapidjson::Value(CameraFarZ_Xml.toUtf8().constData(), allocator).Move(),
		static_cast<double>(m_FarZ),
		allocator);
	root.AddMember(
		rapidjson::Value(CameraAspect_Xml.toUtf8().constData(), allocator).Move(),
		static_cast<double>(m_Aspect),
		allocator);
	root.AddMember(
		rapidjson::Value(CameraFovY_Xml.toUtf8().constData(), allocator).Move(),
		static_cast<double>(m_FovY),
		allocator);

	rapidjson::Value viewPortObject(rapidjson::kObjectType);
	viewPortObject.AddMember(
		rapidjson::Value(CameraViewPort_TopLeftX_Xml.toUtf8().constData(), allocator).Move(),
		static_cast<double>(m_ViewPort.TopLeftX),
		allocator);
	viewPortObject.AddMember(
		rapidjson::Value(CameraViewPort_TopLeftY_Xml.toUtf8().constData(), allocator).Move(),
		static_cast<double>(m_ViewPort.TopLeftY),
		allocator);
	viewPortObject.AddMember(
		rapidjson::Value(CameraViewPort_Width_Xml.toUtf8().constData(), allocator).Move(),
		static_cast<double>(m_ViewPort.Width),
		allocator);
	viewPortObject.AddMember(
		rapidjson::Value(CameraViewPort_Height_Xml.toUtf8().constData(), allocator).Move(),
		static_cast<double>(m_ViewPort.Height),
		allocator);
	viewPortObject.AddMember(
		rapidjson::Value(CameraViewPort_MinDepth_Xml.toUtf8().constData(), allocator).Move(),
		static_cast<double>(m_ViewPort.MinDepth),
		allocator);
	viewPortObject.AddMember(
		rapidjson::Value(CameraViewPort_MaxDepth_Xml.toUtf8().constData(), allocator).Move(),
		static_cast<double>(m_ViewPort.MaxDepth),
		allocator);

	root.AddMember(
		rapidjson::Value(CameraViewPort_Xml.toUtf8().constData(), allocator).Move(),
		viewPortObject,
		allocator);
}

bool Camera::LoadDataFromRapidJson(const rapidjson::Value& object)
{
	if (!object.IsObject())
		return false;

	QByteArray transformKeyUtf8 = CameraTransform_Xml.toUtf8();
	if (!object.HasMember(transformKeyUtf8.constData()) || !object[transformKeyUtf8.constData()].IsObject())
		return false;
	if (!m_Transform.LoadDataFromRapidJson(object[transformKeyUtf8.constData()]))
		return false;

	auto loadDouble = [&](const QByteArray& keyUtf8, float& out) -> bool {
		auto it = object.FindMember(keyUtf8.constData());
		if (it == object.MemberEnd())
			return false;
		double v = 0.0;
		if (!RapidJsonValueToDouble(it->value, v))
			return false;
		out = static_cast<float>(v);
		return true;
	};

	if (!loadDouble(CameraNearZ_Xml.toUtf8(), m_NearZ)) return false;
	if (!loadDouble(CameraFarZ_Xml.toUtf8(), m_FarZ)) return false;
	if (!loadDouble(CameraAspect_Xml.toUtf8(), m_Aspect)) return false;
	if (!loadDouble(CameraFovY_Xml.toUtf8(), m_FovY)) return false;

	QByteArray viewPortKeyUtf8 = CameraViewPort_Xml.toUtf8();
	if (!object.HasMember(viewPortKeyUtf8.constData()) || !object[viewPortKeyUtf8.constData()].IsObject())
		return false;
	const rapidjson::Value& viewPortObject = object[viewPortKeyUtf8.constData()];

	double temp = 0.0;
	auto loadSubDouble = [&](const QByteArray& subKeyUtf8, float& out) -> bool {
		if (!viewPortObject.HasMember(subKeyUtf8.constData()))
			return false;
		if (!RapidJsonValueToDouble(viewPortObject[subKeyUtf8.constData()], temp))
			return false;
		out = static_cast<float>(temp);
		return true;
	};

	if (!loadSubDouble(CameraViewPort_TopLeftX_Xml.toUtf8(), m_ViewPort.TopLeftX)) return false;
	if (!loadSubDouble(CameraViewPort_TopLeftY_Xml.toUtf8(), m_ViewPort.TopLeftY)) return false;
	if (!loadSubDouble(CameraViewPort_Width_Xml.toUtf8(), m_ViewPort.Width)) return false;
	if (!loadSubDouble(CameraViewPort_Height_Xml.toUtf8(), m_ViewPort.Height)) return false;
	if (!loadSubDouble(CameraViewPort_MinDepth_Xml.toUtf8(), m_ViewPort.MinDepth)) return false;
	if (!loadSubDouble(CameraViewPort_MaxDepth_Xml.toUtf8(), m_ViewPort.MaxDepth)) return false;

	return true;
}

static void SaveFloat3RapidJson(const DirectX::XMFLOAT3& float3, rapidjson::Value& jsonArr, rapidjson::Document::AllocatorType& allocator)
{
	jsonArr.SetArray();
	jsonArr.PushBack(static_cast<double>(float3.x), allocator);
	jsonArr.PushBack(static_cast<double>(float3.y), allocator);
	jsonArr.PushBack(static_cast<double>(float3.z), allocator);
}

using namespace DirectX;

Camera::~Camera()
{
}

Transform& Camera::GetTransform()
{
	return m_Transform;
}

void Camera::SetTransform(const Transform& transform)
{
	m_Transform = transform;
}

XMVECTOR Camera::GetPositionXM() const
{
	return m_Transform.GetPositionXM();
}

XMFLOAT3 Camera::GetPosition() const
{
	return m_Transform.GetPosition();
}

XMFLOAT3 Camera::GetRotation() const
{
	return m_Transform.GetRotation();
}

float Camera::GetRotationX() const
{
	return m_Transform.GetRotation().x;
}

float Camera::GetRotationY() const
{
	return m_Transform.GetRotation().y;
}

float Camera::GetRotationZ() const
{
	return m_Transform.GetRotation().z;
}


XMVECTOR Camera::GetRightAxisXM() const
{
	return m_Transform.GetRightAxisXM();
}

XMFLOAT3 Camera::GetRightAxis() const
{
	return m_Transform.GetRightAxis();
}

XMVECTOR Camera::GetUpAxisXM() const
{
	return m_Transform.GetUpAxisXM();
}

XMFLOAT3 Camera::GetUpAxis() const
{
	return m_Transform.GetUpAxis();
}

XMVECTOR Camera::GetLookAxisXM() const
{
	return m_Transform.GetForwardAxisXM();
}

XMFLOAT3 Camera::GetLookAxis() const
{
	return m_Transform.GetForwardAxis();
}

DirectX::XMMATRIX Camera::GetLocalToWorldMatrixXM() const
{
	return m_Transform.GetLocalToWorldMatrixXM();
}

XMMATRIX Camera::GetViewMatrixXM() const
{
	return m_Transform.GetWorldToLocalMatrixXM();
}

XMMATRIX Camera::GetProjMatrixXM(bool reversedZ) const
{
	if (reversedZ)
		return XMMatrixPerspectiveFovLH(m_FovY, m_Aspect, m_FarZ, m_NearZ);
	else
		return XMMatrixPerspectiveFovLH(m_FovY, m_Aspect, m_NearZ, m_FarZ);
}

XMMATRIX Camera::GetViewProjMatrixXM(bool reversedZ) const
{
	return GetViewMatrixXM() * GetProjMatrixXM(reversedZ);
}

D3D11_VIEWPORT Camera::GetViewPort() const
{
	return m_ViewPort;
}

DirectX::SimpleMath::Viewport Camera::GetMathViewPort() const
{
	return DirectX::SimpleMath::Viewport(m_ViewPort);
}

float Camera::GetNearZ() const
{
	return m_NearZ;
}

float Camera::GetFarZ() const
{
	return m_FarZ;
}

float Camera::GetFovY() const
{
	return m_FovY;
}

float Camera::GetAspectRatio() const
{
	return m_Aspect;
}

void Camera::SetNearZ(float value)
{
	m_NearZ = value;
}
void Camera::SetFarZ(float value)
{
	m_FarZ = value;
}
void Camera::SetFovY(float value)
{
	m_FovY = value;
}
void Camera::SetAspectRatio(float value)
{
	m_Aspect = value;
}

void Camera::SetFrustum(float fovY, float aspect, float nearZ, float farZ)
{
	m_FovY = fovY;
	m_Aspect = aspect;
	m_NearZ = nearZ;
	m_FarZ = farZ;
}

void Camera::SetViewPort(const D3D11_VIEWPORT & viewPort)
{
	m_ViewPort = viewPort;
}

void Camera::SetViewPort(float topLeftX, float topLeftY, float width, float height, float minDepth, float maxDepth)
{
	m_ViewPort.TopLeftX = topLeftX;
	m_ViewPort.TopLeftY = topLeftY;
	m_ViewPort.Width = width;
	m_ViewPort.Height = height;
	m_ViewPort.MinDepth = minDepth;
	m_ViewPort.MaxDepth = maxDepth;
}

void Camera::SetRotationQuaternion(const DirectX::XMFLOAT4 & rotationQuaternion)
{
	m_Transform.SetRotationQuaternion(rotationQuaternion);
}

DirectX::SimpleMath::Vector4 Camera::GetRotationQuaternion() const
{
	return m_Transform.GetRotationQuaternion();
}

XMMATRIX Camera::GetViewMatrix()
{
	XMMATRIX worldMatrix = XMMatrixAffineTransformation(DirectX::g_XMOne, DirectX::g_XMZero,
		XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&m_Transform.GetRotationQuaternion())),
		XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&m_Transform.GetPosition())));


	/* XMMATRIX zAxisFix = XMMatrixScaling(-1.0f, 1.0f, 1.0f);
	XMMATRIX correctedWorldMatrix = XMMatrixMultiply(worldMatrix, zAxisFix);
*/
	XMMATRIX viewMatrix = XMMatrixInverse(nullptr, worldMatrix);


	return viewMatrix;
}

DirectX::XMFLOAT2 Camera::ProjectPointToScreen(DirectX::XMFLOAT3 & position)
{
	using namespace DirectX;

	XMMATRIX viewMatrix = GetViewMatrixXM();
	XMMATRIX projMatrix = GetProjMatrixXM();
	D3D11_VIEWPORT viewPort = GetViewPort();

	DirectX::XMVECTOR worldPosition = XMLoadFloat3(&position);

	// 将世界坐标转换为视图空间
	XMVECTOR viewPosition = XMVector3Transform(worldPosition, viewMatrix);

	// 将视图空间转换为裁剪空间
	XMVECTOR clipPosition = XMVector3Transform(viewPosition, projMatrix);

	// 将裁剪空间坐标转换为 NDC
	clipPosition /= XMVectorGetW(clipPosition);

	// 获取 NDC 中的 x 和 y 坐标
	float ndcX = XMVectorGetX(clipPosition);
	float ndcY = XMVectorGetY(clipPosition);

	// 将 NDC 坐标转换为屏幕坐标 
	XMFLOAT2 pt;
	pt.x = (ndcX + 1.0f) * 0.5f * viewPort.Width + viewPort.TopLeftX;
	pt.y = (1.0f - ndcY) * 0.5f * viewPort.Height + viewPort.TopLeftY;

	return pt;
}

void Camera::AutoLockObservation(const DirectX::XMVECTOR& objectModelCenter, const DirectX::XMFLOAT3& objectModelYAxisPosVector, const DirectX::XMFLOAT3& objectModelXAxisPosVector)
{
	auto IsZeroVectorDX = [&](const DirectX::XMFLOAT3& vec, float epsilon = 1e-6f) {
		DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&vec);
		// XMVector3NearEqual 会在每个分量上比较是否在 epsilon 范围内相等
		return DirectX::XMVector3NearEqual(v, g_XMZero, DirectX::XMVectorReplicate(epsilon));
	};

	DirectX::XMVECTOR directionVector = XMVectorSubtract(objectModelCenter, m_Transform.GetPositionXM());
	DirectX::XMFLOAT3 lookToVector;
	DirectX::XMStoreFloat3(&lookToVector, directionVector);

	if (IsZeroVectorDX(lookToVector))
		return;

	auto AreVectorsParallel = [&](const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2, float epsilon = 1e-6f) {
		DirectX::XMVECTOR vec1 = DirectX::XMLoadFloat3(&v1);
		DirectX::XMVECTOR vec2 = DirectX::XMLoadFloat3(&v2);

		// 计算叉积
		DirectX::XMVECTOR crossProduct = DirectX::XMVector3Cross(vec1, vec2);

		// 计算叉积的模长（平方）
		DirectX::XMVECTOR crossLengthSq = DirectX::XMVector3LengthSq(crossProduct);

		// 将结果转换为浮点数并与一个小的阈值（epsilon）比较
		float lengthSq;
		DirectX::XMStoreFloat(&lengthSq, crossLengthSq);

		// 如果模长平方接近于零，则认为两向量平行
		return (lengthSq < epsilon);
	};

	float a, b, c;
	if (!AreVectorsParallel(lookToVector, objectModelYAxisPosVector))
	{
		a = objectModelYAxisPosVector.x;
		b = objectModelYAxisPosVector.y;
		c = objectModelYAxisPosVector.z;
	}
	else
	{
		a = objectModelXAxisPosVector.x;
		b = objectModelXAxisPosVector.y;
		c = objectModelXAxisPosVector.z;
	}

	float lambda = (lookToVector.x * a + lookToVector.y * b + lookToVector.z * c) 
		/ (lookToVector.x * lookToVector.x + lookToVector.y * lookToVector.y + lookToVector.z * lookToVector.z);

	float xNominator = a - lambda * lookToVector.x;
	float yNominator = b - lambda * lookToVector.y;
	float zNominator = c - lambda * lookToVector.z;

	float denominator = sqrt(xNominator * xNominator + yNominator * yNominator + zNominator * zNominator);

	std::vector<float> upXs = { xNominator/ denominator , -xNominator / denominator };
	std::vector<float> upYs = { yNominator / denominator, -yNominator / denominator };
	std::vector<float> upZs = { zNominator / denominator, -zNominator / denominator };

	float upX, upY, upZ, max = -2.0;

	for (const auto & x : upXs)
	{
		for (const auto & y : upYs)
			for (const auto & z : upZs)
				if (x*a + y * b + z * c > max)
				{
					max = x * a + y * b + z * c;
					upX = x;
					upY = y;
					upZ = z;
				}
	}

	/*float upX = xNominator / denominator;
	float upY = yNominator / denominator;
	float upZ = zNominator / denominator;
*/

	DirectX::XMFLOAT3 upVector(upX, upY, upZ);

	m_Transform.LookTo(lookToVector, upVector);
}

#define CameraTransform_Xml          QString("CameraTransform")
#define CameraNearZ_Xml          QString("CameraNearZ")
#define CameraFarZ_Xml          QString("CameraFarZ")
#define CameraAspect_Xml          QString("CameraAspect")
#define CameraFovY_Xml          QString("CameraFovY")
#define CameraViewPort_Xml          QString("CameraViewPort")

QJsonObject Camera::SaveDataToJson() const
{
	QJsonObject root;

	QJsonObject transformObject = m_Transform.SaveDataToJson();
	root.insert(CameraTransform_Xml, transformObject);

	root.insert(CameraNearZ_Xml, QString::number(m_NearZ, 'f', 6));
	root.insert(CameraFarZ_Xml, QString::number(m_FarZ, 'f', 6));
	root.insert(CameraAspect_Xml, QString::number(m_Aspect, 'f', 6));
	root.insert(CameraFovY_Xml, QString::number(m_FovY, 'f', 6));

	QJsonObject viewPortObject;
	viewPortObject.insert(CameraViewPort_TopLeftX_Xml, QString::number(m_ViewPort.TopLeftX, 'f', 6));
	viewPortObject.insert(CameraViewPort_TopLeftY_Xml, QString::number(m_ViewPort.TopLeftY, 'f', 6));
	viewPortObject.insert(CameraViewPort_Width_Xml, QString::number(m_ViewPort.Width, 'f', 6));
	viewPortObject.insert(CameraViewPort_Height_Xml, QString::number(m_ViewPort.Height, 'f', 6));
	viewPortObject.insert(CameraViewPort_MinDepth_Xml, QString::number(m_ViewPort.MinDepth, 'f', 6));
	viewPortObject.insert(CameraViewPort_MaxDepth_Xml, QString::number(m_ViewPort.MaxDepth, 'f', 6));
	root.insert(CameraViewPort_Xml, viewPortObject);

	return root;
}

bool Camera::LoadDataFromJson(const QJsonObject & object)
{
	if (!object.contains(CameraTransform_Xml)) return false;
	if (!m_Transform.LoadDataFromJson(object.value(CameraTransform_Xml).toObject()))
		return false;

	if (!object.contains(CameraNearZ_Xml)) return false;
	m_NearZ = object.value(CameraNearZ_Xml).toString().toFloat();
	if (!object.contains(CameraFarZ_Xml)) return false;
	m_FarZ = object.value(CameraFarZ_Xml).toString().toFloat();
	if (!object.contains(CameraAspect_Xml)) return false;
	m_Aspect = object.value(CameraAspect_Xml).toString().toFloat();
	if (!object.contains(CameraFovY_Xml)) return false;
	m_FovY = object.value(CameraFovY_Xml).toString().toFloat();

	if (!object.contains(CameraViewPort_Xml)) return false;
	QJsonObject viewPortObject = object.value(CameraViewPort_Xml).toObject();

	if (!viewPortObject.contains(CameraViewPort_TopLeftX_Xml)) return false;
	if (!viewPortObject.contains(CameraViewPort_TopLeftY_Xml)) return false;
	if (!viewPortObject.contains(CameraViewPort_Width_Xml)) return false;
	if (!viewPortObject.contains(CameraViewPort_Height_Xml)) return false;
	if (!viewPortObject.contains(CameraViewPort_MinDepth_Xml)) return false;
	if (!viewPortObject.contains(CameraViewPort_MaxDepth_Xml)) return false;

	
	m_ViewPort.TopLeftX = viewPortObject.value(CameraViewPort_TopLeftX_Xml).toString().toFloat();
	m_ViewPort.TopLeftY = viewPortObject.value(CameraViewPort_TopLeftY_Xml).toString().toFloat();
	m_ViewPort.Width = viewPortObject.value(CameraViewPort_Width_Xml).toString().toFloat();
	m_ViewPort.Height = viewPortObject.value(CameraViewPort_Height_Xml).toString().toFloat();
	m_ViewPort.MinDepth = viewPortObject.value(CameraViewPort_MinDepth_Xml).toString().toFloat();
	m_ViewPort.MaxDepth = viewPortObject.value(CameraViewPort_MaxDepth_Xml).toString().toFloat();

	return true;
}

// ******************
// 第一人称/自由视角摄像机
//

FirstPersonCamera::~FirstPersonCamera()
{
}

void FirstPersonCamera::SetPosition(float x, float y, float z)
{
	SetPosition(XMFLOAT3(x, y, z));

	ToThirdPerson();
}

void FirstPersonCamera::SetPosition(const XMFLOAT3& pos)
{
	m_Transform.SetPosition(pos);

	ToThirdPerson();
}

void FirstPersonCamera::LookAt(const XMFLOAT3 & pos, const XMFLOAT3 & target, const XMFLOAT3 & up)
{
	m_Transform.SetPosition(pos);
	m_Transform.LookAt(target, up);

	ToThirdPerson();
}

void FirstPersonCamera::LookTo(const XMFLOAT3 & pos, const XMFLOAT3 & to, const XMFLOAT3 & up)
{
	m_Transform.SetPosition(pos);
	m_Transform.LookTo(to, up);

	ToThirdPerson();
}

void FirstPersonCamera::Strafe(float d)
{
	m_Transform.Translate(m_Transform.GetRightAxis(), d);

	ToThirdPerson();
}

void FirstPersonCamera::Walk(float d)
{
	XMVECTOR rightVec = m_Transform.GetRightAxisXM();
	XMVECTOR frontVec = XMVector3Normalize(XMVector3Cross(rightVec, g_XMIdentityR1));
	XMFLOAT3 front;
	XMStoreFloat3(&front, frontVec);
	m_Transform.Translate(front, d);

	ToThirdPerson();
}

void FirstPersonCamera::MoveForward(float d)
{
	m_Transform.Translate(m_Transform.GetForwardAxis(), d);

	ToThirdPerson();
}

void FirstPersonCamera::Translate(const DirectX::XMFLOAT3& dir, float magnitude)
{
	m_Transform.Translate(dir, magnitude);

	ToThirdPerson();
}

void FirstPersonCamera::Pitch(float rad)
{
	XMFLOAT3 rotation = m_Transform.GetRotation();
	// 将绕x轴旋转弧度限制在[-7pi/18, 7pi/18]之间
	rotation.x += rad;
	if (rotation.x > XM_PI * 7 / 18)
		rotation.x = XM_PI * 7 / 18;
	else if (rotation.x < -XM_PI * 7 / 18)
		rotation.x = -XM_PI * 7 / 18;

	m_Transform.SetRotation(rotation);

	ToThirdPerson();
}

void FirstPersonCamera::Yaw(float rad)
{
	XMFLOAT3 rotation = m_Transform.GetRotation();
	rotation.y = XMScalarModAngle(rotation.y + rad);
	m_Transform.SetRotation(rotation);

	ToThirdPerson();
}



// ******************
// 第三人称摄像机
//

ThirdPersonCamera::~ThirdPersonCamera()
{
}

XMFLOAT3 ThirdPersonCamera::GetTargetPosition() const
{
	return m_Target; 
}

float ThirdPersonCamera::GetDistance() const
{
	return m_Distance;
}

void ThirdPersonCamera::RotateX(float rad)
{
	XMFLOAT3 rotation = m_Transform.GetRotation();
	rotation.x = XMScalarModAngle(rotation.x + rad);

	m_Transform.SetRotation(rotation);
	m_Transform.SetPosition(m_Target);

	if (m_bPerspective)
		m_Transform.Translate(m_Transform.GetForwardAxis(), -m_Distance);
	else
		m_Transform.Translate(m_Transform.GetForwardAxis(), -m_MaxDist);
}

void ThirdPersonCamera::RotateY(float rad)
{
	XMFLOAT3 rotation = m_Transform.GetRotation();
	rotation.y = XMScalarModAngle(rotation.y + rad);

	m_Transform.SetRotation(rotation);
	m_Transform.SetPosition(m_Target);

	if (m_bPerspective)
		m_Transform.Translate(m_Transform.GetForwardAxis(), -m_Distance);
	else
		m_Transform.Translate(m_Transform.GetForwardAxis(), -m_MaxDist);
}

void ThirdPersonCamera::Approach(float dist)
{
	static constexpr float kTargetPushDistance = 6.0f;

	float newDistance = m_Distance + dist;

	if (newDistance < m_MinDist && dist < 0.f)
	{
		const float zoomHeadroom = kTargetPushDistance - m_MinDist;
		while (newDistance < m_MinDist && zoomHeadroom > 0.f)
		{
			m_Transform.SetPosition(m_Target);
			if (m_bPerspective)
				m_Transform.Translate(m_Transform.GetForwardAxis(), -m_Distance);
			else
				m_Transform.Translate(m_Transform.GetForwardAxis(), -m_MaxDist);

			const XMFLOAT3 camPos = m_Transform.GetPosition();
			const XMFLOAT3 forward = m_Transform.GetForwardAxis();

			m_Target.x = camPos.x + forward.x * kTargetPushDistance;
			m_Target.y = camPos.y + forward.y * kTargetPushDistance;
			m_Target.z = camPos.z + forward.z * kTargetPushDistance;

			newDistance += zoomHeadroom;
		}
	}

	// 限制距离在[m_MinDist, m_MaxDist]之间
	if (newDistance < m_MinDist)
		m_Distance = m_MinDist;
	else if (newDistance > m_MaxDist)
		m_Distance = m_MaxDist;
	else
		m_Distance = newDistance;

	m_Transform.SetPosition(m_Target);

	if (m_bPerspective)
		m_Transform.Translate(m_Transform.GetForwardAxis(), -m_Distance);
	else
		m_Transform.Translate(m_Transform.GetForwardAxis(), -m_MaxDist);
}

void ThirdPersonCamera::SetRotationX(float rad)
{
	XMFLOAT3 rotation = m_Transform.GetRotation();
	rotation.x = XMScalarModAngle(rad);
	m_Transform.SetRotation(rotation);
	m_Transform.SetPosition(m_Target);

	if (m_bPerspective)
		m_Transform.Translate(m_Transform.GetForwardAxis(), -m_Distance);
	else
		m_Transform.Translate(m_Transform.GetForwardAxis(), -m_MaxDist);
}

void ThirdPersonCamera::SetRotationY(float rad)
{
	XMFLOAT3 rotation = m_Transform.GetRotation();
	rotation.y = XMScalarModAngle(rad);
	m_Transform.SetRotation(rotation);
	m_Transform.SetPosition(m_Target);

	if (m_bPerspective)
		m_Transform.Translate(m_Transform.GetForwardAxis(), -m_Distance);
	else
		m_Transform.Translate(m_Transform.GetForwardAxis(), -m_MaxDist);
}

void ThirdPersonCamera::SetRotationZ(float rad)
{
	XMFLOAT3 rotation = m_Transform.GetRotation();
	rotation.z = XMScalarModAngle(rad);
	m_Transform.SetRotation(rotation);
	m_Transform.SetPosition(m_Target);

	if (m_bPerspective)
		m_Transform.Translate(m_Transform.GetForwardAxis(), -m_Distance);
	else
		m_Transform.Translate(m_Transform.GetForwardAxis(), -m_MaxDist);
}

void ThirdPersonCamera::SetTarget(const XMFLOAT3 & target)
{
	m_Target = target;

	m_Transform.SetPosition(m_Target);

	if (m_bPerspective)
		m_Transform.Translate(m_Transform.GetForwardAxis(), -m_Distance);
	else
		m_Transform.Translate(m_Transform.GetForwardAxis(), -m_MaxDist);
}

void ThirdPersonCamera::SetDistance(float dist)
{
	m_Distance = dist;

	m_Transform.SetPosition(m_Target);

	if (m_bPerspective)
		m_Transform.Translate(m_Transform.GetForwardAxis(), -m_Distance);
	else
		m_Transform.Translate(m_Transform.GetForwardAxis(), -m_MaxDist);
}

void ThirdPersonCamera::SetDistanceMinMax(float minDist, float maxDist)
{
	m_MinDist = minDist;
	m_MaxDist = maxDist;
}

float ThirdPersonCamera::GetDistanceMax()
{
	return m_MaxDist;
}

DirectX::XMMATRIX ThirdPersonCamera::GetProjMatrixXM(bool reversedZ) const
{
	if (m_bPerspective)
		return __super::GetProjMatrixXM(reversedZ);
	else
	{
		if (reversedZ)
			return XMMatrixOrthographicLH(2 * tan(m_FovY / 2)*m_Distance*m_Aspect, 2 * tan(m_FovY / 2)*m_Distance, m_FarZ, m_NearZ);
		else
			return XMMatrixOrthographicLH(2 * tan(m_FovY / 2)*m_Distance*m_Aspect, 2 * tan(m_FovY / 2)*m_Distance, m_NearZ, m_FarZ);

	}
}

bool ThirdPersonCamera::GetPerspective()
{
	return m_bPerspective;
}

void ThirdPersonCamera::SetPerspective(bool b, bool bMove)
{
	m_bPerspective = b;

	if (bMove)
	{
		m_Transform.SetPosition(m_Target);
		if (m_bPerspective)
			m_Transform.Translate(m_Transform.GetForwardAxis(), -m_Distance);
		else
			m_Transform.Translate(m_Transform.GetForwardAxis(), -m_MaxDist);
	}
}

bool ThirdPersonCamera::LoadDataFromJson(const QJsonObject & object)
{
	if (!object.contains(Camera_Xml)) return false;
	QJsonObject cameraObject = object.value(Camera_Xml).toObject();
	if (!Camera::LoadDataFromJson(cameraObject))
		return false;

	if (!object.contains(ThridCameraTarget_Xml) || !object.value(ThridCameraTarget_Xml).isArray()) return false;
	QJsonArray targetArr = object.value(ThridCameraTarget_Xml).toArray();
	if (!LoadFloat3(m_Target, targetArr))
		return false;

	if (!object.contains(ThridCameraDistance_Xml)) return false;
	m_Distance = object.value(ThridCameraDistance_Xml).toString().toFloat();


	if (!object.contains(ThridCameraMinDist_Xml)) return false;
	m_MinDist = object.value(ThridCameraMinDist_Xml).toString().toFloat();

	if (!object.contains(ThridCameraMaxDist_Xml)) return false;
	m_MaxDist = object.value(ThridCameraMaxDist_Xml).toString().toFloat();

	if (!object.contains(ThridCameraPerspective_Xml)) return false;
	m_bPerspective = object.value(ThridCameraPerspective_Xml).toBool();

	return true;
}

void ThirdPersonCamera::ToThirdPerson()
{
	auto temp = m_Transform;
	temp.Translate(GetLookAxis(), m_Distance);

	m_Target = temp.GetPosition();
}

QJsonObject ThirdPersonCamera::SaveDataToJson() const
{
	QJsonObject root;

	QJsonObject cameraObject = Camera::SaveDataToJson();
	root.insert(Camera_Xml, cameraObject);

	QJsonArray targetArr;
	SaveFloat3(m_Target, targetArr);
	root.insert(ThridCameraTarget_Xml, targetArr);
	root.insert(ThridCameraDistance_Xml, QString::number(m_Distance, 'f', 6));
	root.insert(ThridCameraMinDist_Xml, QString::number(m_MinDist, 'f', 6));
	root.insert(ThridCameraMaxDist_Xml, QString::number(m_MaxDist, 'f', 6));
	root.insert(ThridCameraPerspective_Xml, m_bPerspective);

	return root;
}

void ThirdPersonCamera::SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) const
{
	root.SetObject();

	// Camera部分
	rapidjson::Value cameraObject(rapidjson::kObjectType);
	Camera::SaveDataToRapidJson(cameraObject, allocator);
	QByteArray cameraKeyUtf8 = Camera_Xml.toUtf8();
	root.AddMember(rapidjson::Value(cameraKeyUtf8.constData(), allocator).Move(), cameraObject, allocator);

	// 目标点
	rapidjson::Value targetArr(rapidjson::kArrayType);
	SaveFloat3RapidJson(m_Target, targetArr, allocator);
	QByteArray targetKeyUtf8 = ThridCameraTarget_Xml.toUtf8();
	root.AddMember(rapidjson::Value(targetKeyUtf8.constData(), allocator).Move(), targetArr, allocator);

	// 参数
	root.AddMember(
		rapidjson::Value(ThridCameraDistance_Xml.toUtf8().constData(), allocator).Move(),
		static_cast<double>(m_Distance),
		allocator);
	root.AddMember(
		rapidjson::Value(ThridCameraMinDist_Xml.toUtf8().constData(), allocator).Move(),
		static_cast<double>(m_MinDist),
		allocator);
	root.AddMember(
		rapidjson::Value(ThridCameraMaxDist_Xml.toUtf8().constData(), allocator).Move(),
		static_cast<double>(m_MaxDist),
		allocator);
	root.AddMember(
		rapidjson::Value(ThridCameraPerspective_Xml.toUtf8().constData(), allocator).Move(),
		m_bPerspective,
		allocator);
}

bool ThirdPersonCamera::LoadDataFromRapidJson(const rapidjson::Value& object)
{
	if (!object.IsObject())
		return false;

	auto loadDouble = [&](const QByteArray& keyUtf8, float& out) -> bool {
		if (!object.HasMember(keyUtf8.constData()))
			return false;
		double v = 0.0;
		if (!RapidJsonValueToDouble(object[keyUtf8.constData()], v))
			return false;
		out = static_cast<float>(v);
		return true;
	};

	// Camera部分
	QByteArray cameraKeyUtf8 = Camera_Xml.toUtf8();
	if (!object.HasMember(cameraKeyUtf8.constData()) || !object[cameraKeyUtf8.constData()].IsObject())
		return false;
	if (!Camera::LoadDataFromRapidJson(object[cameraKeyUtf8.constData()]))
		return false;

	// 目标点
	QByteArray targetKeyUtf8 = ThridCameraTarget_Xml.toUtf8();
	if (!object.HasMember(targetKeyUtf8.constData()) || !object[targetKeyUtf8.constData()].IsArray())
		return false;
	if (!LoadFloat3RapidJson(m_Target, object[targetKeyUtf8.constData()]))
		return false;

	if (!loadDouble(ThridCameraDistance_Xml.toUtf8(), m_Distance))
		return false;
	if (!loadDouble(ThridCameraMinDist_Xml.toUtf8(), m_MinDist))
		return false;
	if (!loadDouble(ThridCameraMaxDist_Xml.toUtf8(), m_MaxDist))
		return false;

	// 透视开关：兼容 bool 或 number/string
	QByteArray perspKeyUtf8 = ThridCameraPerspective_Xml.toUtf8();
	if (!object.HasMember(perspKeyUtf8.constData()))
		return false;
	const rapidjson::Value& perspVal = object[perspKeyUtf8.constData()];
	if (perspVal.IsBool())
	{
		m_bPerspective = perspVal.GetBool();
	}
	else
	{
		double tmp = 0.0;
		if (!RapidJsonValueToDouble(perspVal, tmp))
			return false;
		m_bPerspective = tmp != 0.0;
	}

	return true;
}
