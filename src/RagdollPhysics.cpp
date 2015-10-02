#include "Physics.h"

enum RagDollParts
{
    NO_PARENT = -1,
    LOWER_SPINE,
    LEFT_PELVIS,
    RIGHT_PELVIS,
    LEFT_UPPER_LEG,
    RIGHT_UPPER_LEG,
    LEFT_LOWER_LEG,
    RIGHT_LOWER_LEG,
    UPPER_SPINE,
    LEFT_CLAVICLE,
    RIGHT_CLAVICLE,
    NECK,
    HEAD,
    LEFT_UPPER_ARM,
    RIGHT_UPPER_ARM,
    LEFT_LOWER_ARM,
    RIGHT_LOWER_ARM,
};

struct RagdollNode
{
    PxQuat global_rotation;
    PxVec3 scaled_global_pos;

    int parent_node_idx;
    float half_len;
    float radius;
    float parent_link_pos;
    PxArticulationLink* link;

    float child_link_pos;

    char* name;

    RagdollNode(PxQuat _global_rotation, int _parent_node_idx, float _half_len, 
            float _radius, float _parent_link_pos, float _child_link_pos, char* _name)
    {
        global_rotation = _global_rotation;
        parent_node_idx = _parent_node_idx;
        half_len = _half_len;
        radius = _radius;
        parent_link_pos = _parent_link_pos;
        child_link_pos = _child_link_pos;
        name = _name;
    }
};

PxVec3 X_AXIS(1,0,0);
PxVec3 Y_AXIS(0,1,0);
PxVec3 Z_AXIS(0,0,1);

RagdollNode* ragdoll_data[] = 
{
    new RagdollNode(PxQuat(PxPi/2.f, Z_AXIS), NO_PARENT, 1,3,1,1,"lower_spine"),
    new RagdollNode(PxQuat(PxPi, Z_AXIS), LOWER_SPINE, 1,1,-1,1,"left_pelvis"),
    new RagdollNode(PxQuat(0, Z_AXIS), LOWER_SPINE, 1,1,-1,1,"right_pelvis"),
    new RagdollNode(PxQuat(PxPi/2.f+0.2f, Z_AXIS), LEFT_PELVIS, 5,2,-1,1,"l_upper_leg"),
    new RagdollNode(PxQuat(PxPi/2.f-0.2f, Z_AXIS), LEFT_PELVIS, 5,2,-1,1,"r_upper_leg"),
    new RagdollNode(PxQuat(PxPi/2.f+0.2f, Z_AXIS), LEFT_UPPER_LEG, 5,1.75f,-1,1,"l_lower_leg"),
    new RagdollNode(PxQuat(PxPi/2.f-0.2f, Z_AXIS), RIGHT_UPPER_LEG, 5,1.75f,-1,1,"r_lower_leg"),
    new RagdollNode(PxQuat(PxPi/2.f, Z_AXIS), LOWER_SPINE, 1,3,1,-1,"upper_spine"),
    new RagdollNode(PxQuat(PxPi, Z_AXIS), UPPER_SPINE, 1,1.5f,1,1,"left_clavicle"),
    new RagdollNode(PxQuat(0, Z_AXIS), UPPER_SPINE, 1, 1.5f, 1, 1, "right_clavicle"),
    new RagdollNode(PxQuat(PxPi/2.0f, Z_AXIS), UPPER_SPINE, 1, 1, 1, -1, "neck"),
    new RagdollNode(PxQuat(PxPi/2.0f, Z_AXIS), NECK, 1, 3, 1, -1, "head"),
    new RagdollNode(PxQuat(PxPi-.3f, Z_AXIS), LEFT_CLAVICLE, 3, 1.5f, -1, 1, "left_upper_arm"),
    new RagdollNode(PxQuat(0.3f, Z_AXIS), RIGHT_CLAVICLE, 3, 1.5f, -1, 1, "right_upper_arm"),
    new RagdollNode(PxQuat(PxPi-.3f, Z_AXIS), LEFT_UPPER_ARM, 3, 1, -1, 1, "left_lower_arm"),
    new RagdollNode(PxQuat(0.3f, Z_AXIS), RIGHT_UPPER_ARM, 3, 1, -1, 1, "right_lower_arm"),
    0
};


PxArticulation* MakeRagdoll(PxPhysics* physics, RagdollNode** node_array, 
                            PxTransform world_pos, float scale_factor, PxMaterial* ragdoll_mat)
{
    PxArticulation* result = physics->createArticulation();
    RagdollNode** current_node = node_array;

    while( *current_node != 0 )
    {
        RagdollNode *node = *current_node;
        RagdollNode* parent = 0;

        float radius = node->radius * scale_factor;
        float half_len = node->half_len * scale_factor;
        float child_half_len = radius + half_len;
        float parent_half_len = 0;

        PxArticulationLink* parent_link = 0;
        node->scaled_global_pos = world_pos.p;

        if ( node->parent_node_idx != -1 )
        {
            parent = *(node_array + node->parent_node_idx);
            parent_link = parent->link;
            parent_half_len = (parent->radius + parent->half_len) * scale_factor;
            
            PxVec3 node_relative = 
                node->child_link_pos * node->global_rotation.rotate(PxVec3(child_half_len,0,0));

            PxVec3 parent_relative =
                -node->parent_link_pos * parent->global_rotation.rotate(PxVec3(parent_half_len,0,0));

            node->scaled_global_pos = parent->scaled_global_pos - (parent_relative + node_relative);

        }

        PxTransform link_transform = PxTransform(node->scaled_global_pos, node->global_rotation);

        PxArticulationLink* link = result->createLink(parent_link, link_transform);
        node->link = link;

        float joint_spacing = 0.01f;
        float capsule_half_len = (half_len > joint_spacing ? half_len - joint_spacing : 0) + 0.01f;

        PxCapsuleGeometry capsule(radius, capsule_half_len);
        link->createShape(capsule, *ragdoll_mat);

        PxRigidBodyExt::updateMassAndInertia(*link, 50.0f);

        if ( node->parent_node_idx != -1 )
        {
            PxArticulationJoint* joint = link->getInboundJoint();
            PxQuat frame_roatation = parent->global_rotation.getConjugate() * node->global_rotation;

            PxTransform parent_constraint_frame = 
                PxTransform(PxVec3(node->parent_link_pos * parent_half_len,0,0), frame_roatation);

            PxTransform node_constraint_frame = PxTransform(node->child_link_pos * child_half_len,0,0);

            joint->setParentPose(parent_constraint_frame);
            joint->setChildPose(node_constraint_frame);

            joint->setStiffness(20);
            joint->setDamping(20);
            joint->setSwingLimit(0.4f,0.4f);
            joint->setSwingLimitEnabled(true);
            joint->setTwistLimit(-0.1f,0.1f);
            joint->setTwistLimitEnabled(true);
        }

        current_node++;
    }


    return result;
}


void Physics::setupRDPTutorial()
{
    //add a plane to the scene
    PxTransform transform = PxTransform(PxVec3(0, 0, 0),
        PxQuat((float)PxHalfPi, PxVec3(0, 0, 1)));

    PxRigidStatic* plane = PxCreateStatic(*m_physics, transform,
        PxPlaneGeometry(), *m_physics_material);
    m_physics_scene->addActor(*plane);

    PxArticulation* ragdoll_articulation =
        MakeRagdoll(m_physics, ragdoll_data, PxTransform(0, 20, 0), 0.1f, m_physics_material);
    m_physics_scene->addArticulation(*ragdoll_articulation);

}

void Physics::updateRDPTutorial()
{
}


