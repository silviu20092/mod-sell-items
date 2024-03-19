#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "Chat.h"
#include "ModUtils.h"

struct ItemInfo
{
    uint32 action;
    ObjectGuid guid;
    std::string name;
    std::string uiName;

    bool operator<(const ItemInfo& a)
    {
        return name < a.name;
    }
};

class si_npc : public CreatureScript
{
private:
    std::vector<ItemInfo> itemCatalogue;
    static constexpr int PAGE_SIZE = 12;
    uint32 totalPages = 0;
    uint32 currentPage = 0;
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

    bool HandleBuybackItem(Player* player, uint32 slot)
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

    void AddItemToCatalogue(const Player* player, const Item* item, bool fromBank)
    {
        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(item->GetEntry());
        if (!itemTemplate)
            return;

        if (itemTemplate->SellPrice <= 0)
            return;

        ItemInfo itemInfo;
        itemInfo.action = GOSSIP_ACTION_INFO_DEF + 800 + itemCatalogue.size();
        itemInfo.guid = item->GetGUID();
        itemInfo.name = sModUtils->ItemNameWithLocale(player, itemTemplate);

        std::ostringstream oss;
        oss << sModUtils->ItemIcon(item->GetEntry());
        oss << sModUtils->ItemLink(player, itemTemplate);
        if (item->GetCount() > 1)
            oss << " - " << item->GetCount() << "x";
        oss << " - ";

        uint32 money = 0;
        if (!sModUtils->CalculateSellPrice(item, itemTemplate, money))
            return;

        oss << sModUtils->CopperToMoneyStr(money, true);
        if (fromBank)
            oss << " - IN BANK";
        itemInfo.uiName = oss.str();

        itemCatalogue.push_back(itemInfo);
    }

    void BuildSellItemsCatalogue(const Player* player)
    {
        itemCatalogue.clear();
        totalPages = 0;

        for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
            if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                AddItemToCatalogue(player, item, false);

        for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
            if (Bag* bag = player->GetBagByPos(i))
                for (uint32 j = 0; j < bag->GetBagSize(); j++)
                    if (Item* item = player->GetItemByPos(i, j))
                        AddItemToCatalogue(player, item, false);

        for (uint8 i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
            if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                AddItemToCatalogue(player, item, false);

        for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
            if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                AddItemToCatalogue(player, item, false);

        for (uint8 i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
            if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                AddItemToCatalogue(player, item, true);

        for (uint8 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
            if (Bag* bag = player->GetBagByPos(i))
                for (uint32 j = 0; j < bag->GetBagSize(); j++)
                    if (Item* item = player->GetItemByPos(i, j))
                        AddItemToCatalogue(player, item, true);

        if (itemCatalogue.size() > 0)
        {
            std::sort(itemCatalogue.begin(), itemCatalogue.end());

            totalPages = itemCatalogue.size() / PAGE_SIZE;
            if (itemCatalogue.size() % PAGE_SIZE != 0)
                totalPages++;
        }
    }

    bool AddSellItemsPage(Player* player, Creature* creature, uint32 page)
    {
        if (itemCatalogue.size() == 0 || (page + 1) > totalPages)
            return false;

        uint32 lowIndex = page * PAGE_SIZE;
        if (itemCatalogue.size() <= lowIndex)
            return false;

        uint32 highIndex = lowIndex + PAGE_SIZE - 1;
        if (highIndex >= itemCatalogue.size())
            highIndex = itemCatalogue.size() - 1;

        for (uint32 i = lowIndex; i <= highIndex; i++)
        {
            const ItemInfo& itemInfo = itemCatalogue[i];
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, itemInfo.uiName, GOSSIP_SENDER_MAIN, itemInfo.action);
        }

        if (page + 1 < totalPages)
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "[Next] ->", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 500 + page + 1);
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "<- [Back]", GOSSIP_SENDER_MAIN, page == 0 ? GOSSIP_ACTION_INFO_DEF : GOSSIP_ACTION_INFO_DEF + 500 + page - 1);

        return true;
    }

    const ItemInfo* FindItemInfo(uint32 identifier) const
    {
        std::vector<ItemInfo>::const_iterator citer = std::find_if(itemCatalogue.begin(), itemCatalogue.end(), [&identifier](const ItemInfo& itemInfo) {
            return itemInfo.action == identifier;
        });
        if (citer != itemCatalogue.end())
            return &*citer;
        return nullptr;
    }

    bool HandleSellItemByInfo(const ItemInfo* itemInfo, Player* player, Creature* creature)
    {
        Item* item = player->GetItemByGuid(itemInfo->guid);
        if (item == nullptr)
        {
            ChatHandler(player->GetSession()).SendSysMessage("This item is no longer available in your inventory.");
            return false;
        }

        return sModUtils->SellItem(player, creature, item);
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

        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Sell individual...", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 500);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Buyback", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 200);
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Nevermind", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
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
        else if (action > GOSSIP_ACTION_INFO_DEF + 200 && action < GOSSIP_ACTION_INFO_DEF + 500)
        {
            const uint32 slot = action - (GOSSIP_ACTION_INFO_DEF + 200);
            if (HandleBuybackItem(player, slot))
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
        else if (action >= GOSSIP_ACTION_INFO_DEF + 500 && action < GOSSIP_ACTION_INFO_DEF + 800)
        {
            ClearGossipMenuFor(player);

            BuildSellItemsCatalogue(player);
            currentPage = action - (GOSSIP_ACTION_INFO_DEF + 500);
            if (!AddSellItemsPage(player, creature, currentPage))
            {
                ChatHandler(player->GetSession()).SendSysMessage("There is nothing to sell on current page.");
                CloseGossipMenuFor(player);
                return false;
            }
            
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            return true;
        }
        else if (action >= GOSSIP_ACTION_INFO_DEF + 800)
        {
            const ItemInfo* itemInfo = FindItemInfo(action);
            if (itemInfo == nullptr)
            {
                ChatHandler(player->GetSession()).SendSysMessage("Could not sell item.");
                CloseGossipMenuFor(player);
                return false;
            }
            if (HandleSellItemByInfo(itemInfo, player, creature))
            {
                ClearGossipMenuFor(player);
                BuildSellItemsCatalogue(player);
                if (!AddSellItemsPage(player, creature, currentPage))
                {
                    ChatHandler(player->GetSession()).SendSysMessage("There is nothing to sell on current page.");
                    CloseGossipMenuFor(player);
                    return false;
                }
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
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
