/*
 * Credits: silviu20092
 */

#include "ScriptMgr.h"
#include "ModUtils.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "CommandScript.h"

using namespace Acore::ChatCommands;

class si_commandscript : public CommandScript
{
public:
    si_commandscript() : CommandScript("si_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable sellCommandTable =
        {
            { "sell", HandleSellItems, SEC_PLAYER, Console::No }
        };

        return sellCommandTable;
    }
private:
    static bool HandleSellItems(ChatHandler* handler, std::string color)
    {
        if (color.empty())
            return false;

        color = sModUtils->ToLower(color);

        uint32 quality = sModUtils->ColorToQuality(color);
        if (quality == MAX_ITEM_QUALITY)
        {
            handler->SendSysMessage(LANG_MOD_SI_INVALID_QUALITY);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return sModUtils->SellItemsOfQuality(handler->GetPlayer(), quality);
    }
};

// Add all scripts in one
void AddSC_si_commandscript()
{
    new si_commandscript();
}
