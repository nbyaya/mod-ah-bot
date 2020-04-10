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
#include "AuctionHouseMgr.h"
#include "AuctionHouseBot.h"
#include "Config.h"
#include "Player.h"
#include "WorldSession.h"
#include <vector>

using namespace std;
//Class, Subclass, then rarity, so 4 fold vector
// AHID: ((weight), (CurrentItems, (MinItems, MaxItems)) -- Class : SubClass : Quality : (TypeDataVector, Items))
//unordered_map < uint32, unordered_map<uint32,unordered_map<uint32, pair< vector<uint32>, vector<uint32>>>>> weightBins;

vector<uint32> npcItems;
vector<uint32> lootItems;
bool debug_shou = true;
#define MAX_CLASS = 16
#define MAX_SUBCLASS = 20
#define MAX_QUALITY = 6
AuctionHouseBot::AuctionHouseBot()
{
    debug_Out = false;
    debug_Out_Filters = false;
    AHBSeller = false;
    AHBBuyer = false;

    //Begin Filters

    Vendor_Items = false;
    Loot_Items = false;
    Other_Items = false;
    Vendor_TGs = false;
    Loot_TGs = false;
    Other_TGs = false;

    No_Bind = false;
    Bind_When_Picked_Up = false;
    Bind_When_Equipped = false;
    Bind_When_Use = false;
    Bind_Quest_Item = false;

    DisablePermEnchant = false;
    DisableConjured = false;
    DisableGems = false;
    DisableMoney = false;
    DisableMoneyLoot = false;
    DisableLootable = false;
    DisableKeys = false;
    DisableDuration = false;
    DisableBOP_Or_Quest_NoReqLevel = false;

    DisableWarriorItems = false;
    DisablePaladinItems = false;
    DisableHunterItems = false;
    DisableRogueItems = false;
    DisablePriestItems = false;
    DisableDKItems = false;
    DisableShamanItems = false;
    DisableMageItems = false;
    DisableWarlockItems = false;
    DisableUnusedClassItems = false;
    DisableDruidItems = false;

    DisableItemsBelowLevel = 0;
    DisableItemsAboveLevel = 0;
    DisableTGsBelowLevel = 0;
    DisableTGsAboveLevel = 0;
    DisableItemsBelowGUID = 0;
    DisableItemsAboveGUID = 0;
    DisableTGsBelowGUID = 0;
    DisableTGsAboveGUID = 0;
    DisableItemsBelowReqLevel = 0;
    DisableItemsAboveReqLevel = 0;
    DisableTGsBelowReqLevel = 0;
    DisableTGsAboveReqLevel = 0;
    DisableItemsBelowReqSkillRank = 0;
    DisableItemsAboveReqSkillRank = 0;
    DisableTGsBelowReqSkillRank = 0;
    DisableTGsAboveReqSkillRank = 0;

    //End Filters

    _lastrun_a = time(NULL);
    _lastrun_h = time(NULL);
    _lastrun_n = time(NULL);

    AllianceConfig = AHBConfig(2);
    HordeConfig = AHBConfig(6);
    NeutralConfig = AHBConfig(7);
}
AuctionHouseBot::~AuctionHouseBot(){}
void AuctionHouseBot::addNewAuctions(Player *AHBplayer, AHBConfig *config)
{
    if (!AHBSeller)
    {
        if (debug_Out) sLog->outString("AHSeller: Disabled");
        return;
    }

    uint32 minItems = config->GetMinItems();

    if (minItems == 0)
    {
        //if (debug_Out) sLog->outString( "AHSeller: Auctions disabled");
        return;
    }

    AuctionHouseEntry const* ahEntry =  sAuctionMgr->GetAuctionHouseEntry(config->GetAHFID());
    if (!ahEntry){return;}
    AuctionHouseObject* auctionHouse =  sAuctionMgr->GetAuctionsMap(config->GetAHFID());
    if (!auctionHouse){return;}

    uint32 auctions = auctionHouse->Getcount();
    uint32 items = 0;
    if (auctions >= minItems){return;}
    if ((minItems - auctions) >= ItemsPerCycle)
        items = ItemsPerCycle;
    else
        items = (minItems - auctions);

    if (debug_Out) sLog->outString("AHSeller: Adding %u Auctions", items);
    if (debug_shou) sLog->outString("AuctionHouseBot: current items %u, minItems %u, items adding now %u", auctions, minItems, items);

    uint32 AuctioneerGUID = 0;

    switch (config->GetAHID())
    {
    case 2:
        AuctioneerGUID = 79707; //Human in stormwind.
        break;
    case 6:
        AuctioneerGUID = 4656; //orc in Orgrimmar
        break;
    case 7:
        AuctioneerGUID = 23442; //goblin in GZ
        break;
    default:
        if (debug_Out) sLog->outError( "AHSeller: GetAHID() - Default switch reached");
        AuctioneerGUID = 23442; //default to neutral 7
        break;
    }

    if (debug_Out) sLog->outError( "AHSeller: Current Auctineer GUID is %u", AuctioneerGUID);
    if (debug_Out) sLog->outError( "AHSeller: %u items", items);

    // only insert a few at a time, so as not to peg the processor
    for (uint32 cnt = 1; cnt <= items; cnt++)
    {
        if (debug_Out) sLog->outError("AHSeller: %u count", cnt);
        uint32 itemID = 0;
        uint32 itemColor = 99;
        //uint32 loopbreaker = 0;
        //while (itemID == 0 && loopbreaker <= 50)
        {
            //++loopbreaker;
            uint32 choice = (uint32)(rand_norm()*(double)config->GetTotalWeights());// urand(0, config->GetTotalWeights);
            if (debug_shou) sLog->outString("AuctionHouseBot: selecting item %u, item position %u",cnt, choice);
            bool breaker = false;
            vector<uint32> qualityConfig;
            unordered_map < uint32, unordered_map<uint32, unordered_map<uint32, qualityData*>>> classMap = config->GetClassMap();
            unordered_map<uint32, unordered_map<uint32, qualityData*>> subclassMap;
            unordered_map<uint32, qualityData*> qualityMap;
            for (auto Class = classMap.begin(); Class != classMap.end(); Class++) {
                subclassMap = Class->second;
                for (auto subclass = subclassMap.begin(); subclass != subclassMap.end(); subclass++) {
                    qualityMap = subclass->second;
                    for (auto quality = qualityMap.begin(); quality != qualityMap.end(); quality++) {
                        qualityConfig = quality->second->itemTypeConfig;
                        if (debug_shou) sLog->outString("AuctionHouseBot: class %u subclass %u quality %u Size left to check %u", Class->first, subclass->first, quality->first, choice);
                        if (choice < qualityConfig.at(0)) {
                            itemID = quality->second->itemListing.at((uint32)(rand_norm()*(double)quality->second->itemListing.size()));
                            breaker = true;
                            break;
                        }
                        else {
                            choice -= qualityConfig.at(0);
                        }
                        if (breaker) break;
                    }
                    if (breaker) break;
                }
                if (breaker) break;
            }
            if (itemID == 0){
                if (debug_Out) sLog->outError( "AHSeller: Item::CreateItem() - ItemID is 0");
                if (debug_shou) sLog->outString("AuctionHouseBot: No Item Selected");
                continue;
            }

            ItemTemplate const* prototype = sObjectMgr->GetItemTemplate(itemID);
            if (prototype == NULL){
                if (debug_Out) sLog->outError( "AHSeller: Huh?!?! prototype == NULL");
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Creating ItemID %u to list on AH", itemID);
            Item* item = Item::CreateItem(itemID, 1, AHBplayer);
            if (item == NULL){
                if (debug_Out) sLog->outError( "AHSeller: Item::CreateItem() returned NULL");
                break;
            }
            item->AddToUpdateQueueOf(AHBplayer);

            uint32 randomPropertyId = Item::GenerateItemRandomPropertyId(itemID);
            if (randomPropertyId != 0)
                item->SetItemRandomProperties(randomPropertyId);

            uint64 buyoutPrice = 0;
            uint64 bidPrice = 0;
            uint32 stackCount = 1;

            if (SellMethod)
                buyoutPrice = prototype->BuyPrice *sWorld->getRate((Rates)(RATE_BUYVALUE_ITEM_POOR + prototype->Quality));
            else
                buyoutPrice = prototype->SellPrice *sWorld->getRate((Rates)(RATE_SELLVALUE_ITEM_POOR + prototype->Quality));

            if (prototype->Quality <= AHB_MAX_QUALITY){
                if (qualityConfig.at(5) > 1 && item->GetMaxStackCount() > 1)
                    stackCount = urand(1, minValue(item->GetMaxStackCount(), qualityConfig.at(5)));
                else if (qualityConfig.at(5) == 0 && item->GetMaxStackCount() > 1)
                    stackCount = urand(1, item->GetMaxStackCount());
                else
                    stackCount = 1;
                buyoutPrice *= urand(qualityConfig.at(1), qualityConfig.at(2));
                buyoutPrice /= 100;
                bidPrice = buyoutPrice * urand(qualityConfig.at(3), qualityConfig.at(4));
                bidPrice /= 100;
            }
            else
            {
                // quality is something it shouldn't be, let's get out of here
                if (debug_Out) sLog->outError( "AHBuyer: Quality %u not Supported", prototype->Quality);
                item->RemoveFromUpdateQueueOf(AHBplayer);
                continue;
            }

            uint32 etime = urand(1,3);
            switch(etime)
            {
            case 1:
                etime = 43200;
                break;
            case 2:
                etime = 86400;
                break;
            case 3:
                etime = 172800;
                break;
            default:
                etime = 86400;
                break;
            }
            item->SetCount(stackCount);

            uint32 dep =  sAuctionMgr->GetAuctionDeposit(ahEntry, etime, item, stackCount);

            SQLTransaction trans = CharacterDatabase.BeginTransaction();
            AuctionEntry* auctionEntry = new AuctionEntry();
            auctionEntry->Id = sObjectMgr->GenerateAuctionID();
            auctionEntry->auctioneer = AuctioneerGUID;
			auctionEntry->item_guidlow = item->GetGUIDLow();
            auctionEntry->item_template = item->GetEntry();
            auctionEntry->itemCount = item->GetCount();
            auctionEntry->owner = AHBplayer->GetGUIDLow();
            auctionEntry->startbid = bidPrice * stackCount;
            auctionEntry->buyout = buyoutPrice * stackCount;
            auctionEntry->bidder = 0;
            auctionEntry->bid = 0;
            auctionEntry->deposit = dep;
            auctionEntry->expire_time = (time_t) etime + time(NULL);
            auctionEntry->auctionHouseEntry = ahEntry;
            item->SaveToDB(trans);
            item->RemoveFromUpdateQueueOf(AHBplayer);
            sAuctionMgr->AddAItem(item);
            auctionHouse->AddAuction(auctionEntry);
            auctionEntry->SaveToDB(trans);
            CharacterDatabase.CommitTransaction(trans);


            if (debug_shou) sLog->outString("AuctionHouseBot: Incrementing Items to %u", config->IncrementItems()); else config->IncrementItems();
        }
    }
}
void AuctionHouseBot::addNewAuctionBuyerBotBid(Player *AHBplayer, AHBConfig *config, WorldSession *session)
{
    if (!AHBBuyer)
    {
        if (debug_Out) sLog->outError( "AHBuyer: Disabled");
        return;
    }

    QueryResult result = CharacterDatabase.PQuery("SELECT id FROM auctionhouse WHERE itemowner<>%u AND buyguid<>%u", AHBplayerGUID, AHBplayerGUID);

    if (!result)
        return;

    if (result->GetRowCount() == 0)
        return;

    // Fetches content of selected AH
    AuctionHouseObject* auctionHouse =  sAuctionMgr->GetAuctionsMap(config->GetAHFID());
    vector<uint32> possibleBids;

    do
    {
        uint32 tmpdata = result->Fetch()->GetUInt32();
        possibleBids.push_back(tmpdata);
    }while (result->NextRow());

    for (uint32 count = 1; count <= config->GetBidsPerInterval(); ++count)
    {
        // Do we have anything to bid? If not, stop here.
        if (possibleBids.empty())
        {
            //if (debug_Out) sLog->outError( "AHBuyer: I have no items to bid on.");
            count = config->GetBidsPerInterval();
            continue;
        }

        // Choose random auction from possible auctions
        uint32 vectorPos = urand(0, possibleBids.size() - 1);
        vector<uint32>::iterator iter = possibleBids.begin();
        advance(iter, vectorPos);

        // from auctionhousehandler.cpp, creates auction pointer & player pointer
        AuctionEntry* auction = auctionHouse->GetAuction(*iter);

        // Erase the auction from the vector to prevent bidding on item in next iteration.
        possibleBids.erase(iter);

        if (!auction)
            continue;

        // get exact item information
		Item *pItem = sAuctionMgr->GetAItem(auction->item_guidlow);
        if (!pItem)
        {
			if (debug_Out) sLog->outError( "AHBuyer: Item %u doesn't exist, perhaps bought already?", auction->item_guidlow);
            continue;
        }

        // get item prototype
        ItemTemplate const* prototype = sObjectMgr->GetItemTemplate(auction->item_template);

        // check which price we have to use, startbid or if it is bidded already
        uint32 currentprice;
        if (auction->bid)
            currentprice = auction->bid;
        else
            currentprice = auction->startbid;

        // Prepare portion from maximum bid
        double bidrate = static_cast<double>(urand(1, 100)) / 100;
        long double bidMax = 0;

        // check that bid has acceptable value and take bid based on vendorprice, stacksize and quality
        // Possible Concern
        uint32 buyerPrice = config->GetClassMap().at(prototype->Class).at(prototype->SubClass).at(prototype->Quality)->itemTypeConfig.at(6);
        if (BuyMethod)
        {
            if (prototype->Quality <= AHB_MAX_QUALITY)
            {
                if (currentprice < prototype->SellPrice * pItem->GetCount() * buyerPrice)
                    bidMax = prototype->SellPrice * pItem->GetCount() * buyerPrice;
            }
            else
            {
                // quality is something it shouldn't be, let's get out of here
                if (debug_Out) sLog->outError( "AHBuyer: Quality %u not Supported", prototype->Quality);
                    continue;
            }
        }
        else
        {
            if (prototype->Quality <= AHB_MAX_QUALITY)
            {
                if (currentprice < prototype->BuyPrice * pItem->GetCount() * buyerPrice)
                    bidMax = prototype->BuyPrice * pItem->GetCount() * buyerPrice;
            }
            else
            {
                // quality is something it shouldn't be, let's get out of here
                if (debug_Out) sLog->outError( "AHBuyer: Quality %u not Supported", prototype->Quality);
                    continue;
            }
        }        

        // check some special items, and do recalculating to their prices
        switch (prototype->Class)
        {
            // ammo
        case 6:
            bidMax = 0;
            break;
        default:
            break;
        }

        if (bidMax == 0)
        {
            // quality check failed to get bidmax, let's get out of here
            continue;
        }

        // Calculate our bid
        long double bidvalue = currentprice + ((bidMax - currentprice) * bidrate);
        // Convert to uint32
        uint32 bidprice = static_cast<uint32>(bidvalue);

        // Check our bid is high enough to be valid. If not, correct it to minimum.
        if ((currentprice + auction->GetAuctionOutBid()) > bidprice)
            bidprice = currentprice + auction->GetAuctionOutBid();

        if (debug_Out)
        {
            sLog->outString("-------------------------------------------------");
            sLog->outString("AHBuyer: Info for Auction #%u:", auction->Id);
            sLog->outString("AHBuyer: AuctionHouse: %u", auction->GetHouseId());
            sLog->outString("AHBuyer: Auctioneer: %u", auction->auctioneer);
            sLog->outString("AHBuyer: Owner: %u", auction->owner);
            sLog->outString("AHBuyer: Bidder: %u", auction->bidder);
            sLog->outString("AHBuyer: Starting Bid: %u", auction->startbid);
            sLog->outString("AHBuyer: Current Bid: %u", currentprice);
            sLog->outString("AHBuyer: Buyout: %u", auction->buyout);
            sLog->outString("AHBuyer: Deposit: %u", auction->deposit);
            sLog->outString("AHBuyer: Expire Time: %u", uint32(auction->expire_time));
            sLog->outString("AHBuyer: Bid Rate: %f", bidrate);
            sLog->outString("AHBuyer: Bid Max: %Lf", bidMax);
            sLog->outString("AHBuyer: Bid Value: %Lf", bidvalue);
            sLog->outString("AHBuyer: Bid Price: %u", bidprice);
            sLog->outString("AHBuyer: Item GUID: %u", auction->item_guidlow);
            sLog->outString("AHBuyer: Item Template: %u", auction->item_template);
            sLog->outString("AHBuyer: Item Info:");
            sLog->outString("AHBuyer: Item ID: %u", prototype->ItemId);
            sLog->outString("AHBuyer: Buy Price: %u", prototype->BuyPrice);
            sLog->outString("AHBuyer: Sell Price: %u", prototype->SellPrice);
            sLog->outString("AHBuyer: Bonding: %u", prototype->Bonding);
            sLog->outString("AHBuyer: Quality: %u", prototype->Quality);
            sLog->outString("AHBuyer: Item Level: %u", prototype->ItemLevel);
            sLog->outString("AHBuyer: Ammo Type: %u", prototype->AmmoType);
            sLog->outString("-------------------------------------------------");
        }

        // Check whether we do normal bid, or buyout
        if ((bidprice < auction->buyout) || (auction->buyout == 0))
        {

            if (auction->bidder > 0)
            {
                if (auction->bidder == AHBplayer->GetGUIDLow())
                {
                    //pl->ModifyMoney(-int32(price - auction->bid));
                }
                else
                {
                    // mail to last bidder and return money
                    SQLTransaction trans = CharacterDatabase.BeginTransaction();
                    sAuctionMgr->SendAuctionOutbiddedMail(auction , bidprice, session->GetPlayer(), trans);
                    CharacterDatabase.CommitTransaction(trans);
                    //pl->ModifyMoney(-int32(price));
                }
           }

            auction->bidder = AHBplayer->GetGUIDLow();
            auction->bid = bidprice;

            // Saving auction into database
            CharacterDatabase.PExecute("UPDATE auctionhouse SET buyguid = '%u',lastbid = '%u' WHERE id = '%u'", auction->bidder, auction->bid, auction->Id);
        }
        else
        {
            SQLTransaction trans = CharacterDatabase.BeginTransaction();
            //buyout
            if ((auction->bidder) && (AHBplayer->GetGUIDLow() != auction->bidder))
            {
                sAuctionMgr->SendAuctionOutbiddedMail(auction, auction->buyout, session->GetPlayer(), trans);
            }
            auction->bidder = AHBplayer->GetGUIDLow();
            auction->bid = auction->buyout;

            // Send mails to buyer & seller
            //sAuctionMgr->SendAuctionSalePendingMail(auction, trans);
            sAuctionMgr->SendAuctionSuccessfulMail(auction, trans);
            sAuctionMgr->SendAuctionWonMail(auction, trans);
            auction->DeleteFromDB( trans);

			sAuctionMgr->RemoveAItem(auction->item_guidlow);
            auctionHouse->RemoveAuction(auction);
            CharacterDatabase.CommitTransaction(trans);
        }
    }
}

void AuctionHouseBot::Update()
{
    time_t _newrun = time(NULL);
    if ((!AHBSeller) && (!AHBBuyer))
        return;

    if (debug_shou) sLog->outString("AuctionHouseBot: starting update");
	WorldSession _session(AHBplayerAccount, NULL, SEC_PLAYER, sWorld->getIntConfig(CONFIG_EXPANSION), 0, LOCALE_zhCN,0,false,false,0);
    Player _AHBplayer(&_session);
    _AHBplayer.Initialize(AHBplayerGUID);
    sObjectAccessor->AddObject(&_AHBplayer);

    // Add New Bids
    if (!sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION))
    {
        if (debug_shou) sLog->outString("AuctionHouseBot: adding to alliance ah");
        addNewAuctions(&_AHBplayer, &AllianceConfig);
        if (((_newrun - _lastrun_a) >= (AllianceConfig.GetBiddingInterval() * MINUTE)) && (AllianceConfig.GetBidsPerInterval() > 0))
        {
            //if (debug_Out) sLog->outError( "AHBuyer: %u seconds have passed since last bid", (_newrun - _lastrun_a));
            //if (debug_Out) sLog->outError( "AHBuyer: Bidding on Alliance Auctions");
            addNewAuctionBuyerBotBid(&_AHBplayer, &AllianceConfig, &_session);
            _lastrun_a = _newrun;
        }

        addNewAuctions(&_AHBplayer, &HordeConfig);
        if (((_newrun - _lastrun_h) >= (HordeConfig.GetBiddingInterval() * MINUTE)) && (HordeConfig.GetBidsPerInterval() > 0))
        {
            //if (debug_Out) sLog->outError( "AHBuyer: %u seconds have passed since last bid", (_newrun - _lastrun_h));
            //if (debug_Out) sLog->outError( "AHBuyer: Bidding on Horde Auctions");
            addNewAuctionBuyerBotBid(&_AHBplayer, &HordeConfig, &_session);
            _lastrun_h = _newrun;
        }
    }

    addNewAuctions(&_AHBplayer, &NeutralConfig);
    if (((_newrun - _lastrun_n) >= (NeutralConfig.GetBiddingInterval() * MINUTE)) && (NeutralConfig.GetBidsPerInterval() > 0))
    {
        //if (debug_Out) sLog->outError( "AHBuyer: %u seconds have passed since last bid", (_newrun - _lastrun_n));
        //if (debug_Out) sLog->outError( "AHBuyer: Bidding on Neutral Auctions");
        addNewAuctionBuyerBotBid(&_AHBplayer, &NeutralConfig, &_session);
        _lastrun_n = _newrun;
    }
    sObjectAccessor->RemoveObject(&_AHBplayer);
}
void AuctionHouseBot::Initialize()
{
    DisableItemStore.clear();
    QueryResult result = WorldDatabase.PQuery("SELECT item FROM mod_auctionhousebot_disabled_items");

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            if(debug_shou) sLog->outString("AuctionHouseBot: Disabling Item %u", fields[0].GetUInt32());
            DisableItemStore.insert(fields[0].GetUInt32());
        } while (result->NextRow());
    }

    //End Filters
    if (debug_shou) sLog->outString("AuctionHouseBot:Proceeding to LoadValues");
    if (!sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION))
    {
        if (debug_shou) sLog->outString("AuctionHouseBot: Loading Alliance");
        LoadValues(&AllianceConfig);
        if (debug_shou) sLog->outString("AuctionHouseBot: Loading Horde");
        LoadValues(&HordeConfig);
    }
    if (debug_shou) sLog->outString("AuctionHouseBot: Loading Neutral");
    LoadValues(&NeutralConfig);
    //
    // check if the AHBot account/GUID in the config actually exists
    //
    if ((AHBplayerAccount != 0) || (AHBplayerGUID != 0))
    {
        QueryResult result = CharacterDatabase.PQuery("SELECT 1 FROM characters WHERE account = %u AND guid = %u", AHBplayerAccount, AHBplayerGUID);
        if (!result)
       {
           sLog->outError( "AuctionHouseBot: The account/GUID-information set for your AHBot is incorrect (account: %u guid: %u)", AHBplayerAccount, AHBplayerGUID);
           return;
        }
    }
    if (AHBSeller)
    {
        if (debug_shou) sLog->outString("AuctionHouseBot: Selecting NPC items");
        QueryResult results = QueryResult(NULL);
        char npcQuery[] = "SELECT distinct item FROM npc_vendor";
        results = WorldDatabase.Query(npcQuery);
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                npcItems.push_back(fields[0].GetUInt32());

            } while (results->NextRow());
        }
        else
        {
            if (debug_Out) sLog->outError( "AuctionHouseBot: \"%s\" failed", npcQuery);
        }

        if (debug_shou) sLog->outString("AuctionHouseBot: Selecting Loot items");
        char lootQuery[] = "SELECT item FROM creature_loot_template UNION "
            "SELECT item FROM reference_loot_template UNION "
            "SELECT item FROM disenchant_loot_template UNION "
            "SELECT item FROM fishing_loot_template UNION "
            "SELECT item FROM gameobject_loot_template UNION "
            "SELECT item FROM item_loot_template UNION "
            "SELECT item FROM milling_loot_template UNION "
            "SELECT item FROM pickpocketing_loot_template UNION "
            "SELECT item FROM prospecting_loot_template UNION "
            "SELECT item FROM skinning_loot_template";

        results = WorldDatabase.Query(lootQuery);
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                lootItems.push_back(fields[0].GetUInt32());

            } while (results->NextRow());
        }
        else
        {
            if (debug_Out) sLog->outError( "AuctionHouseBot: \"%s\" failed", lootQuery);
        }

        if (debug_shou) sLog->outString("AuctionHouseBot:Getting Item Template Store");
        ItemTemplateContainer const* its = sObjectMgr->GetItemTemplateStore();
        if (debug_shou) sLog->outString("AuctionHouseBot: Iterating over store");
        for (ItemTemplateContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
        {
            if (debug_shou) sLog->outString("AuctionHouseBot: Bonding");
            switch (itr->second.Bonding)
            {
            case NO_BIND:
                if (!No_Bind)
                    continue;
                break;
            case BIND_WHEN_PICKED_UP:
                if (!Bind_When_Picked_Up)
                    continue;
                break;
            case BIND_WHEN_EQUIPED:
                if (!Bind_When_Equipped)
                    continue;
                break;
            case BIND_WHEN_USE:
                if (!Bind_When_Use)
                    continue;
                break;
            case BIND_QUEST_ITEM:
                if (!Bind_Quest_Item)
                    continue;
                break;
            default:
                continue;
                break;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Ignore No price");
            if (SellMethod)
            {
                if (itr->second.BuyPrice == 0)
                    continue;
            }
            else
            {
                if (itr->second.SellPrice == 0)
                    continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Ignore Heirlooms");
            if (itr->second.Quality > 6)
                continue;

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if Non-TG is Vendor");
            if ((Vendor_Items == 0) && !(itr->second.Class == ITEM_CLASS_TRADE_GOODS))
            {
                bool isVendorItem = false;

                for (unsigned int i = 0; (i < npcItems.size()) && (!isVendorItem); i++)
                {
                    if (itr->second.ItemId == npcItems[i])
                        isVendorItem = true;
                }

                if (isVendorItem)
                    continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if TG is Vendor");
            if ((Vendor_TGs == 0) && (itr->second.Class == ITEM_CLASS_TRADE_GOODS))
            {
                bool isVendorTG = false;

                for (unsigned int i = 0; (i < npcItems.size()) && (!isVendorTG); i++)
                {
                    if (itr->second.ItemId == npcItems[i])
                        isVendorTG = true;
                }

                if (isVendorTG)
                    continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if Non-TG is Loot");
            if ((Loot_Items == 0) && !(itr->second.Class == ITEM_CLASS_TRADE_GOODS))
            {
                bool isLootItem = false;

                for (unsigned int i = 0; (i < lootItems.size()) && (!isLootItem); i++)
                {
                    if (itr->second.ItemId == lootItems[i])
                        isLootItem = true;
                }

                if (isLootItem)
                    continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if TG is Loot");
            if ((Loot_TGs == 0) && (itr->second.Class == ITEM_CLASS_TRADE_GOODS))
            {
                bool isLootTG = false;

                for (unsigned int i = 0; (i < lootItems.size()) && (!isLootTG); i++)
                {
                    if (itr->second.ItemId == lootItems[i])
                        isLootTG = true;
                }

                if (isLootTG)
                    continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if Other-Non-TG is Loot or vendor");
            if ((Other_Items == 0) && !(itr->second.Class == ITEM_CLASS_TRADE_GOODS))
            {
                bool isVendorItem = false;
                bool isLootItem = false;

                for (unsigned int i = 0; (i < npcItems.size()) && (!isVendorItem); i++)
                {
                    if (itr->second.ItemId == npcItems[i])
                        isVendorItem = true;
                }
                for (unsigned int i = 0; (i < lootItems.size()) && (!isLootItem); i++)
                {
                    if (itr->second.ItemId == lootItems[i])
                        isLootItem = true;
                }
                if ((!isLootItem) && (!isVendorItem))
                    continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if Other-Non-TG is Loot or vendor");
            if ((Other_TGs == 0) && (itr->second.Class == ITEM_CLASS_TRADE_GOODS))
            {
                bool isVendorTG = false;
                bool isLootTG = false;

                for (unsigned int i = 0; (i < npcItems.size()) && (!isVendorTG); i++)
                {
                    if (itr->second.ItemId == npcItems[i])
                        isVendorTG = true;
                }
                for (unsigned int i = 0; (i < lootItems.size()) && (!isLootTG); i++)
                {
                    if (itr->second.ItemId == lootItems[i])
                        isLootTG = true;
                }
                if ((!isLootTG) && (!isVendorTG))
                    continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if Disabled");
            // Disable items by Id
            if (DisableItemStore.find(itr->second.ItemId) != DisableItemStore.end())
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (PTR/Beta/Unused Item)", itr->second.ItemId);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if Perm Enchant");
            // Disable permanent enchants items
            if ((DisablePermEnchant) && (itr->second.Class == ITEM_CLASS_PERMANENT))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (Permanent Enchant Item)", itr->second.ItemId);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if Conjured");
            // Disable conjured items
            if ((DisableConjured) && (itr->second.IsConjuredConsumable()))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (Conjured Consumable)", itr->second.ItemId);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if Gem");
            // Disable gems
            if ((DisableGems) && (itr->second.Class == ITEM_CLASS_GEM))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (Gem)", itr->second.ItemId);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if Money");
            // Disable money
            if ((DisableMoney) && (itr->second.Class == ITEM_CLASS_MONEY))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (Money)", itr->second.ItemId);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if MoneyLoot");
            // Disable moneyloot
            if ((DisableMoneyLoot) && (itr->second.MinMoneyLoot > 0))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (MoneyLoot)", itr->second.ItemId);
                continue;
            }

            // Disable lootable items
            if ((DisableLootable) && (itr->second.Flags & 4))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (Lootable Item)", itr->second.ItemId);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if Key");
            // Disable Keys
            if ((DisableKeys) && (itr->second.Class == ITEM_CLASS_KEY))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (Quest Item)", itr->second.ItemId);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if Limited Duration");
            // Disable items with duration
            if ((DisableDuration) && (itr->second.Duration > 0))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (Has a Duration)", itr->second.ItemId);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if low level BOP/Quest");
            // Disable items which are BOP or Quest Items and have a required level lower than the item level
            if ((DisableBOP_Or_Quest_NoReqLevel) && ((itr->second.Bonding == BIND_WHEN_PICKED_UP || itr->second.Bonding == BIND_QUEST_ITEM) && (itr->second.RequiredLevel < itr->second.ItemLevel)))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (BOP or BQI and Required Level is less than Item Level)", itr->second.ItemId);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if Warrior");
            // Disable items specifically for Warrior
            if ((DisableWarriorItems) && (itr->second.AllowableClass == 1))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (Warrior Item)", itr->second.ItemId);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if Paladin");
            // Disable items specifically for Paladin
            if ((DisablePaladinItems) && (itr->second.AllowableClass == 2))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (Paladin Item)", itr->second.ItemId);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if Hunter");
            // Disable items specifically for Hunter
            if ((DisableHunterItems) && (itr->second.AllowableClass == 4))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (Hunter Item)", itr->second.ItemId);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if Rogue");
            // Disable items specifically for Rogue
            if ((DisableRogueItems) && (itr->second.AllowableClass == 8))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (Rogue Item)", itr->second.ItemId);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if Priest");
            // Disable items specifically for Priest
            if ((DisablePriestItems) && (itr->second.AllowableClass == 16))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (Priest Item)", itr->second.ItemId);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if DK");
            // Disable items specifically for DK
            if ((DisableDKItems) && (itr->second.AllowableClass == 32))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (DK Item)", itr->second.ItemId);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if Shaman");
            // Disable items specifically for Shaman
            if ((DisableShamanItems) && (itr->second.AllowableClass == 64))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (Shaman Item)", itr->second.ItemId);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if Mage");
            // Disable items specifically for Mage
            if ((DisableMageItems) && (itr->second.AllowableClass == 128))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (Mage Item)", itr->second.ItemId);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if Warlock");
            // Disable items specifically for Warlock
            if ((DisableWarlockItems) && (itr->second.AllowableClass == 256))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (Warlock Item)", itr->second.ItemId);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if Unused Class");
            // Disable items specifically for Unused Class
            if ((DisableUnusedClassItems) && (itr->second.AllowableClass == 512))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (Unused Item)", itr->second.ItemId);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if Druid");
            // Disable items specifically for Druid
            if ((DisableDruidItems) && (itr->second.AllowableClass == 1024))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (Druid Item)", itr->second.ItemId);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if low level");
            // Disable Items below level X
            if ((DisableItemsBelowLevel) && (itr->second.Class != ITEM_CLASS_TRADE_GOODS) && (itr->second.ItemLevel < DisableItemsBelowLevel))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (Item Level = %u)", itr->second.ItemId, itr->second.ItemLevel);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if high level");
            // Disable Items above level X
            if ((DisableItemsAboveLevel) && (itr->second.Class != ITEM_CLASS_TRADE_GOODS) && (itr->second.ItemLevel > DisableItemsAboveLevel))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (Item Level = %u)", itr->second.ItemId, itr->second.ItemLevel);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if low level TG");
            // Disable Trade Goods below level X
            if ((DisableTGsBelowLevel) && (itr->second.Class == ITEM_CLASS_TRADE_GOODS) && (itr->second.ItemLevel < DisableTGsBelowLevel))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Trade Good %u disabled (Trade Good Level = %u)", itr->second.ItemId, itr->second.ItemLevel);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if high level TG");
            // Disable Trade Goods above level X
            if ((DisableTGsAboveLevel) && (itr->second.Class == ITEM_CLASS_TRADE_GOODS) && (itr->second.ItemLevel > DisableTGsAboveLevel))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Trade Good %u disabled (Trade Good Level = %u)", itr->second.ItemId, itr->second.ItemLevel);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if low GUID");
            // Disable Items below GUID X
            if ((DisableItemsBelowGUID) && (itr->second.Class != ITEM_CLASS_TRADE_GOODS) && (itr->second.ItemId < DisableItemsBelowGUID))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (Item Level = %u)", itr->second.ItemId, itr->second.ItemLevel);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if high GUID");
            // Disable Items above GUID X
            if ((DisableItemsAboveGUID) && (itr->second.Class != ITEM_CLASS_TRADE_GOODS) && (itr->second.ItemId > DisableItemsAboveGUID))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (Item Level = %u)", itr->second.ItemId, itr->second.ItemLevel);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if low GUID TG");
            // Disable Trade Goods below GUID X
            if ((DisableTGsBelowGUID) && (itr->second.Class == ITEM_CLASS_TRADE_GOODS) && (itr->second.ItemId < DisableTGsBelowGUID))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (Trade Good Level = %u)", itr->second.ItemId, itr->second.ItemLevel);
                continue;
            }


            if (debug_shou) sLog->outString("AuctionHouseBot: Check if high GUID TG");
            // Disable Trade Goods above GUID X
            if ((DisableTGsAboveGUID) && (itr->second.Class == ITEM_CLASS_TRADE_GOODS) && (itr->second.ItemId > DisableTGsAboveGUID))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (Trade Good Level = %u)", itr->second.ItemId, itr->second.ItemLevel);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if low level req");
            // Disable Items for level lower than X
            if ((DisableItemsBelowReqLevel) && (itr->second.RequiredLevel < DisableItemsBelowReqLevel))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (RequiredLevel = %u)", itr->second.ItemId, itr->second.RequiredLevel);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if high level req");
            // Disable Items for level higher than X
            if ((DisableItemsAboveReqLevel) && (itr->second.RequiredLevel > DisableItemsAboveReqLevel))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Item %u disabled (RequiredLevel = %u)", itr->second.ItemId, itr->second.RequiredLevel);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if low level req TG");
            // Disable Trade Goods for level lower than X
            if ((DisableTGsBelowReqLevel) && (itr->second.RequiredLevel < DisableTGsBelowReqLevel))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Trade Good %u disabled (RequiredLevel = %u)", itr->second.ItemId, itr->second.RequiredLevel);
                continue;
            }

            if (debug_shou) sLog->outString("AuctionHouseBot: Check if high level req TG");
            // Disable Trade Goods for level higher than X
            if ((DisableTGsAboveReqLevel) && (itr->second.RequiredLevel > DisableTGsAboveReqLevel))
            {
                if (debug_Out_Filters) sLog->outError("AuctionHouseBot: Trade Good %u disabled (RequiredLevel = %u)", itr->second.ItemId, itr->second.RequiredLevel);
                continue;
            }

            // Disable Items that require skill lower than X
            // if ((DisableItemsBelowReqSkillRank) && (itr->second.RequiredSkillRank < DisableItemsBelowReqSkillRank))
            // {
            //    if (debug_Out_Filters) sLog->outError( "AuctionHouseBot: Item %u disabled (RequiredSkillRank = %u)", itr->second.ItemId, itr->second.RequiredSkillRank);
            //    continue;
            // }

            // Disable Items that require skill higher than X
            // if ((DisableItemsAboveReqSkillRank) && (itr->second.RequiredSkillRank > DisableItemsAboveReqSkillRank))
            // {
            //    if (debug_Out_Filters) sLog->outError( "AuctionHouseBot: Item %u disabled (RequiredSkillRank = %u)", itr->second.ItemId, itr->second.RequiredSkillRank);
            //    continue;
            // }

            // Disable Trade Goods that require skill lower than X
            // if ((DisableTGsBelowReqSkillRank) && (itr->second.RequiredSkillRank < DisableTGsBelowReqSkillRank))
            // {
            //    if (debug_Out_Filters) sLog->outError( "AuctionHouseBot: Item %u disabled (RequiredSkillRank = %u)", itr->second.ItemId, itr->second.RequiredSkillRank);
            //    continue;
            // }

            // Disable Trade Goods that require skill higher than X
            // if ((DisableTGsAboveReqSkillRank) && (itr->second.?RequiredSkillRank > DisableTGsAboveReqSkillRank))
            // {
            //    if (debug_Out_Filters) sLog->outError( "AuctionHouseBot: Item %u disabled (RequiredSkillRank = %u)", itr->second.ItemId, itr->second.RequiredSkillRank);
            //    continue;
            // }

            if (debug_shou) sLog->outString("AuctionHouseBot: Ready to load item");
            if (!sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION))
            {
                if (debug_shou) sLog->outString("AuctionHouseBot: Loading for Alliance");
                loadItem(&AllianceConfig, itr->second);
                if (debug_shou) sLog->outString("AuctionHouseBot: Loading for Horde");
                loadItem(&HordeConfig, itr->second);
            }
            if (debug_shou) sLog->outString("AuctionHouseBot: Loading for Neutral");
            loadItem(&NeutralConfig, itr->second);
        }
        sLog->outString("AuctionHouseBot:");
        sLog->outString("%u disabled items", uint32(DisableItemStore.size()));
    }
    sLog->outString("AuctionHouseBot and AuctionHouseBuyer have been loaded.");
}
void AuctionHouseBot::loadItem(AHBConfig *config, ItemTemplate item) {
    if (debug_shou) sLog->outString("AuctionHouseBot: Routing Load to Config");
    config->LoadItem(item.Class, item.SubClass, item.Quality, item.ItemId);
}
void AuctionHouseBot::InitializeConfiguration()
{
    debug_Out = sConfigMgr->GetBoolDefault("AuctionHouseBot.DEBUG", false);
    debug_Out_Filters = sConfigMgr->GetBoolDefault("AuctionHouseBot.DEBUG_FILTERS", false);

    AHBSeller = sConfigMgr->GetBoolDefault("AuctionHouseBot.EnableSeller", false);
    AHBBuyer = sConfigMgr->GetBoolDefault("AuctionHouseBot.EnableBuyer", false);
    SellMethod = sConfigMgr->GetBoolDefault("AuctionHouseBot.UseBuyPriceForSeller", false);
    BuyMethod = sConfigMgr->GetBoolDefault("AuctionHouseBot.UseBuyPriceForBuyer", false);

    AHBplayerAccount = sConfigMgr->GetIntDefault("AuctionHouseBot.Account", 0);
    AHBplayerGUID = sConfigMgr->GetIntDefault("AuctionHouseBot.GUID", 0);
    ItemsPerCycle = sConfigMgr->GetIntDefault("AuctionHouseBot.ItemsPerCycle", 200);

    //Begin Filters

    Vendor_Items = sConfigMgr->GetBoolDefault("AuctionHouseBot.VendorItems", false);
    Loot_Items = sConfigMgr->GetBoolDefault("AuctionHouseBot.LootItems", true);
    Other_Items = sConfigMgr->GetBoolDefault("AuctionHouseBot.OtherItems", false);
    Vendor_TGs = sConfigMgr->GetBoolDefault("AuctionHouseBot.VendorTradeGoods", false);
    Loot_TGs = sConfigMgr->GetBoolDefault("AuctionHouseBot.LootTradeGoods", true);
    Other_TGs = sConfigMgr->GetBoolDefault("AuctionHouseBot.OtherTradeGoods", false);

    No_Bind = sConfigMgr->GetBoolDefault("AuctionHouseBot.No_Bind", true);
    Bind_When_Picked_Up = sConfigMgr->GetBoolDefault("AuctionHouseBot.Bind_When_Picked_Up", false);
    Bind_When_Equipped = sConfigMgr->GetBoolDefault("AuctionHouseBot.Bind_When_Equipped", true);
    Bind_When_Use = sConfigMgr->GetBoolDefault("AuctionHouseBot.Bind_When_Use", true);
    Bind_Quest_Item = sConfigMgr->GetBoolDefault("AuctionHouseBot.Bind_Quest_Item", false);

    DisablePermEnchant = sConfigMgr->GetBoolDefault("AuctionHouseBot.DisablePermEnchant", false);
    DisableConjured = sConfigMgr->GetBoolDefault("AuctionHouseBot.DisableConjured", false);
    DisableGems = sConfigMgr->GetBoolDefault("AuctionHouseBot.DisableGems", false);
    DisableMoney = sConfigMgr->GetBoolDefault("AuctionHouseBot.DisableMoney", false);
    DisableMoneyLoot = sConfigMgr->GetBoolDefault("AuctionHouseBot.DisableMoneyLoot", false);
    DisableLootable = sConfigMgr->GetBoolDefault("AuctionHouseBot.DisableLootable", false);
    DisableKeys = sConfigMgr->GetBoolDefault("AuctionHouseBot.DisableKeys", false);
    DisableDuration = sConfigMgr->GetBoolDefault("AuctionHouseBot.DisableDuration", false);
    DisableBOP_Or_Quest_NoReqLevel = sConfigMgr->GetBoolDefault("AuctionHouseBot.DisableBOP_Or_Quest_NoReqLevel", false);

    DisableWarriorItems = sConfigMgr->GetBoolDefault("AuctionHouseBot.DisableWarriorItems", false);
    DisablePaladinItems = sConfigMgr->GetBoolDefault("AuctionHouseBot.DisablePaladinItems", false);
    DisableHunterItems = sConfigMgr->GetBoolDefault("AuctionHouseBot.DisableHunterItems", false);
    DisableRogueItems = sConfigMgr->GetBoolDefault("AuctionHouseBot.DisableRogueItems", false);
    DisablePriestItems = sConfigMgr->GetBoolDefault("AuctionHouseBot.DisablePriestItems", false);
    DisableDKItems = sConfigMgr->GetBoolDefault("AuctionHouseBot.DisableDKItems", false);
    DisableShamanItems = sConfigMgr->GetBoolDefault("AuctionHouseBot.DisableShamanItems", false);
    DisableMageItems = sConfigMgr->GetBoolDefault("AuctionHouseBot.DisableMageItems", false);
    DisableWarlockItems = sConfigMgr->GetBoolDefault("AuctionHouseBot.DisableWarlockItems", false);
    DisableUnusedClassItems = sConfigMgr->GetBoolDefault("AuctionHouseBot.DisableUnusedClassItems", false);
    DisableDruidItems = sConfigMgr->GetBoolDefault("AuctionHouseBot.DisableDruidItems", false);

    DisableItemsBelowLevel = sConfigMgr->GetIntDefault("AuctionHouseBot.DisableItemsBelowLevel", 0);
    DisableItemsAboveLevel = sConfigMgr->GetIntDefault("AuctionHouseBot.DisableItemsAboveLevel", 0);
    DisableTGsBelowLevel = sConfigMgr->GetIntDefault("AuctionHouseBot.DisableTGsBelowLevel", 0);
    DisableTGsAboveLevel = sConfigMgr->GetIntDefault("AuctionHouseBot.DisableTGsAboveLevel", 0);
    DisableItemsBelowGUID = sConfigMgr->GetIntDefault("AuctionHouseBot.DisableItemsBelowGUID", 0);
    DisableItemsAboveGUID = sConfigMgr->GetIntDefault("AuctionHouseBot.DisableItemsAboveGUID", 0);
    DisableTGsBelowGUID = sConfigMgr->GetIntDefault("AuctionHouseBot.DisableTGsBelowGUID", 0);
    DisableTGsAboveGUID = sConfigMgr->GetIntDefault("AuctionHouseBot.DisableTGsAboveGUID", 0);
    DisableItemsBelowReqLevel = sConfigMgr->GetIntDefault("AuctionHouseBot.DisableItemsBelowReqLevel", 0);
    DisableItemsAboveReqLevel = sConfigMgr->GetIntDefault("AuctionHouseBot.DisableItemsAboveReqLevel", 0);
    DisableTGsBelowReqLevel = sConfigMgr->GetIntDefault("AuctionHouseBot.DisableTGsBelowReqLevel", 0);
    DisableTGsAboveReqLevel = sConfigMgr->GetIntDefault("AuctionHouseBot.DisableTGsAboveReqLevel", 0);
    DisableItemsBelowReqSkillRank = sConfigMgr->GetIntDefault("AuctionHouseBot.DisableItemsBelowReqSkillRank", 0);
    DisableItemsAboveReqSkillRank = sConfigMgr->GetIntDefault("AuctionHouseBot.DisableItemsAboveReqSkillRank", 0);
    DisableTGsBelowReqSkillRank = sConfigMgr->GetIntDefault("AuctionHouseBot.DisableTGsBelowReqSkillRank", 0);
    DisableTGsAboveReqSkillRank = sConfigMgr->GetIntDefault("AuctionHouseBot.DisableTGsAboveReqSkillRank", 0);
}
void AuctionHouseBot::IncrementItemCounts(AuctionEntry* ah)
{
    // from auctionhousehandler.cpp, creates auction pointer & player pointer

    // get exact item information
    Item *pItem =  sAuctionMgr->GetAItem(ah->item_guidlow);
    if (!pItem)
    {
		if (debug_Out) sLog->outError( "AHBot: Item %u doesn't exist, perhaps bought already?", ah->item_guidlow);
        return;
    }

    // get item prototype
    ItemTemplate const* prototype = sObjectMgr->GetItemTemplate(ah->item_template);

    AHBConfig *config;

    FactionTemplateEntry const* u_entry = sFactionTemplateStore.LookupEntry(ah->GetHouseFaction());
    if (!u_entry)
    {
        if (debug_Out) sLog->outError( "AHBot: %u returned as House Faction. Neutral", ah->GetHouseFaction());
        config = &NeutralConfig;
    }
    else if (u_entry->ourMask & FACTION_MASK_ALLIANCE)
    {
        if (debug_Out) sLog->outError( "AHBot: %u returned as House Faction. Alliance", ah->GetHouseFaction());
        config = &AllianceConfig;
    }
    else if (u_entry->ourMask & FACTION_MASK_HORDE)
    {
        if (debug_Out) sLog->outError( "AHBot: %u returned as House Faction. Horde", ah->GetHouseFaction());
        config = &HordeConfig;
    }
    else
    {
        if (debug_Out) sLog->outError( "AHBot: %u returned as House Faction. Neutral", ah->GetHouseFaction());
        config = &NeutralConfig;
    }
    config->IncrementItems();
}
void AuctionHouseBot::DecrementItemCounts(AuctionEntry* ah, uint32 itemEntry)
{
    // get item prototype
    ItemTemplate const* prototype = sObjectMgr->GetItemTemplate(itemEntry);

    AHBConfig *config;

    FactionTemplateEntry const* u_entry = sFactionTemplateStore.LookupEntry(ah->GetHouseFaction());
    if (!u_entry)
    {
        if (debug_Out) sLog->outError( "AHBot: %u returned as House Faction. Neutral", ah->GetHouseFaction());
        config = &NeutralConfig;
    }
    else if (u_entry->ourMask & FACTION_MASK_ALLIANCE)
    {
        if (debug_Out) sLog->outError( "AHBot: %u returned as House Faction. Alliance", ah->GetHouseFaction());
        config = &AllianceConfig;
    }
    else if (u_entry->ourMask & FACTION_MASK_HORDE)
    {
        if (debug_Out) sLog->outError( "AHBot: %u returned as House Faction. Horde", ah->GetHouseFaction());
        config = &HordeConfig;
    }
    else
    {
        if (debug_Out) sLog->outError( "AHBot: %u returned as House Faction. Neutral", ah->GetHouseFaction());
        config = &NeutralConfig;
    }
    config->DecrementItems();
}
void AuctionHouseBot::Commands(uint32 command, uint32 ahMapID, uint32 col, char* args)
{
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
    switch (command)
    {
    case 0:     //ahexpire
        {
            AuctionHouseObject* auctionHouse =  sAuctionMgr->GetAuctionsMap(config->GetAHFID());

            AuctionHouseObject::AuctionEntryMap::iterator itr;
            itr = auctionHouse->GetAuctionsBegin();

            while (itr != auctionHouse->GetAuctionsEnd())
            {
                if (itr->second->owner == AHBplayerGUID)
                {
                    itr->second->expire_time = sWorld->GetGameTime();
                    uint32 id = itr->second->Id;
                    uint32 expire_time = itr->second->expire_time;
                    CharacterDatabase.PExecute("UPDATE auctionhouse SET time = '%u' WHERE id = '%u'", expire_time, id);
                }
                ++itr;
            }
        }
        break;
    case 1:     //min items
        {
            char * param1 = strtok(args, " ");
            uint32 minItems = (uint32) strtoul(param1, NULL, 0);
            WorldDatabase.PExecute("UPDATE mod_auctionhousebot SET minitems = '%u' WHERE auctionhouse = '%u'", minItems, ahMapID);
            config->SetMinItems(minItems);
        }
        break;
    case 2:     //max items Deprecated
        {
            /*
            char * param1 = strtok(args, " ");
            uint32 maxItems = (uint32) strtoul(param1, NULL, 0);
			WorldDatabase.PExecute("UPDATE mod_auctionhousebot SET maxitems = '%u' WHERE auctionhouse = '%u'", maxItems, ahMapID);
            weightBins.at(config->GetAHID()).first.second.second.second = maxItems;
            */
        }
        break;
    case 3:     //min time Deprecated (Place holder for future commands)
        break;
    case 4:     //max time Deprecated (Place holder for future commands)
        break;
    case 5:     //percentages Deprecated
        {/*
            char * param1 = strtok(args, " ");
            char * param2 = strtok(NULL, " ");
            char * param3 = strtok(NULL, " ");
            char * param4 = strtok(NULL, " ");
            char * param5 = strtok(NULL, " ");
            char * param6 = strtok(NULL, " ");
            char * param7 = strtok(NULL, " ");
            char * param8 = strtok(NULL, " ");
            char * param9 = strtok(NULL, " ");
            char * param10 = strtok(NULL, " ");
            char * param11 = strtok(NULL, " ");
            char * param12 = strtok(NULL, " ");
            char * param13 = strtok(NULL, " ");
            char * param14 = strtok(NULL, " ");
            uint32 greytg = (uint32) strtoul(param1, NULL, 0);
            uint32 whitetg = (uint32) strtoul(param2, NULL, 0);
            uint32 greentg = (uint32) strtoul(param3, NULL, 0);
            uint32 bluetg = (uint32) strtoul(param4, NULL, 0);
            uint32 purpletg = (uint32) strtoul(param5, NULL, 0);
            uint32 orangetg = (uint32) strtoul(param6, NULL, 0);
            uint32 yellowtg = (uint32) strtoul(param7, NULL, 0);
            uint32 greyi = (uint32) strtoul(param8, NULL, 0);
            uint32 whitei = (uint32) strtoul(param9, NULL, 0);
            uint32 greeni = (uint32) strtoul(param10, NULL, 0);
            uint32 bluei = (uint32) strtoul(param11, NULL, 0);
            uint32 purplei = (uint32) strtoul(param12, NULL, 0);
            uint32 orangei = (uint32) strtoul(param13, NULL, 0);
            uint32 yellowi = (uint32) strtoul(param14, NULL, 0);

			SQLTransaction trans = WorldDatabase.BeginTransaction();
            trans->PAppend("UPDATE mod_auctionhousebot SET percentgreytradegoods = '%u' WHERE auctionhouse = '%u'", greytg, ahMapID);
            trans->PAppend("UPDATE mod_auctionhousebot SET percentwhitetradegoods = '%u' WHERE auctionhouse = '%u'", whitetg, ahMapID);
            trans->PAppend("UPDATE mod_auctionhousebot SET percentgreentradegoods = '%u' WHERE auctionhouse = '%u'", greentg, ahMapID);
            trans->PAppend("UPDATE mod_auctionhousebot SET percentbluetradegoods = '%u' WHERE auctionhouse = '%u'", bluetg, ahMapID);
            trans->PAppend("UPDATE mod_auctionhousebot SET percentpurpletradegoods = '%u' WHERE auctionhouse = '%u'", purpletg, ahMapID);
            trans->PAppend("UPDATE mod_auctionhousebot SET percentorangetradegoods = '%u' WHERE auctionhouse = '%u'", orangetg, ahMapID);
            trans->PAppend("UPDATE mod_auctionhousebot SET percentyellowtradegoods = '%u' WHERE auctionhouse = '%u'", yellowtg, ahMapID);
            trans->PAppend("UPDATE mod_auctionhousebot SET percentgreyitems = '%u' WHERE auctionhouse = '%u'", greyi, ahMapID);
            trans->PAppend("UPDATE mod_auctionhousebot SET percentwhiteitems = '%u' WHERE auctionhouse = '%u'", whitei, ahMapID);
            trans->PAppend("UPDATE mod_auctionhousebot SET percentgreenitems = '%u' WHERE auctionhouse = '%u'", greeni, ahMapID);
            trans->PAppend("UPDATE mod_auctionhousebot SET percentblueitems = '%u' WHERE auctionhouse = '%u'", bluei, ahMapID);
            trans->PAppend("UPDATE mod_auctionhousebot SET percentpurpleitems = '%u' WHERE auctionhouse = '%u'", purplei, ahMapID);
            trans->PAppend("UPDATE mod_auctionhousebot SET percentorangeitems = '%u' WHERE auctionhouse = '%u'", orangei, ahMapID);
            trans->PAppend("UPDATE mod_auctionhousebot SET percentyellowitems = '%u' WHERE auctionhouse = '%u'", yellowi, ahMapID);
			WorldDatabase.CommitTransaction(trans);
            //config->SetPercentages(greytg, whitetg, greentg, bluetg, purpletg, orangetg, yellowtg, greyi, whitei, greeni, bluei, purplei, orangei, yellowi);
            */
        }
        break;
    case 6:     //min prices
        {/*
            char * param1 = strtok(args, " ");
            uint32 minPrice = (uint32) strtoul(param1, NULL, 0);
			WorldDatabase.PExecute("UPDATE mod_auctionhousebot SET minprice%s = '%u' WHERE auctionhouse = '%u'", color.c_str(), minPrice, ahMapID);
            config->SetMinPrice(col, minPrice);*/
        }
        break;
    case 7:     //max prices
        {/*
            char * param1 = strtok(args, " ");
            uint32 maxPrice = (uint32) strtoul(param1, NULL, 0);
			WorldDatabase.PExecute("UPDATE mod_auctionhousebot SET maxprice%s = '%u' WHERE auctionhouse = '%u'", color.c_str(), maxPrice, ahMapID);
            config->SetMaxPrice(col, maxPrice);*/
        }
        break;
    case 8:     //min bid price
        {/*
            char * param1 = strtok(args, " ");
            uint32 minBidPrice = (uint32) strtoul(param1, NULL, 0);
			WorldDatabase.PExecute("UPDATE mod_auctionhousebot SET minbidprice%s = '%u' WHERE auctionhouse = '%u'", color.c_str(), minBidPrice, ahMapID);
            config->SetMinBidPrice(col, minBidPrice);*/
        }
        break;
    case 9:     //max bid price
        {/*
            char * param1 = strtok(args, " ");
            uint32 maxBidPrice = (uint32) strtoul(param1, NULL, 0);
			WorldDatabase.PExecute("UPDATE mod_auctionhousebot SET maxbidprice%s = '%u' WHERE auctionhouse = '%u'", color.c_str(), maxBidPrice, ahMapID);
            config->SetMaxBidPrice(col, maxBidPrice);*/
        }
        break;
    case 10:        //max stacks
        {/*
            char * param1 = strtok(args, " ");
            uint32 maxStack = (uint32) strtoul(param1, NULL, 0);
			WorldDatabase.PExecute("UPDATE mod_auctionhousebot SET maxstack%s = '%u' WHERE auctionhouse = '%u'", color.c_str(), maxStack, ahMapID);
            config->SetMaxStack(col, maxStack);*/
        }
        break;
    case 11:        //buyer bid prices
        {/*
            char * param1 = strtok(args, " ");
            uint32 buyerPrice = (uint32) strtoul(param1, NULL, 0);
			WorldDatabase.PExecute("UPDATE mod_auctionhousebot SET buyerprice%s = '%u' WHERE auctionhouse = '%u'", color.c_str(), buyerPrice, ahMapID);
            config->SetBuyerPrice(col, buyerPrice);*/
        }
        break;
    case 12:        //buyer bidding interval
        {
            char * param1 = strtok(args, " ");
            uint32 bidInterval = (uint32) strtoul(param1, NULL, 0);
			WorldDatabase.PExecute("UPDATE mod_auctionhousebot SET buyerbiddinginterval = '%u' WHERE auctionhouse = '%u'", bidInterval, ahMapID);
            config->SetBiddingInterval(bidInterval);
        }
        break;
    case 13:        //buyer bids per interval
        {
            char * param1 = strtok(args, " ");
            uint32 bidsPerInterval = (uint32) strtoul(param1, NULL, 0);
			WorldDatabase.PExecute("UPDATE mod_auctionhousebot SET buyerbidsperinterval = '%u' WHERE auctionhouse = '%u'", bidsPerInterval, ahMapID);
            config->SetBidsPerInterval(bidsPerInterval);
        }
        break;
    default:
        break;
    }
}
void AuctionHouseBot::LoadValues(AHBConfig *config)
{
    //if (debug_Out)
	if(debug_shou)	sLog->outError( "Start Settings for %s Auctionhouses:", WorldDatabase.PQuery("SELECT name FROM mod_auctionhousebot WHERE auctionhouse = %u", config->GetAHID())->Fetch()->GetCString());
    if (AHBSeller)
    {
        if (debug_shou) sLog->outString("AuctionHouseBot: setting minitems");
        config->SetMinItems(WorldDatabase.PQuery("SELECT minitems FROM mod_auctionhousebot WHERE auctionhouse = %u", config->GetAHID())->Fetch()->GetUInt32());
        if (debug_shou) sLog->outString("AuctionHouseBot: setting bidding Interval");
        config->SetBiddingInterval(WorldDatabase.PQuery("SELECT buyerbiddinginterval FROM mod_auctionhousebot WHERE auctionhouse = %u", config->GetAHID())->Fetch()->GetUInt32());
        if (debug_shou) sLog->outString("AuctionHouseBot: setting bidsPerInterval");
        config->SetBidsPerInterval(WorldDatabase.PQuery("SELECT buyerbidsperinterval FROM mod_auctionhousebot WHERE auctionhouse = %u", config->GetAHID())->Fetch()->GetUInt32());
        uint32 totalWeight = 0;
        if (debug_shou) sLog->outString("AuctionHouseBot: SELECT * FROM mod_ah_bot_weight_table WHERE AHID = %u", config->GetAHID());
        QueryResult result = WorldDatabase.PQuery("SELECT * FROM mod_ah_bot_weight_table WHERE AHID = %u", config->GetAHID());
        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                if (debug_shou) sLog->outString("AuctionHouseBot: Loading Class %u Subclass %u Quality %u", fields[0].GetUInt32(), fields[1].GetUInt32(), fields[2].GetUInt32());
                if (fields[4].GetUInt32() > 0) {
                    unordered_map < uint32, unordered_map<uint32, unordered_map<uint32, qualityData*>>> classMap;
                    //if (debug_shou) sLog->outString("AuctionHouseBot: getting ClassMap");
                    classMap = config->GetClassMap();
                    unordered_map<uint32, unordered_map<uint32, qualityData*>> subclassMap;
                    try {
                        //if (debug_shou) sLog->outString("AuctionHouseBot: getting subclassMap");
                        subclassMap = classMap.at(fields[0].GetUInt32());
                        //if (debug_shou) sLog->outString("AuctionHouseBot: got subclassMap");
                    }
                    catch (out_of_range) {
                        classMap.insert_or_assign(fields[0].GetUInt32(), subclassMap);
                    }
                    unordered_map<uint32, qualityData*> qualities;
                    try {
                        qualities = subclassMap.at(fields[1].GetUInt32());
                    }
                    catch (out_of_range) {
                        subclassMap.insert_or_assign(fields[1].GetUInt32(), qualities);
                    }
                    qualityData* qd;
                    try {
                        //if (debug_shou) sLog->outString("AuctionHouseBot: getting qualityData");
                        qd = qualities.at(fields[2].GetUInt32());
                        //if (debug_shou) sLog->outString("AuctionHouseBot: got qualityData");
                    }
                    catch (out_of_range) {
                        //if (debug_shou) sLog->outString("AuctionHouseBot: making new qualityData");
                        qd = new qualityData();
                        //if (debug_shou) sLog->outString("AuctionHouseBot: adding qualityData");
                        qualities.insert_or_assign(fields[2].GetUInt32(), qd);
                    }
                    //0 weight, 1 MinPrice, 2 Max Price, 3 Min bid, 4 Max bid, 5 Max Stack, 6 Buyer Price
                    //if (debug_shou) sLog->outString("AuctionHouseBot: getting typeConfig");
                    vector<uint32> itemTypeConfig = qd->itemTypeConfig;
                    //if (debug_shou) sLog->outString("AuctionHouseBot: pushing in data");
                    itemTypeConfig.push_back(fields[4].GetUInt32());
                    itemTypeConfig.push_back(fields[5].GetUInt32());
                    itemTypeConfig.push_back(fields[6].GetUInt32());
                    itemTypeConfig.push_back(fields[7].GetUInt32());
                    itemTypeConfig.push_back(fields[8].GetUInt32());
                    itemTypeConfig.push_back(fields[9].GetUInt32());
                    itemTypeConfig.push_back(fields[10].GetUInt32());
                    totalWeight += fields[4].GetUInt32();
                }else {
                    if (debug_shou) sLog->outString("AuctionHouseBot: 0 weight");
                }

            } while (result->NextRow());
        }
        config->SetTotalWeights(totalWeight);
        //AuctionHouseEntry const* ahEntry =  sAuctionMgr->GetAuctionHouseEntry(config->GetAHFID());
        AuctionHouseObject* auctionHouse =  sAuctionMgr->GetAuctionsMap(config->GetAHFID());

        //config->ResetItemCounts();
        //uint32 auctions = auctionHouse->Getcount();
        config->SetCurrentItems(auctionHouse->Getcount());
        /*if (auctions)
        {
            for (AuctionHouseObject::AuctionEntryMap::const_iterator itr = auctionHouse->GetAuctionsBegin(); itr != auctionHouse->GetAuctionsEnd(); ++itr)
            {
                AuctionEntry *Aentry = itr->second;
				Item *item = sAuctionMgr->GetAItem(Aentry->item_guidlow);
                if (item)
                {
                    ItemTemplate const *prototype = item->GetTemplate();
                    if (prototype)
                    {
                        config->IncrementItems();
                    }
                }
            }
        }*/
    }

	if (debug_Out) sLog->outError( "End Settings for %s Auctionhouses:", WorldDatabase.PQuery("SELECT name FROM mod_auctionhousebot WHERE auctionhouse = %u", config->GetAHID())->Fetch()->GetCString());
}
