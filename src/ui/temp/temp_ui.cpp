#include "temp_ui.h"

void TempUI::render()
{
    if (!ImGui::CollapsingHeader("Others", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    if (ImGui::Button("refresh shaders"))
    {
        m_shaderManager->initShaders();
    }
    ImGui::Text("Post process");
    ImGui::DragFloat("exposure", &m_postProcess->m_exposure, 0.001);
    ImGui::Separator();
    ImGui::Text("Physics");
    bool debugEnabled = m_debugDrawer->getDebugMode();
    if (ImGui::Checkbox("wireframe", &debugEnabled))
    {
        m_debugDrawer->setDebugMode(debugEnabled ? btIDebugDraw::DBG_DrawWireframe : btIDebugDraw::DBG_NoDebug);
    }
    int lines = m_debugDrawer->getLines().size();
    ImGui::DragInt("lines", &lines);
    ImGui::Text("Light");
    ImGui::DragFloat("power", &m_lightPower, 0.1);
    ImGui::DragFloat("radius", &m_radius, 0.1);
    ImGui::DragFloat("speed", &m_speed, 0.01);
    ImGui::ColorEdit3("lightColor", m_lightColor);
    ImGui::ColorEdit3("ambientColor", m_ambientColor);
    ImGui::ColorEdit3("specularColor", m_specularColor);
}
