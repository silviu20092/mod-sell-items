/*
 * Credits: silviu20092
 */

#include "Chat.h"
#include "ObjectMgr.h"
#include "Config.h"
#include "Player.h"
#include "ModUtils.h"

ModUtils::ModUtils()
{
    searchBank = false;
    searchBankBags = false;
    searchKeyring = true;
    searchEquipped = false;
    searchBackpack = true;
    searchBags = true;
    sellTradeGoods = false;
    sellQuestItems = false;
}

ModUtils::~ModUtils() { }

ModUtils* ModUtils::instance()
{
    static ModUtils instance;
    return &instance;
}

std::string ModUtils::ToLower(const std::string& text)
{
    std::string result = text;
    std::transform(result.begin(), result.end(), result.begin(), tolower);
    return result;
}

uint32 ModUtils::ColorToQuality(const std::string& color)
{
    for (auto it = itemQualityColorIdentifier.begin(); it != itemQualityColorIdentifier.end(); ++it)
        if (it->second == color)
            return it->first;

    return MAX_ITEM_QUALITY;
}

std::string ModUtils::ItemLink(const Player* player, const ItemTemplate* itemTemplate) const
{
    LocaleConstant loc_idx = player->GetSession()->GetSessionDbLocaleIndex();
    std::string name = itemTemplate->Name1;
    if (ItemLocale const* il = sObjectMgr->GetItemLocale(itemTemplate->ItemId))
        ObjectMgr::GetLocaleString(il->Name, loc_idx, name);

    std::stringstream oss;
    oss << "|c";
    oss << std::hex << ItemQualityColors[itemTemplate->Quality] << std::dec;
    oss << "|Hitem:";
    oss << itemTemplate->ItemId;
    oss << ":0:0:0:0:0:0:0:0:0|h[";
    oss << name;
    oss << "]|h|r";

    return oss.str();
}

std::string ModUtils::ItemLink(const Player* player, uint32 entry) const
{
    const ItemTemplate* itemTemplate = sObjectMgr->GetItemTemplate(entry);
    return ItemLink(player, itemTemplate);
}

void ModUtils::SellItem(Player* player, Item* item, const ItemTemplate* itemTemplate, uint32& totalSellPrice, uint32& totalCount)
{
    if (!GetSellTradeGoods() && itemTemplate->Class == ITEM_CLASS_TRADE_GOODS)
        return;

    if (!GetSellQuestItems() && itemTemplate->Class == ITEM_CLASS_QUEST)
        return;

    if (itemTemplate->SellPrice <= 0)
        return;

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

    ChatHandler chatHandler(player->GetSession());
    chatHandler.PSendSysMessage(LANG_MOD_SI_SOLD_ITEM, count, ItemLink(player, itemTemplate).c_str(), money);
}

bool ModUtils::SellItemsOfQuality(Player* player, uint32 quality)
{
    uint32 totalSellPrice = 0;
    uint32 soldItems = 0;

    // check in Backpack (main bag)
    if (GetSearchBackpack())
    {
        for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
        {
            if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            {
                ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(item->GetEntry());
                if (itemTemplate && itemTemplate->Quality == quality)
                    SellItem(player, item, itemTemplate, totalSellPrice, soldItems);
            }
        }
    }

    // check in the 4-bag slots
    if (GetSearchBags())
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
                            SellItem(player, item, itemTemplate, totalSellPrice, soldItems);
                    }
                }
            }
        }
    }

    // check in keyring slots
    if (GetSearchKeyring())
    {
        for (uint8 i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
        {
            if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            {
                ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(item->GetEntry());
                if (itemTemplate && itemTemplate->Quality == quality)
                    SellItem(player, item, itemTemplate, totalSellPrice, soldItems);
            }
        }
    }

    // check in equipped items
    if (GetSearchEquipped())
    {
        for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
        {
            if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            {
                ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(item->GetEntry());
                if (itemTemplate && itemTemplate->Quality == quality)
                    SellItem(player, item, itemTemplate, totalSellPrice, soldItems);
            }
        }
    }

    // check in Bank
    if (GetSearchBank())
    {
        for (uint8 i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
        {
            if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            {
                ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(item->GetEntry());
                if (itemTemplate && itemTemplate->Quality == quality)
                    SellItem(player, item, itemTemplate, totalSellPrice, soldItems);
            }
        }
    }

    // check in Bank bags
    if (GetSearchBankBags())
    {
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
                            SellItem(player, item, itemTemplate, totalSellPrice, soldItems);
                    }
                }
            }
        }
    }

    ChatHandler chatHandler(player->GetSession());

    if (soldItems > 0)
        chatHandler.PSendSysMessage(LANG_MOD_SI_SOLD_ALL_ITEMS, soldItems, totalSellPrice);
    else
        chatHandler.SendSysMessage(LANG_MOD_SI_NOTHING_NO_SELL);

    return true;
}

void ModUtils::BuildItemQualityColorIdentifier()
{
    itemQualityColorIdentifier[ITEM_QUALITY_POOR] = sConfigMgr->GetOption<std::string>("SellItems.ItemQualityColorIdentifier.Poor", "grey");
    itemQualityColorIdentifier[ITEM_QUALITY_NORMAL] = sConfigMgr->GetOption<std::string>("SellItems.ItemQualityColorIdentifier.Normal", "white");
    itemQualityColorIdentifier[ITEM_QUALITY_UNCOMMON] = sConfigMgr->GetOption<std::string>("SellItems.ItemQualityColorIdentifier.Uncommon", "green");
    itemQualityColorIdentifier[ITEM_QUALITY_RARE] = sConfigMgr->GetOption<std::string>("SellItems.ItemQualityColorIdentifier.Rare", "blue");
    itemQualityColorIdentifier[ITEM_QUALITY_EPIC] = sConfigMgr->GetOption<std::string>("SellItems.ItemQualityColorIdentifier.Epic", "epic");
    itemQualityColorIdentifier[ITEM_QUALITY_LEGENDARY] = sConfigMgr->GetOption<std::string>("SellItems.ItemQualityColorIdentifier.Legendary", "orange");
}

std::string ModUtils::ItemIcon(uint32 entry, uint32 width, uint32 height, int x, int y) const
{
    std::ostringstream ss;
    ss << "|TInterface";
    const ItemTemplate* temp = sObjectMgr->GetItemTemplate(entry);
    const ItemDisplayInfoEntry* dispInfo = NULL;
    if (temp)
    {
        dispInfo = sItemDisplayInfoStore.LookupEntry(temp->DisplayInfoID);
        if (dispInfo)
            ss << "/ICONS/" << dispInfo->inventoryIcon;
    }
    if (!dispInfo)
        ss << "/InventoryItems/WoWUnknownItem01";
    ss << ":" << width << ":" << height << ":" << x << ":" << y << "|t";
    return ss.str();
}

std::string ModUtils::ItemIcon(uint32 entry) const
{
    return ItemIcon(entry, 30, 30, 0, 0);
}

std::string ModUtils::CopperToMoneyStr(uint32 money, bool colored) const
{
    uint32 gold = money / GOLD;
    uint32 silver = (money % GOLD) / SILVER;
    uint32 copper = (money % GOLD) % SILVER;

    std::ostringstream oss;
    if (gold > 0)
    {
        if (colored)
            oss << gold << "|cffD9B430g|r";
        else
            oss << gold << "g";
    }
    if (silver > 0)
    {
        if (colored)
            oss << silver << "|cff7E7C7Fs|r";
        else
            oss << silver << "s";
    }
    if (copper > 0)
    {
        if (colored)
            oss << copper << "|cff974B29c|r";
        else
            oss << copper << "c";
    }

    return oss.str();
}
