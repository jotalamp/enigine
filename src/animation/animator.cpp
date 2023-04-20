#include <vector>

#include "animator.h"

Animator::Animator(std::vector<Animation *> animations)
{
    m_animations = animations;
    for (int i = 0; i < m_animations.size(); i++)
        m_timers.push_back(0.0f);

    m_state.fromIndex = 0;
    m_state.toIndex = 0;
    m_state.blendFactor = 0.0f;

    // TODO: better way?
    m_FinalBoneMatrices.reserve(MAX_BONES);
    for (int i = 0; i < MAX_BONES; i++)
        m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
}

Animator::~Animator()
{
    // TODO: destruction
}

// TODO: move validations to setters
void Animator::update(float deltaTime)
{
    if (m_animations.size() == 0)
        return;

    if ((m_state.fromIndex < 0) || (m_state.fromIndex >= m_animations.size()))
        return;

    if ((m_state.toIndex < 0) || (m_state.toIndex >= m_animations.size()))
        return;

    if ((m_state.blendFactor < 0.0f) || (m_state.blendFactor > 1.0f))
        return;

    for (int i = 0; i < m_animations.size(); i++)
    {
        m_timers[i] += m_animations[i]->m_TicksPerSecond * deltaTime;
        m_timers[i] = fmod(m_timers[i], m_animations[i]->m_Duration);
    }

    calculateBoneTransform(&m_animations[0]->m_RootNode, glm::mat4(1.0f));
}

void Animator::calculateBoneTransform(const AssimpNodeData *node, glm::mat4 parentTransform)
{
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;

    Bone *bone1 = m_animations[m_state.fromIndex]->getBone(nodeName);
    Bone *bone2 = m_animations[m_state.toIndex]->getBone(nodeName);

    bool validBonePair = bone1 && bone2;
    bool seperatedPair = (m_state.blendFactor == 0.0f) || (m_state.blendFactor == 1.0f);
    bool singleAnimation = seperatedPair || (bone1 == bone2);

    if (validBonePair && singleAnimation)
    {
        int singleAnimIndex = seperatedPair ? (m_state.blendFactor == 0 ? m_state.fromIndex : m_state.toIndex) : m_state.fromIndex;
        Bone *bone = singleAnimIndex == m_state.fromIndex ? bone1 : bone2;
        bone->update(m_timers[singleAnimIndex]);

        glm::mat4 blendedT = glm::translate(glm::mat4(1), bone->m_translation);
        glm::mat4 blendedR = glm::toMat4(glm::normalize(bone->m_rotation));
        glm::mat4 blendedS = glm::scale(glm::mat4(1), bone->m_scale);

        nodeTransform = blendedT * blendedR * blendedS;
    }
    else if (validBonePair)
    {
        bone1->update(m_timers[m_state.fromIndex]);
        bone2->update(m_timers[m_state.toIndex]);

        glm::mat4 blendedT = glm::translate(glm::mat4(1), glm::mix(bone1->m_translation, bone2->m_translation, m_state.blendFactor));
        glm::mat4 blendedR = glm::toMat4(glm::normalize(glm::slerp(bone1->m_rotation, bone2->m_rotation, m_state.blendFactor)));
        glm::mat4 blendedS = glm::scale(glm::mat4(1), glm::mix(bone1->m_scale, bone2->m_scale, m_state.blendFactor));

        nodeTransform = blendedT * blendedR * blendedS;
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransform;

    // TODO: always first index is correct?
    auto boneInfoMap = m_animations[0]->m_BoneInfoMap;
    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        int index = boneInfoMap[nodeName].id;
        glm::mat4 offset = boneInfoMap[nodeName].offset;
        m_FinalBoneMatrices[index] = globalTransformation * offset;
    }

    for (int i = 0; i < node->childrenCount; i++)
        calculateBoneTransform(&node->children[i], globalTransformation);
}
