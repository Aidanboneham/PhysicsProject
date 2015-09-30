#ifndef SOUND_PROGRAMMING_H_
#define SOUND_PROGRAMMING_H_

#include "Application.h"
#include "Camera.h"

#include <PxPhysicsAPI.h>
#include <PxScene.h>
#include <pvd/PxVisualDebugger.h>

using namespace physx;
const int BOX_COUNT = 256;
class Physics : public Application
{
public:
    virtual bool startup();
    virtual void shutdown();
    virtual bool update();
    virtual void draw();

    void setupPhysx();
    void setupTutorial1();
    void setupVisualDebugger();

    PxFoundation* m_physics_foundation;
    PxPhysics* m_physics;
    PxScene* m_physics_scene;

    PxDefaultErrorCallback m_default_error_callback;
    PxDefaultAllocator m_default_allocator;
    PxSimulationFilterShader m_default_filter_shader;

    PxMaterial* m_physics_material;
    PxMaterial* m_box_material;
    PxCooking* m_physics_cooker;

    PxRigidDynamic* m_box_actor[BOX_COUNT];

    FlyCamera m_camera;
};

#endif //CAM_PROJ_H_