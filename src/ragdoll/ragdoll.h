#ifndef ragdoll_hpp
#define ragdoll_hpp

#include "btBulletDynamicsCommon.h"

#include "../physics_world/physics_world.h"
#include "../animation/animation.h"
#include "../animation/animator.h"
#include "../animation/bone.h"
#include "../utils/bullet_glm.h"

enum
{
    BODYPART_PELVIS = 0,
    BODYPART_SPINE,
    BODYPART_HEAD,
    BODYPART_LEFT_UPPER_LEG,
    BODYPART_LEFT_LOWER_LEG,
    BODYPART_RIGHT_UPPER_LEG,
    BODYPART_RIGHT_LOWER_LEG,
    BODYPART_LEFT_UPPER_ARM,
    BODYPART_LEFT_LOWER_ARM,
    BODYPART_RIGHT_UPPER_ARM,
    BODYPART_RIGHT_LOWER_ARM,
    BODYPART_COUNT
};

enum
{
    JOINT_PELVIS_SPINE = 0,
    JOINT_SPINE_HEAD,
    JOINT_LEFT_HIP,
    JOINT_LEFT_KNEE,
    JOINT_RIGHT_HIP,
    JOINT_RIGHT_KNEE,
    JOINT_LEFT_SHOULDER,
    JOINT_LEFT_ELBOW,
    JOINT_RIGHT_SHOULDER,
    JOINT_RIGHT_ELBOW,
    JOINT_COUNT
};

class Ragdoll
{

public:
    Ragdoll(PhysicsWorld *physicsWorld, Animation *animation, const btVector3 &positionOffset, btScalar scale);
    ~Ragdoll();

    PhysicsWorld *m_physicsWorld;
    Animation *m_animation;
    btScalar m_scale;
    btCollisionShape *m_shapes[BODYPART_COUNT];
    btRigidBody *m_bodies[BODYPART_COUNT];
    btTypedConstraint *m_joints[JOINT_COUNT];

    // debug
    btQuaternion orients[JOINT_COUNT];
    btQuaternion parentOrients[JOINT_COUNT];
    btQuaternion diffs[JOINT_COUNT];
    btQuaternion nodeOrients[JOINT_COUNT];
    btQuaternion boneOrients[JOINT_COUNT];
    btQuaternion skips[JOINT_COUNT];
    Bone *bones[JOINT_COUNT];
    btTransform boneTransforms[JOINT_COUNT];

    glm::vec3 m_modelOffset = glm::vec3(0.0f, -1.9f, 0);

    void resetTransforms(const btVector3 &positionOffset, float angleY);
    void freezeBodies();
    void unFreezeBodies();

    void syncToAnimation(glm::vec3 &position);
    void syncNodeToAnimation(AssimpNodeData node, btQuaternion skipNodeOrientation, btQuaternion parentBoneOrientation, glm::vec3 &position, bool fromParent);
};

#endif /* ragdoll_hpp */