# ![logo](https://raw.githubusercontent.com/azerothcore/azerothcore.github.io/master/images/logo-github.png) AzerothCore

# Sell items mod

Automatically sell items based on quality. For example, .sell green will sell all green (uncommon) items from bags, bank, equipped (customizable via .conf). Only items with sell price > 0 will be sold and player will be given the correct amount of gold. Can be used to sell those annoying grey items.

There is also a Gossip NPC that can be used to sell items, entry is 200000. This npc also features a confirmation message before selling the items.

## How to install

Simply clone the repository and add mod-sell-items to the modules folder. Re-run cmake and rebuild the project. The .sql files should be imported automatically, if not then you should manually apply them.

## Credits
- silviu20092