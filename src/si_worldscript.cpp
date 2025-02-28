#include "ScriptMgr.h"
#include "Config.h"
#include "ModUtils.h"

class si_worldscript : public WorldScript
{
public:
    si_worldscript() : WorldScript("si_worldscript")
    {
    }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        sModUtils->SetSearchBank(sConfigMgr->GetOption<bool>("SellItems.SearchBank", false));
        sModUtils->SetSearchBankBags(sConfigMgr->GetOption<bool>("SellItems.SearchBankBags", false));
        sModUtils->SetSearchKeyring(sConfigMgr->GetOption<bool>("SellItems.SearchKeyring", true));
        sModUtils->SetSearchEquipped(sConfigMgr->GetOption<bool>("SellItems.SearchEquipped", false));
        sModUtils->SetSearchBackpack(sConfigMgr->GetOption<bool>("SellItems.SearchBackpack", true));
        sModUtils->SetSearchBags(sConfigMgr->GetOption<bool>("SellItems.SearchBags", true));
        sModUtils->SetSellTradeGoods(sConfigMgr->GetOption<bool>("SellItems.SellTradeGoods", false));
        sModUtils->SetSellQuestItems(sConfigMgr->GetOption<bool>("SellItems.SellQuestItems", false));

        sModUtils->BuildItemQualityColorIdentifier();
    }
};

void AddSC_si_worldscript()
{
    new si_worldscript();
}
