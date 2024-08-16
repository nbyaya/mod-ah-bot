#include "AuctionHouseBot.h"
#include "AuctionHouseBotAuctionHouseScript.h"

AHBot_AuctionHouseScript::AHBot_AuctionHouseScript() : AuctionHouseScript("AHBot_AuctionHouseScript")
{

}

void AHBot_AuctionHouseScript::OnBeforeAuctionHouseMgrSendAuctionSuccessfulMail(
    AuctionHouseMgr*,                /*auctionHouseMgr*/
    AuctionEntry*,                   /*auction*/
    Player* owner,
    uint32&,                         /*owner_accId*/
    uint32&,                         /*profit*/
    bool& sendNotification,
    bool& updateAchievementCriteria,
    bool&                            /*sendMail*/)
{
    if (owner && owner->GetGUID().GetCounter() == auctionbot->GetAHBplayerGUID())
    {
        sendNotification          = false;
        updateAchievementCriteria = false;
    }
}

void AHBot_AuctionHouseScript::OnBeforeAuctionHouseMgrSendAuctionExpiredMail(
    AuctionHouseMgr*,       /* auctionHouseMgr */
    AuctionEntry*,          /* auction */
    Player* owner,
    uint32&,                /* owner_accId */
    bool& sendNotification,
    bool&                   /* sendMail */)
{
    if (owner && owner->GetGUID().GetCounter() == auctionbot->GetAHBplayerGUID())
    {
        sendNotification = false;
    }
}

void AHBot_AuctionHouseScript::OnBeforeAuctionHouseMgrSendAuctionOutbiddedMail(
    AuctionHouseMgr*,      /* auctionHouseMgr */
    AuctionEntry* auction,
    Player* oldBidder,
    uint32&,               /* oldBidder_accId */
    Player* newBidder,
    uint32& newPrice,
    bool&,                 /* sendNotification */
    bool&                  /* sendMail */)
{
    if (oldBidder && !newBidder)
    {
        oldBidder->GetSession()->SendAuctionBidderNotification(
            auction->GetHouseId(),
            auction->Id,
            ObjectGuid::Create<HighGuid::Player>(auctionbot->GetAHBplayerGUID()),
            newPrice,
            auction->GetAuctionOutBid(),
            auction->item_template);
    }
}

void AHBot_AuctionHouseScript::OnAuctionAdd(AuctionHouseObject* /*ah*/, AuctionEntry* auction)
{
    //
    // Keeps updated the amount of items in the auction
    //

    auctionbot->IncrementItemCounts(auction);
}

void AHBot_AuctionHouseScript::OnAuctionRemove(AuctionHouseObject* /*ah*/, AuctionEntry* auction)
{
    //
    // Keeps updated the amount of items in the auction
    //

    auctionbot->DecrementItemCounts(auction);
}

void AHBot_AuctionHouseScript::OnBeforeAuctionHouseMgrUpdate()
{
    auctionbot->Update();
}

