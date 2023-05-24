#include "ragdoll_ui.h"

void RagdollUI::render()
{
    if (!ImGui::CollapsingHeader("Ragdoll", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    ImGui::Checkbox("floatObject", &m_floatObject);
    ImGui::Checkbox("activateObject", &m_activateObject);
    ImGui::DragInt("floatIndex", &m_floatIndex, 1, 0, BODYPART_COUNT - 1);
    ImGui::DragFloat("stateChangeSpeed", &m_characterController->m_stateChangeSpeed, 0.1f);
    ImGui::DragFloat("impulseStrength", &m_characterController->m_impulseStrength, 0.1f);
    btRigidBody *rb = m_ragdoll->m_bodies[m_floatIndex];
    float cX = rb->getWorldTransform().getOrigin().getX();
    float cY = rb->getWorldTransform().getOrigin().getY();
    float cZ = rb->getWorldTransform().getOrigin().getZ();
    if (ImGui::DragFloat("cX", &cX, 0.1f))
    {
        rb->getWorldTransform().setOrigin(btVector3(cX, cY, cZ));
        rb->setLinearVelocity(btVector3(0, 0, 0));
    }
    if (ImGui::DragFloat("cY", &cY, 0.1))
    {
        rb->getWorldTransform().setOrigin(btVector3(cX, cY, cZ));
        rb->setLinearVelocity(btVector3(0, 0, 0));
    }
    if (ImGui::DragFloat("cZ", &cZ, 0.1))
    {
        rb->getWorldTransform().setOrigin(btVector3(cX, cY, cZ));
        rb->setLinearVelocity(btVector3(0, 0, 0));
    }
    if (m_floatObject)
    {
        if (m_activateObject)
            rb->setActivationState(1);
        rb->getWorldTransform().setOrigin(btVector3(cX, m_camera->followDistance, cZ));
        rb->setLinearVelocity(btVector3(0, 0, 0));
    }
}
