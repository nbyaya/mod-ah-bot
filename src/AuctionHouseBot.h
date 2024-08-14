/*
 * Copyright (C) 2008-2010 Trinity <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef AUCTION_HOUSE_BOT_H
#define AUCTION_HOUSE_BOT_H

#include "Common.h"
#include "ObjectGuid.h"
#include "AuctionHouseMgr.h"
#include "AuctionHouseBotCommon.h"
#include "AuctionHouseBotConfig.h"

struct AuctionEntry;
class Player;
class WorldSession;

class AuctionHouseBot
{
private:

    bool debug_Out;
    bool debug_Out_Config;
    bool debug_Out_Buyer;
    bool debug_Out_Seller;
    bool trace_Seller;
    bool debug_Out_Filters;

    bool AHBSeller;
    bool AHBBuyer;
    bool BuyMethod;
    bool SellMethod;
    bool ConsiderOnlyBotAuctions;

    uint32 ItemsPerCycle;
    uint32 AHBplayerAccount;

    ObjectGuid::LowType AHBplayerGUID;

    //
    // Begin Filters
    //

    bool Vendor_Items;
    bool Loot_Items;
    bool Other_Items;
    bool Vendor_TGs;
    bool Loot_TGs;
    bool Other_TGs;

    bool No_Bind;
    bool Bind_When_Picked_Up;
    bool Bind_When_Equipped;
    bool Bind_When_Use;
    bool Bind_Quest_Item;

    uint32 DuplicatesCount;
    uint32 ElapsingTimeClass;

    bool DivisibleStacks;
    bool DisablePermEnchant;
    bool DisableConjured;
    bool DisableGems;
    bool DisableMoney;
    bool DisableMoneyLoot;
    bool DisableLootable;
    bool DisableKeys;
    bool DisableDuration;
    bool DisableBOP_Or_Quest_NoReqLevel;

    bool DisableWarriorItems;
    bool DisablePaladinItems;
    bool DisableHunterItems;
    bool DisableRogueItems;
    bool DisablePriestItems;
    bool DisableDKItems;
    bool DisableShamanItems;
    bool DisableMageItems;
    bool DisableWarlockItems;
    bool DisableUnusedClassItems;
    bool DisableDruidItems;

    uint32 DisableItemsBelowLevel;
    uint32 DisableItemsAboveLevel;

    uint32 DisableTGsBelowLevel;
    uint32 DisableTGsAboveLevel;

    uint32 DisableItemsBelowGUID;
    uint32 DisableItemsAboveGUID;

    uint32 DisableTGsBelowGUID;
    uint32 DisableTGsAboveGUID;

    uint32 DisableItemsBelowReqLevel;
    uint32 DisableItemsAboveReqLevel;

    uint32 DisableTGsBelowReqLevel;
    uint32 DisableTGsAboveReqLevel;

    uint32 DisableItemsBelowReqSkillRank;
    uint32 DisableItemsAboveReqSkillRank;

    uint32 DisableTGsBelowReqSkillRank;
    uint32 DisableTGsAboveReqSkillRank;

    std::set<uint32> DisableItemStore;
    std::set<uint32> SellerWhiteList;

    //
    // End Filters
    //

    AHBConfig AllianceConfig;
    AHBConfig HordeConfig;
    AHBConfig NeutralConfig;

    time_t _lastrun_a_sec;
    time_t _lastrun_h_sec;
    time_t _lastrun_n_sec;

    //
    // Main operations
    //

    void Sell(Player *AHBplayer, AHBConfig *config);
    void Buy (Player *AHBplayer, AHBConfig *config, WorldSession *session);

    //
    // Utilities
    //

    inline uint32 minValue(uint32 a, uint32 b) { return a <= b ? a : b; };

    uint32 getNofAuctions(AuctionHouseObject* auctionHouse, ObjectGuid guid);
    uint32 getStackCount(uint32 max);
    uint32 getElapsedTime(uint32 timeClass);

    std::set<uint32> getCommaSeparatedIntegers(std::string text);

//    friend class ACE_Singleton<AuctionHouseBot, ACE_Null_Mutex>;
    AuctionHouseBot();

public:
    static AuctionHouseBot* instance()
    {
        static AuctionHouseBot instance;
        return &instance;
    }

    ~AuctionHouseBot();

    void Update();

    void Initialize();
    void InitializeConfiguration();
    void LoadValues(AHBConfig*);

    void DecrementItemCounts(AuctionEntry* ah);
    void IncrementItemCounts(AuctionEntry* ah);

    void Commands(AHBotCommand, uint32, uint32, char*);

    ObjectGuid::LowType GetAHBplayerGUID() { return AHBplayerGUID; };
};

#define auctionbot AuctionHouseBot::instance()

#endif
