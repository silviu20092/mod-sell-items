#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "Player.h"
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

    static std::string BuybackItemLink(const Item* item, const Player* player, uint32 slot)
    {
        std::ostringstream oss;
        oss << sModUtils->ItemIcon(item->GetEntry());
        oss << sModUtils->ItemLink(player, item->GetEntry());
        if (item->GetCount() > 1)
            oss << " - " << item->GetCount() << "x";
        oss << " - " << sModUtils->CopperToMoneyStr(player->GetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1 + slot - BUYBACK_SLOT_START), true);
        return oss.str();
    }
private:
    void AddBuybackMenu(Player* player, Creature* creature)
    {
        bool found = false;
        for (uint32 slot = BUYBACK_SLOT_START; slot < BUYBACK_SLOT_END; slot++)
        {
            Item* item = player->GetItemFromBuyBackSlot(slot);
            if (item)
            {
                found = true;
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, BuybackItemLink(item, player, slot), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 200 + slot);
            }
        }

        if (!found)
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "[NO ITEMS IN BUYBACK]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 200);

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "<- [Back]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    }

    bool HandleBuybackItem(Player* player, Creature* creature, uint32 slot)
    {
        Item* item = player->GetItemFromBuyBackSlot(slot);
        if (!item)
        {
            player->SendBuyError(BUY_ERR_CANT_FIND_ITEM, nullptr, 0, 0);
            return false;
        }

        uint32 price = player->GetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1 + slot - BUYBACK_SLOT_START);
        if (!player->HasEnoughMoney(price))
        {
            player->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, nullptr, item->GetEntry(), 0);
            return false;
        }

        ItemPosCountVec dest;
        InventoryResult msg = player->CanStoreItem(NULL_BAG, NULL_SLOT, dest, item, false);
        if (msg == EQUIP_ERR_OK)
        {
            if (sWorld->getBoolConfig(CONFIG_ITEMDELETE_VENDOR))
            {
                CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_RECOVERY_ITEM);
                stmt->SetData(0, player->GetGUID().GetCounter());
                stmt->SetData(1, item->GetEntry());
                stmt->SetData(2, item->GetCount());
                CharacterDatabase.Execute(stmt);
            }

            player->ModifyMoney(-(int32)price);
            player->RemoveItemFromBuyBackSlot(slot, false);
            player->ItemAddedQuestCheck(item->GetEntry(), item->GetCount());
            player->StoreItem(dest, item, true);
        }
        else
        {
            player->SendEquipError(msg, item, nullptr);
            return false;
        }

        return true;
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

        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Buyback", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 200);
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
        else if (action == GOSSIP_ACTION_INFO_DEF + 200)
        {
            ClearGossipMenuFor(player);
            AddBuybackMenu(player, creature);
            return true;
        }
        else if (action > GOSSIP_ACTION_INFO_DEF + 200)
        {
            const uint32 slot = action - (GOSSIP_ACTION_INFO_DEF + 200);
            if (HandleBuybackItem(player, creature, slot))
            {
                ClearGossipMenuFor(player);
                AddBuybackMenu(player, creature);
                return true;
            }
            else
            {
                CloseGossipMenuFor(player);
                return false;
            }
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
