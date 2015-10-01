#ifndef SOUND_PROGRAMMING_H_
#define SOUND_PROGRAMMING_H_

#include "Application.h"
#include "Camera.h"
#include "Render.h"

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

    void setupVisualDebugger();
    void renderGizmos();

    Renderer* renderer;
    
    PxFoundation* m_physics_foundation;
    PxPhysics* m_physics;
    PxScene* m_physics_scene;

    PxDefaultErrorCallback m_default_error_callback;
    PxDefaultAllocator m_default_allocator;
    PxSimulationFilterShader m_default_filter_shader;

    PxMaterial* m_physics_material;
    PxMaterial* m_box_material;
    PxCooking* m_physics_cooker;

    FlyCamera m_camera;


    Mesh* meshes;
    Material* materials;
    int material_mesh_count;

    bool mouse_down;
    bool last_mouse_down;

    //*******TUTORIAL FUNCTIONS*******

    //Rigid Body Dynamics
    void setupRBDTutorial();
    void updateRBDTutorial();

    //Collision Shape Heirarchies
    void setupCSHTutorial();
    void updateCSHTutorial();
    mat4 tank_transform;

    //Mesh Based Collision
    void setupMBCTutorial();
    void updateMBCTutorial();

    //Joints and Springs
    void setupJASTutorial();
    void updateJASTutorial();

    //Ragdoll Physics
    void setupRDPTutorial();
    void updateRDPTutorial();

    //Motors and Impulse
    void setupMAITutorial();
    void updateMAITutorial();

    //Events and Triggers
    void setupEATTutorial();
    void updateEATTutorial();

    //Fluid Dynamics and Particles
    void setupFDPTutorial();
    void updateFDPTutorial();

    //Mass Systems and Soft Bodies
    void setupMSBTutorial();
    void updateMSBTutorial();

    //Swept Collision and Ray Casting
    void setupSCRTutorial();
    void updateSCRTutorial();

};

#endif //CAM_PROJ_H_