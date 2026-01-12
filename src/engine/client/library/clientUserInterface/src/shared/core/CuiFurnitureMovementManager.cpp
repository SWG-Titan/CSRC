//======================================================================
//
// CuiFurnitureMovementManager.cpp
// copyright (c) 2024
//
// Manager for interactive furniture movement mode with gimbal controls
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiFurnitureMovementManager.h"

#include "clientDirectInput/DirectInput.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientObject.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/Extent.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Crc.h"
#include "sharedMath/PackedArgb.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/NetworkIdManager.h"
#include "UnicodeUtils.h"

#include <cmath>
#include <dinput.h>

#ifndef PI
#define PI 3.14159265359f
#endif

//======================================================================

namespace CuiFurnitureMovementManagerNamespace
{
	// Type aliases
	typedef CuiFurnitureMovementManager::GizmoComponent GizmoComponent;
	typedef CuiFurnitureMovementManager::GizmoMode GizmoMode;
	GizmoComponent const GC_None = CuiFurnitureMovementManager::GC_None;
	GizmoComponent const GC_AxisX = CuiFurnitureMovementManager::GC_AxisX;
	GizmoComponent const GC_AxisY = CuiFurnitureMovementManager::GC_AxisY;
	GizmoComponent const GC_AxisZ = CuiFurnitureMovementManager::GC_AxisZ;
	GizmoComponent const GC_PlaneXY = CuiFurnitureMovementManager::GC_PlaneXY;
	GizmoComponent const GC_PlaneXZ = CuiFurnitureMovementManager::GC_PlaneXZ;
	GizmoComponent const GC_PlaneYZ = CuiFurnitureMovementManager::GC_PlaneYZ;
	GizmoComponent const GC_RotateYaw = CuiFurnitureMovementManager::GC_RotateYaw;
	GizmoComponent const GC_RotatePitch = CuiFurnitureMovementManager::GC_RotatePitch;
	GizmoComponent const GC_RotateRoll = CuiFurnitureMovementManager::GC_RotateRoll;
	GizmoComponent const GC_Sphere = CuiFurnitureMovementManager::GC_Sphere;
	GizmoMode const GM_Translate = CuiFurnitureMovementManager::GM_Translate;
	GizmoMode const GM_Rotate = CuiFurnitureMovementManager::GM_Rotate;
	GizmoMode const GM_Scale = CuiFurnitureMovementManager::GM_Scale;

	bool s_installed = false;
	bool s_active = false;

	CachedNetworkId s_selectedFurniture;
	Transform s_originalTransform;
	
	Vector s_positionDelta;
	float s_yawDelta = 0.0f;
	float s_pitchDelta = 0.0f;
	float s_rollDelta = 0.0f;

	float s_movementSpeed = 0.5f;
	float s_rotationSpeed = 45.0f;
	bool s_fineMode = false;
	float s_fineModeFactor = 0.1f;

	GizmoMode s_gizmoMode = CuiFurnitureMovementManager::GM_Translate;
	GizmoComponent s_activeComponent = CuiFurnitureMovementManager::GC_None;
	GizmoComponent s_hoveredComponent = CuiFurnitureMovementManager::GC_None;
	float s_gizmoScale = 1.0f;
	float s_cachedObjectRadius = 1.0f;
	bool s_isDragging = false;
	int s_lastMouseX = 0;
	int s_lastMouseY = 0;
	Vector s_dragStartPosition;

	// Gizmo colors
	PackedArgb const s_colorRed(255, 255, 50, 50);
	PackedArgb const s_colorGreen(255, 50, 255, 50);
	PackedArgb const s_colorBlue(255, 50, 50, 255);
	PackedArgb const s_colorRedHighlight(255, 255, 128, 128);
	PackedArgb const s_colorGreenHighlight(255, 128, 255, 128);
	PackedArgb const s_colorBlueHighlight(255, 128, 128, 255);
	PackedArgb const s_colorWhite(255, 255, 255, 255);
	PackedArgb const s_colorYellow(255, 255, 255, 50);

	uint32 s_hashMoveFurniture = 0;
	uint32 s_hashRotateFurniture = 0;

	void resetDeltas()
	{
		s_positionDelta = Vector::zero;
		s_yawDelta = 0.0f;
		s_pitchDelta = 0.0f;
		s_rollDelta = 0.0f;
	}

	void resetGizmoState()
	{
		s_activeComponent = CuiFurnitureMovementManager::GC_None;
		s_hoveredComponent = CuiFurnitureMovementManager::GC_None;
		s_isDragging = false;
		s_lastMouseX = 0;
		s_lastMouseY = 0;
	}

	float getObjectRadius(Object const * obj)
	{
		if (!obj) return 1.0f;
		Appearance const * const appearance = obj->getAppearance();
		if (!appearance) return 1.0f;
		
		Extent const * const extent = appearance->getExtent();
		if (extent)
		{
			BoxExtent const * const boxExtent = dynamic_cast<BoxExtent const *>(extent);
			if (boxExtent)
			{
				float const h = boxExtent->getHeight();
				float const w = boxExtent->getWidth();
				float const l = boxExtent->getLength();
				return std::max(h, std::max(w, l)) * 0.5f;
			}
			return extent->getSphere().getRadius();
		}
		
		Sphere const & sphere = appearance->getSphere();
		return std::max(0.5f, sphere.getRadius());
	}
}

using namespace CuiFurnitureMovementManagerNamespace;

//======================================================================

void CuiFurnitureMovementManager::install()
{
	DEBUG_FATAL(s_installed, ("CuiFurnitureMovementManager already installed"));
	s_installed = true;
	s_active = false;
	s_hashMoveFurniture = Crc::normalizeAndCalculate("moveFurniture");
	s_hashRotateFurniture = Crc::normalizeAndCalculate("rotateFurniture");
	resetDeltas();
	resetGizmoState();
}

//----------------------------------------------------------------------

void CuiFurnitureMovementManager::remove()
{
	DEBUG_FATAL(!s_installed, ("CuiFurnitureMovementManager not installed"));
	if (s_active) exitMovementMode(false);
	s_installed = false;
}

//----------------------------------------------------------------------

void CuiFurnitureMovementManager::update(float const deltaTimeSecs)
{
	if (!s_installed || !s_active) return;
	
	Object * const obj = getSelectedObject();
	if (!obj)
	{
		exitMovementMode(false);
		return;
	}
	
	// Only apply keyboard movement if not dragging gizmo
	if (!s_isDragging)
	{
		updateFurnitureTransform();
	}
	
	UNREF(deltaTimeSecs);
}

//----------------------------------------------------------------------

bool CuiFurnitureMovementManager::enterMovementMode(NetworkId const & furnitureId)
{
	if (!s_installed) return false;
	if (s_active) exitMovementMode(false);
	
	Object * const obj = NetworkIdManager::getObjectById(furnitureId);
	if (!obj)
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("Cannot find furniture object."));
		return false;
	}
	
	ClientObject * const clientObj = obj->asClientObject();
	if (!clientObj)
	{
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("Invalid furniture object."));
		return false;
	}
	
	s_selectedFurniture = furnitureId;
	s_originalTransform = obj->getTransform_o2p();
	s_cachedObjectRadius = getObjectRadius(obj);
	
	resetDeltas();
	resetGizmoState();
	
	s_active = true;
	CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("Furniture Mode: Drag gizmo, R=mode, Enter/Esc"));
	return true;
}

//----------------------------------------------------------------------

void CuiFurnitureMovementManager::exitMovementMode(bool const applyChanges)
{
	if (!s_active) return;
	
	if (applyChanges)
	{
		applyMovementToServer();
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("Furniture placement confirmed."));
	}
	else
	{
		revertToOriginalPosition();
		CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("Furniture placement cancelled."));
	}
	
	s_selectedFurniture = NetworkId::cms_invalid;
	s_active = false;
	resetDeltas();
	resetGizmoState();
}

//----------------------------------------------------------------------

bool CuiFurnitureMovementManager::isActive() { return s_active; }
NetworkId const & CuiFurnitureMovementManager::getSelectedFurniture() { return s_selectedFurniture; }

Object * CuiFurnitureMovementManager::getSelectedObject()
{
	if (!s_selectedFurniture.isValid()) return NULL;
	return s_selectedFurniture.getObject();
}

//----------------------------------------------------------------------

void CuiFurnitureMovementManager::moveX(float const amount) { s_positionDelta.x += amount; }
void CuiFurnitureMovementManager::moveY(float const amount) { s_positionDelta.y += amount; }
void CuiFurnitureMovementManager::moveZ(float const amount) { s_positionDelta.z += amount; }
void CuiFurnitureMovementManager::rotateYaw(float const degrees) { s_yawDelta += degrees; }
void CuiFurnitureMovementManager::rotatePitch(float const degrees) { s_pitchDelta += degrees; }
void CuiFurnitureMovementManager::rotateRoll(float const degrees) { s_rollDelta += degrees; }

float CuiFurnitureMovementManager::getMovementSpeed() { return s_movementSpeed; }
void CuiFurnitureMovementManager::setMovementSpeed(float const v) { s_movementSpeed = v; }
float CuiFurnitureMovementManager::getRotationSpeed() { return s_rotationSpeed; }
void CuiFurnitureMovementManager::setRotationSpeed(float const v) { s_rotationSpeed = v; }
bool CuiFurnitureMovementManager::isFineMode() { return s_fineMode; }
void CuiFurnitureMovementManager::setFineMode(bool const v) { s_fineMode = v; }

CuiFurnitureMovementManager::GizmoMode CuiFurnitureMovementManager::getGizmoMode() { return s_gizmoMode; }
void CuiFurnitureMovementManager::setGizmoMode(GizmoMode mode) { s_gizmoMode = mode; }
void CuiFurnitureMovementManager::cycleGizmoMode() 
{ 
	s_gizmoMode = (s_gizmoMode == GM_Translate) ? GM_Rotate : GM_Translate; 
}
float CuiFurnitureMovementManager::getGizmoScale() { return s_gizmoScale; }
void CuiFurnitureMovementManager::setGizmoScale(float scale) { s_gizmoScale = scale; }

Vector const & CuiFurnitureMovementManager::getPositionDelta() { return s_positionDelta; }
float CuiFurnitureMovementManager::getYawDelta() { return s_yawDelta; }
float CuiFurnitureMovementManager::getPitchDelta() { return s_pitchDelta; }
float CuiFurnitureMovementManager::getRollDelta() { return s_rollDelta; }

//----------------------------------------------------------------------

bool CuiFurnitureMovementManager::processKeyDown(int const keystroke)
{
	if (!s_active) return false;
	
	switch (keystroke)
	{
	case DIK_R:
		cycleGizmoMode();
		return true;
	case DIK_LSHIFT:
	case DIK_RSHIFT:
		s_fineMode = true;
		return true;
	case DIK_RETURN:
	case DIK_NUMPADENTER:
		exitMovementMode(true);
		return true;
	case DIK_ESCAPE:
		exitMovementMode(false);
		return true;
	default:
		break;
	}
	return false;
}

//----------------------------------------------------------------------

bool CuiFurnitureMovementManager::processKeyUp(int const keystroke)
{
	if (!s_active) return false;
	
	switch (keystroke)
	{
	case DIK_LSHIFT:
	case DIK_RSHIFT:
		s_fineMode = false;
		return true;
	default:
		break;
	}
	return false;
}

//----------------------------------------------------------------------

bool CuiFurnitureMovementManager::processMouseWheel(float const delta)
{
	if (!s_active) return false;
	
	// Mouse wheel adjusts gizmo scale
	float const scaleDelta = delta * 0.1f;
	s_gizmoScale = std::max(0.5f, std::min(3.0f, s_gizmoScale + scaleDelta));
	return true;
}

//----------------------------------------------------------------------

bool CuiFurnitureMovementManager::processMouseInput(int x, int y, bool leftButton, bool rightButton)
{
	if (!s_active) return false;
	
	// Right-click cancels
	if (rightButton)
	{
		if (s_isDragging)
		{
			s_isDragging = false;
			s_activeComponent = GC_None;
			return true;
		}
		exitMovementMode(false);
		return true;
	}
	
	Object * const obj = getSelectedObject();
	if (!obj) return false;
	
	Camera const * const camera = Game::getCamera();
	if (!camera) return false;
	
	Vector const objectPosition = obj->getPosition_w();
	float const gizmoSize = s_cachedObjectRadius * s_gizmoScale * 2.0f;
	
	// Handle dragging
	if (s_isDragging && leftButton)
	{
		int const deltaX = x - s_lastMouseX;
		int const deltaY = y - s_lastMouseY;
		
		if (deltaX != 0 || deltaY != 0)
		{
			processMouseDrag(x, y, deltaX, deltaY);
		}
		
		s_lastMouseX = x;
		s_lastMouseY = y;
		return true;
	}
	
	// Handle mouse down - start drag
	if (leftButton && !s_isDragging)
	{
		GizmoComponent const hit = hitTestGizmo(x, y, camera, objectPosition, gizmoSize);
		if (hit != GC_None)
		{
			s_isDragging = true;
			s_activeComponent = hit;
			s_lastMouseX = x;
			s_lastMouseY = y;
			s_dragStartPosition = objectPosition;
			return true;
		}
	}
	
	// Handle mouse up
	if (!leftButton && s_isDragging)
	{
		s_isDragging = false;
		s_activeComponent = GC_None;
	}
	
	// Update hover state
	s_hoveredComponent = hitTestGizmo(x, y, camera, objectPosition, gizmoSize);
	s_lastMouseX = x;
	s_lastMouseY = y;
	
	return s_hoveredComponent != GC_None || s_isDragging;
}

//----------------------------------------------------------------------

bool CuiFurnitureMovementManager::processMouseDown(int x, int y, bool leftButton, bool rightButton)
{
	return processMouseInput(x, y, leftButton, rightButton);
}

//----------------------------------------------------------------------

bool CuiFurnitureMovementManager::processMouseUp(int x, int y, bool leftButton, bool rightButton)
{
	UNREF(leftButton);
	UNREF(rightButton);
	
	if (!s_active) return false;
	
	if (s_isDragging)
	{
		s_isDragging = false;
		s_activeComponent = GC_None;
		s_lastMouseX = x;
		s_lastMouseY = y;
		return true;
	}
	return false;
}

//----------------------------------------------------------------------

bool CuiFurnitureMovementManager::processMouseDrag(int x, int y, int deltaX, int deltaY)
{
	UNREF(x);
	UNREF(y);
	
	if (!s_active || !s_isDragging || s_activeComponent == GC_None) return false;
	
	float const sensitivity = s_fineMode ? 0.002f : 0.02f;
	
	switch (s_activeComponent)
	{
	case GC_AxisX:
		moveX(static_cast<float>(deltaX) * sensitivity);
		break;
	case GC_AxisY:
		moveY(static_cast<float>(-deltaY) * sensitivity);
		break;
	case GC_AxisZ:
		moveZ(static_cast<float>(-deltaY) * sensitivity);
		break;
	case GC_PlaneXY:
		moveX(static_cast<float>(deltaX) * sensitivity);
		moveY(static_cast<float>(-deltaY) * sensitivity);
		break;
	case GC_PlaneXZ:
		moveX(static_cast<float>(deltaX) * sensitivity);
		moveZ(static_cast<float>(-deltaY) * sensitivity);
		break;
	case GC_PlaneYZ:
		moveY(static_cast<float>(deltaX) * sensitivity);
		moveZ(static_cast<float>(-deltaY) * sensitivity);
		break;
	case GC_RotateYaw:
		rotateYaw(static_cast<float>(deltaX) * sensitivity * 50.0f);
		break;
	case GC_RotatePitch:
		rotatePitch(static_cast<float>(deltaY) * sensitivity * 50.0f);
		break;
	case GC_RotateRoll:
		rotateRoll(static_cast<float>(deltaX) * sensitivity * 50.0f);
		break;
	case GC_Sphere:
		rotateYaw(static_cast<float>(deltaX) * sensitivity * 25.0f);
		rotatePitch(static_cast<float>(deltaY) * sensitivity * 25.0f);
		break;
	default:
		return false;
	}
	
	updateFurnitureTransform();
	return true;
}

//----------------------------------------------------------------------

void CuiFurnitureMovementManager::render()
{
	if (!s_active || !s_selectedFurniture.isValid()) return;
	
	Object * const obj = getSelectedObject();
	if (!obj) return;
	
	Camera const * const camera = Game::getCamera();
	if (!camera) return;
	
	// Set up 3D rendering shader (like GodClient does)
	Graphics::setStaticShader(ShaderTemplateList::get3dVertexColorZStaticShader());
	
	// Get world transform and calculate gizmo size
	Transform const & objTransform = obj->getTransform_o2w();
	float const gizmoSize = s_cachedObjectRadius * s_gizmoScale * 2.0f;
	
	// Set up transform for 3D rendering at object location
	Graphics::setObjectToWorldTransformAndScale(objTransform, obj->getScale());
	
	// Draw axis frame using built-in function (RGB lines for XYZ axes)
	Graphics::drawFrame(gizmoSize);
	
	// Draw additional gizmo elements based on mode
	if (s_gizmoMode == GM_Translate)
	{
		// Draw axis endpoint cubes
		bool const xH = (s_hoveredComponent == GC_AxisX || s_activeComponent == GC_AxisX);
		bool const yH = (s_hoveredComponent == GC_AxisY || s_activeComponent == GC_AxisY);
		bool const zH = (s_hoveredComponent == GC_AxisZ || s_activeComponent == GC_AxisZ);
		
		float const cubeSize = gizmoSize * 0.1f;
		VectorArgb const xColor = xH ? VectorArgb(1.0f, 1.0f, 0.5f, 0.5f) : VectorArgb(1.0f, 1.0f, 0.2f, 0.2f);
		VectorArgb const yColor = yH ? VectorArgb(1.0f, 0.5f, 1.0f, 0.5f) : VectorArgb(1.0f, 0.2f, 1.0f, 0.2f);
		VectorArgb const zColor = zH ? VectorArgb(1.0f, 0.5f, 0.5f, 1.0f) : VectorArgb(1.0f, 0.2f, 0.2f, 1.0f);
		
		Graphics::drawCube(Vector(gizmoSize, 0.0f, 0.0f), cubeSize, xColor);
		Graphics::drawCube(Vector(0.0f, gizmoSize, 0.0f), cubeSize, yColor);
		Graphics::drawCube(Vector(0.0f, 0.0f, gizmoSize), cubeSize, zColor);
	}
	else if (s_gizmoMode == GM_Rotate)
	{
		// Draw rotation circles
		bool const yawH = (s_hoveredComponent == GC_RotateYaw || s_activeComponent == GC_RotateYaw);
		bool const pitchH = (s_hoveredComponent == GC_RotatePitch || s_activeComponent == GC_RotatePitch);
		bool const rollH = (s_hoveredComponent == GC_RotateRoll || s_activeComponent == GC_RotateRoll);
		
		float const radius = gizmoSize * 0.8f;
		int const segments = 32;
		
		VectorArgb const yawColor = yawH ? VectorArgb(1.0f, 0.5f, 1.0f, 0.5f) : VectorArgb(1.0f, 0.2f, 1.0f, 0.2f);
		VectorArgb const pitchColor = pitchH ? VectorArgb(1.0f, 1.0f, 0.5f, 0.5f) : VectorArgb(1.0f, 1.0f, 0.2f, 0.2f);
		VectorArgb const rollColor = rollH ? VectorArgb(1.0f, 0.5f, 0.5f, 1.0f) : VectorArgb(1.0f, 0.2f, 0.2f, 1.0f);
		
		Graphics::drawCircle(Vector::zero, Vector::unitY, radius, segments, yawColor);
		Graphics::drawCircle(Vector::zero, Vector::unitX, radius, segments, pitchColor);
		Graphics::drawCircle(Vector::zero, Vector::unitZ, radius, segments, rollColor);
	}
	
	// Draw center indicator
	bool const sphereH = (s_hoveredComponent == GC_Sphere || s_activeComponent == GC_Sphere);
	VectorArgb const sphereColor = sphereH ? VectorArgb(1.0f, 1.0f, 1.0f, 0.8f) : VectorArgb(1.0f, 1.0f, 1.0f, 0.4f);
	Graphics::drawOctahedron(Vector::zero, gizmoSize * 0.15f, sphereColor);
}

//----------------------------------------------------------------------

void CuiFurnitureMovementManager::renderTranslationGizmo(Vector const & position, float scale)
{
	UNREF(position);
	UNREF(scale);
	// Handled in render()
}

//----------------------------------------------------------------------

void CuiFurnitureMovementManager::renderRotationGizmo(Vector const & position, float scale)
{
	UNREF(position);
	UNREF(scale);
	// Handled in render()
}

//----------------------------------------------------------------------

void CuiFurnitureMovementManager::renderAxisLine(Vector const & start, Vector const & end, unsigned int color, bool highlighted)
{
	UNREF(color);
	UNREF(highlighted);
	Graphics::drawLine(start, end, VectorArgb::solidWhite);
}

//----------------------------------------------------------------------

void CuiFurnitureMovementManager::renderCircle(Vector const & center, Vector const & normal, float radius, unsigned int color, bool highlighted, int segments)
{
	UNREF(color);
	UNREF(highlighted);
	Graphics::drawCircle(center, normal, radius, segments, VectorArgb::solidWhite);
}

//----------------------------------------------------------------------

CuiFurnitureMovementManager::GizmoComponent CuiFurnitureMovementManager::hitTestGizmo(int screenX, int screenY, Camera const * camera, Vector const & gizmoPosition, float gizmoSize)
{
	if (!camera) return GC_None;
	
	float const threshold = 20.0f; // pixels
	
	if (s_gizmoMode == GM_Translate)
	{
		// Test axis lines
		if (hitTestAxis(screenX, screenY, camera, gizmoPosition, gizmoPosition + Vector(gizmoSize, 0.0f, 0.0f), threshold))
			return GC_AxisX;
		if (hitTestAxis(screenX, screenY, camera, gizmoPosition, gizmoPosition + Vector(0.0f, gizmoSize, 0.0f), threshold))
			return GC_AxisY;
		if (hitTestAxis(screenX, screenY, camera, gizmoPosition, gizmoPosition + Vector(0.0f, 0.0f, gizmoSize), threshold))
			return GC_AxisZ;
	}
	else if (s_gizmoMode == GM_Rotate)
	{
		float const radius = gizmoSize * 0.8f;
		
		if (hitTestCircle(screenX, screenY, camera, gizmoPosition, Vector::unitY, radius, threshold))
			return GC_RotateYaw;
		if (hitTestCircle(screenX, screenY, camera, gizmoPosition, Vector::unitX, radius, threshold))
			return GC_RotatePitch;
		if (hitTestCircle(screenX, screenY, camera, gizmoPosition, Vector::unitZ, radius, threshold))
			return GC_RotateRoll;
	}
	
	// Test center sphere
	Vector screenPos;
	if (camera->projectInWorldSpace(gizmoPosition, &screenPos.x, &screenPos.y, &screenPos.z, false))
	{
		float const dx = static_cast<float>(screenX) - screenPos.x;
		float const dy = static_cast<float>(screenY) - screenPos.y;
		if (std::sqrt(dx * dx + dy * dy) < threshold * 1.5f)
			return GC_Sphere;
	}
	
	return GC_None;
}

//----------------------------------------------------------------------

bool CuiFurnitureMovementManager::hitTestAxis(int screenX, int screenY, Camera const * camera, Vector const & start, Vector const & end, float threshold)
{
	if (!camera) return false;
	
	Vector screenStart, screenEnd;
	if (!camera->projectInWorldSpace(start, &screenStart.x, &screenStart.y, &screenStart.z, false))
		return false;
	if (!camera->projectInWorldSpace(end, &screenEnd.x, &screenEnd.y, &screenEnd.z, false))
		return false;
	
	// Point to line segment distance in 2D
	float const ax = screenEnd.x - screenStart.x;
	float const ay = screenEnd.y - screenStart.y;
	float const bx = static_cast<float>(screenX) - screenStart.x;
	float const by = static_cast<float>(screenY) - screenStart.y;
	
	float const lenSq = ax * ax + ay * ay;
	if (lenSq < 0.001f) return false;
	
	float t = (ax * bx + ay * by) / lenSq;
	t = std::max(0.0f, std::min(1.0f, t));
	
	float const projX = screenStart.x + t * ax;
	float const projY = screenStart.y + t * ay;
	
	float const dx = static_cast<float>(screenX) - projX;
	float const dy = static_cast<float>(screenY) - projY;
	float const dist = std::sqrt(dx * dx + dy * dy);
	
	return dist < threshold;
}

//----------------------------------------------------------------------

bool CuiFurnitureMovementManager::hitTestCircle(int screenX, int screenY, Camera const * camera, Vector const & center, Vector const & normal, float radius, float threshold)
{
	if (!camera) return false;
	
	// Sample points on the circle and test distance
	int const numSamples = 16;
	Vector tangent, bitangent;
	
	// Create orthonormal basis
	if (std::fabs(normal.y) < 0.99f)
		tangent = normal.cross(Vector::unitY);
	else
		tangent = normal.cross(Vector::unitX);
	tangent.normalize();
	bitangent = normal.cross(tangent);
	
	for (int i = 0; i < numSamples; ++i)
	{
		float const angle = (static_cast<float>(i) / static_cast<float>(numSamples)) * 2.0f * PI;
		Vector const worldPoint = center + tangent * (std::cos(angle) * radius) + bitangent * (std::sin(angle) * radius);
		
		Vector screenPoint;
		if (camera->projectInWorldSpace(worldPoint, &screenPoint.x, &screenPoint.y, &screenPoint.z, false))
		{
			float const dx = static_cast<float>(screenX) - screenPoint.x;
			float const dy = static_cast<float>(screenY) - screenPoint.y;
			if (std::sqrt(dx * dx + dy * dy) < threshold)
				return true;
		}
	}
	
	return false;
}

//----------------------------------------------------------------------

void CuiFurnitureMovementManager::applyMovementToServer()
{
	if (!s_selectedFurniture.isValid()) return;
	
	NetworkId const & targetId = s_selectedFurniture;
	char buffer[256];
	float const epsilon = 0.001f;
	
	if (std::fabs(s_positionDelta.z) > epsilon)
	{
		if (s_positionDelta.z > 0)
			snprintf(buffer, sizeof(buffer), "forward %f", s_positionDelta.z);
		else
			snprintf(buffer, sizeof(buffer), "back %f", -s_positionDelta.z);
		ClientCommandQueue::enqueueCommand(s_hashMoveFurniture, targetId, Unicode::narrowToWide(buffer));
	}
	
	if (std::fabs(s_positionDelta.x) > epsilon)
	{
		if (s_positionDelta.x > 0)
			snprintf(buffer, sizeof(buffer), "right %f", s_positionDelta.x);
		else
			snprintf(buffer, sizeof(buffer), "left %f", -s_positionDelta.x);
		ClientCommandQueue::enqueueCommand(s_hashMoveFurniture, targetId, Unicode::narrowToWide(buffer));
	}
	
	if (std::fabs(s_positionDelta.y) > epsilon)
	{
		if (s_positionDelta.y > 0)
			snprintf(buffer, sizeof(buffer), "up %f", s_positionDelta.y);
		else
			snprintf(buffer, sizeof(buffer), "down %f", -s_positionDelta.y);
		ClientCommandQueue::enqueueCommand(s_hashMoveFurniture, targetId, Unicode::narrowToWide(buffer));
	}
	
	if (std::fabs(s_yawDelta) > epsilon)
	{
		snprintf(buffer, sizeof(buffer), "yaw %f", s_yawDelta);
		ClientCommandQueue::enqueueCommand(s_hashRotateFurniture, targetId, Unicode::narrowToWide(buffer));
	}
	
	if (std::fabs(s_pitchDelta) > epsilon)
	{
		snprintf(buffer, sizeof(buffer), "pitch %f", s_pitchDelta);
		ClientCommandQueue::enqueueCommand(s_hashRotateFurniture, targetId, Unicode::narrowToWide(buffer));
	}
	
	if (std::fabs(s_rollDelta) > epsilon)
	{
		snprintf(buffer, sizeof(buffer), "roll %f", s_rollDelta);
		ClientCommandQueue::enqueueCommand(s_hashRotateFurniture, targetId, Unicode::narrowToWide(buffer));
	}
}

//----------------------------------------------------------------------

void CuiFurnitureMovementManager::revertToOriginalPosition()
{
	Object * const obj = getSelectedObject();
	if (obj)
	{
		obj->setTransform_o2p(s_originalTransform);
	}
}

//----------------------------------------------------------------------

void CuiFurnitureMovementManager::updateFurnitureTransform()
{
	Object * const obj = getSelectedObject();
	if (!obj) return;
	
	Transform newTransform = s_originalTransform;
	
	Vector position = newTransform.getPosition_p();
	position += s_positionDelta;
	newTransform.setPosition_p(position);
	
	float const yawRad = s_yawDelta * PI / 180.0f;
	float const pitchRad = s_pitchDelta * PI / 180.0f;
	float const rollRad = s_rollDelta * PI / 180.0f;
	
	newTransform.yaw_l(yawRad);
	newTransform.pitch_l(pitchRad);
	newTransform.roll_l(rollRad);
	
	obj->setTransform_o2p(newTransform);
}

//======================================================================
