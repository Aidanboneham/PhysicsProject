#include "Physics.h"

#include "gl_core_4_4.h"
#include "GLFW/glfw3.h"
#include "Gizmos.h"

#include "glm/gtc/quaternion.hpp"

#define Assert(val) if (val){}else{ *((char*)0) = 0;}
#define ArrayCount(val) (sizeof(val)/sizeof(val[0]))

class AIEAllocator : public PxAllocatorCallback
{
public:
    virtual ~AIEAllocator(){}

    virtual void* allocate(size_t bytes, const char* type_name, const char * filename, int line)
    {
        void* ptr = _aligned_malloc(bytes, 16);
        return ptr;
    }

    virtual void deallocate(void* ptr)
    {
        _aligned_free(ptr);
    }
};


void Physics::setupVisualDebugger()
{
    if (m_physics->getPvdConnectionManager() == 0)
    {
        return;
    }

    const char* pvd_host_ip = "127.0.0.1";
    int port = 5425;
    unsigned int timeout = 100;

    PxVisualDebuggerConnectionFlags flags = 
        PxVisualDebuggerExt::getAllConnectionFlags();

    auto connection = PxVisualDebuggerExt::createConnection(
        m_physics->getPvdConnectionManager(), pvd_host_ip, port, timeout, flags);

}

bool Physics::startup()
{
    if (Application::startup() == false)
    {
        return false;
    }

    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    Gizmos::create();

    m_camera = FlyCamera(1280.0f / 720.0f, 10.0f);
    m_camera.setLookAt(vec3(10, 10, 10), vec3(0), vec3(0, 1, 0));
    m_camera.sensitivity = 3;

    setupPhysx();
    setupTutorial1();
    setupVisualDebugger();
    return true;
}

void Physics::setupTutorial1()
{
    //add a plane to the scene
    PxTransform transform = PxTransform(PxVec3(0, 0, 0),
        PxQuat((float)PxHalfPi, PxVec3(0, 0, 1)));

    PxRigidStatic* plane = PxCreateStatic(*m_physics, transform,
                                    PxPlaneGeometry(), *m_physics_material);
    m_physics_scene->addActor(*plane);

    //add a box to the scene
    float density = 10;
    PxBoxGeometry box(0.5f, 0.5f, 0.5f);

    PxTransform box_transform(PxVec3(0, 15, 0));
    
    PxVec3 positions[4] =
    {
        PxVec3(-0.5f, 0, -0.5f),
        PxVec3(-0.5f, 0, 0.5f),
        PxVec3(0.5f, 0, 0.5f),
        PxVec3(0.5f, 0, -0.5f),
    };

    for (int i = 0; i < BOX_COUNT; ++i)
    {
        PxTransform box_transform(PxVec3(0, 0.5f + i, 0));
        m_box_actor[i] = PxCreateDynamic(*m_physics, box_transform, box,
            *m_physics_material, density);

        m_physics_scene->addActor(*m_box_actor[i]);
    }
}

void Physics::setupPhysx()
{
    m_default_filter_shader = PxDefaultSimulationFilterShader;
    PxAllocatorCallback* my_callback = new AIEAllocator();

    m_physics_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, *my_callback, 
                                                                m_default_error_callback);

    m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_physics_foundation, PxTolerancesScale());
    PxInitExtensions(*m_physics);

    m_physics_material = m_physics->createMaterial(1,1,0);


    PxSceneDesc scene_desc(m_physics->getTolerancesScale());
    scene_desc.gravity = PxVec3(0, -9.807f, 0);
    scene_desc.filterShader = &PxDefaultSimulationFilterShader;
    scene_desc.cpuDispatcher = PxDefaultCpuDispatcherCreate(8);

    m_physics_scene = m_physics->createScene(scene_desc);
}

void Physics::shutdown()
{
    m_physics_scene->release();
    m_physics->release();
    m_physics_foundation->release();

    Gizmos::destroy();
    Application::shutdown();
}

bool Physics::update()
{
    if (Application::update() == false)
    {
        return false;
    }

    Gizmos::clear();

    float dt = (float)glfwGetTime();
    glfwSetTime(0.0);

    vec4 white(1);
    vec4 black(0, 0, 0, 1);

    for (int i = 0; i <= 20; ++i)
    {
        Gizmos::addLine(vec3(-10 + i, -0.01, -10), vec3(-10 + i, -0.01, 10),
            i == 10 ? white : black);
        Gizmos::addLine(vec3(-10, -0.01, -10 + i), vec3(10, -0.01, -10 + i),
            i == 10 ? white : black);
    }
    
    m_physics_scene->simulate(dt > 0.033f ? 0.033f : dt);

    while (m_physics_scene->fetchResults() == false);

    for (int i = 0; i < BOX_COUNT ; ++i)
    {
        //get the box transform
        PxTransform box_transform = m_box_actor[i]->getGlobalPose();

        //get its position
        vec3 box_position(box_transform.p.x, box_transform.p.y, box_transform.p.z);

        //get its rotation
        glm::quat box_rotation(box_transform.q.w, 
                                box_transform.q.x,
                                box_transform.q.y, 
                                box_transform.q.z);

        //add it as a gizmo
        glm::mat4 rot(box_rotation);
        Gizmos::addAABBFilled(box_position, vec3(0.5f,0.5f,0.5f), vec4(1, 0, 0, 1), &rot);
    }
    m_camera.update(dt);

    return true;
}

void Physics::draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    Gizmos::draw(m_camera.proj, m_camera.view);

    glfwSwapBuffers(m_window);
    glfwPollEvents();
}
