#include "Physics.h"


void Physics::setupCSHTutorial()
{
    FBXFile file;
    file.load("./models/tank/battle_tank.fbx");

    material_mesh_count = file.getMaterialCount();
    meshes = new Mesh[material_mesh_count];
    materials = new Material[material_mesh_count];

    for (int i = 0; i < material_mesh_count; ++i)
    {
        meshes[i] = BuildStaticMeshByMaterial(&file, file.getMaterialByIndex(i)->name.c_str());
        materials[i] = GetMaterial(&file, file.getMaterialByIndex(i)->name.c_str());
    }

    file.unload();

    //add a plane to the scene
    PxTransform transform = PxTransform(PxVec3(0, 0, 0),
        PxQuat((float)PxHalfPi, PxVec3(0, 0, 1)));

    PxRigidStatic* plane = PxCreateStatic(*m_physics, transform,
        PxPlaneGeometry(), *m_physics_material);
    m_physics_scene->addActor(*plane);

    PxBoxGeometry box_main = PxBoxGeometry(0.9f, 0.3f, 1.5f);
    
    tank_transform = mat4(1);

    PxTransform px_tank_transform(*(PxMat44*)(&tank_transform[0]));
    PxRigidDynamic* tank_actor = PxCreateDynamic(*m_physics, px_tank_transform, box_main,
                                                    *m_physics_material, 2);
    tank_actor->userData = &tank_transform;

    PxShape* shape;
    tank_actor->getShapes(&shape, 1);
    PxTransform local_shape_transform = PxTransform(0, 0.35f, -0.2f);
    shape->setLocalPose(local_shape_transform);

    PxBoxGeometry box_top = PxBoxGeometry(0.6f, 0.2f, 0.7f);
    local_shape_transform = PxTransform(0, 0.9f, -0.3f);
    PxShape* top_shape = tank_actor->createShape(box_top, *m_physics_material, local_shape_transform);


    m_physics_scene->addActor(*tank_actor);

}
    
void Physics::updateCSHTutorial()
{
    updateRBDTutorial();

    PxActorTypeFlags flags = PxActorTypeFlag::eRIGID_DYNAMIC;
    int actor_count = m_physics_scene->getNbActors(flags);
    for (int i = 0; i < actor_count; ++i)
    {
        PxActor* actor;
        m_physics_scene->getActors(flags, &actor, 1, i);

        if (actor->userData)
        {
            PxRigidActor* rigid_actor = (PxRigidActor*)actor;
            PxMat44 m = rigid_actor->getGlobalPose();
            mat4 *transform = (mat4*)actor->userData;
            *transform = *(mat4*)&m;
        }
    }



    for (int i = 0; i < material_mesh_count; ++i)
    {
        renderer->PushMesh(meshes + i, materials + i, tank_transform);
    }
}