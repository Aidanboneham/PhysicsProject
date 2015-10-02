#include "Physics.h"


void Physics::setupJASTutorial()
{
    PxRigidDynamic* dynamic_sphere;
    PxRigidStatic * static_sphere;

    PxSphereGeometry s(3);

    PxTransform t = PxTransform(0, 30, 0);
    float density = 3.f;

    static_sphere = PxCreateStatic(*m_physics, t, s, *m_physics_material);
    m_physics_scene->addActor(*static_sphere);

    t.p.y = 22;
    dynamic_sphere = PxCreateDynamic(*m_physics, t, s, *m_physics_material, density);
    m_physics_scene->addActor(*dynamic_sphere);

    PxTransform i = PxTransform::createIdentity();
    PxDistanceJoint *joint = PxDistanceJointCreate(*m_physics, static_sphere, i, dynamic_sphere, i);
    joint->setStiffness(300);
    joint->setDistanceJointFlag(PxDistanceJointFlag::eSPRING_ENABLED, true);
}

void Physics::updateJASTutorial()
{
    updateRBDTutorial();
}