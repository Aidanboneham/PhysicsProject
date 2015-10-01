#include "Physics.h"


void Physics::setupMBCTutorial()
{
    FBXFile file;
    file.load("./models/tank/battle_tank.fbx");

    material_mesh_count = file.getMaterialCount();
    meshes = new Mesh[material_mesh_count];
    materials = new Material[material_mesh_count];

    for (int i = 0; i < material_mesh_count; ++i)
    {
        meshes[i] = BuildStaticMeshByMaterial(&file, file.getMaterialByIndex(i)->name.c_str(), true);
        materials[i] = GetMaterial(&file, file.getMaterialByIndex(i)->name.c_str());
    }

    file.unload();

    //add a plane to the scene
    PxTransform transform = PxTransform(PxVec3(0, 0, 0),
        PxQuat((float)PxHalfPi, PxVec3(0, 0, 1)));
    PxRigidStatic* plane = PxCreateStatic(*m_physics, transform,
        PxPlaneGeometry(), *m_physics_material);
    m_physics_scene->addActor(*plane);

    int vert_count = 0;
    for (int i = 0; i < material_mesh_count; ++i)
    {
        vert_count += meshes[i].vertex_count;
    }

    PxVec3 *verts = new PxVec3[vert_count];
    int vert_index = 0;

    for (int i = 0; i < material_mesh_count; ++i)
    {
        for (int j = 0; j < meshes[i].vertex_count; ++j)
        {
            memcpy(verts + (vert_index++), &meshes[i].vertex_data[j].pos, sizeof(PxVec3));
        }
    }

    PxConvexMeshDesc convex_mesh_desc;
    convex_mesh_desc.points.count = vert_count;
    convex_mesh_desc.points.stride = sizeof(PxVec3);
    convex_mesh_desc.points.data = verts;
    convex_mesh_desc.flags = PxConvexFlag::eCOMPUTE_CONVEX;
    convex_mesh_desc.vertexLimit = 128;

    PxDefaultMemoryOutputStream buf;

    m_physics_cooker->cookConvexMesh(convex_mesh_desc, buf);

    PxU8* contents = buf.getData();
    PxU32 size = buf.getSize();

    PxDefaultMemoryInputData input(contents, size);

    PxConvexMesh* convex_mesh = m_physics->createConvexMesh(input);
    PxConvexMeshGeometry geo = PxConvexMeshGeometry(convex_mesh);


    tank_transform = mat4(1);
    PxTransform px_tank_transform(*(PxMat44*)(&tank_transform[0]));
    PxRigidDynamic* tank_actor = PxCreateDynamic(*m_physics, px_tank_transform, geo,
        *m_physics_material, 2);
    tank_actor->userData = &tank_transform;

    m_physics_scene->addActor(*tank_actor);

    delete[] verts;
}

void Physics::updateMBCTutorial()
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