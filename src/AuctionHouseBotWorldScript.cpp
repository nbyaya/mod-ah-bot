#include "AuctionHouseBot.h"
#include "AuctionHouseBotWorldScript.h"

// =============================================================================
// Initialization of the bot during the world startup
// =============================================================================

AHBot_WorldScript::AHBot_WorldScript() : WorldScript("AHBot_WorldScript")
{

}

void AHBot_WorldScript::OnBeforeConfigLoad(bool /*reload*/)
{
    auctionbot->InitializeConfiguration();
}

void AHBot_WorldScript::OnStartup()
{
    LOG_INFO("server.loading", "Initialize AuctionHouseBot...");
    auctionbot->Initialize();
}
