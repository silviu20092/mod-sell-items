/*
 * Credits: silviu20092
 */

#include "Chat.h"
#include "ObjectMgr.h"
#include "Config.h"
#include "Player.h"
#include "Mod_Utils.h"

std::string SI_Utils::ToLower(const std::string& text)
{
    std::string result = text;
    std::transform(result.begin(), result.end(), result.begin(), tolower);
    return result;
}

uint32 SI_Utils::ColorToQuality(const std::string& color)
{
    if (color == sConfigMgr->GetOption<std::string>("SellItems.ItemQualityColorIdentifier.Poor", "grey"))
        return ITEM_QUALITY_POOR;
    else if (color == sConfigMgr->GetOption<std::string>("SellItems.ItemQualityColorIdentifier.Normal", "white"))
        return ITEM_QUALITY_NORMAL;
    else if (color == sConfigMgr->GetOption<std::string>("SellItems.ItemQualityColorIdentifier.Uncommon", "green"))
        return ITEM_QUALITY_UNCOMMON;
    else if (color == sConfigMgr->GetOption<std::string>("SellItems.ItemQualityColorIdentifier.Rare", "blue"))
        return ITEM_QUALITY_RARE;
    else if (color == sConfigMgr->GetOption<std::string>("SellItems.ItemQualityColorIdentifier.Epic", "epic"))
        return ITEM_QUALITY_EPIC;
    else if (color == sConfigMgr->GetOption<std::string>("SellItems.ItemQualityColorIdentifier.Legendary", "orange"))
        return ITEM_QUALITY_LEGENDARY;

    return MAX_ITEM_QUALITY;
}

std::string SI_Utils::ItemLink(ChatHandler* handler, const ItemTemplate* itemTemplate)
{
    LocaleConstant loc_idx = handler->GetSession()->GetSessionDbLocaleIndex();
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
