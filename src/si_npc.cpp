#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "ModUtils.h"

class si_npc : public CreatureScript
{
private:
    static std::string FormatSellMenu(uint32 quality, const std::string& text)
    {
        std::stringstream oss;
        oss << "Sell ";
        oss << "|c";
        oss << std::hex << ItemQualityColors[quality] << std::dec;
        oss << text << "|r";

        return oss.str();
    }

    static std::string FormatFeature(const std::string& text, const int32 configValue)
    {
        std::stringstream oss;
        oss << text << ": ";
        oss << "|cff" << (configValue ? "048a2cYES" : "b50f04NO") << "|r";
        return oss.str();
    }
public:
    si_npc() : CreatureScript("si_npc")
    {
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "Features", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        const std::map<uint32, std::string>& itemQualityMap = sModUtils->GetItemQualityColorIdentifier();
        for (auto it = itemQualityMap.begin(); it != itemQualityMap.end(); ++it)
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, FormatSellMenu(it->first, it->second), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2 + std::distance(itemQualityMap.begin(), it), "Are you sure?", 0, false);

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Nevermind", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32  /*sender*/, uint32 action) override
    {
        if (action == GOSSIP_ACTION_INFO_DEF)
        {
            ClearGossipMenuFor(player);
            OnGossipHello(player, creature);
            return true;
        }
        else if (action == GOSSIP_ACTION_INFO_DEF + 100)
        {
            CloseGossipMenuFor(player);
            return true;
        }
        else if (action == GOSSIP_ACTION_INFO_DEF + 1)
        {
            ClearGossipMenuFor(player);
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, FormatFeature("Search bank", sModUtils->GetSearchBank()), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, FormatFeature("Search bank bags", sModUtils->GetSearchBankBags()), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, FormatFeature("Search keyring", sModUtils->GetSearchKeyring()), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, FormatFeature("Search equipped", sModUtils->GetSearchEquipped()), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, FormatFeature("Search backpack", sModUtils->GetSearchBackpack()), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, FormatFeature("Search bags", sModUtils->GetSearchBags()), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, FormatFeature("Sell trade goods", sModUtils->GetSellTradeGoods()), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, FormatFeature("Sell quest items", sModUtils->GetSellQuestItems()), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "<- [Back]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());

            return true;
        }
        else
        {
            const uint32 quality = action - (GOSSIP_ACTION_INFO_DEF + 2);
            sModUtils->SellItemsOfQuality(player, quality);
            CloseGossipMenuFor(player);
            return true;
        }

        return false;
    }
};

void AddSC_si_npc()
{
    new si_npc();
}
