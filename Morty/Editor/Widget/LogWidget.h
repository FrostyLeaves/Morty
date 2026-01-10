#pragma once

#include "Main/BaseWidget.h"
#include "Utility/MLogger.h"

#include <deque>
#include <mutex>

namespace morty
{

class LogWidget : public BaseWidget
{
public:
    LogWidget();
    ~LogWidget() = default;

    void Render() override;
    void Initialize(MainEditor* pMainEditor) override;
    void Release() override;

    void AddLog(MLogType type, const MString& message);
    void Clear();

private:
    struct LogEntry
    {
        MLogType type;
        MString  message;
    };

    ImVec4 GetLogColor(MLogType type) const;

    std::deque<LogEntry> m_logs;
    std::mutex           m_logMutex;
    size_t               m_maxLogCount  = 1000;
    bool                 m_autoScroll   = true;
    bool                 m_scrollToBottom = false;

    bool                 m_showInfo     = true;
    bool                 m_showWarning  = true;
    bool                 m_showError    = true;
    bool                 m_showDefault  = true;
};

}// namespace morty
