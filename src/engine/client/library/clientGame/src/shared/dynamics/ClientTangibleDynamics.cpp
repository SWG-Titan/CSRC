// ======================================================================
//
// ClientTangibleDynamics.cpp
// Copyright 2025, 2026 Titan SWG, All Rights Reserved
//
// Client-side TangibleDynamics: push, spin, breathing,
// bounce, wobble, orbit with drag and easing support.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientTangibleDynamics.h"

#include "sharedMath/Vector.h"
#include "sharedObject/Object.h"
#include "sharedMath/Transform.h"

#include <cmath>

// ======================================================================

namespace ClientTangibleDynamicsNamespace
{
	static float const s_bounceMinVelocity = 0.05f;
}

using namespace ClientTangibleDynamicsNamespace;

// Use global PI and PI_TIMES_2 from FloatMath.h (included via FirstClientGame.h)

// ======================================================================
// EASING
// ======================================================================

float ClientTangibleDynamics::computeEaseFactor(EaseType easeType, float elapsed, float duration, float easeDuration)
{
	if (easeType == ET_none || easeDuration <= 0.0f)
		return 1.0f;

	float factor = 1.0f;

	switch (easeType)
	{
	case ET_easeIn:
		if (elapsed < easeDuration)
			factor = elapsed / easeDuration;
		break;
	case ET_easeOut:
		if (duration > 0.0f && elapsed > (duration - easeDuration))
		{
			float remaining = duration - elapsed;
			factor = (remaining > 0.0f) ? (remaining / easeDuration) : 0.0f;
		}
		break;
	case ET_easeInOut:
		if (elapsed < easeDuration)
			factor = elapsed / easeDuration;
		else if (duration > 0.0f && elapsed > (duration - easeDuration))
		{
			float remaining = duration - elapsed;
			factor = (remaining > 0.0f) ? (remaining / easeDuration) : 0.0f;
		}
		break;
	default:
		break;
	}

	return factor * factor * (3.0f - 2.0f * factor);
}

// ======================================================================
// CONSTRUCTOR / DESTRUCTOR
// ======================================================================

ClientTangibleDynamics::ClientTangibleDynamics(Object* owner) :
	Dynamics(owner),
	m_pushVelocity(Vector::zero),
	m_pushDuration(-1.0f),
	m_pushElapsed(0.0f),
	m_pushDrag(0.0f),
	m_pushSpace(MS_world),
	m_pushForceActive(false),
	m_spinAngular(Vector::zero),
	m_spinDuration(-1.0f),
	m_spinElapsed(0.0f),
	m_spinForceActive(false),
	m_spinAroundAppearanceCenter(false),
	m_baseScale(owner ? owner->getScale() : Vector::one),
	m_breathingMin(1.0f),
	m_breathingMax(1.0f),
	m_breathingSpeed(1.0f),
	m_breathingDuration(-1.0f),
	m_breathingElapsed(0.0f),
	m_breathingPhase(0.0f),
	m_breathingEffectActive(false),
	m_bounceGravity(9.8f),
	m_bounceElasticity(0.6f),
	m_bounceVerticalVelocity(0.0f),
	m_bounceFloorY(0.0f),
	m_bounceDuration(-1.0f),
	m_bounceElapsed(0.0f),
	m_bounceEffectActive(false),
	m_wobbleAmplitude(Vector::zero),
	m_wobbleFrequency(Vector::zero),
	m_wobbleDuration(-1.0f),
	m_wobbleElapsed(0.0f),
	m_wobblePhase(0.0f),
	m_wobbleOrigin(Vector::zero),
	m_wobbleEffectActive(false),
	m_orbitCenter(Vector::zero),
	m_orbitRadius(1.0f),
	m_orbitSpeed(1.0f),
	m_orbitAngle(0.0f),
	m_orbitDuration(-1.0f),
	m_orbitElapsed(0.0f),
	m_orbitEffectActive(false),
	m_easeType(ET_none),
	m_easeDuration(0.5f),
	m_activeForceMask(FM_none)
{
}

// ----------------------------------------------------------------------

ClientTangibleDynamics::~ClientTangibleDynamics()
{
	Object* const owner = getOwner();
	if (owner != NULL && m_breathingEffectActive)
	{
		owner->setRecursiveScale(m_baseScale);
	}
}

// ======================================================================
// PUSH
// ======================================================================

void ClientTangibleDynamics::setPushForce(const Vector& velocity, float duration, MovementSpace space)
{
	m_pushVelocity = velocity;
	m_pushDuration = duration;
	m_pushElapsed = 0.0f;
	m_pushDrag = 0.0f;
	m_pushSpace = space;
	m_pushForceActive = true;
	recalculateMode();
}

void ClientTangibleDynamics::setPushForceWithDrag(const Vector& velocity, float drag, float duration, MovementSpace space)
{
	m_pushVelocity = velocity;
	m_pushDuration = duration;
	m_pushElapsed = 0.0f;
	m_pushDrag = drag;
	m_pushSpace = space;
	m_pushForceActive = true;
	recalculateMode();
}

void ClientTangibleDynamics::clearPushForce()
{
	m_pushVelocity = Vector::zero;
	m_pushDuration = -1.0f;
	m_pushElapsed = 0.0f;
	m_pushDrag = 0.0f;
	m_pushForceActive = false;
	recalculateMode();
}

Vector ClientTangibleDynamics::getPushForce() const              { return m_pushVelocity; }
void   ClientTangibleDynamics::setPushDrag(float drag)           { m_pushDrag = drag; }
float  ClientTangibleDynamics::getPushDrag() const               { return m_pushDrag; }

float ClientTangibleDynamics::getPushForceDuration() const
{
	return (m_pushDuration < 0.0f) ? -1.0f : (m_pushDuration - m_pushElapsed);
}

// ======================================================================
// SPIN
// ======================================================================

void ClientTangibleDynamics::setSpinForce(const Vector& r, float duration)
{
	m_spinAngular = r;
	m_spinDuration = duration;
	m_spinElapsed = 0.0f;
	m_spinForceActive = true;
	recalculateMode();
}

void ClientTangibleDynamics::clearSpinForce()
{
	m_spinAngular = Vector::zero;
	m_spinDuration = -1.0f;
	m_spinElapsed = 0.0f;
	m_spinForceActive = false;
	recalculateMode();
}

Vector ClientTangibleDynamics::getSpinForce() const                  { return m_spinAngular; }
bool   ClientTangibleDynamics::getSpinAroundAppearanceCenter() const { return m_spinAroundAppearanceCenter; }
void   ClientTangibleDynamics::setSpinAroundAppearanceCenter(bool v) { m_spinAroundAppearanceCenter = v; }

float ClientTangibleDynamics::getSpinForceDuration() const
{
	return (m_spinDuration < 0.0f) ? -1.0f : (m_spinDuration - m_spinElapsed);
}

// ======================================================================
// BREATHING
// ======================================================================

void ClientTangibleDynamics::setBreathingEffect(float min, float max, float speed, float duration)
{
	m_breathingMin = min;
	m_breathingMax = max;
	m_breathingSpeed = speed;
	m_breathingDuration = duration;
	m_breathingElapsed = 0.0f;
	m_breathingPhase = 0.0f;
	m_breathingEffectActive = true;

	Object* const owner = getOwner();
	if (owner != NULL)
		m_baseScale = owner->getScale();

	recalculateMode();
}

void ClientTangibleDynamics::clearBreathingEffect()
{
	Object* const owner = getOwner();
	if (owner != NULL && m_breathingEffectActive)
		owner->setRecursiveScale(m_baseScale);

	m_breathingMin = 1.0f;
	m_breathingMax = 1.0f;
	m_breathingSpeed = 1.0f;
	m_breathingDuration = -1.0f;
	m_breathingElapsed = 0.0f;
	m_breathingPhase = 0.0f;
	m_breathingEffectActive = false;
	recalculateMode();
}

float ClientTangibleDynamics::getBreathingMinScale() const  { return m_breathingMin; }
float ClientTangibleDynamics::getBreathingMaxScale() const  { return m_breathingMax; }
float ClientTangibleDynamics::getBreathingSpeed() const     { return m_breathingSpeed; }

float ClientTangibleDynamics::getBreathingDuration() const
{
	return (m_breathingDuration < 0.0f) ? -1.0f : (m_breathingDuration - m_breathingElapsed);
}

// ======================================================================
// BOUNCE
// ======================================================================

void ClientTangibleDynamics::setBounceEffect(float gravity, float elasticity, float initialUpVelocity, float duration)
{
	Object* const owner = getOwner();
	m_bounceGravity = gravity;
	m_bounceElasticity = clamp(0.0f, elasticity, 1.0f);
	m_bounceVerticalVelocity = initialUpVelocity;
	m_bounceFloorY = owner ? owner->getPosition_w().y : 0.0f;
	m_bounceDuration = duration;
	m_bounceElapsed = 0.0f;
	m_bounceEffectActive = true;
	recalculateMode();
}

void ClientTangibleDynamics::clearBounceEffect()
{
	m_bounceGravity = 9.8f;
	m_bounceElasticity = 0.6f;
	m_bounceVerticalVelocity = 0.0f;
	m_bounceDuration = -1.0f;
	m_bounceElapsed = 0.0f;
	m_bounceEffectActive = false;
	recalculateMode();
}

float ClientTangibleDynamics::getBounceGravity() const     { return m_bounceGravity; }
float ClientTangibleDynamics::getBounceElasticity() const  { return m_bounceElasticity; }

// ======================================================================
// WOBBLE
// ======================================================================

void ClientTangibleDynamics::setWobbleEffect(const Vector& amplitude, const Vector& frequency, float duration)
{
	Object* const owner = getOwner();
	m_wobbleAmplitude = amplitude;
	m_wobbleFrequency = frequency;
	m_wobbleDuration = duration;
	m_wobbleElapsed = 0.0f;
	m_wobblePhase = 0.0f;
	m_wobbleOrigin = owner ? owner->getPosition_w() : Vector::zero;
	m_wobbleEffectActive = true;
	recalculateMode();
}

void ClientTangibleDynamics::clearWobbleEffect()
{
	Object* const owner = getOwner();
	if (owner != NULL && m_wobbleEffectActive)
		owner->setPosition_w(m_wobbleOrigin);

	m_wobbleAmplitude = Vector::zero;
	m_wobbleFrequency = Vector::zero;
	m_wobbleDuration = -1.0f;
	m_wobbleElapsed = 0.0f;
	m_wobblePhase = 0.0f;
	m_wobbleEffectActive = false;
	recalculateMode();
}

Vector ClientTangibleDynamics::getWobbleAmplitude() const  { return m_wobbleAmplitude; }
Vector ClientTangibleDynamics::getWobbleFrequency() const  { return m_wobbleFrequency; }

// ======================================================================
// ORBIT
// ======================================================================

void ClientTangibleDynamics::setOrbitEffect(const Vector& center, float radius, float radiansPerSecond, float duration)
{
	Object* const owner = getOwner();
	m_orbitCenter = center;
	m_orbitRadius = radius;
	m_orbitSpeed = radiansPerSecond;
	m_orbitDuration = duration;
	m_orbitElapsed = 0.0f;
	m_orbitEffectActive = true;

	if (owner != NULL)
	{
		Vector delta = owner->getPosition_w() - center;
		m_orbitAngle = atan2(delta.x, delta.z);
	}
	else
	{
		m_orbitAngle = 0.0f;
	}

	recalculateMode();
}

void ClientTangibleDynamics::clearOrbitEffect()
{
	m_orbitCenter = Vector::zero;
	m_orbitRadius = 1.0f;
	m_orbitSpeed = 1.0f;
	m_orbitAngle = 0.0f;
	m_orbitDuration = -1.0f;
	m_orbitElapsed = 0.0f;
	m_orbitEffectActive = false;
	recalculateMode();
}

Vector ClientTangibleDynamics::getOrbitCenter() const  { return m_orbitCenter; }
float  ClientTangibleDynamics::getOrbitRadius() const  { return m_orbitRadius; }

// ======================================================================
// EASING / COMBINED / QUERY
// ======================================================================

void ClientTangibleDynamics::setEasing(EaseType easeType, float easeDuration)
{
	m_easeType = easeType;
	m_easeDuration = easeDuration;
}

void ClientTangibleDynamics::setCombinedForces(const Vector& pushVelocity, const Vector& spinAngular,
	float minScale, float maxScale, float breatheSpeed, float duration)
{
	setPushForce(pushVelocity, duration, MS_world);
	setSpinForce(spinAngular, duration);
	setBreathingEffect(minScale, maxScale, breatheSpeed, duration);
}

void ClientTangibleDynamics::clearAllForces()
{
	clearPushForce();
	clearSpinForce();
	clearBreathingEffect();
	clearBounceEffect();
	clearWobbleEffect();
	clearOrbitEffect();
}

int  ClientTangibleDynamics::getActiveForceMask() const          { return m_activeForceMask; }
bool ClientTangibleDynamics::isActive() const                    { return m_activeForceMask != FM_none; }
bool ClientTangibleDynamics::isForceActive(ForceMode mode) const { return (m_activeForceMask & static_cast<int>(mode)) != 0; }

// ======================================================================
// ALTER
// ======================================================================

float ClientTangibleDynamics::alter(float elapsedTime)
{
	float const baseAlterResult = Dynamics::alter(elapsedTime);

	if (m_activeForceMask != FM_none)
	{
		if (m_pushForceActive)       updatePushForce(elapsedTime);
		if (m_spinForceActive)       updateSpinForce(elapsedTime);
		if (m_breathingEffectActive) updateBreathingEffect(elapsedTime);
		if (m_bounceEffectActive)    updateBounceEffect(elapsedTime);
		if (m_wobbleEffectActive)    updateWobbleEffect(elapsedTime);
		if (m_orbitEffectActive)     updateOrbitEffect(elapsedTime);
	}

	// Return 0 (alter next frame) if any force active
	return (m_activeForceMask != FM_none) ? 0.0f : baseAlterResult;
}

// ======================================================================
// UPDATE HELPERS
// ======================================================================

void ClientTangibleDynamics::updatePushForce(float elapsedTime)
{
	Object* const owner = getOwner();
	if (owner == NULL) { clearPushForce(); return; }

	if (m_pushDuration >= 0.0f)
	{
		m_pushElapsed += elapsedTime;
		if (m_pushElapsed >= m_pushDuration) { clearPushForce(); return; }
	}

	float const ease = computeEaseFactor(m_easeType, m_pushElapsed, m_pushDuration, m_easeDuration);

	if (m_pushDrag > 0.0f)
	{
		float const dragFactor = exp(-m_pushDrag * elapsedTime);
		m_pushVelocity *= dragFactor;
		if (m_pushVelocity.magnitudeSquared() < 0.001f) { clearPushForce(); return; }
	}

	Vector const vel = m_pushVelocity * ease;

	switch (m_pushSpace)
	{
	case MS_world:
		owner->move_o(owner->rotate_w2o(vel * elapsedTime));
		break;
	case MS_parent:
		owner->move_o(owner->rotate_p2o(vel * elapsedTime));
		break;
	case MS_object:
		owner->move_o(vel * elapsedTime);
		break;
	}
}

// ----------------------------------------------------------------------

void ClientTangibleDynamics::updateSpinForce(float elapsedTime)
{
	Object* const owner = getOwner();
	if (owner == NULL) { clearSpinForce(); return; }

	if (m_spinDuration >= 0.0f)
	{
		m_spinElapsed += elapsedTime;
		if (m_spinElapsed >= m_spinDuration) { clearSpinForce(); return; }
	}

	float const ease = computeEaseFactor(m_easeType, m_spinElapsed, m_spinDuration, m_easeDuration);
	Vector const angular = m_spinAngular * ease;

	if (m_spinAroundAppearanceCenter)
	{
		Transform t = owner->getTransform_o2p();
		t.move_l(owner->getAppearanceSphereCenter());
		t.yaw_l(angular.x * elapsedTime);
		t.pitch_l(angular.y * elapsedTime);
		t.roll_l(angular.z * elapsedTime);
		t.move_l(-owner->getAppearanceSphereCenter());
		owner->setTransform_o2p(t);
	}
	else
	{
		owner->yaw_o(angular.x * elapsedTime);
		owner->pitch_o(angular.y * elapsedTime);
		owner->roll_o(angular.z * elapsedTime);
	}
}

// ----------------------------------------------------------------------

void ClientTangibleDynamics::updateBreathingEffect(float elapsedTime)
{
	Object* const owner = getOwner();
	if (owner == NULL) { clearBreathingEffect(); return; }

	if (m_breathingDuration >= 0.0f)
	{
		m_breathingElapsed += elapsedTime;
		if (m_breathingElapsed >= m_breathingDuration) { clearBreathingEffect(); return; }
	}

	float const ease = computeEaseFactor(m_easeType, m_breathingElapsed, m_breathingDuration, m_easeDuration);

	m_breathingPhase += elapsedTime * m_breathingSpeed;
	float const halfRange = (m_breathingMax - m_breathingMin) * 0.5f;
	float const midpoint  = (m_breathingMax + m_breathingMin) * 0.5f;
	float const rawScale  = midpoint + halfRange * cos(m_breathingPhase * PI_TIMES_2);
	float const effectiveScale = 1.0f + (rawScale - 1.0f) * ease;

	owner->setRecursiveScale(m_baseScale * effectiveScale);
}

// ----------------------------------------------------------------------

void ClientTangibleDynamics::updateBounceEffect(float elapsedTime)
{
	Object* const owner = getOwner();
	if (owner == NULL) { clearBounceEffect(); return; }

	if (m_bounceDuration >= 0.0f)
	{
		m_bounceElapsed += elapsedTime;
		if (m_bounceElapsed >= m_bounceDuration) { clearBounceEffect(); return; }
	}

	m_bounceVerticalVelocity -= m_bounceGravity * elapsedTime;

	Vector pos = owner->getPosition_w();
	pos.y += m_bounceVerticalVelocity * elapsedTime;

	if (pos.y <= m_bounceFloorY)
	{
		pos.y = m_bounceFloorY;
		m_bounceVerticalVelocity = -m_bounceVerticalVelocity * m_bounceElasticity;
		if (fabs(m_bounceVerticalVelocity) < s_bounceMinVelocity) { clearBounceEffect(); return; }
	}

	owner->setPosition_w(pos);
}

// ----------------------------------------------------------------------

void ClientTangibleDynamics::updateWobbleEffect(float elapsedTime)
{
	Object* const owner = getOwner();
	if (owner == NULL) { clearWobbleEffect(); return; }

	if (m_wobbleDuration >= 0.0f)
	{
		m_wobbleElapsed += elapsedTime;
		if (m_wobbleElapsed >= m_wobbleDuration) { clearWobbleEffect(); return; }
	}

	float const ease = computeEaseFactor(m_easeType, m_wobbleElapsed, m_wobbleDuration, m_easeDuration);
	m_wobblePhase += elapsedTime;

	Vector pos = m_wobbleOrigin;
	pos.x += m_wobbleAmplitude.x * sin(m_wobbleFrequency.x * m_wobblePhase * PI_TIMES_2) * ease;
	pos.y += m_wobbleAmplitude.y * sin(m_wobbleFrequency.y * m_wobblePhase * PI_TIMES_2) * ease;
	pos.z += m_wobbleAmplitude.z * sin(m_wobbleFrequency.z * m_wobblePhase * PI_TIMES_2) * ease;

	owner->setPosition_w(pos);
}

// ----------------------------------------------------------------------

void ClientTangibleDynamics::updateOrbitEffect(float elapsedTime)
{
	Object* const owner = getOwner();
	if (owner == NULL) { clearOrbitEffect(); return; }

	if (m_orbitDuration >= 0.0f)
	{
		m_orbitElapsed += elapsedTime;
		if (m_orbitElapsed >= m_orbitDuration) { clearOrbitEffect(); return; }
	}

	float const ease = computeEaseFactor(m_easeType, m_orbitElapsed, m_orbitDuration, m_easeDuration);
	m_orbitAngle += m_orbitSpeed * elapsedTime * ease;
	float const effectiveRadius = m_orbitRadius * ease;

	Vector pos;
	pos.x = m_orbitCenter.x + sin(m_orbitAngle) * effectiveRadius;
	pos.y = owner->getPosition_w().y;
	pos.z = m_orbitCenter.z + cos(m_orbitAngle) * effectiveRadius;

	owner->setPosition_w(pos);
}

// ======================================================================
// RECALCULATE
// ======================================================================

void ClientTangibleDynamics::recalculateMode()
{
	m_activeForceMask = FM_none;
	if (m_pushForceActive)       m_activeForceMask |= FM_push;
	if (m_spinForceActive)       m_activeForceMask |= FM_spin;
	if (m_breathingEffectActive) m_activeForceMask |= FM_breathing;
	if (m_bounceEffectActive)    m_activeForceMask |= FM_bounce;
	if (m_wobbleEffectActive)    m_activeForceMask |= FM_wobble;
	if (m_orbitEffectActive)     m_activeForceMask |= FM_orbit;
}

// ======================================================================
