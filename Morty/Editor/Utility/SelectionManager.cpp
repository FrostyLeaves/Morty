#include "SelectionManager.h"

using namespace morty;

void SelectionManager::BroadcastSelection(const Selection& selection)
{
    for (const auto& listener: m_listeners)
    {
        if (listener.callback) { listener.callback(selection); }
    }
}

void SelectionManager::RegisterListener(void* listenerID, SelectionCallback callback)
{
    // Check if already registered
    for (auto& listener: m_listeners)
    {
        if (listener.listenerID == listenerID)
        {
            listener.callback = callback;
            return;
        }
    }

    // Add new listener
    m_listeners.push_back({listenerID, callback});
}

void SelectionManager::UnregisterListener(void* listenerID)
{
    m_listeners.erase(
            std::remove_if(
                    m_listeners.begin(),
                    m_listeners.end(),
                    [listenerID](const ListenerInfo& info) { return info.listenerID == listenerID; }
            ),
            m_listeners.end()
    );
}
