#include "ScriptMgr.h"
#include "Config.h"
#include "ModUtils.h"

class si_worldscript : public WorldScript
{
public:
    si_worldscript() : WorldScript("si_worldscript")
    {
    }

    void OnAfterConfigLoad(bool reload)
    {
        if (reload)
            sModUtils->BuildItemQualityColorIdentifier();
    }
};

void AddSC_si_worldscript()
{
    new si_worldscript();
}
