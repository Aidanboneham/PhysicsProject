#include "Physics.h"
#include "GLFW/glfw3.h"


class MyControllerHitReport : public PxUserControllerHitReport
{
public:
    virtual void onShapeHit(const PxControllerShapeHit &hit)
    {
        PxRigidActor* actor = hit.shape->getActor();

        player_contact_normal = hit.worldNormal;

        PxRigidDynamic* dynamic_actor = actor->is<PxRigidDynamic>();

        if (dynamic_actor)
        {
            //we can apply force to actors we hit here
        }
    }

    virtual void onControllerHit(const PxControllersHit& hit){}
    virtual void onObstacleHit(const PxControllerObstacleHit& hit){}
    MyControllerHitReport() : PxUserControllerHitReport() {};

    PxVec3 player_contact_normal;
};


static MyControllerHitReport* my_hit_report = 0;
static PxController* player_controller = 0;

void Physics::setupPCTutorial()
{
    //add a plane to the scene
    PxTransform transform = PxTransform(PxVec3(0, 0, 0),
        PxQuat((float)PxHalfPi, PxVec3(0, 0, 1)));

    PxRigidStatic* plane = PxCreateStatic(*m_physics, transform,
        PxPlaneGeometry(), *m_physics_material);
    m_physics_scene->addActor(*plane);


    my_hit_report = new MyControllerHitReport();
    m_controller_manager = PxCreateControllerManager(*m_physics_scene);

    PxCapsuleControllerDesc desc;
    desc.height = 1.6f;
    desc.radius = 0.4f;
    desc.position.set(0, 0, 0);
    desc.material = m_physics_material;
    desc.reportCallback = my_hit_report;
    desc.density = 10;

    player_controller = m_controller_manager->createController(desc);
    player_controller->setPosition(desc.position);
    my_hit_report->player_contact_normal = PxVec3(0, 0, 0);
}

void Physics::updatePCTutorial()
{
    bool on_ground; //set to true if we are on the ground
    float movement_speed = 10.0f; //forward and back movement speed
    float rotation_speed = 1.0f; //turn speed
    float gravity = -10;

    static float y_vel = 0;
    static float character_rotation = 0;

    //check if we have a contact normal. if y is greater than .3 we assume this is 
    //solid ground. This is a rather primitive way to do this. Can you do better?
    if (my_hit_report->player_contact_normal.y > 0.3f)
    {
        y_vel = -0.1f;
        on_ground = true;
    }
    else
    {
        y_vel += gravity * delta_time;
        on_ground = false;
    }

    my_hit_report->player_contact_normal = PxVec3(0,0,0);
    const PxVec3 up(0, 1, 0);

    //scan the keys and set up our intended velocity based on a global transform
    PxVec3 velocity(0, y_vel, 0);
    if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        velocity.x -= movement_speed * delta_time;
    }
    if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        velocity.x += movement_speed * delta_time;
    }

    //To do.. add code to control z movement and jumping
    float min_distance = 0.001f;
    PxControllerFilters filter;

    //make controls relative to player facing
    PxQuat rotation(character_rotation, PxVec3(0, 1, 0));

    //move the controller
    player_controller->move(rotation.rotate(velocity), min_distance, delta_time, filter);

}



