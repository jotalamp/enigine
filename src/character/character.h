#ifndef character_hpp
#define character_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <future>
#include <atomic>
#include <chrono>

#include <glm/gtx/rotate_vector.hpp>
#include "btBulletDynamicsCommon.h"

#include "../camera/camera.h"
#include "../model/model.h"
#include "../ragdoll/ragdoll.h"
#include "../character_controller/character_controller.h"
#include "../physics_world/physics_world.h"
#include "../utils/common.h"
#include "../utils/bullet_glm.h"
#include "../shader_manager/shader_manager.h"
#include "../resource_manager/resource_manager.h"

class Character
{
public:
    ResourceManager *m_resourceManager;
    ShaderManager *m_shaderManager;
    PhysicsWorld *m_physicsWorld;
    Camera *m_followCamera;
    CharacterController *m_controller;
    Animator *m_animator;
    Ragdoll *m_ragdoll;
    btRigidBody *m_rigidbody;
    Model *m_model;

    // TODO: transformation struct
    glm::vec3 m_position = glm::vec3(200.f, 5.f, 250.f);
    glm::vec3 m_rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    float m_scale = 2.0;

    // animation
    float m_blendTargets[13];
    float m_blendTargetsPose[2];
    float m_blendSpeed = 0.2f;

    float m_leftBlendEdge = 1.45f;
    float m_rightBlendEdge = 1.2f;
    float m_leftForward = 0.35f;
    float m_rightForward = 0.05f;

    float m_aimBlend = 0.0f;
    float m_aimStateChangeSpeed = 3.f;

    float m_firingBlend = 0.0f;
    float m_firingStateChangeSpeed = 3.f;

    std::atomic_bool m_firing;

    // ragdoll
    bool m_ragdollActive = false;
    float m_impulseStrength = 600.f;
    float m_ragdolActivateTreshold = 3000.f;
    float m_ragdolActivateFactor = 0.1f;
    float m_stateChangeSpeed = 10.f;

    Character(ResourceManager *resourceManager, PhysicsWorld *physicsWorld, Camera *followCamera);
    ~Character();
    void init();
    void update(float deltaTime);
    void interpolateBlendTargets();
    void activateRagdoll(glm::vec3 impulse);
    void resetRagdoll();
    void checkPhysicsStateChange();
    AnimPose &getRagdolPose();
    AnimPose &getAimPose();
    AnimPose &getFiringPose();
    void updateAimPoseBlendMask(float blendFactor);
};

#endif /* character_hpp */
