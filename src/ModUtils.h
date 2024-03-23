/*
 * Credits: silviu20092
 */

#ifndef _MOD_UTILS_H_
#define _MOD_UTILS_H_

#include <string>

class ChatHandler;
class Item;
class ItemTemplate;
class Player;

enum SellItemsModStrings
{
    LANG_MOD_SI_SOLD_ITEM = 40000,
    LANG_MOD_SI_SOLD_ALL_ITEMS = 40001,
    LANG_MOD_SI_NOTHING_NO_SELL = 40002,
    LANG_MOD_SI_INVALID_QUALITY = 40003
};

class ModUtils
{
private:
    ModUtils();
    ~ModUtils();

    std::map<uint32, std::string> itemQualityColorIdentifier;

    void SellItem(Player* player, Item* item, const ItemTemplate* itemTemplate, uint32& totalSellPrice, uint32& totalCount);
private:
    bool searchBank;
    bool searchBankBags;
    bool searchKeyring;
    bool searchEquipped;
    bool searchBackpack;
    bool searchBags;
    bool sellTradeGoods;
    bool sellQuestItems;
public:
    static ModUtils* instance();

    std::string ToLower(const std::string& text);
    uint32 ColorToQuality(const std::string& color);

    bool SellItemsOfQuality(Player* player, uint32 quality);

    void BuildItemQualityColorIdentifier();
    const std::map<uint32, std::string>& GetItemQualityColorIdentifier() const
    {
        return itemQualityColorIdentifier;
    }

    std::string ItemNameWithLocale(const Player* player, const ItemTemplate* itemTemplate, int32 randomPropertyId) const;
    std::string ItemLink(const Player* player, const ItemTemplate* itemTemplate, int32 randomPropertyId) const;
    std::string ItemLink(const Player* player, uint32 entry, int32 randomPropertyId) const;
    std::string ItemLink(const Player* player, const Item* item) const;
    std::string ItemIcon(uint32 entry, uint32 width, uint32 height, int x, int y) const;
    std::string ItemIcon(uint32 entry) const;

    std::string CopperToMoneyStr(uint32 money, bool colored) const;

    bool CalculateDurabilityMoney(const Item* item, const ItemTemplate* itemTemplate, uint32& money) const;
    bool CalculateSellPrice(const Item* item, const ItemTemplate* itemTemplate, uint32& money) const;

    bool SellItem(Player* player, Creature* creature, Item* item) const;
public:
    void SetSearchBank(bool value) { searchBank = value; }
    void SetSearchBankBags(bool value) { searchBankBags = value; }
    void SetSearchKeyring(bool value) { searchKeyring = value; }
    void SetSearchEquipped(bool value) { searchEquipped = value; }
    void SetSearchBackpack(bool value) { searchBackpack = value; }
    void SetSearchBags(bool value) { searchBags = value; }
    void SetSellTradeGoods(bool value) { sellTradeGoods = value; }
    void SetSellQuestItems(bool value) { sellQuestItems = value; }

    bool GetSearchBank() { return searchBank; }
    bool GetSearchBankBags() { return searchBankBags; }
    bool GetSearchKeyring() { return searchKeyring; }
    bool GetSearchEquipped() { return searchEquipped; }
    bool GetSearchBackpack() { return searchBackpack; }
    bool GetSearchBags() { return searchBags; }
    bool GetSellTradeGoods() { return sellTradeGoods; }
    bool GetSellQuestItems() { return sellQuestItems; }
};

#define sModUtils ModUtils::instance()

#endif
