#include "Physics.h"


void Physics::setupRBDTutorial()
{
    //add a plane to the scene
    PxTransform transform = PxTransform(PxVec3(0, 0, 0),
        PxQuat((float)PxHalfPi, PxVec3(0, 0, 1)));

    PxRigidStatic* plane = PxCreateStatic(*m_physics, transform,
        PxPlaneGeometry(), *m_physics_material);
    m_physics_scene->addActor(*plane);
}

void Physics::updateRBDTutorial()
{
    if (mouse_down && !last_mouse_down)
    {
        //add a box to the scene
        float density = 10;
        PxBoxGeometry box(0.5f, 0.5f, 0.5f);

        vec3 cam_pos = m_camera.world[3].xyz();
        vec3 box_vel = -m_camera.world[2].xyz() * 20.f;

        PxQuat q(0, 0, 0, 1);
        PxTransform box_transform(PxVec3(cam_pos.x, cam_pos.y, cam_pos.z), q);

        PxVec3 positions[4] =
        {
            PxVec3(-0.5f, 0, -0.5f),
            PxVec3(-0.5f, 0, 0.5f),
            PxVec3(0.5f, 0, 0.5f),
            PxVec3(0.5f, 0, -0.5f),
        };

        PxRigidDynamic* new_box = PxCreateDynamic(*m_physics,
            box_transform,
            box,
            *m_physics_material,
            density);
        PxVec3 velocity(box_vel.x, box_vel.y, box_vel.z);
        new_box->setLinearVelocity(velocity, true);
        m_physics_scene->addActor(*new_box);

    }
}