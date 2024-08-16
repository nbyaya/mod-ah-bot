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

#include "ObjectMgr.h"
#include "AuctionHouseBotCommon.h"
#include "AuctionHouseMgr.h"
#include "AuctionHouseBot.h"
#include "Config.h"
#include "Player.h"
#include "WorldSession.h"
#include "GameTime.h"
#include "DatabaseEnv.h"
#include <vector>

using namespace std;

//
// Bins for selected items to be tested
//

vector<uint32> npcItems;
vector<uint32> lootItems;

//
// Bins for usable trade goods items
//

vector<uint32> greyTradeGoodsBin;
vector<uint32> whiteTradeGoodsBin;
vector<uint32> greenTradeGoodsBin;
vector<uint32> blueTradeGoodsBin;
vector<uint32> purpleTradeGoodsBin;
vector<uint32> orangeTradeGoodsBin;
vector<uint32> yellowTradeGoodsBin;

//
// Bins for usable items
//

vector<uint32> greyItemsBin;
vector<uint32> whiteItemsBin;
vector<uint32> greenItemsBin;
vector<uint32> blueItemsBin;
vector<uint32> purpleItemsBin;
vector<uint32> orangeItemsBin;
vector<uint32> yellowItemsBin;

//
// Bins for whitelists
//

vector<uint32> sellerItems;

AuctionHouseBot::AuctionHouseBot()
{
    debug_Out                      = false;
    debug_Out_Config               = false;
    debug_Out_Filters              = false;
    debug_Out_Buyer                = false;
    debug_Out_Seller               = false;
    trace_Seller                   = false;
    AHBSeller                      = false;
    AHBBuyer                       = false;
    DuplicatesCount                = 0;
    DivisibleStacks                = false;
    ElapsingTimeClass              = 1;
    ConsiderOnlyBotAuctions        = false;

    // Begin Filters

    Vendor_Items                   = false;
    Loot_Items                     = false;
    Other_Items                    = false;
    Vendor_TGs                     = false;
    Loot_TGs                       = false;
    Other_TGs                      = false;

    No_Bind                        = false;
    Bind_When_Picked_Up            = false;
    Bind_When_Equipped             = false;
    Bind_When_Use                  = false;
    Bind_Quest_Item                = false;

    DisablePermEnchant             = false;
    DisableConjured                = false;
    DisableGems                    = false;
    DisableMoney                   = false;
    DisableMoneyLoot               = false;
    DisableLootable                = false;
    DisableKeys                    = false;
    DisableDuration                = false;
    DisableBOP_Or_Quest_NoReqLevel = false;

    DisableWarriorItems            = false;
    DisablePaladinItems            = false;
    DisableHunterItems             = false;
    DisableRogueItems              = false;
    DisablePriestItems             = false;
    DisableDKItems                 = false;
    DisableShamanItems             = false;
    DisableMageItems               = false;
    DisableWarlockItems            = false;
    DisableUnusedClassItems        = false;
    DisableDruidItems              = false;

    DisableItemsBelowLevel         = 0;
    DisableItemsAboveLevel         = 0;
    DisableTGsBelowLevel           = 0;
    DisableTGsAboveLevel           = 0;
    DisableItemsBelowGUID          = 0;
    DisableItemsAboveGUID          = 0;
    DisableTGsBelowGUID            = 0;
    DisableTGsAboveGUID            = 0;
    DisableItemsBelowReqLevel      = 0;
    DisableItemsAboveReqLevel      = 0;
    DisableTGsBelowReqLevel        = 0;
    DisableTGsAboveReqLevel        = 0;
    DisableItemsBelowReqSkillRank  = 0;
    DisableItemsAboveReqSkillRank  = 0;
    DisableTGsBelowReqSkillRank    = 0;
    DisableTGsAboveReqSkillRank    = 0;

    // End Filters

    _lastrun_a_sec                 = time(NULL);
    _lastrun_h_sec                 = time(NULL);
    _lastrun_n_sec                 = time(NULL);

    AllianceConfig                 = AHBConfig(2);
    HordeConfig                    = AHBConfig(6);
    NeutralConfig                  = AHBConfig(7);
}

AuctionHouseBot::~AuctionHouseBot()
{
    // Nothing
}

// =============================================================================
// Extracts integers from a string and return a set of them
// =============================================================================

std::set<uint32> AuctionHouseBot::getCommaSeparatedIntegers(std::string text)
{
    std::string       value;
    std::stringstream stream;
    std::set<uint32>  ret;

    stream.str(text);

    //
    // Continue to precess comma separated values
    //

    while (std::getline(stream, value, ','))
    {
        ret.insert(atoi(value.c_str()));
    }

    return ret;
}

// =============================================================================
// Returns a stack size depending on the configuration
// =============================================================================

uint32 AuctionHouseBot::getStackCount(uint32 max)
{
    if (max == 1)
    {
        return 1;
    }

    // 
    // Organize the stacks in a pseudo random way
    // 

    if (DivisibleStacks)
    {
        if (max % 5 == 0) // 5, 10, 15, 20
        {
            return urand(1, 4) * 5;
        }

        if (max % 4 == 0) // 4, 8, 12, 16
        {
            return urand(1, 4) * 4;
        }

        if (max % 3 == 0) // 3, 6, 9
        {
            return urand(1, 3) * 3;
        }
    }

    // 
    // Totally random
    // 

    return urand(1, max);
}

// =============================================================================
// Gets a random elapsing time depending on the provided class
// =============================================================================

uint32 AuctionHouseBot::getElapsedTime(uint32 timeClass)
{
    switch (timeClass)
    {
    case 2:
        return urand(1, 5) * 600;   // SHORT = In the range of one hour

    case 1:
        return urand(1, 23) * 3600; // MEDIUM = In the range of one day

    default:
        return urand(1, 3) * 86400; // LONG = More than one day but less than three
    }
}

// =============================================================================
// Gets the number of items actually present in the auction house
// =============================================================================

uint32 AuctionHouseBot::getNofAuctions(AuctionHouseObject* auctionHouse, ObjectGuid guid)
{
    //
    // All the auctions
    //

    if (!ConsiderOnlyBotAuctions)
    {
        return auctionHouse->Getcount();
    }

    //
    // Just the one handled by the bot
    //

    uint32 count = 0;

    for (AuctionHouseObject::AuctionEntryMap::const_iterator itr = auctionHouse->GetAuctionsBegin(); itr != auctionHouse->GetAuctionsEnd(); ++itr)
    {
        AuctionEntry* Aentry = itr->second;

        if (guid == Aentry->owner)
        {
            count++;
            break;
        }
    }

    return count;
}

// =============================================================================
// This routine performs the bidding operations for the bot
// =============================================================================

void AuctionHouseBot::Buy(Player* AHBplayer, AHBConfig* config, WorldSession* session)
{
    //
    // Check if disabled
    //

    if (!AHBBuyer)
    {
        if (debug_Out_Buyer)
        {
            LOG_ERROR("module", "AHBuyer: Disabled");
        }

        return;
    }

    //
    // Retrieve items not owner by the bot and not bought by the bot
    //

    QueryResult result = CharacterDatabase.Query("SELECT id FROM auctionhouse WHERE itemowner<>{} AND buyguid<>{}", AHBplayerGUID, AHBplayerGUID);

    if (!result)
    {
        return;
    }

    if (result->GetRowCount() == 0)
    {
        return;
    }

    //
    // Fetches content of selected AH to look for possible bids
    //

    AuctionHouseObject* auctionHouse = sAuctionMgr->GetAuctionsMap(config->GetAHFID());

    vector<uint32> possibleBids;

    do
    {
        uint32 tmpdata = result->Fetch()->Get<uint32>();
        possibleBids.push_back(tmpdata);
    } while (result->NextRow());

    //
    // If it's not possible to bid stop here
    //

    if (possibleBids.empty())
    {
        if (debug_Out_Buyer)
        {
            LOG_INFO("module", "No auctions to bid on has been recovered");
        }

        return;
    }

    //
    // Perform the operation for a maximum amount of bids configured
    //

    for (uint32 count = 1; count <= config->GetBidsPerInterval(); ++count)
    {
        //
        // Choose a random auction from possible auctions
        //

        uint32 vectorPos = urand(0, possibleBids.size() - 1);

        vector<uint32>::iterator iter = possibleBids.begin();
        advance(iter, vectorPos);

        AuctionEntry* auction = auctionHouse->GetAuction(*iter);

        //
        // Prevent to bid again on the same auction
        //

        possibleBids.erase(iter);

        if (!auction)
        {
            continue;
        }

        //
        // Get the item information
        //

        Item* pItem = sAuctionMgr->GetAItem(auction->item_guid);

        if (!pItem)
        {
            if (debug_Out_Buyer)
            {
                LOG_ERROR("module", "AHBuyer: Item {} doesn't exist, perhaps bought already?", auction->item_guid.ToString());
            }

            continue;
        }

        //
        // Get the item prototype
        //

        ItemTemplate const* prototype = sObjectMgr->GetItemTemplate(auction->item_template);

        //
        // Check which price we have to use, startbid or if it is bidded already
        //

        uint32 currentprice;

        if (auction->bid)
        {
            currentprice = auction->bid;
        }
        else
        {
            currentprice = auction->startbid;
        }

        //
        // Prepare portion from maximum bid
        //

        double      bidrate = static_cast<double>(urand(1, 100)) / 100;
        long double bidMax  = 0;

        //
        // Check that bid has an acceptable value and take bid based on vendorprice, stacksize and quality
        //

        if (BuyMethod)
        {
            if (prototype->Quality <= AHB_MAX_QUALITY)
            {
                if (currentprice < prototype->SellPrice * pItem->GetCount() * config->GetBuyerPrice(prototype->Quality))
                {
                    bidMax = prototype->SellPrice * pItem->GetCount() * config->GetBuyerPrice(prototype->Quality);
                }
            }
            else
            {
                if (debug_Out_Buyer)
                {
                    LOG_ERROR("module", "AHBuyer: Quality {} not Supported", prototype->Quality);
                }

                continue;
            }
        }
        else
        {
            if (prototype->Quality <= AHB_MAX_QUALITY)
            {
                if (currentprice < prototype->BuyPrice * pItem->GetCount() * config->GetBuyerPrice(prototype->Quality))
                {
                    bidMax = prototype->BuyPrice * pItem->GetCount() * config->GetBuyerPrice(prototype->Quality);
                }
            }
            else
            {
                if (debug_Out)
                {
                    LOG_ERROR("module", "AHBuyer: Quality {} not Supported", prototype->Quality);
                }

                continue;
            }
        }

        //
        // Recalculate the bid depending on the type of the item
        //

        switch (prototype->Class)
        {
            // ammo
        case 6:
            bidMax = 0;
            break;
        default:
            break;
        }

        //
        // Test the computed bid
        //

        if (bidMax == 0)
        {
            continue;
        }

        //
        // Calculate our bid
        //

        long double bidvalue = currentprice + ((bidMax - currentprice) * bidrate);
        uint32      bidprice = static_cast<uint32>(bidvalue);

        //
        // Check our bid is high enough to be valid. If not, correct it to minimum.
        //

        if ((currentprice + auction->GetAuctionOutBid()) > bidprice)
        {
            bidprice = currentprice + auction->GetAuctionOutBid();
        }

        if (debug_Out_Buyer)
        {
            LOG_INFO("module", "-------------------------------------------------");
            LOG_INFO("module", "AHBuyer: Info for Auction #{}:", auction->Id);
            LOG_INFO("module", "AHBuyer: AuctionHouse: {}", auction->GetHouseId());
            LOG_INFO("module", "AHBuyer: Owner: {}", auction->owner.ToString());
            LOG_INFO("module", "AHBuyer: Bidder: {}", auction->bidder.ToString());
            LOG_INFO("module", "AHBuyer: Starting Bid: {}", auction->startbid);
            LOG_INFO("module", "AHBuyer: Current Bid: {}", currentprice);
            LOG_INFO("module", "AHBuyer: Buyout: {}", auction->buyout);
            LOG_INFO("module", "AHBuyer: Deposit: {}", auction->deposit);
            LOG_INFO("module", "AHBuyer: Expire Time: {}", uint32(auction->expire_time));
            LOG_INFO("module", "AHBuyer: Bid Rate: {}", bidrate);
            LOG_INFO("module", "AHBuyer: Bid Max: {}", bidMax);
            LOG_INFO("module", "AHBuyer: Bid Value: {}", bidvalue);
            LOG_INFO("module", "AHBuyer: Bid Price: {}", bidprice);
            LOG_INFO("module", "AHBuyer: Item GUID: {}", auction->item_guid.ToString());
            LOG_INFO("module", "AHBuyer: Item Template: {}", auction->item_template);
            LOG_INFO("module", "AHBuyer: Item Info:");
            LOG_INFO("module", "AHBuyer: Item ID: {}", prototype->ItemId);
            LOG_INFO("module", "AHBuyer: Buy Price: {}", prototype->BuyPrice);
            LOG_INFO("module", "AHBuyer: Sell Price: {}", prototype->SellPrice);
            LOG_INFO("module", "AHBuyer: Bonding: {}", prototype->Bonding);
            LOG_INFO("module", "AHBuyer: Quality: {}", prototype->Quality);
            LOG_INFO("module", "AHBuyer: Item Level: {}", prototype->ItemLevel);
            LOG_INFO("module", "AHBuyer: Ammo Type: {}", prototype->AmmoType);
            LOG_INFO("module", "-------------------------------------------------");
        }

        //
        // Check whether we do normal bid, or buyout
        //

        if ((bidprice < auction->buyout) || (auction->buyout == 0))
        {
            //
            // Perform a new bid on the auction
            //

            if (auction->bidder)
            {
                if (auction->bidder != AHBplayer->GetGUID())
                {
                    //
                    // Mail to last bidder and return their money
                    //

                    auto trans = CharacterDatabase.BeginTransaction();

                    sAuctionMgr->SendAuctionOutbiddedMail(auction, bidprice, session->GetPlayer(), trans);
                    CharacterDatabase.CommitTransaction(trans);
                }
            }

            auction->bidder = AHBplayer->GetGUID();
            auction->bid    = bidprice;

            //
            // Save the auction into database
            //

            CharacterDatabase.Execute("UPDATE auctionhouse SET buyguid = '{}',lastbid = '{}' WHERE id = '{}'", auction->bidder.GetCounter(), auction->bid, auction->Id);
        }
        else
        {
            //
            // Perform the buyout
            //

            auto trans = CharacterDatabase.BeginTransaction();

            if ((auction->bidder) && (AHBplayer->GetGUID() != auction->bidder))
            {
                //
                // Send the mail to the last bidder
                //

                sAuctionMgr->SendAuctionOutbiddedMail(auction, auction->buyout, session->GetPlayer(), trans);
            }

            auction->bidder = AHBplayer->GetGUID();
            auction->bid    = auction->buyout;

            // 
            // Send mails to buyer & seller
            // 

            sAuctionMgr->SendAuctionSuccessfulMail(auction, trans);
            sAuctionMgr->SendAuctionWonMail(auction, trans);

            // 
            // Removes any trace of the item
            // 

            auction->DeleteFromDB(trans);

            sAuctionMgr->RemoveAItem(auction->item_guid);
            auctionHouse->RemoveAuction(auction);

            CharacterDatabase.CommitTransaction(trans);
        }
    }
}

// =============================================================================
// This routine performs the selling operations for the bot
// =============================================================================

void AuctionHouseBot::Sell(Player* AHBplayer, AHBConfig* config)
{
    // 
    // Check if disabled
    // 

    if (!AHBSeller)
    {
        return;
    }

    // 
    // Check the given limits
    // 

    uint32 minItems = config->GetMinItems();
    uint32 maxItems = config->GetMaxItems();

    if (maxItems == 0)
    {
        return;
    }

    // 
    // Retrieve the auction house situation
    // 

    AuctionHouseEntry const* ahEntry = sAuctionMgr->GetAuctionHouseEntry(config->GetAHFID());

    if (!ahEntry)
    {
        return;
    }

    AuctionHouseObject* auctionHouse = sAuctionMgr->GetAuctionsMap(config->GetAHFID());

    if (!auctionHouse)
    {
        return;
    }

    auctionHouse->Update();

    // 
    // Check if we are clear to proceed
    // 

    bool   aboveMin = false;
    bool   aboveMax = false;
    uint32 auctions = getNofAuctions(auctionHouse, AHBplayer->GetGUID());
    uint32 items    = 0;

    if (auctions >= minItems)
    {
        aboveMin = true;

        if (debug_Out_Seller)
        {
            LOG_ERROR("module", "AHSeller: Auctions above minimum");
        }

        return;
    }

    if (auctions >= maxItems)
    {
        aboveMax = true;

        if (debug_Out_Seller)
        {
            LOG_ERROR("module", "AHSeller: Auctions at or above maximum");
        }

        return;
    }

    if ((maxItems - auctions) >= ItemsPerCycle)
    {
        items = ItemsPerCycle;
    }
    else
    {
        items = (maxItems - auctions);
    }

    // 
    // Retrieve the configuration for this run
    // 

    uint32 greyTGcount   = config->GetMaximum(AHB_GREY_TG);
    uint32 whiteTGcount  = config->GetMaximum(AHB_WHITE_TG);
    uint32 greenTGcount  = config->GetMaximum(AHB_GREEN_TG);
    uint32 blueTGcount   = config->GetMaximum(AHB_BLUE_TG);
    uint32 purpleTGcount = config->GetMaximum(AHB_PURPLE_TG);
    uint32 orangeTGcount = config->GetMaximum(AHB_ORANGE_TG);
    uint32 yellowTGcount = config->GetMaximum(AHB_YELLOW_TG);

    uint32 greyIcount    = config->GetMaximum(AHB_GREY_I);
    uint32 whiteIcount   = config->GetMaximum(AHB_WHITE_I);
    uint32 greenIcount   = config->GetMaximum(AHB_GREEN_I);
    uint32 blueIcount    = config->GetMaximum(AHB_BLUE_I);
    uint32 purpleIcount  = config->GetMaximum(AHB_PURPLE_I);
    uint32 orangeIcount  = config->GetMaximum(AHB_ORANGE_I);
    uint32 yellowIcount  = config->GetMaximum(AHB_YELLOW_I);

    uint32 greyTGoods    = config->GetItemCounts(AHB_GREY_TG);
    uint32 whiteTGoods   = config->GetItemCounts(AHB_WHITE_TG);
    uint32 greenTGoods   = config->GetItemCounts(AHB_GREEN_TG);
    uint32 blueTGoods    = config->GetItemCounts(AHB_BLUE_TG);
    uint32 purpleTGoods  = config->GetItemCounts(AHB_PURPLE_TG);
    uint32 orangeTGoods  = config->GetItemCounts(AHB_ORANGE_TG);
    uint32 yellowTGoods  = config->GetItemCounts(AHB_YELLOW_TG);

    uint32 greyItems     = config->GetItemCounts(AHB_GREY_I);
    uint32 whiteItems    = config->GetItemCounts(AHB_WHITE_I);
    uint32 greenItems    = config->GetItemCounts(AHB_GREEN_I);
    uint32 blueItems     = config->GetItemCounts(AHB_BLUE_I);
    uint32 purpleItems   = config->GetItemCounts(AHB_PURPLE_I);
    uint32 orangeItems   = config->GetItemCounts(AHB_ORANGE_I);
    uint32 yellowItems   = config->GetItemCounts(AHB_YELLOW_I);

    //
    // Only insert a few at a time, so they dont appear all at once
    //

    uint32 noSold   = 0; // Tracing counter
    uint32 binEmpty = 0; // Tracing counter
    uint32 noNeed   = 0; // Tracing counter
    uint32 tooMany  = 0; // Tracing counter
    uint32 loopBrk  = 0; // Tracing counter
    uint32 err      = 0; // Tracing counter

    for (uint32 cnt = 1; cnt <= items; cnt++)
    {
        uint32 itemID      = 0;
        uint32 loopbreaker = 0;

        // 
        // Attempts for some times to insert a single item stack as an auction.
        // The attempt can be stopped by several internal checks.
        //

        while (itemID == 0 && loopbreaker <= 32)
        {
            ++loopbreaker;

            // 
            // Get a random item from the bins
            // 

            uint32 choice = urand(0, 13);

            switch (choice)
            {
            case AHB_GREY_I:
                if ((greyItemsBin.size() > 0) && (greyItems < greyIcount))
                {
                    itemID = greyItemsBin[urand(0, greyItemsBin.size() - 1)];
                }
                else
                {
                    noNeed++;
                    continue;
                }

                break;

            case AHB_WHITE_I:
                if ((whiteItemsBin.size() > 0) && (whiteItems < whiteIcount))
                {
                    itemID = whiteItemsBin[urand(0, whiteItemsBin.size() - 1)];
                }
                else
                {
                    noNeed++;
                    continue;
                }

                break;

            case AHB_GREEN_I:
                if ((greenItemsBin.size() > 0) && (greenItems < greenIcount))
                {
                    itemID = greenItemsBin[urand(0, greenItemsBin.size() - 1)];
                }
                else
                {
                    noNeed++;
                    continue;
                }

                break;

            case AHB_BLUE_I:
                if ((blueItemsBin.size() > 0) && (blueItems < blueIcount))
                {
                    itemID = blueItemsBin[urand(0, blueItemsBin.size() - 1)];
                }
                else
                {
                    noNeed++;
                    continue;
                }

                break;

            case AHB_PURPLE_I:
                if ((purpleItemsBin.size() > 0) && (purpleItems < purpleIcount))
                {
                    itemID = purpleItemsBin[urand(0, purpleItemsBin.size() - 1)];
                }
                else
                {
                    noNeed++;
                    continue;
                }

                break;

            case AHB_ORANGE_I:
                if ((orangeItemsBin.size() > 0) && (orangeItems < orangeIcount))
                {
                    itemID = orangeItemsBin[urand(0, orangeItemsBin.size() - 1)];
                }
                else
                {
                    noNeed++;
                    continue;
                }

                break;

            case AHB_YELLOW_I:
                if ((yellowItemsBin.size() > 0) && (yellowItems < yellowIcount))
                {
                    itemID = yellowItemsBin[urand(0, yellowItemsBin.size() - 1)];
                }
                else
                {
                    noNeed++;
                    continue;
                }

                break;

            case AHB_GREY_TG:
                if ((greyTradeGoodsBin.size() > 0) && (greyTGoods < greyTGcount))
                {
                    itemID = greyTradeGoodsBin[urand(0, greyTradeGoodsBin.size() - 1)];
                }
                else
                {
                    noNeed++;
                    continue;
                }

                break;

            case AHB_WHITE_TG:
                if ((whiteTradeGoodsBin.size() > 0) && (whiteTGoods < whiteTGcount))
                {
                    itemID = whiteTradeGoodsBin[urand(0, whiteTradeGoodsBin.size() - 1)];
                }
                else
                {
                    noNeed++;
                    continue;
                }

                break;

            case AHB_GREEN_TG:
                if ((greenTradeGoodsBin.size() > 0) && (greenTGoods < greenTGcount))
                {
                    itemID = greenTradeGoodsBin[urand(0, greenTradeGoodsBin.size() - 1)];
                }
                else
                {
                    noNeed++;
                    continue;
                }

                break;

            case AHB_BLUE_TG:
                if ((blueTradeGoodsBin.size() > 0) && (blueTGoods < blueTGcount))
                {
                    itemID = blueTradeGoodsBin[urand(0, blueTradeGoodsBin.size() - 1)];
                }
                else
                {
                    noNeed++;
                    continue;
                }

                break;

            case AHB_PURPLE_TG:
                if ((purpleTradeGoodsBin.size() > 0) && (purpleTGoods < purpleTGcount))
                {
                    itemID = purpleTradeGoodsBin[urand(0, purpleTradeGoodsBin.size() - 1)];
                }
                else
                {
                    noNeed++;
                    continue;
                }

                break;

            case AHB_ORANGE_TG:
                if ((orangeTradeGoodsBin.size() > 0) && (orangeTGoods < orangeTGcount))
                {
                    itemID = orangeTradeGoodsBin[urand(0, orangeTradeGoodsBin.size() - 1)];
                }
                else
                {
                    noNeed++;
                    continue;
                }

                break;

            case AHB_YELLOW_TG:
                if ((yellowTradeGoodsBin.size() > 0) && (yellowTGoods < yellowTGcount))
                {
                    itemID = yellowTradeGoodsBin[urand(0, yellowTradeGoodsBin.size() - 1)];
                }
                else
                {
                    noNeed++;
                    continue;
                }

                break;

            default:
                err++;

                if (debug_Out_Seller)
                {
                    LOG_ERROR("module", "AHSeller: itemID Switch - Default Reached");
                }

                break;
            }

            // 
            // Check if it's a valid selection
            // 

            if (itemID == 0)
            {
                binEmpty++;

                if (debug_Out_Seller)
                {
                    LOG_ERROR("module", "AHSeller: No item could be selected in the bin {}", choice);
                }

                continue;
            }

            // 
            // Check how many stacks of this items are sold by the bot.
            // This avoid unfortunate rolls to overwhelm the market with the same products on low population, especially with whitelists.
            // 

            if (DuplicatesCount > 0)
            {
                uint32 noStacks = 0;

                for (AuctionHouseObject::AuctionEntryMap::const_iterator itr = auctionHouse->GetAuctionsBegin(); itr != auctionHouse->GetAuctionsEnd(); ++itr)
                {
                    AuctionEntry* Aentry = itr->second;

                    if (AHBplayer->GetGUID() == Aentry->owner)
                    {
                        if (itemID == Aentry->item_template)
                        {
                            noStacks++;
                        }
                    }
                }

                if (noStacks >= DuplicatesCount)
                {
                    tooMany++;
                    continue;
                }
            }

            // 
            // Retrieve information about the selected item
            // 

            ItemTemplate const* prototype = sObjectMgr->GetItemTemplate(itemID);

            if (prototype == NULL)
            {
                err++;

                if (debug_Out_Seller)
                {
                    LOG_ERROR("module", "AHSeller: could not get prototype of item {}", itemID);
                }

                continue;
            }

            Item* item = Item::CreateItem(itemID, 1, AHBplayer);

            if (item == NULL)
            {
                err++;

                if (debug_Out_Seller)
                {
                    LOG_ERROR("module", "AHSeller: could not create item from prototype {}", itemID);
                }

                break;
            }

            // 
            // Start interacting with the item by adding a random property
            // 

            item->AddToUpdateQueueOf(AHBplayer);

            uint32 randomPropertyId = Item::GenerateItemRandomPropertyId(itemID);

            if (randomPropertyId != 0)
            {
                item->SetItemRandomProperties(randomPropertyId);
            }

            // 
            // Determine the price and stack size
            // 

            uint64 buyoutPrice = 0;
            uint64 bidPrice    = 0;
            uint32 stackCount  = 1;

            if (SellMethod)
            {
                buyoutPrice = prototype->BuyPrice;
            }
            else
            {
                buyoutPrice = prototype->SellPrice;
            }

            if (prototype->Quality <= AHB_MAX_QUALITY)
            {
                if (config->GetMaxStack(prototype->Quality) > 1 && item->GetMaxStackCount() > 1)
                {
                    stackCount = minValue(getStackCount(item->GetMaxStackCount()), config->GetMaxStack(prototype->Quality));
                }
                else if (config->GetMaxStack(prototype->Quality) == 0 && item->GetMaxStackCount() > 1)
                {
                    stackCount = getStackCount(item->GetMaxStackCount());
                }
                else
                {
                    stackCount = 1;
                }

                buyoutPrice *= urand(config->GetMinPrice(prototype->Quality), config->GetMaxPrice(prototype->Quality));
                buyoutPrice /= 100;
                bidPrice     = buyoutPrice * urand(config->GetMinBidPrice(prototype->Quality), config->GetMaxBidPrice(prototype->Quality));
                bidPrice    /= 100;
            }
            else
            {
                err++;

                if (debug_Out_Seller)
                {
                    LOG_ERROR("module", "AHSeller: Quality {} TOO HIGH for item {}", prototype->Quality, itemID);
                }

                item->RemoveFromUpdateQueueOf(AHBplayer);
                continue;
            }

            item->SetCount(stackCount);

            // 
            // Determine the auction time
            // 

            uint32 etime = getElapsedTime(ElapsingTimeClass);

            // 
            // Determine the deposit
            // 

            uint32 dep   = sAuctionMgr->GetAuctionDeposit(ahEntry, etime, item, stackCount);

            // 
            // Perform the auction
            // 

            auto trans = CharacterDatabase.BeginTransaction();

            AuctionEntry* auctionEntry      = new AuctionEntry();
            auctionEntry->Id                = sObjectMgr->GenerateAuctionID();
            auctionEntry->houseId           = config->GetAHID();
            auctionEntry->item_guid         = item->GetGUID();
            auctionEntry->item_template     = item->GetEntry();
            auctionEntry->itemCount         = item->GetCount();
            auctionEntry->owner             = AHBplayer->GetGUID();
            auctionEntry->startbid          = bidPrice * stackCount;
            auctionEntry->buyout            = buyoutPrice * stackCount;
            auctionEntry->bid               = 0;
            auctionEntry->deposit           = dep;
            auctionEntry->expire_time       = (time_t)etime + time(NULL);
            auctionEntry->auctionHouseEntry = ahEntry;

            item->SaveToDB(trans);
            item->RemoveFromUpdateQueueOf(AHBplayer);
            sAuctionMgr->AddAItem(item);
            auctionHouse->AddAuction(auctionEntry);
            auctionEntry->SaveToDB(trans);

            CharacterDatabase.CommitTransaction(trans);

            // 
            // Increments the number of items presents in the auction
            // 

            switch (choice)
            {
            case 0:
                ++greyItems;
                break;

            case 1:
                ++whiteItems;
                break;

            case 2:
                ++greenItems;
                break;

            case 3:
                ++blueItems;
                break;

            case 4:
                ++purpleItems;
                break;

            case 5:
                ++orangeItems;
                break;

            case 6:
                ++yellowItems;
                break;

            case 7:
                ++greyTGoods;
                break;

            case 8:
                ++whiteTGoods;
                break;

            case 9:
                ++greenTGoods;
                break;

            case 10:
                ++blueTGoods;
                break;

            case 11:
                ++purpleTGoods;
                break;

            case 12:
                ++orangeTGoods;
                break;

            case 13:
                ++yellowTGoods;
                break;

            default:
                break;
            }

            noSold++;

            if (trace_Seller)
            {
                LOG_INFO("module", "AHSeller: New stack ah={}, id={}, stack={}, bid={}, buyout={}", config->GetAHID(), itemID, stackCount, auctionEntry->startbid, auctionEntry->buyout);
            }
        }

        if (itemID == 0 || loopbreaker > 50)
        {
            loopBrk++;
        }
    }

    if (trace_Seller)
    {
        LOG_INFO("module", "AHSeller: auctionhouse {}, req={}, sold={}, aboveMin={}, aboveMax={}, loopBrk={}, noNeed={}, tooMany={}, binEmpty={}, err={}", config->GetAHID(), items, noSold, aboveMin, aboveMax, loopBrk, noNeed, tooMany, binEmpty, err);
    }
}

// =============================================================================
// Perform an update cycle
// =============================================================================

void AuctionHouseBot::Update()
{
    time_t _newrun = time(NULL);

    //
    // If there is no seller or buyer then terminate
    //

    if ((!AHBSeller) && (!AHBBuyer))
    {
        return;
    }

    //
    // Preprare for operation
    //

    std::string accountName = "AuctionHouseBot" + std::to_string(AHBplayerAccount);

    WorldSession _session(AHBplayerAccount, std::move(accountName), nullptr, SEC_PLAYER, sWorld->getIntConfig(CONFIG_EXPANSION), 0, LOCALE_enUS, 0, false, false, 0);

    Player _AHBplayer(&_session);
    _AHBplayer.Initialize(AHBplayerGUID);

    ObjectAccessor::AddObject(&_AHBplayer);

    //
    // Perform update for the factions markets
    //

    if (!sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION))
    {
        //
        // Alliance
        //

        Sell(&_AHBplayer, &AllianceConfig);

        if (((_newrun - _lastrun_a_sec) >= (AllianceConfig.GetBiddingInterval() * MINUTE)) && (AllianceConfig.GetBidsPerInterval() > 0))
        {
            Buy(&_AHBplayer, &AllianceConfig, &_session);
            _lastrun_a_sec = _newrun;
        }

        //
        // Horde
        //

        Sell(&_AHBplayer, &HordeConfig);

        if (((_newrun - _lastrun_h_sec) >= (HordeConfig.GetBiddingInterval() * MINUTE)) && (HordeConfig.GetBidsPerInterval() > 0))
        {
            Buy(&_AHBplayer, &HordeConfig, &_session);
            _lastrun_h_sec = _newrun;
        }
    }

    //
    // Neutral
    //

    Sell(&_AHBplayer, &NeutralConfig);

    if (((_newrun - _lastrun_n_sec) >= (NeutralConfig.GetBiddingInterval() * MINUTE)) && (NeutralConfig.GetBidsPerInterval() > 0))
    {
        Buy(&_AHBplayer, &NeutralConfig, &_session);
        _lastrun_n_sec = _newrun;
    }

    ObjectAccessor::RemoveObject(&_AHBplayer);
}

// =============================================================================
// Increments the item count
// =============================================================================

void AuctionHouseBot::IncrementItemCounts(AuctionEntry* ah)
{
    // 
    // Consider only those auctions handled by this bot
    // 

    if (ConsiderOnlyBotAuctions)
    {
        // ObjectGuid playerGuid(HighGuid::Player, 0, AHBplayerGUID);

        if (AHBplayerGUID != ah->owner.GetCounter())
        {
            return;
        }
    }

    //
    // Get exact item information
    //

    Item *pItem = sAuctionMgr->GetAItem(ah->item_guid);

    if (!pItem)
    {
        if (debug_Out)
        {
            LOG_ERROR("module", "AHBot: Item {} doesn't exist, perhaps bought already?", ah->item_guid.ToString());
        }

        return;
    }

    // 
    // Get item prototype
    // 

    ItemTemplate const* prototype = sObjectMgr->GetItemTemplate(ah->item_template);

    // 
    // The the configuration for the auction house
    // 

    AHBConfig *config;

    AuctionHouseEntry const* ahEntry = sAuctionHouseStore.LookupEntry(ah->GetHouseId());

    if (!ahEntry)
    {
        if (ahEntry->houseId == AUCTIONHOUSE_ALLIANCE)
        {
            if (debug_Out)
            {
                LOG_INFO("module", "AHBot: {} returned as House Faction. Alliance", ah->GetHouseId());
            }

            config = &AllianceConfig;
        }
        else if (ahEntry->houseId == AUCTIONHOUSE_HORDE)
        {
            if (debug_Out)
            {
                LOG_INFO("module", "AHBot: {} returned as House Faction. Horde", ah->GetHouseId());
            }

            config = &HordeConfig;
        }
        else
        {
            if (debug_Out)
            {
                LOG_INFO("module", "AHBot: {} returned as House Faction. Neutral", ah->GetHouseId());
            }

            config = &NeutralConfig;
        }
    }
    else
    {
        if (debug_Out)
        {
            LOG_INFO("module", "AHBot: {} returned as House Faction. Neutral", ah->GetHouseId());
        }

        config = &NeutralConfig;
    }

    // 
    // Increments
    // 

    config->IncItemCounts(prototype->Class, prototype->Quality);
}

// =============================================================================
// Decrements the item count
// =============================================================================

void AuctionHouseBot::DecrementItemCounts(AuctionEntry* ah)
{
    // 
    // Consider only those auctions handled by this bot
    // 

    if (ConsiderOnlyBotAuctions)
    {
        // ObjectGuid playerGuid(HighGuid::Player, 0, AHBplayerGUID);

        if (AHBplayerGUID != ah->owner.GetCounter())
        {
            return;
        }
    }

    //
    // Get exact item information
    //

    Item* pItem = sAuctionMgr->GetAItem(ah->item_guid);

    if (!pItem)
    {
        if (debug_Out)
        {
            LOG_ERROR("module", "AHBot: Item {} doesn't exist, perhaps bought already?", ah->item_guid.ToString());
        }

        return;
    }

    //
    // Get item prototype
    //

    ItemTemplate const* prototype = sObjectMgr->GetItemTemplate(ah->item_template);

    // 
    // The the configuration for the auction house
    // 

    AHBConfig* config;

    AuctionHouseEntry const* ahEntry = sAuctionHouseStore.LookupEntry(ah->GetHouseId());

    if (!ahEntry)
    {
        if (ahEntry->houseId == AUCTIONHOUSE_ALLIANCE)
        {
            if (debug_Out)
            {
                LOG_INFO("module", "AHBot: {} returned as House Faction. Alliance", ah->GetHouseId());
            }

            config = &AllianceConfig;
        }
        else if (ahEntry->houseId == AUCTIONHOUSE_HORDE)
        {
            if (debug_Out)
            {
                LOG_INFO("module", "AHBot: {} returned as House Faction. Horde", ah->GetHouseId());
            }

            config = &HordeConfig;
        }
        else
        {
            if (debug_Out)
            {
                LOG_INFO("module", "AHBot: {} returned as House Faction. Neutral", ah->GetHouseId());
            }

            config = &NeutralConfig;
        }
    }
    else
    {
        if (debug_Out)
        {
            LOG_INFO("module", "AHBot: {} returned as House Faction. Neutral", ah->GetHouseId());
        }

        config = &NeutralConfig;
    }

    //
    // Decrements
    //

    config->DecItemCounts(prototype->Class, prototype->Quality);
}

// =============================================================================
// Execute commands coming from the console
// =============================================================================

void AuctionHouseBot::Commands(AHBotCommand command, uint32 ahMapID, uint32 col, char* args)
{
    //
    // Retrieve the auction house configuration
    //

    AHBConfig *config = NULL;

    switch (ahMapID)
    {
    case 2:
        config = &AllianceConfig;
        break;
    case 6:
        config = &HordeConfig;
        break;
    case 7:
        config = &NeutralConfig;
        break;
    }

    //
    // Retrive the item quality
    //

    std::string color;

    switch (col)
    {
    case AHB_GREY:
        color = "grey";
        break;
    case AHB_WHITE:
        color = "white";
        break;
    case AHB_GREEN:
        color = "green";
        break;
    case AHB_BLUE:
        color = "blue";
        break;
    case AHB_PURPLE:
        color = "purple";
        break;
    case AHB_ORANGE:
        color = "orange";
        break;
    case AHB_YELLOW:
        color = "yellow";
        break;
    default:
        break;
    }

    //
    // Perform the command
    //

    switch (command)
    {
    case AHBotCommand::buyer:
    {
        char* param1 = strtok(args, " ");
        uint32 state = (uint32)strtoul(param1, NULL, 0);

        if (state == 0)
        {
            AHBBuyer = false;
            LOG_ERROR("module", "AHBot: Buyer disabled from console");
        }
        else
        {
            AHBBuyer = true;
            LOG_ERROR("module", "AHBot: Buyer enabled from console");
        }

        break;
    }
    case AHBotCommand::seller:
    {
        char* param1 = strtok(args, " ");
        uint32 state = (uint32)strtoul(param1, NULL, 0);

        if (state == 0)
        {
            AHBSeller = false;
            LOG_ERROR("module", "AHBot: Seller disabled from console");
        }
        else
        {
            AHBSeller = true;
            LOG_ERROR("module", "AHBot: Seller enabled from console");
        }

        break;
    }
    case AHBotCommand::ahexpire:
    {
        ObjectGuid          playerGuid(HighGuid::Player, 0, AHBplayerGUID);
        AuctionHouseObject* auctionHouse =  sAuctionMgr->GetAuctionsMap(config->GetAHFID());

        AuctionHouseObject::AuctionEntryMap::iterator itr;
        itr = auctionHouse->GetAuctionsBegin();

        while (itr != auctionHouse->GetAuctionsEnd())
        {
            if (itr->second->owner == playerGuid)
            {
                // Expired NOW.
                itr->second->expire_time = GameTime::GetGameTime().count();

                uint32 id                = itr->second->Id;
                uint32 expire_time       = itr->second->expire_time;

                CharacterDatabase.Execute("UPDATE auctionhouse SET time = '{}' WHERE id = '{}'", expire_time, id);
            }

            ++itr;
        }

        break;
    }

    case AHBotCommand::minitems:
    {
        char * param1   = strtok(args, " ");
        uint32 minItems = (uint32) strtoul(param1, NULL, 0);

        WorldDatabase.Execute("UPDATE mod_auctionhousebot SET minitems = '{}' WHERE auctionhouse = '{}'", minItems, ahMapID);

        config->SetMinItems(minItems);

        break;
    }
    case AHBotCommand::maxitems:
    {
        char * param1   = strtok(args, " ");
        uint32 maxItems = (uint32) strtoul(param1, NULL, 0);

        WorldDatabase.Execute("UPDATE mod_auctionhousebot SET maxitems = '{}' WHERE auctionhouse = '{}'", maxItems, ahMapID);

        config->SetMaxItems(maxItems);
        config->CalculatePercents();
        break;
    }
    case AHBotCommand::percentages:
    {
        char * param1   = strtok(args, " ");
        char * param2   = strtok(NULL, " ");
        char * param3   = strtok(NULL, " ");
        char * param4   = strtok(NULL, " ");
        char * param5   = strtok(NULL, " ");
        char * param6   = strtok(NULL, " ");
        char * param7   = strtok(NULL, " ");
        char * param8   = strtok(NULL, " ");
        char * param9   = strtok(NULL, " ");
        char * param10  = strtok(NULL, " ");
        char * param11  = strtok(NULL, " ");
        char * param12  = strtok(NULL, " ");
        char * param13  = strtok(NULL, " ");
        char * param14  = strtok(NULL, " ");

        uint32 greytg   = (uint32) strtoul(param1, NULL, 0);
        uint32 whitetg  = (uint32) strtoul(param2, NULL, 0);
        uint32 greentg  = (uint32) strtoul(param3, NULL, 0);
        uint32 bluetg   = (uint32) strtoul(param4, NULL, 0);
        uint32 purpletg = (uint32) strtoul(param5, NULL, 0);
        uint32 orangetg = (uint32) strtoul(param6, NULL, 0);
        uint32 yellowtg = (uint32) strtoul(param7, NULL, 0);
        uint32 greyi    = (uint32) strtoul(param8, NULL, 0);
        uint32 whitei   = (uint32) strtoul(param9, NULL, 0);
        uint32 greeni   = (uint32) strtoul(param10, NULL, 0);
        uint32 bluei    = (uint32) strtoul(param11, NULL, 0);
        uint32 purplei  = (uint32) strtoul(param12, NULL, 0);
        uint32 orangei  = (uint32) strtoul(param13, NULL, 0);
        uint32 yellowi  = (uint32) strtoul(param14, NULL, 0);

        auto trans = WorldDatabase.BeginTransaction();

        trans->Append("UPDATE mod_auctionhousebot SET percentgreytradegoods = '{}'   WHERE auctionhouse = '{}'", greytg, ahMapID);
        trans->Append("UPDATE mod_auctionhousebot SET percentwhitetradegoods = '{}'  WHERE auctionhouse = '{}'", whitetg, ahMapID);
        trans->Append("UPDATE mod_auctionhousebot SET percentgreentradegoods = '{}'  WHERE auctionhouse = '{}'", greentg, ahMapID);
        trans->Append("UPDATE mod_auctionhousebot SET percentbluetradegoods = '{}'   WHERE auctionhouse = '{}'", bluetg, ahMapID);
        trans->Append("UPDATE mod_auctionhousebot SET percentpurpletradegoods = '{}' WHERE auctionhouse = '{}'", purpletg, ahMapID);
        trans->Append("UPDATE mod_auctionhousebot SET percentorangetradegoods = '{}' WHERE auctionhouse = '{}'", orangetg, ahMapID);
        trans->Append("UPDATE mod_auctionhousebot SET percentyellowtradegoods = '{}' WHERE auctionhouse = '{}'", yellowtg, ahMapID);
        trans->Append("UPDATE mod_auctionhousebot SET percentgreyitems = '{}'        WHERE auctionhouse = '{}'", greyi, ahMapID);
        trans->Append("UPDATE mod_auctionhousebot SET percentwhiteitems = '{}'       WHERE auctionhouse = '{}'", whitei, ahMapID);
        trans->Append("UPDATE mod_auctionhousebot SET percentgreenitems = '{}'       WHERE auctionhouse = '{}'", greeni, ahMapID);
        trans->Append("UPDATE mod_auctionhousebot SET percentblueitems = '{}'        WHERE auctionhouse = '{}'", bluei, ahMapID);
        trans->Append("UPDATE mod_auctionhousebot SET percentpurpleitems = '{}'      WHERE auctionhouse = '{}'", purplei, ahMapID);
        trans->Append("UPDATE mod_auctionhousebot SET percentorangeitems = '{}'      WHERE auctionhouse = '{}'", orangei, ahMapID);
        trans->Append("UPDATE mod_auctionhousebot SET percentyellowitems = '{}'      WHERE auctionhouse = '{}'", yellowi, ahMapID);

        WorldDatabase.CommitTransaction(trans);

        config->SetPercentages(greytg, whitetg, greentg, bluetg, purpletg, orangetg, yellowtg, greyi, whitei, greeni, bluei, purplei, orangei, yellowi);

        break;
    }
    case AHBotCommand::minprice:
    {
        char * param1   = strtok(args, " ");
        uint32 minPrice = (uint32) strtoul(param1, NULL, 0);

        WorldDatabase.Execute("UPDATE mod_auctionhousebot SET minprice{} = '{}' WHERE auctionhouse = '{}'", color, minPrice, ahMapID);

        config->SetMinPrice(col, minPrice);

        break;
    }
    case AHBotCommand::maxprice:
    {
        char * param1   = strtok(args, " ");
        uint32 maxPrice = (uint32) strtoul(param1, NULL, 0);

        WorldDatabase.Execute("UPDATE mod_auctionhousebot SET maxprice{} = '{}' WHERE auctionhouse = '{}'", color, maxPrice, ahMapID);

        config->SetMaxPrice(col, maxPrice);

        break;
    }
    case AHBotCommand::minbidprice:
    {
        char * param1      = strtok(args, " ");
        uint32 minBidPrice = (uint32) strtoul(param1, NULL, 0);

        WorldDatabase.Execute("UPDATE mod_auctionhousebot SET minbidprice{} = '{}' WHERE auctionhouse = '{}'", color, minBidPrice, ahMapID);

        config->SetMinBidPrice(col, minBidPrice);

        break;
    }
    case AHBotCommand::maxbidprice:
    {
        char * param1      = strtok(args, " ");
        uint32 maxBidPrice = (uint32) strtoul(param1, NULL, 0);

        WorldDatabase.Execute("UPDATE mod_auctionhousebot SET maxbidprice{} = '{}' WHERE auctionhouse = '{}'", color, maxBidPrice, ahMapID);

        config->SetMaxBidPrice(col, maxBidPrice);

        break;
    }
    case AHBotCommand::maxstack:
    {
        char * param1   = strtok(args, " ");
        uint32 maxStack = (uint32) strtoul(param1, NULL, 0);

        WorldDatabase.Execute("UPDATE mod_auctionhousebot SET maxstack{} = '{}' WHERE auctionhouse = '{}'", color, maxStack, ahMapID);

        config->SetMaxStack(col, maxStack);

        break;
    }
    case AHBotCommand::buyerprice:
    {
        char * param1     = strtok(args, " ");
        uint32 buyerPrice = (uint32) strtoul(param1, NULL, 0);

        WorldDatabase.Execute("UPDATE mod_auctionhousebot SET buyerprice{} = '{}' WHERE auctionhouse = '{}'", color, buyerPrice, ahMapID);

        config->SetBuyerPrice(col, buyerPrice);

        break;
    }
    case AHBotCommand::bidinterval:
    {
        char * param1      = strtok(args, " ");
        uint32 bidInterval = (uint32) strtoul(param1, NULL, 0);

        WorldDatabase.Execute("UPDATE mod_auctionhousebot SET buyerbiddinginterval = '{}' WHERE auctionhouse = '{}'", bidInterval, ahMapID);

        config->SetBiddingInterval(bidInterval);

        break;
    }
    case AHBotCommand::bidsperinterval:
    {
        char * param1          = strtok(args, " ");
        uint32 bidsPerInterval = (uint32) strtoul(param1, NULL, 0);

        WorldDatabase.Execute("UPDATE mod_auctionhousebot SET buyerbidsperinterval = '{}' WHERE auctionhouse = '{}'", bidsPerInterval, ahMapID);

        config->SetBidsPerInterval(bidsPerInterval);

        break;
    }
    default:
        break;
    }
}

// =============================================================================
// Loads options from the configuration file
// =============================================================================

void AuctionHouseBot::InitializeConfiguration()
{
    debug_Out                      = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DEBUG", false);
    debug_Out_Config               = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DEBUG_CONFIG", false);
    debug_Out_Filters              = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DEBUG_FILTERS", false);
    debug_Out_Buyer                = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DEBUG_BUYER", false);
    debug_Out_Seller               = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DEBUG_SELLER", false);
    trace_Seller                   = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.TRACE_SELLER", false);

    AHBSeller                      = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.EnableSeller", false);
    AHBBuyer                       = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.EnableBuyer", false);
    SellMethod                     = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.UseBuyPriceForSeller", false);
    BuyMethod                      = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.UseBuyPriceForBuyer", false);
    DuplicatesCount                = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DuplicatesCount", 0);
    DivisibleStacks                = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DivisibleStacks", false);
    ElapsingTimeClass              = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DuplicatesCount", 1);
    ConsiderOnlyBotAuctions        = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.ConsiderOnlyBotAuctions", false);

    AHBplayerAccount               = sConfigMgr->GetOption<uint32>("AuctionHouseBot.Account", 0);
    AHBplayerGUID                  = sConfigMgr->GetOption<uint32>("AuctionHouseBot.GUID", 0);
    ItemsPerCycle                  = sConfigMgr->GetOption<uint32>("AuctionHouseBot.ItemsPerCycle", 200);

    //
    // Flags: item types
    //

    Vendor_Items                   = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.VendorItems", false);
    Loot_Items                     = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.LootItems", true);
    Other_Items                    = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.OtherItems", false);
    Vendor_TGs                     = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.VendorTradeGoods", false);
    Loot_TGs                       = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.LootTradeGoods", true);
    Other_TGs                      = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.OtherTradeGoods", false);

    //
    // Flags: items binding
    //

    No_Bind                        = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.No_Bind", true);
    Bind_When_Picked_Up            = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.Bind_When_Picked_Up", false);
    Bind_When_Equipped             = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.Bind_When_Equipped", true);
    Bind_When_Use                  = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.Bind_When_Use", true);
    Bind_Quest_Item                = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.Bind_Quest_Item", false);

    //
    // Flags: misc
    //

    DisableConjured                = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DisableConjured", false);
    DisableGems                    = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DisableGems", false);
    DisableMoney                   = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DisableMoney", false);
    DisableMoneyLoot               = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DisableMoneyLoot", false);
    DisableLootable                = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DisableLootable", false);
    DisableKeys                    = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DisableKeys", false);
    DisableDuration                = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DisableDuration", false);
    DisableBOP_Or_Quest_NoReqLevel = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DisableBOP_Or_Quest_NoReqLevel", false);

    //
    // Flags: items per class
    //

    DisableWarriorItems            = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DisableWarriorItems", false);
    DisablePaladinItems            = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DisablePaladinItems", false);
    DisableHunterItems             = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DisableHunterItems", false);
    DisableRogueItems              = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DisableRogueItems", false);
    DisablePriestItems             = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DisablePriestItems", false);
    DisableDKItems                 = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DisableDKItems", false);
    DisableShamanItems             = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DisableShamanItems", false);
    DisableMageItems               = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DisableMageItems", false);
    DisableWarlockItems            = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DisableWarlockItems", false);
    DisableUnusedClassItems        = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DisableUnusedClassItems", false);
    DisableDruidItems              = sConfigMgr->GetOption<bool>  ("AuctionHouseBot.DisableDruidItems", false);

    //
    // Items level and skills
    //

    DisableItemsBelowLevel         = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableItemsBelowLevel", 0);
    DisableItemsAboveLevel         = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableItemsAboveLevel", 0);
    DisableItemsBelowGUID          = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableItemsBelowGUID", 0);
    DisableItemsAboveGUID          = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableItemsAboveGUID", 0);
    DisableItemsBelowReqLevel      = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableItemsBelowReqLevel", 0);
    DisableItemsAboveReqLevel      = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableItemsAboveReqLevel", 0);
    DisableItemsBelowReqSkillRank  = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableItemsBelowReqSkillRank", 0);
    DisableItemsAboveReqSkillRank  = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableItemsAboveReqSkillRank", 0);

    //
    // Trade goods level and skills
    //

    DisableTGsBelowLevel           = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableTGsBelowLevel", 0);
    DisableTGsAboveLevel           = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableTGsAboveLevel", 0);
    DisableTGsBelowGUID            = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableTGsBelowGUID", 0);
    DisableTGsAboveGUID            = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableTGsAboveGUID", 0);
    DisableTGsBelowReqLevel        = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableTGsBelowReqLevel", 0);
    DisableTGsAboveReqLevel        = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableTGsAboveReqLevel", 0);
    DisableTGsBelowReqSkillRank    = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableTGsBelowReqSkillRank", 0);
    DisableTGsAboveReqSkillRank    = sConfigMgr->GetOption<uint32>("AuctionHouseBot.DisableTGsAboveReqSkillRank", 0);

    //
    // Whitelists
    //

    SellerWhiteList                = getCommaSeparatedIntegers(sConfigMgr->GetOption<std::string>("AuctionHouseBot.SellerWhiteList", ""));
}

// =============================================================================
// Loads options from the database
// =============================================================================

void AuctionHouseBot::LoadValues(AHBConfig* config)
{
    //
    // Auctions seller
    //

    if (AHBSeller)
    {
        //
        // Load min and max items
        //

        config->SetMinItems(WorldDatabase.Query("SELECT minitems FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMaxItems(WorldDatabase.Query("SELECT maxitems FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());

        //
        // Load percentages
        //

        uint32 greytg   = WorldDatabase.Query("SELECT percentgreytradegoods   FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>();
        uint32 whitetg  = WorldDatabase.Query("SELECT percentwhitetradegoods  FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>();
        uint32 greentg  = WorldDatabase.Query("SELECT percentgreentradegoods  FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>();
        uint32 bluetg   = WorldDatabase.Query("SELECT percentbluetradegoods   FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>();
        uint32 purpletg = WorldDatabase.Query("SELECT percentpurpletradegoods FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>();
        uint32 orangetg = WorldDatabase.Query("SELECT percentorangetradegoods FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>();
        uint32 yellowtg = WorldDatabase.Query("SELECT percentyellowtradegoods FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>();

        uint32 greyi    = WorldDatabase.Query("SELECT percentgreyitems        FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>();
        uint32 whitei   = WorldDatabase.Query("SELECT percentwhiteitems       FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>();
        uint32 greeni   = WorldDatabase.Query("SELECT percentgreenitems       FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>();
        uint32 bluei    = WorldDatabase.Query("SELECT percentblueitems        FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>();
        uint32 purplei  = WorldDatabase.Query("SELECT percentpurpleitems      FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>();
        uint32 orangei  = WorldDatabase.Query("SELECT percentorangeitems      FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>();
        uint32 yellowi  = WorldDatabase.Query("SELECT percentyellowitems      FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>();

        config->SetPercentages(greytg, whitetg, greentg, bluetg, purpletg, orangetg, yellowtg, greyi, whitei, greeni, bluei, purplei, orangei, yellowi);

        // 
        // Load min and max prices
        // 

        config->SetMinPrice(AHB_GREY  , WorldDatabase.Query("SELECT minpricegrey   FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMaxPrice(AHB_GREY  , WorldDatabase.Query("SELECT maxpricegrey   FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMinPrice(AHB_WHITE , WorldDatabase.Query("SELECT minpricewhite  FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMaxPrice(AHB_WHITE , WorldDatabase.Query("SELECT maxpricewhite  FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMinPrice(AHB_GREEN , WorldDatabase.Query("SELECT minpricegreen  FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMaxPrice(AHB_GREEN , WorldDatabase.Query("SELECT maxpricegreen  FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMinPrice(AHB_BLUE  , WorldDatabase.Query("SELECT minpriceblue   FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMaxPrice(AHB_BLUE  , WorldDatabase.Query("SELECT maxpriceblue   FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMinPrice(AHB_PURPLE, WorldDatabase.Query("SELECT minpricepurple FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMaxPrice(AHB_PURPLE, WorldDatabase.Query("SELECT maxpricepurple FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMinPrice(AHB_ORANGE, WorldDatabase.Query("SELECT minpriceorange FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMaxPrice(AHB_ORANGE, WorldDatabase.Query("SELECT maxpriceorange FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMinPrice(AHB_YELLOW, WorldDatabase.Query("SELECT minpriceyellow FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMaxPrice(AHB_YELLOW, WorldDatabase.Query("SELECT maxpriceyellow FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());

        // 
        // Load min and max bid prices
        // 

        config->SetMinBidPrice(AHB_GREY  , WorldDatabase.Query("SELECT minbidpricegrey   FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMaxBidPrice(AHB_GREY  , WorldDatabase.Query("SELECT maxbidpricegrey   FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMinBidPrice(AHB_WHITE , WorldDatabase.Query("SELECT minbidpricewhite  FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMaxBidPrice(AHB_WHITE , WorldDatabase.Query("SELECT maxbidpricewhite  FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMinBidPrice(AHB_GREEN , WorldDatabase.Query("SELECT minbidpricegreen  FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMaxBidPrice(AHB_GREEN , WorldDatabase.Query("SELECT maxbidpricegreen  FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMinBidPrice(AHB_BLUE  , WorldDatabase.Query("SELECT minbidpriceblue   FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMaxBidPrice(AHB_BLUE  , WorldDatabase.Query("SELECT maxbidpriceblue   FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMinBidPrice(AHB_PURPLE, WorldDatabase.Query("SELECT minbidpricepurple FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMaxBidPrice(AHB_PURPLE, WorldDatabase.Query("SELECT maxbidpricepurple FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMinBidPrice(AHB_ORANGE, WorldDatabase.Query("SELECT minbidpriceorange FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMaxBidPrice(AHB_ORANGE, WorldDatabase.Query("SELECT maxbidpriceorange FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMinBidPrice(AHB_YELLOW, WorldDatabase.Query("SELECT minbidpriceyellow FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMaxBidPrice(AHB_YELLOW, WorldDatabase.Query("SELECT maxbidpriceyellow FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());

        // 
        // Load max stacks
        // 

        config->SetMaxStack(AHB_GREY  , WorldDatabase.Query("SELECT maxstackgrey   FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMaxStack(AHB_WHITE , WorldDatabase.Query("SELECT maxstackwhite  FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMaxStack(AHB_GREEN , WorldDatabase.Query("SELECT maxstackgreen  FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMaxStack(AHB_BLUE  , WorldDatabase.Query("SELECT maxstackblue   FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMaxStack(AHB_PURPLE, WorldDatabase.Query("SELECT maxstackpurple FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMaxStack(AHB_ORANGE, WorldDatabase.Query("SELECT maxstackorange FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetMaxStack(AHB_YELLOW, WorldDatabase.Query("SELECT maxstackyellow FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());

        if (debug_Out_Config)
        {
            LOG_INFO("module", "Settings for Auctionhouse {}", config->GetAHID());
            LOG_INFO("module", "minItems                = {}", config->GetMinItems());
            LOG_INFO("module", "maxItems                = {}", config->GetMaxItems());
            LOG_INFO("module", "percentGreyTradeGoods   = {}", config->GetPercentages(AHB_GREY_TG));
            LOG_INFO("module", "percentWhiteTradeGoods  = {}", config->GetPercentages(AHB_WHITE_TG));
            LOG_INFO("module", "percentGreenTradeGoods  = {}", config->GetPercentages(AHB_GREEN_TG));
            LOG_INFO("module", "percentBlueTradeGoods   = {}", config->GetPercentages(AHB_BLUE_TG));
            LOG_INFO("module", "percentPurpleTradeGoods = {}", config->GetPercentages(AHB_PURPLE_TG));
            LOG_INFO("module", "percentOrangeTradeGoods = {}", config->GetPercentages(AHB_ORANGE_TG));
            LOG_INFO("module", "percentYellowTradeGoods = {}", config->GetPercentages(AHB_YELLOW_TG));
            LOG_INFO("module", "percentGreyItems        = {}", config->GetPercentages(AHB_GREY_I));
            LOG_INFO("module", "percentWhiteItems       = {}", config->GetPercentages(AHB_WHITE_I));
            LOG_INFO("module", "percentGreenItems       = {}", config->GetPercentages(AHB_GREEN_I));
            LOG_INFO("module", "percentBlueItems        = {}", config->GetPercentages(AHB_BLUE_I));
            LOG_INFO("module", "percentPurpleItems      = {}", config->GetPercentages(AHB_PURPLE_I));
            LOG_INFO("module", "percentOrangeItems      = {}", config->GetPercentages(AHB_ORANGE_I));
            LOG_INFO("module", "percentYellowItems      = {}", config->GetPercentages(AHB_YELLOW_I));
            LOG_INFO("module", "minPriceGrey            = {}", config->GetMinPrice(AHB_GREY));
            LOG_INFO("module", "maxPriceGrey            = {}", config->GetMaxPrice(AHB_GREY));
            LOG_INFO("module", "minPriceWhite           = {}", config->GetMinPrice(AHB_WHITE));
            LOG_INFO("module", "maxPriceWhite           = {}", config->GetMaxPrice(AHB_WHITE));
            LOG_INFO("module", "minPriceGreen           = {}", config->GetMinPrice(AHB_GREEN));
            LOG_INFO("module", "maxPriceGreen           = {}", config->GetMaxPrice(AHB_GREEN));
            LOG_INFO("module", "minPriceBlue            = {}", config->GetMinPrice(AHB_BLUE));
            LOG_INFO("module", "maxPriceBlue            = {}", config->GetMaxPrice(AHB_BLUE));
            LOG_INFO("module", "minPricePurple          = {}", config->GetMinPrice(AHB_PURPLE));
            LOG_INFO("module", "maxPricePurple          = {}", config->GetMaxPrice(AHB_PURPLE));
            LOG_INFO("module", "minPriceOrange          = {}", config->GetMinPrice(AHB_ORANGE));
            LOG_INFO("module", "maxPriceOrange          = {}", config->GetMaxPrice(AHB_ORANGE));
            LOG_INFO("module", "minPriceYellow          = {}", config->GetMinPrice(AHB_YELLOW));
            LOG_INFO("module", "maxPriceYellow          = {}", config->GetMaxPrice(AHB_YELLOW));
            LOG_INFO("module", "minBidPriceGrey         = {}", config->GetMinBidPrice(AHB_GREY));
            LOG_INFO("module", "maxBidPriceGrey         = {}", config->GetMaxBidPrice(AHB_GREY));
            LOG_INFO("module", "minBidPriceWhite        = {}", config->GetMinBidPrice(AHB_WHITE));
            LOG_INFO("module", "maxBidPriceWhite        = {}", config->GetMaxBidPrice(AHB_WHITE));
            LOG_INFO("module", "minBidPriceGreen        = {}", config->GetMinBidPrice(AHB_GREEN));
            LOG_INFO("module", "maxBidPriceGreen        = {}", config->GetMaxBidPrice(AHB_GREEN));
            LOG_INFO("module", "minBidPriceBlue         = {}", config->GetMinBidPrice(AHB_BLUE));
            LOG_INFO("module", "maxBidPriceBlue         = {}", config->GetMinBidPrice(AHB_BLUE));
            LOG_INFO("module", "minBidPricePurple       = {}", config->GetMinBidPrice(AHB_PURPLE));
            LOG_INFO("module", "maxBidPricePurple       = {}", config->GetMaxBidPrice(AHB_PURPLE));
            LOG_INFO("module", "minBidPriceOrange       = {}", config->GetMinBidPrice(AHB_ORANGE));
            LOG_INFO("module", "maxBidPriceOrange       = {}", config->GetMaxBidPrice(AHB_ORANGE));
            LOG_INFO("module", "minBidPriceYellow       = {}", config->GetMinBidPrice(AHB_YELLOW));
            LOG_INFO("module", "maxBidPriceYellow       = {}", config->GetMaxBidPrice(AHB_YELLOW));
            LOG_INFO("module", "maxStackGrey            = {}", config->GetMaxStack(AHB_GREY));
            LOG_INFO("module", "maxStackWhite           = {}", config->GetMaxStack(AHB_WHITE));
            LOG_INFO("module", "maxStackGreen           = {}", config->GetMaxStack(AHB_GREEN));
            LOG_INFO("module", "maxStackBlue            = {}", config->GetMaxStack(AHB_BLUE));
            LOG_INFO("module", "maxStackPurple          = {}", config->GetMaxStack(AHB_PURPLE));
            LOG_INFO("module", "maxStackOrange          = {}", config->GetMaxStack(AHB_ORANGE));
            LOG_INFO("module", "maxStackYellow          = {}", config->GetMaxStack(AHB_YELLOW));
        }

        //
        // Collect how many items are currently present in the auction house
        //

        config->ResetItemCounts();

        AuctionHouseObject* auctionHouse = sAuctionMgr->GetAuctionsMap(config->GetAHFID());
        uint32              auctions     = auctionHouse->Getcount();
        ObjectGuid          playerGuid(HighGuid::Player, 0, AHBplayerGUID);

        if (auctions)
        {
            for (AuctionHouseObject::AuctionEntryMap::const_iterator itr = auctionHouse->GetAuctionsBegin(); itr != auctionHouse->GetAuctionsEnd(); ++itr)
            {
                AuctionEntry* Aentry = itr->second;
                Item*         item   = sAuctionMgr->GetAItem(Aentry->item_guid);

                //
                // If it has to only consider its auctions, skip the one belonging to the players
                //

                if (ConsiderOnlyBotAuctions)
                {
                    if (playerGuid != Aentry->owner)
                    {
                        continue;
                    }
                }

                if (item)
                {
                    ItemTemplate const* prototype = item->GetTemplate();

                    if (prototype)
                    {
                        switch (prototype->Quality)
                        {
                        case AHB_GREY:
                            if (prototype->Class == ITEM_CLASS_TRADE_GOODS)
                            {
                                config->IncItemCounts(AHB_GREY_TG);
                            }
                            else
                            {
                                config->IncItemCounts(AHB_GREY_I);
                            }
                            break;

                        case AHB_WHITE:
                            if (prototype->Class == ITEM_CLASS_TRADE_GOODS)
                            {
                                config->IncItemCounts(AHB_WHITE_TG);
                            }
                            else
                            {
                                config->IncItemCounts(AHB_WHITE_I);
                            }

                            break;

                        case AHB_GREEN:
                            if (prototype->Class == ITEM_CLASS_TRADE_GOODS)
                            {
                                config->IncItemCounts(AHB_GREEN_TG);
                            }
                            else
                            {
                                config->IncItemCounts(AHB_GREEN_I);
                            }

                            break;

                        case AHB_BLUE:
                            if (prototype->Class == ITEM_CLASS_TRADE_GOODS)
                            {
                                config->IncItemCounts(AHB_BLUE_TG);
                            }
                            else
                            {
                                config->IncItemCounts(AHB_BLUE_I);
                            }

                            break;

                        case AHB_PURPLE:
                            if (prototype->Class == ITEM_CLASS_TRADE_GOODS)
                            {
                                config->IncItemCounts(AHB_PURPLE_TG);
                            }
                            else
                            {
                                config->IncItemCounts(AHB_PURPLE_I);
                            }

                            break;

                        case AHB_ORANGE:
                            if (prototype->Class == ITEM_CLASS_TRADE_GOODS)
                            {
                                config->IncItemCounts(AHB_ORANGE_TG);
                            }
                            else
                            {
                                config->IncItemCounts(AHB_ORANGE_I);
                            }

                            break;

                        case AHB_YELLOW:
                            if (prototype->Class == ITEM_CLASS_TRADE_GOODS)
                            {
                                config->IncItemCounts(AHB_YELLOW_TG);
                            }
                            else
                            {
                                config->IncItemCounts(AHB_YELLOW_I);
                            }

                            break;
                        }
                    }
                }
            }
        }

        if (debug_Out_Config)
        {
            LOG_INFO("module", "Current compatible auctions presents on auctionhouse {}", config->GetAHID());
            LOG_INFO("module", "    Grey   Trade Goods {}", config->GetItemCounts(AHB_GREY_TG));
            LOG_INFO("module", "    White  Trade Goods {}", config->GetItemCounts(AHB_WHITE_TG));
            LOG_INFO("module", "    Green  Trade Goods {}", config->GetItemCounts(AHB_GREEN_TG));
            LOG_INFO("module", "    Blue   Trade Goods {}", config->GetItemCounts(AHB_BLUE_TG));
            LOG_INFO("module", "    Purple Trade Goods {}", config->GetItemCounts(AHB_PURPLE_TG));
            LOG_INFO("module", "    Orange Trade Goods {}", config->GetItemCounts(AHB_ORANGE_TG));
            LOG_INFO("module", "    Yellow Trade Goods {}", config->GetItemCounts(AHB_YELLOW_TG));
            LOG_INFO("module", "    Grey   Items       {}", config->GetItemCounts(AHB_GREY_I));
            LOG_INFO("module", "    White  Items       {}", config->GetItemCounts(AHB_WHITE_I));
            LOG_INFO("module", "    Green  Items       {}", config->GetItemCounts(AHB_GREEN_I));
            LOG_INFO("module", "    Blue   Items       {}", config->GetItemCounts(AHB_BLUE_I));
            LOG_INFO("module", "    Purple Items       {}", config->GetItemCounts(AHB_PURPLE_I));
            LOG_INFO("module", "    Orange Items       {}", config->GetItemCounts(AHB_ORANGE_I));
            LOG_INFO("module", "    Yellow Items       {}", config->GetItemCounts(AHB_YELLOW_I));
        }
    }

    //
    // Auctions buyer
    //

    if (AHBBuyer)
    {
        //
        // Load buyer bid prices
        //

        config->SetBuyerPrice(AHB_GREY  , WorldDatabase.Query("SELECT buyerpricegrey   FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetBuyerPrice(AHB_WHITE , WorldDatabase.Query("SELECT buyerpricewhite  FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetBuyerPrice(AHB_GREEN , WorldDatabase.Query("SELECT buyerpricegreen  FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetBuyerPrice(AHB_BLUE  , WorldDatabase.Query("SELECT buyerpriceblue   FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetBuyerPrice(AHB_PURPLE, WorldDatabase.Query("SELECT buyerpricepurple FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetBuyerPrice(AHB_ORANGE, WorldDatabase.Query("SELECT buyerpriceorange FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());
        config->SetBuyerPrice(AHB_YELLOW, WorldDatabase.Query("SELECT buyerpriceyellow FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());

        //
        // Load bidding interval
        //

        config->SetBiddingInterval(WorldDatabase.Query("SELECT buyerbiddinginterval FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());

        //
        // Load bids per interval
        //

        config->SetBidsPerInterval(WorldDatabase.Query("SELECT buyerbidsperinterval FROM mod_auctionhousebot WHERE auctionhouse = {}", config->GetAHID())->Fetch()->Get<uint32>());

        if (debug_Out_Config)
        {
            LOG_INFO("module", "Current Settings for Auctionhouse {} buyer", config->GetAHID());
            LOG_INFO("module", "buyerPriceGrey          = {}", config->GetBuyerPrice(AHB_GREY));
            LOG_INFO("module", "buyerPriceWhite         = {}", config->GetBuyerPrice(AHB_WHITE));
            LOG_INFO("module", "buyerPriceGreen         = {}", config->GetBuyerPrice(AHB_GREEN));
            LOG_INFO("module", "buyerPriceBlue          = {}", config->GetBuyerPrice(AHB_BLUE));
            LOG_INFO("module", "buyerPricePurple        = {}", config->GetBuyerPrice(AHB_PURPLE));
            LOG_INFO("module", "buyerPriceOrange        = {}", config->GetBuyerPrice(AHB_ORANGE));
            LOG_INFO("module", "buyerPriceYellow        = {}", config->GetBuyerPrice(AHB_YELLOW));
            LOG_INFO("module", "buyerBiddingInterval    = {}", config->GetBiddingInterval());
            LOG_INFO("module", "buyerBidsPerInterval    = {}", config->GetBidsPerInterval());
        }
    }
}

// =============================================================================
// Initialization of the bot
// =============================================================================

void AuctionHouseBot::Initialize()
{
    //
    // Reload the list of disabled items
    //

    DisableItemStore.clear();

    QueryResult result = WorldDatabase.Query("SELECT item FROM mod_auctionhousebot_disabled_items");

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            DisableItemStore.insert(fields[0].Get<uint32>());
        } while (result->NextRow());
    }

    //
    // Check if the AHBot account/GUID in the config actually exists
    //

    if ((AHBplayerAccount != 0) || (AHBplayerGUID != 0))
    {
        QueryResult result = CharacterDatabase.Query("SELECT 1 FROM characters WHERE account = {} AND guid = {}", AHBplayerAccount, AHBplayerGUID);

        if (!result)
        {
            LOG_ERROR("module", "AuctionHouseBot: The account/GUID-information set for your AHBot is incorrect (account: {} guid: {})", AHBplayerAccount, AHBplayerGUID);
            return;
        }
    }

    //
    // Load the configuration for every auction house denpending on the configuration
    //

    if (!sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION))
    {
        LoadValues(&AllianceConfig);
        LoadValues(&HordeConfig);
    }

    LoadValues(&NeutralConfig);

    //
    // Organize the lists of items to be sold
    //

    if (AHBSeller)
    {
        //
        // Process vendors items
        //

        QueryResult results = WorldDatabase.Query("SELECT distinct item FROM npc_vendor");

        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                npcItems.push_back(fields[0].Get<int32>());

            } while (results->NextRow());
        }
        else
        {
            if (debug_Out)
            {
                LOG_ERROR("module", "AuctionHouseBot: failed to retrieve npc items");
            }
        }

        //
        // Process loot items
        //

        results = WorldDatabase.Query(
            "SELECT item FROM creature_loot_template      UNION "
            "SELECT item FROM reference_loot_template     UNION "
            "SELECT item FROM disenchant_loot_template    UNION "
            "SELECT item FROM fishing_loot_template       UNION "
            "SELECT item FROM gameobject_loot_template    UNION "
            "SELECT item FROM item_loot_template          UNION "
            "SELECT item FROM milling_loot_template       UNION "
            "SELECT item FROM pickpocketing_loot_template UNION "
            "SELECT item FROM prospecting_loot_template   UNION "
            "SELECT item FROM skinning_loot_template");

        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                lootItems.push_back(fields[0].Get<uint32>());

            } while (results->NextRow());
        }
        else
        {
            if (debug_Out)
            {
                LOG_ERROR("module", "AuctionHouseBot: failed to retrieve loot items");
            }
        }

        //
        // Exclude items depending on the configuration; whatever passes all the tests is put in the lists.
        //

        ItemTemplateContainer const* its = sObjectMgr->GetItemTemplateStore();

        for (ItemTemplateContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
        {
            //
            // Exclude items with the blocked binding type
            //

            if (itr->second.Bonding == NO_BIND && !No_Bind)
            {
                continue;
            }

            if (itr->second.Bonding == BIND_WHEN_PICKED_UP && !Bind_When_Picked_Up)
            {
                continue;
            }

            if (itr->second.Bonding == BIND_WHEN_EQUIPED && !Bind_When_Equipped)
            {
                continue;
            }

            if (itr->second.Bonding == BIND_WHEN_USE && !Bind_When_Use)
            {
                continue;
            }

            if (itr->second.Bonding == BIND_QUEST_ITEM && !Bind_Quest_Item)
            {
                continue;
            }

            //
            // Exclude items with no possible price
            //

            if (SellMethod)
            {
                if (itr->second.BuyPrice == 0)
                {
                    continue;
                }
            }
            else
            {
                if (itr->second.SellPrice == 0)
                {
                    continue;
                }
            }

            //
            // Exclude items with no costs associated, in any case
            //

            if ((itr->second.BuyPrice == 0) && (itr->second.SellPrice == 0))
            {
                continue;
            }

            //
            // Exlude items superior to the limit quality
            //

            if (itr->second.Quality > 6)
            {
                continue;
            }

            //
            // Exclude trade goods items
            //

            if (itr->second.Class == ITEM_CLASS_TRADE_GOODS)
            {
                bool isNpc   = false;
                bool isLoot  = false;
                bool exclude = false;

                for (unsigned int i = 0; i < npcItems.size(); i++)
                {
                    if (itr->second.ItemId == npcItems[i])
                    {
                        isNpc = true;

                        if (!Vendor_TGs)
                        {
                            exclude = true;
                        }

                        break;
                    }
                }

                // Perform the loop only if exlude is not already true
                for (unsigned int i = 0; i < lootItems.size() && !exclude; i++)
                {
                    if (itr->second.ItemId == lootItems[i])
                    {
                        isLoot = true;

                        if (!Loot_TGs)
                        {
                            exclude = true;
                        }

                        break;
                    }
                }

                if (exclude)
                {
                    continue;
                }

                if (!Other_TGs)
                {
                    if (!isNpc && !isLoot)
                    {
                        continue;
                    }
                }
            }

            //
            // Exclude loot items
            //

            if (itr->second.Class != ITEM_CLASS_TRADE_GOODS)
            {
                bool isNpc   = false;
                bool isLoot  = false;
                bool exclude = false;

                for (unsigned int i = 0; i < npcItems.size(); i++)
                {
                    if (itr->second.ItemId == npcItems[i])
                    {
                        isNpc = true;

                        if (!Vendor_Items)
                        {
                            exclude = true;
                        }

                        break;
                    }
                }

                // Perform the loop only if exlude is not already true
                for (unsigned int i = 0; i < lootItems.size() && !exclude; i++)
                {
                    if (itr->second.ItemId == lootItems[i])
                    {
                        isLoot = true;

                        if (!Loot_Items)
                        {
                            exclude = true;
                        }

                        break;
                    }

                }

                if (exclude)
                {
                    continue;
                }

                if (!Other_Items)
                {
                    if (!isNpc && !isLoot)
                    {
                        continue;
                    }
                }
            }

            //
            // Verify if the item is disabled or not in the whitelist
            //

            if (SellerWhiteList.size() == 0)
            {
                if (DisableItemStore.find(itr->second.ItemId) != DisableItemStore.end())
                {
                    if (debug_Out_Filters)
                    {
                        LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (PTR/Beta/Unused Item)", itr->second.ItemId);
                    }

                    continue;
                }
            }
            else
            {
                if (SellerWhiteList.find(itr->second.ItemId) == SellerWhiteList.end())
                {
                    if (debug_Out_Filters)
                    {
                        LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (no in the whitelist)", itr->second.ItemId);
                    }

                    continue;
                }
            }

            //
            // Disable permanent enchants items
            //

            if ((DisablePermEnchant) && (itr->second.Class == ITEM_CLASS_PERMANENT))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (Permanent Enchant Item)", itr->second.ItemId);
                }

                continue;
            }

            //
            // Disable conjured items
            //

            if ((DisableConjured) && (itr->second.IsConjuredConsumable()))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (Conjured Consumable)", itr->second.ItemId);
                }

                continue;
            }

            //
            // Disable gems
            //

            if ((DisableGems) && (itr->second.Class == ITEM_CLASS_GEM))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (Gem)", itr->second.ItemId);
                }

                continue;
            }

            //
            // Disable money
            //

            if ((DisableMoney) && (itr->second.Class == ITEM_CLASS_MONEY))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (Money)", itr->second.ItemId);
                }

                continue;
            }

            //
            // Disable moneyloot
            //

            if ((DisableMoneyLoot) && (itr->second.MinMoneyLoot > 0))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (MoneyLoot)", itr->second.ItemId);
                }

                continue;
            }

            //
            // Disable lootable items
            //

            if ((DisableLootable) && (itr->second.Flags & 4))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (Lootable Item)", itr->second.ItemId);
                }

                continue;
            }

            //
            // Disable Keys
            //

            if ((DisableKeys) && (itr->second.Class == ITEM_CLASS_KEY))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (Quest Item)", itr->second.ItemId);
                }

                continue;
            }

            //
            // Disable items with duration
            //

            if ((DisableDuration) && (itr->second.Duration > 0))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (Has a Duration)", itr->second.ItemId);
                }

                continue;
            }

            //
            // Disable items which are BOP or Quest Items and have a required level lower than the item level
            //

            if ((DisableBOP_Or_Quest_NoReqLevel) && ((itr->second.Bonding == BIND_WHEN_PICKED_UP || itr->second.Bonding == BIND_QUEST_ITEM) && (itr->second.RequiredLevel < itr->second.ItemLevel)))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (BOP or BQI and Required Level is less than Item Level)", itr->second.ItemId);
                }

                continue;
            }

            //
            // Disable items specifically for Warrior
            //

            if ((DisableWarriorItems) && (itr->second.AllowableClass == AHB_CLASS_WARRIOR))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (Warrior Item)", itr->second.ItemId);
                }

                continue;
            }

            //
            // Disable items specifically for Paladin
            //

            if ((DisablePaladinItems) && (itr->second.AllowableClass == AHB_CLASS_PALADIN))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (Paladin Item)", itr->second.ItemId);
                }

                continue;
            }

            //
            // Disable items specifically for Hunter
            //

            if ((DisableHunterItems) && (itr->second.AllowableClass == AHB_CLASS_HUNTER))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (Hunter Item)", itr->second.ItemId);
                }

                continue;
            }

            //
            // Disable items specifically for Rogue
            //

            if ((DisableRogueItems) && (itr->second.AllowableClass == AHB_CLASS_ROGUE))
            {
                if (debug_Out_Filters)
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (Rogue Item)", itr->second.ItemId);
                continue;
            }

            //
            // Disable items specifically for Priest
            //

            if ((DisablePriestItems) && (itr->second.AllowableClass == AHB_CLASS_PRIEST))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (Priest Item)", itr->second.ItemId);
                }

                continue;
            }

            //
            // Disable items specifically for DK
            //

            if ((DisableDKItems) && (itr->second.AllowableClass == AHB_CLASS_DK))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (DK Item)", itr->second.ItemId);
                }

                continue;
            }

            //
            // Disable items specifically for Shaman
            //

            if ((DisableShamanItems) && (itr->second.AllowableClass == AHB_CLASS_SHAMAN))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (Shaman Item)", itr->second.ItemId);
                }

                continue;
            }

            //
            // Disable items specifically for Mage
            //

            if ((DisableMageItems) && (itr->second.AllowableClass == AHB_CLASS_MAGE))
            {
                if (debug_Out_Filters)
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (Mage Item)", itr->second.ItemId);
                continue;
            }

            //
            // Disable items specifically for Warlock
            //

            if ((DisableWarlockItems) && (itr->second.AllowableClass == AHB_CLASS_WARLOCK))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (Warlock Item)", itr->second.ItemId);
                }

                continue;
            }

            //
            // Disable items specifically for Unused Class
            //

            if ((DisableUnusedClassItems) && (itr->second.AllowableClass == AHB_CLASS_UNUSED))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (Unused Item)", itr->second.ItemId);
                }

                continue;
            }

            //
            // Disable items specifically for Druid
            //

            if ((DisableDruidItems) && (itr->second.AllowableClass == AHB_CLASS_DRUID))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (Druid Item)", itr->second.ItemId);
                }

                continue;
            }

            //
            // Disable Items below level X
            //

            if ((DisableItemsBelowLevel) && (itr->second.Class != ITEM_CLASS_TRADE_GOODS) && (itr->second.ItemLevel < DisableItemsBelowLevel))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (Item Level = {})", itr->second.ItemId, itr->second.ItemLevel);
                }

                continue;
            }

            //
            // Disable Items above level X
            //

            if ((DisableItemsAboveLevel) && (itr->second.Class != ITEM_CLASS_TRADE_GOODS) && (itr->second.ItemLevel > DisableItemsAboveLevel))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (Item Level = {})", itr->second.ItemId, itr->second.ItemLevel);
                }

                continue;
            }

            //
            // Disable Trade Goods below level X
            //

            if ((DisableTGsBelowLevel) && (itr->second.Class == ITEM_CLASS_TRADE_GOODS) && (itr->second.ItemLevel < DisableTGsBelowLevel))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Trade Good {} disabled (Trade Good Level = {})", itr->second.ItemId, itr->second.ItemLevel);
                }

                continue;
            }

            //
            // Disable Trade Goods above level X
            //

            if ((DisableTGsAboveLevel) && (itr->second.Class == ITEM_CLASS_TRADE_GOODS) && (itr->second.ItemLevel > DisableTGsAboveLevel))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Trade Good {} disabled (Trade Good Level = {})", itr->second.ItemId, itr->second.ItemLevel);
                }

                continue;
            }

            //
            // Disable Items below GUID X
            //

            if ((DisableItemsBelowGUID) && (itr->second.Class != ITEM_CLASS_TRADE_GOODS) && (itr->second.ItemId < DisableItemsBelowGUID))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (Item Level = {})", itr->second.ItemId, itr->second.ItemLevel);
                }

                continue;
            }

            //
            // Disable Items above GUID X
            //

            if ((DisableItemsAboveGUID) && (itr->second.Class != ITEM_CLASS_TRADE_GOODS) && (itr->second.ItemId > DisableItemsAboveGUID))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (Item Level = {})", itr->second.ItemId, itr->second.ItemLevel);
                }

                continue;
            }

            //
            // Disable Trade Goods below GUID X
            //

            if ((DisableTGsBelowGUID) && (itr->second.Class == ITEM_CLASS_TRADE_GOODS) && (itr->second.ItemId < DisableTGsBelowGUID))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (Trade Good Level = {})", itr->second.ItemId, itr->second.ItemLevel);
                }

                continue;
            }

            //
            // Disable Trade Goods above GUID X
            //

            if ((DisableTGsAboveGUID) && (itr->second.Class == ITEM_CLASS_TRADE_GOODS) && (itr->second.ItemId > DisableTGsAboveGUID))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (Trade Good Level = {})", itr->second.ItemId, itr->second.ItemLevel);
                }

                continue;
            }

            //
            // Disable Items for level lower than X
            //

            if ((DisableItemsBelowReqLevel) && (itr->second.Class != ITEM_CLASS_TRADE_GOODS) && (itr->second.RequiredLevel < DisableItemsBelowReqLevel))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (RequiredLevel = {})", itr->second.ItemId, itr->second.RequiredLevel);
                }

                continue;
            }

            //
            // Disable Items for level higher than X
            //

            if ((DisableItemsAboveReqLevel) && (itr->second.Class != ITEM_CLASS_TRADE_GOODS) && (itr->second.RequiredLevel > DisableItemsAboveReqLevel))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (RequiredLevel = {})", itr->second.ItemId, itr->second.RequiredLevel);
                }

                continue;
            }

            //
            // Disable Trade Goods for level lower than X
            //

            if ((DisableTGsBelowReqLevel) && (itr->second.Class == ITEM_CLASS_TRADE_GOODS) && (itr->second.RequiredLevel < DisableTGsBelowReqLevel))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Trade Good {} disabled (RequiredLevel = {})", itr->second.ItemId, itr->second.RequiredLevel);
                }

                continue;
            }

            //
            // Disable Trade Goods for level higher than X
            //

            if ((DisableTGsAboveReqLevel) && (itr->second.Class == ITEM_CLASS_TRADE_GOODS) && (itr->second.RequiredLevel > DisableTGsAboveReqLevel))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Trade Good {} disabled (RequiredLevel = {})", itr->second.ItemId, itr->second.RequiredLevel);
                }

                continue;
            }

            //
            // Disable Items that require skill lower than X
            //

            if ((DisableItemsBelowReqSkillRank) && (itr->second.Class != ITEM_CLASS_TRADE_GOODS) && (itr->second.RequiredSkillRank < DisableItemsBelowReqSkillRank))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (RequiredSkillRank = {})", itr->second.ItemId, itr->second.RequiredSkillRank);
                }

                continue;
            }

            //
            // Disable Items that require skill higher than X
            //

            if ((DisableItemsAboveReqSkillRank) && (itr->second.Class != ITEM_CLASS_TRADE_GOODS) && (itr->second.RequiredSkillRank > DisableItemsAboveReqSkillRank))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (RequiredSkillRank = {})", itr->second.ItemId, itr->second.RequiredSkillRank);
                }

                continue;
            }

            //
            // Disable Trade Goods that require skill lower than X
            //

            if ((DisableTGsBelowReqSkillRank) && (itr->second.Class == ITEM_CLASS_TRADE_GOODS) && (itr->second.RequiredSkillRank < DisableTGsBelowReqSkillRank))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (RequiredSkillRank = {})", itr->second.ItemId, itr->second.RequiredSkillRank);
                }

                continue;
            }

            //
            // Disable Trade Goods that require skill higher than X
            //

            if ((DisableTGsAboveReqSkillRank) && (itr->second.Class == ITEM_CLASS_TRADE_GOODS) && (itr->second.RequiredSkillRank > DisableTGsAboveReqSkillRank))
            {
                if (debug_Out_Filters)
                {
                    LOG_ERROR("module", "AuctionHouseBot: Item {} disabled (RequiredSkillRank = {})", itr->second.ItemId, itr->second.RequiredSkillRank);
                }

                continue;
            }

            //
            // Now that the items passed all the tests, organize it by quality
            //

            if (itr->second.Class == ITEM_CLASS_TRADE_GOODS)
            {
                switch (itr->second.Quality)
                {
                case AHB_GREY:
                    greyTradeGoodsBin.push_back(itr->second.ItemId);
                    break;

                case AHB_WHITE:
                    whiteTradeGoodsBin.push_back(itr->second.ItemId);
                    break;

                case AHB_GREEN:
                    greenTradeGoodsBin.push_back(itr->second.ItemId);
                    break;

                case AHB_BLUE:
                    blueTradeGoodsBin.push_back(itr->second.ItemId);
                    break;

                case AHB_PURPLE:
                    purpleTradeGoodsBin.push_back(itr->second.ItemId);
                    break;

                case AHB_ORANGE:
                    orangeTradeGoodsBin.push_back(itr->second.ItemId);
                    break;

                case AHB_YELLOW:
                    yellowTradeGoodsBin.push_back(itr->second.ItemId);
                    break;
                }
            }
            else
            {
                switch (itr->second.Quality)
                {
                case AHB_GREY:
                    greyItemsBin.push_back(itr->second.ItemId);
                    break;

                case AHB_WHITE:
                    whiteItemsBin.push_back(itr->second.ItemId);
                    break;

                case AHB_GREEN:
                    greenItemsBin.push_back(itr->second.ItemId);
                    break;

                case AHB_BLUE:
                    blueItemsBin.push_back(itr->second.ItemId);
                    break;

                case AHB_PURPLE:
                    purpleItemsBin.push_back(itr->second.ItemId);
                    break;

                case AHB_ORANGE:
                    orangeItemsBin.push_back(itr->second.ItemId);
                    break;

                case AHB_YELLOW:
                    yellowItemsBin.push_back(itr->second.ItemId);
                    break;
                }
            }
        }

        //
        // Disable the seller if no items passed the test
        //

        if ((greyTradeGoodsBin.size()   == 0) &&
            (whiteTradeGoodsBin.size()  == 0) &&
            (greenTradeGoodsBin.size()  == 0) &&
            (blueTradeGoodsBin.size()   == 0) &&
            (purpleTradeGoodsBin.size() == 0) &&
            (orangeTradeGoodsBin.size() == 0) &&
            (yellowTradeGoodsBin.size() == 0) &&
            (greyItemsBin.size()        == 0) &&
            (whiteItemsBin.size()       == 0) &&
            (greenItemsBin.size()       == 0) &&
            (blueItemsBin.size()        == 0) &&
            (purpleItemsBin.size()      == 0) &&
            (orangeItemsBin.size()      == 0) &&
            (yellowItemsBin.size()      == 0))
        {
            LOG_ERROR("module", "AuctionHouseBot: No items, seller disabled");
            AHBSeller = 0;
        }

        LOG_INFO("module", "AuctionHouseBot:");

        if (SellerWhiteList.size() == 0)
        {
            LOG_INFO("module", "{} disabled items", uint32(DisableItemStore.size()));
        }
        else
        {
            LOG_INFO("module", "Using a whitelist of {} items", uint32(SellerWhiteList.size()));
        }

        LOG_INFO("module", "loaded {} grey   trade goods", uint32(greyTradeGoodsBin.size()));
        LOG_INFO("module", "loaded {} white  trade goods", uint32(whiteTradeGoodsBin.size()));
        LOG_INFO("module", "loaded {} green  trade goods", uint32(greenTradeGoodsBin.size()));
        LOG_INFO("module", "loaded {} blue   trade goods", uint32(blueTradeGoodsBin.size()));
        LOG_INFO("module", "loaded {} purple trade goods", uint32(purpleTradeGoodsBin.size()));
        LOG_INFO("module", "loaded {} orange trade goods", uint32(orangeTradeGoodsBin.size()));
        LOG_INFO("module", "loaded {} yellow trade goods", uint32(yellowTradeGoodsBin.size()));
        LOG_INFO("module", "loaded {} grey   items"      , uint32(greyItemsBin.size()));
        LOG_INFO("module", "loaded {} white  items"      , uint32(whiteItemsBin.size()));
        LOG_INFO("module", "loaded {} green  items"      , uint32(greenItemsBin.size()));
        LOG_INFO("module", "loaded {} blue   items"      , uint32(blueItemsBin.size()));
        LOG_INFO("module", "loaded {} purple items"      , uint32(purpleItemsBin.size()));
        LOG_INFO("module", "loaded {} orange items"      , uint32(orangeItemsBin.size()));
        LOG_INFO("module", "loaded {} yellow items"      , uint32(yellowItemsBin.size()));
    }

    LOG_INFO("module", "AuctionHouseBot and AuctionHouseBuyer have been loaded.");
}
