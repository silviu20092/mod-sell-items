/*
 * Credits: silviu20092
 */

#ifndef _MOD_UTILS_H_
#define _MOD_UTILS_H_

#include <string>

class ChatHandler;
class Item;
class ItemTemplate;

class ModUtils
{
public:
    static std::string ToLower(const std::string& text);
    static uint32 ColorToQuality(const std::string& color);
    static std::string ItemLink(ChatHandler* handler, const ItemTemplate* itemTemplate);
};

#endif
