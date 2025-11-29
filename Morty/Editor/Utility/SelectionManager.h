#pragma once

#include "SingletonInstance.h"
#include "Utility/MString.h"
#include <functional>
#include <memory>
#include <vector>

namespace morty
{

class MEntity;
class MResource;

enum class SelectionType
{
    Entity,
    Resource,
    None
};

struct Selection
{
    SelectionType              type = SelectionType::None;
    MEntity*                   entity = nullptr;
    std::shared_ptr<MResource> resource = nullptr;

    Selection() = default;
    Selection(MEntity* e)
        : type(SelectionType::Entity)
        , entity(e)
    {}
    Selection(std::shared_ptr<MResource> r)
        : type(SelectionType::Resource)
        , resource(r)
    {}

    bool IsValid() const { return type != SelectionType::None && (entity != nullptr || resource != nullptr); }
};

class SelectionManager : public SingletonInstance<SelectionManager>
{
public:
    using SelectionCallback = std::function<void(const Selection&)>;

    void BroadcastSelection(const Selection& selection);

    void RegisterListener(void* listenerID, SelectionCallback callback);

    void UnregisterListener(void* listenerID);

private:
    struct ListenerInfo
    {
        void*             listenerID;
        SelectionCallback callback;
    };

    std::vector<ListenerInfo> m_listeners;
};

}// namespace morty
