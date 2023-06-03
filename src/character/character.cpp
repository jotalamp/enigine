#include "character.h"

Character::Character(ShaderManager *shaderManager, PhysicsWorld *physicsWorld, Camera *followCamera)
    : m_shaderManager(shaderManager),
      m_physicsWorld(physicsWorld),
      m_followCamera(followCamera),
      m_firing(false)
{
    init();
}

void Character::init()
{
    // Animation
    m_model = new Model("../src/assets/gltf/char5.glb");
    Animation *animation0 = new Animation("idle", m_model);
    Animation *animation1 = new Animation("walking-forward", m_model);
    Animation *animation2 = new Animation("left", m_model);
    Animation *animation3 = new Animation("right", m_model);
    Animation *animation4 = new Animation("running-forward", m_model);
    Animation *animation5 = new Animation("walking-left", m_model);
    Animation *animation6 = new Animation("walking-right", m_model);
    Animation *animation7 = new Animation("walking-back", m_model);
    Animation *animation8 = new Animation("running-left", m_model);
    Animation *animation9 = new Animation("running-right", m_model);
    Animation *animation10 = new Animation("running-back", m_model);
    Animation *animation11 = new Animation("walking-back-left", m_model);
    Animation *animation12 = new Animation("walking-back-right", m_model);
    Animation *animation13 = new Animation("running-back-left", m_model);
    Animation *animation14 = new Animation("running-back-right", m_model);
    Animation *animation15 = new Animation("pistol-aim-1", m_model);
    // TODO: create empty at runtime?
    Animation *animationRagdoll = new Animation("pose", m_model);
    Animation *animation17 = new Animation("firing", m_model);

    // TODO: inside Model
    std::vector<Animation *> animations;
    animations.push_back(animation0);
    animations.push_back(animation1);
    animations.push_back(animation2);
    animations.push_back(animation3);
    animations.push_back(animation4);
    animations.push_back(animation5);
    animations.push_back(animation6);
    animations.push_back(animation7);
    animations.push_back(animation8);
    animations.push_back(animation9);
    animations.push_back(animation10);
    animations.push_back(animation11);
    animations.push_back(animation12);
    animations.push_back(animation13);
    animations.push_back(animation14);
    animations.push_back(animation15);
    animations.push_back(animationRagdoll);
    animations.push_back(animation17);

    // TODO: setup multiple animators from same Model
    m_animator = new Animator(animations);

    // state
    // TODO: increase accessibility - name or pointer to Animation
    Anim anim;
    anim.index = 0; // idle
    anim.blendFactor = 1.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 1; // forward
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 7; // backward
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 11; // backward-left
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 12; // backward-right
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 5; // left
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 6; // right
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 4; // run-forward
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 10; // run-back
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 8; // run-left
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 9; // run-right
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 13; // run-back-left
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 14; // run-back-right
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);

    // set blend mask for turn-right and turn-left
    std::unordered_map<std::string, float> blendMask;
    blendMask["mixamorig:Spine"] = 1.0f;
    blendMask["mixamorig:Spine1"] = 1.0f;
    blendMask["mixamorig:Spine2"] = 1.0f;
    blendMask["mixamorig:Neck"] = 1.0f;
    blendMask["mixamorig:Head"] = 1.0f;

    animation2->setBlendMask(blendMask, 0.f);
    animation3->setBlendMask(blendMask, 0.f);
    // TODO: ragdoll mask for ragdoll hands - default position?

    // TODO: fix animation - no arms
    blendMask.clear();
    blendMask["mixamorig:Hips"] = 1.0f;
    blendMask["mixamorig:Spine"] = 1.0f;
    blendMask["mixamorig:Spine1"] = 1.0f;
    blendMask["mixamorig:Spine2"] = 1.0f;
    blendMask["mixamorig:Neck"] = 1.0f;
    blendMask["mixamorig:Head"] = 1.0f;
    blendMask["mixamorig:LeftShoulder"] = 1.0f;
    blendMask["mixamorig:RightShoulder"] = 1.0f;
    blendMask["mixamorig:RightUpLeg"] = 1.0f;
    blendMask["mixamorig:RightLeg"] = 1.0f;
    blendMask["mixamorig:RightFoot"] = 1.0f;
    blendMask["mixamorig:RightToeBase"] = 1.0f;
    blendMask["mixamorig:LeftUpLeg"] = 1.0f;
    blendMask["mixamorig:LeftLeg"] = 1.0f;
    blendMask["mixamorig:LeftFoot"] = 1.0f;
    blendMask["mixamorig:LeftToeBase"] = 1.0f;

    animation11->setBlendMask(blendMask, 0.f);
    animation12->setBlendMask(blendMask, 0.f);
    animation13->setBlendMask(blendMask, 0.f);
    animation14->setBlendMask(blendMask, 0.f);

    // firing
    blendMask.clear();
    blendMask["mixamorig:Neck"] = 1.0f;
    blendMask["mixamorig:Spine2"] = 1.0f;
    blendMask["mixamorig:RightShoulder"] = 1.0f;
    blendMask["mixamorig:RightArm"] = 1.0f;
    blendMask["mixamorig:RightForeArm"] = 1.0f;
    blendMask["mixamorig:RightHand"] = 1.0f;

    animation17->setBlendMask(blendMask, 0.f);

    // turn-left pose
    AnimPose animPose;
    animPose.index = 2;
    animPose.blendFactor = 0.0f;
    m_animator->m_state.poses.push_back(animPose);
    // turn-right pose
    animPose.index = 3;
    m_animator->m_state.poses.push_back(animPose);
    // pistol-aim
    animPose.index = 15;
    animPose.blendFactor = 1.0f;
    m_animator->m_state.poses.push_back(animPose);
    // ragdoll
    animPose.index = 16;
    animPose.blendFactor = 0.0f;
    m_animator->m_state.poses.push_back(animPose);
    // firing
    animPose.index = 17;
    animPose.blendFactor = 0.0f;
    m_animator->m_state.poses.push_back(animPose);

    // TODO:
    for (int i = 0; i < 13; i++)
        m_blendTargets[i] = 0.0f;

    // Character
    m_rigidbody = m_physicsWorld->createCapsule(10.0f, 1.0f, 0.5f, 2.0f, BulletGLM::getBulletVec3(m_position));
    m_rigidbody->setAngularFactor(btVector3(0.0f, 0.0f, 0.0f));
    m_rigidbody->setDamping(0.9f, 0.9f);
    m_rigidbody->setFriction(0.0f);
    m_rigidbody->setGravity(btVector3(0, -20.0f, 0));

    m_controller = new CharacterController(m_physicsWorld->dynamicsWorld, m_rigidbody, m_followCamera);

    // TODO: make independent or manageable - fix
    // init shaders after model read && before ragdoll ???? :D
    m_shaderManager->initShaders();

    m_ragdoll = new Ragdoll(m_physicsWorld, animationRagdoll, BulletGLM::getBulletVec3(m_position), 2.0f);
}

Character::~Character()
{
    delete m_controller;
    for (int i = 0; i < m_animator->m_animations.size(); i++)
        delete m_animator->m_animations[i];
    m_animator->m_animations.clear();
    delete m_animator;
    delete m_ragdoll;
    delete m_model;
}

void Character::update(float deltaTime)
{
    // update character
    if (!m_ragdollActive)
        m_controller->update(deltaTime);

    // update ragdoll
    if (m_ragdollActive)
        m_ragdoll->syncToAnimation(m_position);

    // update animation
    m_animator->update(deltaTime);

    // update ragdoll blend
    AnimPose &ragdolPose = getRagdolPose();
    ragdolPose.blendFactor += deltaTime * m_stateChangeSpeed * (m_ragdollActive ? 1.f : -1.f);
    ragdolPose.blendFactor = std::max(0.0f, std::min(ragdolPose.blendFactor, 1.0f));

    // update pistol-aim blend
    m_aimBlend += deltaTime * m_aimStateChangeSpeed * (m_controller->m_aimLocked ? 1.f : -1.f);
    m_aimBlend = std::max(0.0f, std::min(m_aimBlend, 1.0f));
    updateAimPoseBlendMask(m_aimBlend);

    // update firing blend
    AnimPose &firingPose = getFiringPose();
    firingPose.blendFactor += deltaTime * m_firingStateChangeSpeed * (m_firing ? 1.f : -1.f);
    firingPose.blendFactor = std::max(0.0f, std::min(firingPose.blendFactor, 1.0f));

    // sync animator with controller
    if (m_rigidbody && m_rigidbody->getMotionState() && !m_ragdollActive)
    {
        btTransform trans;
        m_rigidbody->getMotionState()->getWorldTransform(trans);
        m_position = glm::vec3(float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
        m_position -= glm::vec3(0, m_controller->m_halfHeight, 0);
        m_rotation.y = glm::atan(m_controller->m_lookDir.x, m_controller->m_lookDir.z);

        float front = 0.f;
        float back = 0.f;
        float right = 0.f;
        float left = 0.f;
        float backRight = 0.f;
        float backLeft = 0.f;
        float moveAngle = m_controller->m_signedMoveAngle;

        if (moveAngle >= 0.f && moveAngle < M_PI_2) // 1
        {
            front = (M_PI_2 - moveAngle) / M_PI_2;
            right = moveAngle / M_PI_2;
        }
        else if (moveAngle >= M_PI_2 && moveAngle < M_PI_2 + M_PI_2 / 2.f) // 2.1
        {
            float angle = moveAngle - M_PI_2;
            backRight = angle / (M_PI_2 / 2.f);
            right = 1.f - backRight;
        }
        else if (moveAngle >= M_PI_2 + M_PI_2 / 2.f && moveAngle <= M_PI) // 2.2
        {
            float angle = moveAngle - (M_PI_2 + M_PI_2 / 2.f);
            back = angle / (M_PI_2 / 2.f);
            backRight = 1.f - back;
        }
        else if (moveAngle >= -M_PI && moveAngle < -(M_PI_2 + M_PI_2 / 2.f)) // 3.1
        {
            float angle = -moveAngle - (M_PI_2 + M_PI_2 / 2.f);
            back = angle / (M_PI_2 / 2.f);
            backLeft = 1.f - back;
        }
        else if (moveAngle >= -(M_PI_2 + M_PI_2 / 2.f) && moveAngle < -M_PI_2) // 3.2
        {
            float angle = -moveAngle - M_PI_2;
            backLeft = angle / (M_PI_2 / 2.f);
            left = 1.f - backLeft;
        }
        else if (moveAngle >= -M_PI_2 && moveAngle < 0.f) // 4
        {
            front = (M_PI_2 + moveAngle) / M_PI_2;
            left = -moveAngle / M_PI_2;
        }

        m_blendTargets[0] = 0.f; // idle
        m_blendTargets[1] = front;
        m_blendTargets[2] = back;
        m_blendTargets[3] = backLeft;
        m_blendTargets[4] = backRight;
        m_blendTargets[5] = left;
        m_blendTargets[6] = right;
        m_blendTargets[7] = 0.f;  // run-forward
        m_blendTargets[8] = 0.f;  // run-back
        m_blendTargets[9] = 0.f;  // run-left
        m_blendTargets[10] = 0.f; // run-right
        m_blendTargets[11] = 0.f; // run-back-left
        m_blendTargets[12] = 0.f; // run-back-right

        float maxWalkSpeed = m_controller->m_maxWalkRelative + m_controller->m_walkToRunAnimTreshold;
        if (m_controller->m_verticalSpeed > maxWalkSpeed)
        {
            float runnningGap = m_controller->m_maxRunRelative - maxWalkSpeed;
            float runningLevel = m_controller->m_verticalSpeed - maxWalkSpeed;
            float clamped = CommonUtil::snappedClamp(runningLevel / runnningGap, 0.0f, 1.0f, 0.08f);

            m_blendTargets[1] *= (1.f - clamped);
            m_blendTargets[2] *= (1.f - clamped);
            m_blendTargets[3] *= (1.f - clamped);
            m_blendTargets[4] *= (1.f - clamped);
            m_blendTargets[5] *= (1.f - clamped);
            m_blendTargets[6] *= (1.f - clamped);
            m_blendTargets[7] = front * clamped;
            m_blendTargets[8] = back * clamped;
            m_blendTargets[9] = left * clamped;
            m_blendTargets[10] = right * clamped;
            m_blendTargets[11] = backLeft * clamped;
            m_blendTargets[12] = backRight * clamped;
        }
        else
        {
            float clamped = CommonUtil::snappedClamp(m_controller->m_verticalSpeed / m_controller->m_maxWalkRelative, 0.0f, 1.0f, 0.0f);

            m_blendTargets[0] = (1.f - clamped);
            m_blendTargets[1] *= clamped;
            m_blendTargets[2] *= clamped;
            m_blendTargets[3] *= clamped;
            m_blendTargets[4] *= clamped;
            m_blendTargets[5] *= clamped;
            m_blendTargets[6] *= clamped;
        }

        // TODO: fix - wrong blend while locked
        // full body rotating
        if (m_controller->m_dotFront <= 0.f && m_controller->m_turnFactor > 0)
        {
            float turn = m_controller->m_turnFactor / m_controller->m_turnAnimMaxFactor;
            m_blendTargets[4] = turn * m_controller->m_turnAnimMult;

            m_blendTargets[3] *= (1.f - turn);
            m_blendTargets[1] *= (1.f - turn);
            m_blendTargets[2] *= (1.f - turn);
            m_blendTargets[5] *= (1.f - turn);
            m_blendTargets[6] *= (1.f - turn);
        }
        else if (m_controller->m_dotFront <= 0.f && m_controller->m_turnFactor < 0)
        {
            float turn = -m_controller->m_turnFactor / m_controller->m_turnAnimMaxFactor;
            m_blendTargets[3] = turn * m_controller->m_turnAnimMult;

            m_blendTargets[4] *= (1.f - turn);
            m_blendTargets[1] *= (1.f - turn);
            m_blendTargets[2] *= (1.f - turn);
            m_blendTargets[5] *= (1.f - turn);
            m_blendTargets[6] *= (1.f - turn);
        }

        float &animL = m_blendTargetsPose[0];
        float &animR = m_blendTargetsPose[1];
        animL = std::max(0.0f, std::min(-m_controller->m_turnFactor, 1.0f));
        animR = std::max(0.0f, std::min(m_controller->m_turnFactor, 1.0f));

        // adaptive turn for pistol-aim
        if (m_controller->m_aimLocked)
        {
            float &leftBlend = m_animator->m_state.animations[5].blendFactor;
            float &rightBlend = m_animator->m_state.animations[6].blendFactor;

            float leftB = leftBlend + m_leftForward;
            float rightB = rightBlend + m_rightForward;

            if (leftB > 0.f)
                animR = std::max(0.0f, std::min(leftB / m_leftBlendEdge, 1.0f));
            if (rightB > 0.f)
                animL = std::max(0.0f, std::min(rightB / m_rightBlendEdge, 1.0f));
        }

        interpolateBlendTargets();
    }
}

void Character::interpolateBlendTargets()
{
    for (int i = 0; i < 13; i++)
        m_animator->m_state.animations[i].blendFactor = CommonUtil::lerp(m_animator->m_state.animations[i].blendFactor, m_blendTargets[i], m_blendSpeed);
    for (int i = 0; i < 2; i++)
        m_animator->m_state.poses[i].blendFactor = CommonUtil::lerp(m_animator->m_state.poses[i].blendFactor, m_blendTargetsPose[i], m_blendSpeed);
}

void Character::activateRagdoll(glm::vec3 impulseDirection, float impulseStrength)
{
    m_rigidbody->setLinearFactor(btVector3(0.0f, 0.0f, 0.0f));
    m_rigidbody->setActivationState(0);
    m_rigidbody->setCollisionFlags(m_rigidbody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    btVector3 modelPos = BulletGLM::getBulletVec3(m_position);
    m_ragdoll->resetTransforms(modelPos, m_rotation.y);
    m_ragdoll->unFreezeBodies();
    m_ragdollActive = true;

    btRigidBody *pelvis = m_ragdoll->m_bodies[BODYPART_PELVIS];
    btRigidBody *spine = m_ragdoll->m_bodies[BODYPART_SPINE];

    pelvis->applyCentralImpulse(BulletGLM::getBulletVec3(impulseDirection * impulseStrength * 0.4f));
    spine->applyCentralImpulse(BulletGLM::getBulletVec3(impulseDirection * impulseStrength * 0.6f));
}

void Character::resetRagdoll()
{
    btVector3 modelPos = BulletGLM::getBulletVec3(m_position + glm::vec3(0.f, 10.f, 0.f));
    m_ragdoll->resetTransforms(modelPos, m_rotation.y);
    m_ragdoll->freezeBodies();
    m_rigidbody->setLinearFactor(btVector3(1.0f, 1.0f, 1.0f));
    m_rigidbody->setActivationState(1);
    m_rigidbody->setCollisionFlags(m_rigidbody->getCollisionFlags() & ~btCollisionObject::CF_NO_CONTACT_RESPONSE);
    m_ragdollActive = false;
}

AnimPose &Character::getRagdolPose()
{
    return m_animator->m_state.poses[3];
}

AnimPose &Character::getAimPose()
{
    return m_animator->m_state.poses[2];
}

AnimPose &Character::getFiringPose()
{
    return m_animator->m_state.poses[4];
}

// TODO: single run in a thread
void Character::updateAimPoseBlendMask(float blendFactor)
{
    // early exit
    Bone *bone = m_animator->m_animations[15]->getBone("mixamorig:Head");
    if (bone && bone->m_blendFactor == blendFactor)
        return;

    std::unordered_map<std::string, float> blendMask;

    glm::vec2 p0(0, 0);
    glm::vec2 p1(0.5, 1);
    glm::vec2 p2(0.15, 1);
    glm::vec2 p3(1, 1);
    float cubicBlend = CommonUtil::cubicBezier(p0, p1, p2, p3, blendFactor).y;

    blendMask["mixamorig:Head"] = cubicBlend;
    blendMask["mixamorig:Neck"] = cubicBlend;
    blendMask["mixamorig:Spine2"] = cubicBlend;
    blendMask["mixamorig:RightShoulder"] = cubicBlend;
    blendMask["mixamorig:RightArm"] = cubicBlend;
    blendMask["mixamorig:RightForeArm"] = cubicBlend;
    blendMask["mixamorig:RightHand"] = cubicBlend;

    // always on
    blendMask["mixamorig:RightHandThumb1"] = 1.0f;
    blendMask["mixamorig:RightHandThumb2"] = 1.0f;
    blendMask["mixamorig:RightHandThumb3"] = 1.0f;
    blendMask["mixamorig:RightHandThumb4"] = 1.0f;
    blendMask["mixamorig:RightHandIndex1"] = 1.0f;
    blendMask["mixamorig:RightHandIndex2"] = 1.0f;
    blendMask["mixamorig:RightHandIndex3"] = 1.0f;
    blendMask["mixamorig:RightHandIndex4"] = 1.0f;
    blendMask["mixamorig:RightHandMiddle1"] = 1.0f;
    blendMask["mixamorig:RightHandMiddle2"] = 1.0f;
    blendMask["mixamorig:RightHandMiddle3"] = 1.0f;
    blendMask["mixamorig:RightHandMiddle4"] = 1.0f;
    blendMask["mixamorig:RightHandRing1"] = 1.0f;
    blendMask["mixamorig:RightHandRing2"] = 1.0f;
    blendMask["mixamorig:RightHandRing3"] = 1.0f;
    blendMask["mixamorig:RightHandRing4"] = 1.0f;
    blendMask["mixamorig:RightHandPinky1"] = 1.0f;
    blendMask["mixamorig:RightHandPinky2"] = 1.0f;
    blendMask["mixamorig:RightHandPinky3"] = 1.0f;
    blendMask["mixamorig:RightHandPinky4"] = 1.0f;

    m_animator->m_animations[15]->setBlendMask(blendMask, 0.0f);
}