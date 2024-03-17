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

enum SellItemsModStrings
{
    LANG_MOD_SI_SOLD_ITEM = 40000,
    LANG_MOD_SI_SOLD_ALL_ITEMS = 40001,
    LANG_MOD_SI_NOTHING_NO_SELL = 40002,
    LANG_MOD_SI_INVALID_QUALITY = 40003
};

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
    static void SellItem(ChatHandler* handler, Item* item, const ItemTemplate* itemTemplate, uint32& totalSellPrice, uint32& totalCount)
    {
        if (!sConfigMgr->GetOption<int32>("SellItems.SellTradeGoods", 0) && itemTemplate->Class == ITEM_CLASS_TRADE_GOODS)
            return;

        if (!sConfigMgr->GetOption<int32>("SellItems.SellQuestItems", 0) && itemTemplate->Class == ITEM_CLASS_QUEST)
            return;

        if (itemTemplate->SellPrice <= 0)
            return;

        Player* player = handler->GetPlayer();

        if (item->GetOwnerGUID() != player->GetGUID())
            return;

        if (item->IsNotEmptyBag())
            return;

        if (player->GetLootGUID() == item->GetGUID())
            return;

        if (item->HasFlag(ITEM_FIELD_FLAGS, ITEM_FIELD_FLAG_REFUNDABLE))
            return;

        uint32 count = item->GetCount();
        uint32 money = itemTemplate->SellPrice * count;
        if (player->GetMoney() >= MAX_MONEY_AMOUNT - money)
            return;

        uint32 maxDurability = item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
        if (maxDurability)
        {
            uint32 curDurability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);
            uint32 LostDurability = maxDurability - curDurability;

            if (LostDurability > 0)
            {
                DurabilityCostsEntry const* dcost = sDurabilityCostsStore.LookupEntry(itemTemplate->ItemLevel);
                if (!dcost)
                    return;

                uint32 dQualitymodEntryId = (itemTemplate->Quality + 1) * 2;
                DurabilityQualityEntry const* dQualitymodEntry = sDurabilityQualityStore.LookupEntry(dQualitymodEntryId);
                if (!dQualitymodEntry)
                    return;

                uint32 dmultiplier = dcost->multiplier[ItemSubClassToDurabilityMultiplierId(itemTemplate->Class, itemTemplate->SubClass)];
                uint32 refund = uint32(std::ceil(LostDurability * dmultiplier * double(dQualitymodEntry->quality_mod)));

                if (!refund)
                    refund = 1;

                if (refund > money)
                    money = 1;
                else
                    money -= refund;
            }
        }

        player->ItemRemovedQuestCheck(item->GetEntry(), item->GetCount());
        player->RemoveItem(item->GetBagSlot(), item->GetSlot(), true);
        item->RemoveFromUpdateQueueOf(player);
        player->AddItemToBuyBackSlot(item, money);
        player->UpdateTitansGrip();

        player->ModifyMoney(money);

        totalSellPrice += money;
        totalCount += count;

        handler->PSendSysMessage(LANG_MOD_SI_SOLD_ITEM, count, ModUtils::ItemLink(handler, itemTemplate).c_str(), money);
    }

    static bool HandleSellItems(ChatHandler* handler, std::string color)
    {
        if (color.empty())
            return false;

        color = ModUtils::ToLower(color);

        uint32 quality = ModUtils::ColorToQuality(color);
        if (quality == MAX_ITEM_QUALITY)
        {
            handler->SendSysMessage(LANG_MOD_SI_INVALID_QUALITY);
            handler->SetSentErrorMessage(true);
            return false;
        }

        const Player* player = handler->GetPlayer();
        uint32 totalSellPrice = 0;
        uint32 soldItems = 0;

        // check in Backpack (main bag)
        if (sConfigMgr->GetOption<int32>("SellItems.SearchBackpack", 1))
        {
            for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
            {
                if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(item->GetEntry());
                    if (itemTemplate && itemTemplate->Quality == quality)
                        SellItem(handler, item, itemTemplate, totalSellPrice, soldItems);
                }
            }
        }

        // check in the 4-bag slots
        if (sConfigMgr->GetOption<int32>("SellItems.SearchBags", 1))
        {
            for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
            {
                if (Bag* bag = player->GetBagByPos(i))
                {
                    for (uint32 j = 0; j < bag->GetBagSize(); j++)
                    {
                        if (Item* item = player->GetItemByPos(i, j))
                        {
                            ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(item->GetEntry());
                            if (itemTemplate && itemTemplate->Quality == quality)
                                SellItem(handler, item, itemTemplate, totalSellPrice, soldItems);
                        }
                    }
                }
            }
        }

        // check in keyring slots
        if (sConfigMgr->GetOption<int32>("SellItems.SearchKeyring", 1))
        {
            for (uint8 i = KEYRING_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
            {
                if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(item->GetEntry());
                    if (itemTemplate && itemTemplate->Quality == quality)
                        SellItem(handler, item, itemTemplate, totalSellPrice, soldItems);
                }
            }
        }

        // check in equipped items
        if (sConfigMgr->GetOption<int32>("SellItems.SearchEquipped", 0))
        {
            for (uint8 i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
            {
                if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(item->GetEntry());
                    if (itemTemplate && itemTemplate->Quality == quality)
                        SellItem(handler, item, itemTemplate, totalSellPrice, soldItems);
                }
            }
        }

        // check in Bank and Bank bags
        if (sConfigMgr->GetOption<int32>("SellItems.SearchBank", 0))
        {
            for (uint8 i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
            {
                if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                {
                    ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(item->GetEntry());
                    if (itemTemplate && itemTemplate->Quality == quality)
                        SellItem(handler, item, itemTemplate, totalSellPrice, soldItems);
                }
            }

            for (uint8 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
            {
                if (Bag* bag = player->GetBagByPos(i))
                {
                    for (uint32 j = 0; j < bag->GetBagSize(); j++)
                    {
                        if (Item* item = player->GetItemByPos(i, j))
                        {
                            ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(item->GetEntry());
                            if (itemTemplate && itemTemplate->Quality == quality)
                                SellItem(handler, item, itemTemplate, totalSellPrice, soldItems);
                        }
                    }
                }
            }
        }

        if (soldItems > 0)
            handler->PSendSysMessage(LANG_MOD_SI_SOLD_ALL_ITEMS, soldItems, totalSellPrice);
        else
            handler->SendSysMessage(LANG_MOD_SI_NOTHING_NO_SELL);

        return true;
    }
};

// Add all scripts in one
void AddSC_si_commandscript()
{
    new si_commandscript();
}
