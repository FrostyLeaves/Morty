#include "LogWidget.h"

#include "Engine/MEngine.h"
#include "Main/MainEditor.h"
#include "imgui.h"

using namespace morty;

LogWidget::LogWidget()
    : BaseWidget()
{
    m_strViewName = "Log";
}

void LogWidget::Initialize(MainEditor* pMainEditor)
{
    BaseWidget::Initialize(pMainEditor);

    MLogger::GetInstance()->SetPrintFunction(
            [this](MLogType type, const char* message) { AddLog(type, MString(message)); }
    );
}

void LogWidget::Release()
{
    MLogger::GetInstance()->SetPrintFunction(nullptr);
}

void LogWidget::AddLog(MLogType type, const MString& message)
{
    std::lock_guard<std::mutex> lock(m_logMutex);

    m_logs.push_back({type, message});

    while (m_logs.size() > m_maxLogCount) { m_logs.pop_front(); }

    m_scrollToBottom = m_autoScroll;
}

void LogWidget::Clear()
{
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_logs.clear();
}

ImVec4 LogWidget::GetLogColor(MLogType type) const
{
    switch (type)
    {
        case MLogType::EError: return ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
        case MLogType::EWarn: return ImVec4(1.0f, 1.0f, 0.4f, 1.0f);
        case MLogType::EInfo: return ImVec4(0.4f, 0.8f, 1.0f, 1.0f);
        case MLogType::EDefault:
        default: return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

void LogWidget::Render()
{
    // Toolbar
    if (ImGui::Button("Clear")) { Clear(); }

    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &m_autoScroll);

    ImGui::SameLine();
    ImGui::Separator();

    ImGui::SameLine();
    ImGui::Checkbox("Info", &m_showInfo);

    ImGui::SameLine();
    ImGui::Checkbox("Warning", &m_showWarning);

    ImGui::SameLine();
    ImGui::Checkbox("Error", &m_showError);

    ImGui::SameLine();
    ImGui::Checkbox("Default", &m_showDefault);

    ImGui::Separator();

    // Log list
    ImGui::BeginChild("LogScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    {
        std::lock_guard<std::mutex> lock(m_logMutex);

        for (const auto& entry: m_logs)
        {
            bool show = false;
            switch (entry.type)
            {
                case MLogType::EInfo: show = m_showInfo; break;
                case MLogType::EWarn: show = m_showWarning; break;
                case MLogType::EError: show = m_showError; break;
                case MLogType::EDefault: show = m_showDefault; break;
            }

            if (show)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, GetLogColor(entry.type));
                ImGui::TextUnformatted(entry.message.c_str());
                ImGui::PopStyleColor();
            }
        }
    }

    if (m_scrollToBottom)
    {
        ImGui::SetScrollHereY(1.0f);
        m_scrollToBottom = false;
    }

    ImGui::EndChild();
}
