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
public:
    static ModUtils* instance();

    std::string ToLower(const std::string& text);
    uint32 ColorToQuality(const std::string& color);
    std::string ItemLink(ChatHandler* handler, const ItemTemplate* itemTemplate);

    bool SellItemsOfQuality(Player* player, uint32 quality);

    const std::map<uint32, std::string>& GetItemQualityColorIdentifier() const
    {
        return itemQualityColorIdentifier;
    }
};

#define sModUtils ModUtils::instance()

#endif
