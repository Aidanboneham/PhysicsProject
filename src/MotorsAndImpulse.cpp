#include "Physics.h"

void Physics::setupMAITutorial()
{
    bool make_rotation_joint = false;

    if ( make_rotation_joint) //make the rotation motor
    {
        PxMaterial* box_material = m_physics->createMaterial(1, 1, 1);
        float box_half_size = 1;
        float gap = .05f;
        float density = 1;

        //create two boxes
        PxBoxGeometry box(box_half_size, box_half_size, box_half_size);
        PxTransform box1_transform(PxVec3(0, 0, 0));
        PxTransform box2_transform(PxVec3((box_half_size + gap) * 2, 0, 0));

        //Create a PhysX static actor with a box as a collider
        PxRigidStatic* static_box = PxCreateStatic(*m_physics, box1_transform, box, *box_material);
        m_physics_scene->addActor(*static_box);

        // Create a PhysX dynamic actor with a box as a collider
        PxRigidDynamic* dynamic_box = PxCreateDynamic(*m_physics, box2_transform, box, *box_material, density);
        m_physics_scene->addActor(*dynamic_box);

        //set up the constraint frames for the joint so boxes are correctly positioned
        PxTransform constraint_frame1 = PxTransform(PxVec3(box_half_size + gap, 0, 0));
        PxTransform constraint_frame2 = PxTransform(PxVec3(-(box_half_size + gap), 0, 0));
        PxRevoluteJoint *joint = NULL;

        //create the revolute (axle) joint
        joint = PxRevoluteJointCreate(*m_physics, static_box, constraint_frame1, dynamic_box, constraint_frame2);

        //if the joint is successfully created then configure it
        if (joint) 
        {
            joint->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, true);
            joint->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, false);

            //give it some drive
            joint->setDriveVelocity(1); 
        }

        box_material = m_physics->createMaterial(1, 1, 0.2f);
    }
    else //make the linear motor instead
    {
        float density = 1;
        PxMaterial* box_material = m_physics->createMaterial(1, 1, 1);
        //create two boxes
        PxBoxGeometry box(1, 1, 1);
        PxTransform box1_transform(PxVec3(0, 0, 0));

        PxVec3 box2_target_position(10, 0, 0);
        PxTransform box2_transform(box2_target_position);

        //Static box
        PxRigidStatic* static_box = PxCreateStatic(*m_physics, box1_transform, box, *box_material);
        m_physics_scene->addActor(*static_box);

        //Dynamic box
        PxRigidDynamic* dynamic_box = PxCreateDynamic(*m_physics, box2_transform, box, *box_material, density);
        dynamic_box->setLinearDamping(0.5f);
        m_physics_scene->addActor(*dynamic_box);

        PxTransform constraint_frame1 = PxTransform(PxVec3(0, 0, 0));
        PxTransform constraint_frame2 = PxTransform(PxVec3(0, 0, 0));

        //The d6 joint
        d6joint = PxD6JointCreate(*m_physics, static_box, constraint_frame1, dynamic_box, constraint_frame2);

        //Unlock just the x axis so it behaves like a prismatic joint
        d6joint->setMotion(PxD6Axis::eX, PxD6Motion::eFREE);
        d6joint->setConstraintFlag(PxConstraintFlag::eCOLLISION_ENABLED, true);

        float drive_stiffness = 100.f;
        float drive_damping = 20.f;
        float force_limit = PX_MAX_F32;
        bool is_acceleration = true;

        PxD6JointDrive drive(drive_stiffness, drive_damping, force_limit, is_acceleration);
        d6joint->setDrive(PxD6Drive::eX, drive);

        PxVec3 linear_target_velocity(10, 0, 0);
        PxVec3 angular_target_velocity(0, 0, 0);
        d6joint->setDriveVelocity(linear_target_velocity, angular_target_velocity);
        d6joint->setDrivePosition(PxTransform(box2_target_position));

    }
}

PxVec3 target_position(10, 0, 0);
float MIN_X_POS = 2;
float MAX_X_POS = 30;

float x_speed = 10;
float current_x_speed = x_speed;

void Physics::updateMAITutorial()
{
    if (d6joint != 0) {
        d6joint->setDrivePosition(PxTransform(target_position));
    }
    if (target_position.x < MIN_X_POS) {
        current_x_speed = x_speed;
    }
    if (target_position.x > MAX_X_POS) {
        current_x_speed = -x_speed;
    }

    target_position.x += current_x_speed * delta_time;

}
