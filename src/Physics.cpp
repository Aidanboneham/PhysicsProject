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

AIEAllocator global_aie_allocator;

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
    setupVisualDebugger();

    //*********SELECT TUTORIAL TO RUN HERE*********
    setupPCTutorial();

    renderer = new Renderer();

    return true;
}

void Physics::setupPhysx()
{
    m_default_filter_shader = PxDefaultSimulationFilterShader;
    PxAllocatorCallback* my_callback = &global_aie_allocator;

    m_physics_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, *my_callback, 
                                                                m_default_error_callback);

    m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_physics_foundation, PxTolerancesScale());
    PxInitExtensions(*m_physics);

    m_physics_material = m_physics->createMaterial(1,1,0);

    m_physics_cooker = PxCreateCooking(PX_PHYSICS_VERSION, *m_physics_foundation, PxCookingParams(PxTolerancesScale()));

    PxSceneDesc scene_desc(m_physics->getTolerancesScale());
    scene_desc.gravity = PxVec3(0, -9.807f, 0);
    scene_desc.filterShader = &PxDefaultSimulationFilterShader;
    scene_desc.cpuDispatcher = PxDefaultCpuDispatcherCreate(8);

    m_physics_scene = m_physics->createScene(scene_desc);
}

void Physics::shutdown()
{
    m_physics_scene->release();
    m_physics_foundation->release();
    m_physics->release();

    Gizmos::destroy();
    Application::shutdown();
}

bool Physics::update()
{
    mouse_down = glfwGetMouseButton(m_window, 0) != 0;

    if (Application::update() == false)
    {
        return false;
    }

    Gizmos::clear();

    float dt = (float)glfwGetTime();
    delta_time = dt;
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
    renderGizmos();  
    m_camera.update(dt);


    //*********SELECT TUTORIAL TO RUN HERE*********
    updatePCTutorial();


    last_mouse_down = mouse_down;
    return true;
}

void Physics::draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    Gizmos::draw(m_camera.proj, m_camera.view);

    renderer->RenderAndClear(m_camera.view_proj);

    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void AddWidget(PxShape* shape, PxRigidActor* actor, vec4 geo_color)
{
    PxTransform full_transform = PxShapeExt::getGlobalPose(*shape, *actor);
    vec3 actor_position(full_transform.p.x, full_transform.p.y, full_transform.p.z);
    glm::quat actor_rotation(full_transform.q.w,
        full_transform.q.x,
        full_transform.q.y,
        full_transform.q.z);
    glm::mat4 rot(actor_rotation);

    PxGeometryType::Enum geo_type = shape->getGeometryType();

    switch (geo_type)
    {
    case (PxGeometryType::eBOX) :
    {
        PxBoxGeometry geo;
        shape->getBoxGeometry(geo);
        vec3 extents(geo.halfExtents.x, geo.halfExtents.y, geo.halfExtents.z);
        Gizmos::addAABBFilled(actor_position, extents, geo_color, &rot);
    } break;
    case (PxGeometryType::eCAPSULE) :
    {
        PxCapsuleGeometry geo;
        shape->getCapsuleGeometry(geo);
        Gizmos::addCapsule(actor_position, geo.halfHeight * 2, geo.radius, 16, 16, geo_color, &rot);
    } break;
    case (PxGeometryType::eSPHERE) :
    {
        PxSphereGeometry geo;
        shape->getSphereGeometry(geo);
        Gizmos::addSphereFilled(actor_position, geo.radius, 16, 16, geo_color, &rot);
    } break;
    case (PxGeometryType::ePLANE) :
    {

    } break;
    }

}

void Physics::renderGizmos()
{
    PxActorTypeFlags desiredTypes = PxActorTypeFlag::eRIGID_STATIC | PxActorTypeFlag::eRIGID_DYNAMIC;
    PxU32 actor_count = m_physics_scene->getNbActors(desiredTypes);
    PxActor** actor_list = new PxActor*[actor_count];
    m_physics_scene->getActors(desiredTypes, actor_list, actor_count);
    
    vec4 geo_color(1, 0, 0, 1);
    for (int actor_index = 0;
        actor_index < (int)actor_count;
        ++actor_index)
    {
        PxActor* curr_actor = actor_list[actor_index];
        if (curr_actor->isRigidActor())
        {
            PxRigidActor* rigid_actor = (PxRigidActor*)curr_actor;
            PxU32 shape_count = rigid_actor->getNbShapes();
            PxShape** shapes = new PxShape*[shape_count];
            rigid_actor->getShapes(shapes, shape_count);

            for (int shape_index = 0;
                shape_index < (int)shape_count;
                ++shape_index)
            {
                PxShape* curr_shape = shapes[shape_index];
                AddWidget(curr_shape, rigid_actor, geo_color);
            }

            delete[]shapes;
        }
    }

    delete[] actor_list;

    int articulation_count = m_physics_scene->getNbArticulations();

    for (int a = 0; a < articulation_count; ++a)
    {
        PxArticulation* articulation;
        m_physics_scene->getArticulations(&articulation, 1, a);

        int link_count = articulation->getNbLinks();

        PxArticulationLink** links = new PxArticulationLink*[link_count];
        articulation->getLinks(links, link_count);

        for (int l = 0; l < link_count; ++l)
        {
            PxArticulationLink* link = links[l];
            int shape_count = link->getNbShapes();

            for (int s = 0; s < shape_count; ++s)
            {
                PxShape* shape;
                link->getShapes(&shape, 1, s);
                AddWidget(shape, link, geo_color);
            }
        }
        delete[] links;
    }
}

