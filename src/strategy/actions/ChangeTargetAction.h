
#pragma once

#include "../Action.h"

namespace ai
{
    class ChangeTargetAction : public Action
    {
    public:
        ChangeTargetAction(PlayerbotAI* ai, std::string name = "change target") : Action(ai, name) {}
        virtual bool Execute(Event event);
        virtual bool isUseful();
    };
}
