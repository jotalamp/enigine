#ifndef system_monitor_ui_hpp
#define system_monitor_ui_hpp

#include "../base_ui.h"

#if defined(__APPLE__)
#include <mach/mach.h>
#else
struct task_basic_info;
#endif

class SystemMonitorUI : public BaseUI
{
private:
    task_basic_info *m_info;

public:
    SystemMonitorUI(task_basic_info *info) : m_info(info) {}

    void render() override;
};

#endif /* system_monitor_ui_hpp */
