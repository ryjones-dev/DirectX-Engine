#include "Transform.h"

#include "ComponentRegistry.h"

using namespace DirectX;

Transform::Transform(Entity& entity) : Component(entity)
{
	m_position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	XMStoreFloat4x4(&m_translationMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_rotationMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_scaleMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_worldMatrix, XMMatrixIdentity());
}

Transform::~Transform()
{
}

void Transform::init()
{
	Component::init();

	Debug::entityDebugWindow->addVariableWithCallbacks(Debug::TW_TYPE_VEC3F, "Position", typeName, entity.getName(), &getTransformPositionDebugEditor, &setTransformPositionDebugEditor, this);
	Debug::entityDebugWindow->addVariableWithCallbacks(Debug::TW_TYPE_VEC3F, "Rotation", typeName, entity.getName(), &getTransformRotationDebugEditor, &setTransformRotationDebugEditor, this);
	Debug::entityDebugWindow->addVariableWithCallbacks(Debug::TW_TYPE_VEC3F, "Scale", typeName, entity.getName(), &getTransformScaleDebugEditor, &setTransformScaleDebugEditor, this);
}

void Transform::loadFromJSON(rapidjson::Value& dataObject)
{
	Component::loadFromJSON(dataObject);

	rapidjson::Value::MemberIterator position = dataObject.FindMember("position");
	rapidjson::Value::MemberIterator rotation = dataObject.FindMember("rotation");
	rapidjson::Value::MemberIterator scale = dataObject.FindMember("scale");

	if (position != dataObject.MemberEnd())
	{
		setPosition(XMFLOAT3(position->value["x"].GetFloat(), position->value["y"].GetFloat(), position->value["z"].GetFloat()));
	}

	if (rotation != dataObject.MemberEnd())
	{
		setRotation(XMFLOAT3(rotation->value["x"].GetFloat(), rotation->value["y"].GetFloat(), rotation->value["z"].GetFloat()));
	}

	if (scale != dataObject.MemberEnd())
	{
		setScale(XMFLOAT3(scale->value["x"].GetFloat(), scale->value["y"].GetFloat(), scale->value["z"].GetFloat()));
	}
}

void Transform::saveToJSON(rapidjson::Writer<rapidjson::StringBuffer>& writer)
{
	Component::saveToJSON(writer);

	writer.Key("position");
	writer.StartObject();

	writer.Key("x");
	writer.Double(m_position.x);

	writer.Key("y");
	writer.Double(m_position.y);

	writer.Key("z");
	writer.Double(m_position.z);

	writer.EndObject();

	writer.Key("rotation");
	writer.StartObject();

	writer.Key("x");
	writer.Double(XMConvertToDegrees(m_rotation.x));

	writer.Key("y");
	writer.Double(XMConvertToDegrees(m_rotation.y));

	writer.Key("z");
	writer.Double(XMConvertToDegrees(m_rotation.z));

	writer.EndObject();

	writer.Key("scale");
	writer.StartObject();

	writer.Key("x");
	writer.Double(m_scale.x);

	writer.Key("y");
	writer.Double(m_scale.y);

	writer.Key("z");
	writer.Double(m_scale.z);

	writer.EndObject();
}

const XMFLOAT3 Transform::getPosition() const
{
	return m_position;
}

const XMFLOAT3 Transform::getRotation() const
{
	return m_rotation;
}

const XMFLOAT3 Transform::getScale() const
{
	return m_scale;
}

void Transform::setPosition(DirectX::XMFLOAT3 position)
{
	m_position = position;
	calcTranslationMatrix();
}

void Transform::setRotation(DirectX::XMFLOAT3 rotation)
{
	m_rotation = XMFLOAT3(XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z));
	calcRotationMatrix();
}

void Transform::setScale(DirectX::XMFLOAT3 scale)
{
	m_scale = scale;
	calcScaleMatrix();
}

const XMMATRIX Transform::getTranslationMatrix() const
{
	return XMLoadFloat4x4(&m_translationMatrix);
}

const XMMATRIX Transform::getRotationMatrix() const
{
	return XMLoadFloat4x4(&m_rotationMatrix);
}

const XMMATRIX Transform::getScaleMatrix() const
{
	return XMLoadFloat4x4(&m_scaleMatrix);
}

const XMMATRIX Transform::getWorldMatrix() const
{
	return XMLoadFloat4x4(&m_worldMatrix);
}

const XMFLOAT3 Transform::getRight() const
{
	XMVECTOR v = XMVector3Transform(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), getRotationMatrix());

	XMFLOAT3 right;
	XMStoreFloat3(&right, v);
	return right;
}

const XMFLOAT3 Transform::getUp() const
{
	XMVECTOR v = XMVector3Transform(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), getRotationMatrix());

	XMFLOAT3 up;
	XMStoreFloat3(&up, v);
	return up;
}

const XMFLOAT3 Transform::getForward() const
{
	XMVECTOR v = XMVector3Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), getRotationMatrix());

	XMFLOAT3 forward;
	XMStoreFloat3(&forward, v);
	return forward;
}

void Transform::calcTranslationMatrix()
{
	XMVECTOR position = XMLoadFloat3(&m_position);
	XMStoreFloat4x4(&m_translationMatrix, XMMatrixTranslationFromVector(position));

	calcWorldMatrix();
}

void Transform::calcRotationMatrix()
{
	XMVECTOR rotation = XMLoadFloat3(&m_rotation);
	XMStoreFloat4x4(&m_rotationMatrix, XMMatrixRotationRollPitchYawFromVector(rotation));

	calcWorldMatrix();
}

void Transform::calcScaleMatrix()
{
	XMVECTOR scale = XMLoadFloat3(&m_scale);
	XMStoreFloat4x4(&m_scaleMatrix, XMMatrixScalingFromVector(scale));

	calcWorldMatrix();
}

void Transform::calcWorldMatrix()
{
	XMMATRIX translation = XMLoadFloat4x4(&m_translationMatrix);
	XMMATRIX rotation = XMLoadFloat4x4(&m_rotationMatrix);
	XMMATRIX scale = XMLoadFloat4x4(&m_scaleMatrix);

	XMMATRIX world = XMMatrixMultiply(XMMatrixMultiply(scale, rotation), translation);
	XMStoreFloat4x4(&m_worldMatrix, world);
}

void Transform::move(XMFLOAT3 delta)
{
	XMVECTOR position = XMLoadFloat3(&m_position);
	position = XMVectorAdd(position, XMLoadFloat3(&delta));
	XMStoreFloat3(&m_position, position);

	calcTranslationMatrix();
}

void Transform::moveX(float delta)
{
	XMVECTOR position = XMLoadFloat3(&m_position);
	position = XMVectorAdd(position, XMVectorScale(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), delta));
	XMStoreFloat3(&m_position, position);

	calcTranslationMatrix();
}

void Transform::moveY(float delta)
{
	XMVECTOR position = XMLoadFloat3(&m_position);
	position = XMVectorAdd(position, XMVectorScale(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), delta));
	XMStoreFloat3(&m_position, position);

	calcTranslationMatrix();
}

void Transform::moveZ(float delta)
{
	XMVECTOR position = XMLoadFloat3(&m_position);
	position = XMVectorAdd(position, XMVectorScale(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), delta));
	XMStoreFloat3(&m_position, position);

	calcTranslationMatrix();
}

void Transform::moveLocal(DirectX::XMFLOAT3 delta)
{
	XMFLOAT3 rightFloat3 = getRight();
	XMFLOAT3 upFloat3 = getUp();
	XMFLOAT3 forwardFloat3 = getForward();

	XMVECTOR right = XMLoadFloat3(&rightFloat3);
	XMVECTOR up = XMLoadFloat3(&upFloat3);
	XMVECTOR forward = XMLoadFloat3(&forwardFloat3);
	XMVECTOR position = XMLoadFloat3(&m_position);

	position = XMVectorAdd(position, XMVectorScale(right, delta.x));
	position = XMVectorAdd(position, XMVectorScale(up, delta.y));
	position = XMVectorAdd(position, XMVectorScale(forward, delta.z));
	XMStoreFloat3(&m_position, position);

	calcTranslationMatrix();
}

void Transform::moveLocalX(float delta)
{
	XMFLOAT3 rightFloat3 = getRight();

	XMVECTOR right = XMLoadFloat3(&rightFloat3);
	XMVECTOR position = XMLoadFloat3(&m_position);
	position = XMVectorAdd(position, XMVectorScale(right, delta));
	XMStoreFloat3(&m_position, position);

	calcTranslationMatrix();
}

void Transform::moveLocalY(float delta)
{
	XMFLOAT3 upFloat3 = getUp();

	XMVECTOR up = XMLoadFloat3(&upFloat3);
	XMVECTOR position = XMLoadFloat3(&m_position);
	position = XMVectorAdd(position, XMVectorScale(up, delta));
	XMStoreFloat3(&m_position, position);

	calcTranslationMatrix();
}

void Transform::moveLocalZ(float delta)
{
	XMFLOAT3 forwardFloat3 = getForward();

	XMVECTOR forward = XMLoadFloat3(&forwardFloat3);
	XMVECTOR position = XMLoadFloat3(&m_position);
	position = XMVectorAdd(position, XMVectorScale(forward, delta));
	XMStoreFloat3(&m_position, position);

	calcTranslationMatrix();
}

void Transform::rotateLocal(DirectX::XMFLOAT3 rotDegrees)
{
	XMFLOAT3 rot = XMFLOAT3(XMConvertToRadians(rotDegrees.x), XMConvertToRadians(rotDegrees.y), XMConvertToRadians(rotDegrees.z));
	XMVECTOR rotation = XMLoadFloat3(&m_rotation);
	XMVECTOR rotVector = XMLoadFloat3(&rot);
	rotation = XMVectorAdd(rotation, rotVector);
	XMStoreFloat3(&m_rotation, rotation);

	calcRotationMatrix();
}

void Transform::rotateLocalX(float degrees)
{
	XMVECTOR rotation = XMLoadFloat3(&m_rotation);
	XMVECTOR angleVector = XMVectorSet(XMConvertToRadians(degrees), 0.0f, 0.0f, 0.0f);
	rotation = XMVectorAdd(rotation, angleVector);
	XMStoreFloat3(&m_rotation, rotation);

	calcRotationMatrix();
}

void Transform::rotateLocalY(float degrees)
{
	XMVECTOR rotation = XMLoadFloat3(&m_rotation);
	XMVECTOR angleVector = XMVectorSet(0.0f, XMConvertToRadians(degrees), 0.0f, 0.0f);
	rotation = XMVectorAdd(rotation, angleVector);
	XMStoreFloat3(&m_rotation, rotation);

	calcRotationMatrix();
}

void Transform::rotateLocalZ(float degrees)
{
	XMVECTOR rotation = XMLoadFloat3(&m_rotation);
	XMVECTOR angleVector = XMVectorSet(0.0f, 0.0f, XMConvertToRadians(degrees), 0.0f);
	rotation = XMVectorAdd(rotation, angleVector);
	XMStoreFloat3(&m_rotation, rotation);

	calcRotationMatrix();
}

void Transform::scale(DirectX::XMFLOAT3 delta)
{
	XMVECTOR scale = XMLoadFloat3(&m_scale);
	scale = XMVectorAdd(scale, XMLoadFloat3(&delta));
	XMStoreFloat3(&m_scale, scale);

	calcScaleMatrix();
}

void Transform::scaleX(float delta)
{
	XMVECTOR scale = XMLoadFloat3(&m_scale);
	scale = XMVectorAdd(scale, XMVectorScale(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), delta));
	XMStoreFloat3(&m_scale, scale);

	calcScaleMatrix();
}

void Transform::scaleY(float delta)
{
	XMVECTOR scale = XMLoadFloat3(&m_scale);
	scale = XMVectorAdd(scale, XMVectorScale(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), delta));
	XMStoreFloat3(&m_scale, scale);

	calcScaleMatrix();
}

void Transform::scaleZ(float delta)
{
	XMVECTOR scale = XMLoadFloat3(&m_scale);
	scale = XMVectorAdd(scale, XMVectorScale(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), delta));
	XMStoreFloat3(&m_scale, scale);

	calcScaleMatrix();
}

void TW_CALL getTransformPositionDebugEditor(void* value, void* clientData)
{
	Transform* transform = static_cast<Transform*>(clientData);
	*static_cast<XMFLOAT3*>(value) = transform->getPosition();
}

void TW_CALL getTransformRotationDebugEditor(void* value, void* clientData)
{
	Transform* transform = static_cast<Transform*>(clientData);
	XMFLOAT3 rotation = transform->getRotation();
	XMFLOAT3 rotationDegrees = XMFLOAT3(XMConvertToDegrees(rotation.x), XMConvertToDegrees(rotation.y), XMConvertToDegrees(rotation.z));
	*static_cast<XMFLOAT3*>(value) = rotationDegrees;
}

void TW_CALL getTransformScaleDebugEditor(void* value, void* clientData)
{
	Transform* transform = static_cast<Transform*>(clientData);
	*static_cast<XMFLOAT3*>(value) = transform->getScale();
}

void TW_CALL setTransformPositionDebugEditor(const void* value, void* clientData)
{
	Transform* transform = static_cast<Transform*>(clientData);
	transform->setPosition(*static_cast<const XMFLOAT3*>(value));
}

void TW_CALL setTransformRotationDebugEditor(const void* value, void* clientData)
{
	Transform* transform = static_cast<Transform*>(clientData);
	transform->setRotation(*static_cast<const XMFLOAT3*>(value));
}

void TW_CALL setTransformScaleDebugEditor(const void* value, void* clientData)
{
	Transform* transform = static_cast<Transform*>(clientData);
	transform->setScale(*static_cast<const XMFLOAT3*>(value));
}