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

#include "Common.h"
#include "ItemTemplate.h"

#include "AuctionHouseBotCommon.h"
#include "AuctionHouseBotConfig.h"

using namespace std;

AHBConfig::AHBConfig()
{
    Reset();
}

AHBConfig::AHBConfig(uint32 ahid)
{
    Reset();

    AHID = ahid;

    switch (ahid)
    {
    case 2:
        AHFID = 55;  // Alliance
        break;

    case 6:
        AHFID = 29;  // Horde
        break;

    case 7:
        AHFID = 120; // Neutral
        break;

    default:
        AHFID = 120; // Alliance
        break;
    }
}

AHBConfig::~AHBConfig()
{
}

void AHBConfig::Reset()
{
    AHID                    = 0;
    AHFID                   = 0;

    minItems                = 0;
    maxItems                = 0;

    percentGreyTradeGoods   = 0;
    percentWhiteTradeGoods  = 0;
    percentGreenTradeGoods  = 0;
    percentBlueTradeGoods   = 0;
    percentPurpleTradeGoods = 0;
    percentOrangeTradeGoods = 0;
    percentYellowTradeGoods = 0;

    percentGreyItems        = 0;
    percentWhiteItems       = 0;
    percentGreenItems       = 0;
    percentBlueItems        = 0;
    percentPurpleItems      = 0;
    percentOrangeItems      = 0;
    percentYellowItems      = 0;

    minPriceGrey            = 0;
    maxPriceGrey            = 0;
    minBidPriceGrey         = 0;
    maxBidPriceGrey         = 0;
    maxStackGrey            = 0;

    minPriceWhite           = 0;
    maxPriceWhite           = 0;
    minBidPriceWhite        = 0;
    maxBidPriceWhite        = 0;
    maxStackWhite           = 0;

    minPriceGreen           = 0;
    maxPriceGreen           = 0;
    minBidPriceGreen        = 0;
    maxBidPriceGreen        = 0;
    maxStackGreen           = 0;

    minPriceBlue            = 0;
    maxPriceBlue            = 0;
    minBidPriceBlue         = 0;
    maxBidPriceBlue         = 0;
    maxStackBlue            = 0;

    minPricePurple          = 0;
    maxPricePurple          = 0;
    minBidPricePurple       = 0;
    maxBidPricePurple       = 0;
    maxStackPurple          = 0;

    minPriceOrange          = 0;
    maxPriceOrange          = 0;
    minBidPriceOrange       = 0;
    maxBidPriceOrange       = 0;
    maxStackOrange          = 0;

    minPriceYellow          = 0;
    maxPriceYellow          = 0;
    minBidPriceYellow       = 0;
    maxBidPriceYellow       = 0;
    maxStackYellow          = 0;

    buyerPriceGrey          = 0;
    buyerPriceWhite         = 0;
    buyerPriceGreen         = 0;
    buyerPriceBlue          = 0;
    buyerPricePurple        = 0;
    buyerPriceOrange        = 0;
    buyerPriceYellow        = 0;

    buyerBiddingInterval    = 0;
    buyerBidsPerInterval    = 0;

    greytgp                 = 0;
    whitetgp                = 0;
    greentgp                = 0;
    bluetgp                 = 0;
    purpletgp               = 0;
    orangetgp               = 0;
    yellowtgp               = 0;

    greyip                  = 0;
    whiteip                 = 0;
    greenip                 = 0;
    blueip                  = 0;
    purpleip                = 0;
    orangeip                = 0;
    yellowip                = 0;

    greyTGoods              = 0;
    whiteTGoods             = 0;
    greenTGoods             = 0;
    blueTGoods              = 0;
    purpleTGoods            = 0;
    orangeTGoods            = 0;
    yellowTGoods            = 0;

    greyItems               = 0;
    whiteItems              = 0;
    greenItems              = 0;
    blueItems               = 0;
    purpleItems             = 0;
    orangeItems             = 0;
    yellowItems             = 0;
}

uint32 AHBConfig::GetAHID()
{
    return AHID;
}

uint32 AHBConfig::GetAHFID()
{
    return AHFID;
}

void AHBConfig::SetMinItems(uint32 value)
{
    minItems = value;
}

uint32 AHBConfig::GetMinItems()
{
    if ((minItems == 0) && (maxItems))
    {
        return maxItems;
    }
    else if ((maxItems) && (minItems > maxItems))
    {
        return maxItems;
    }
    else
    {
        return minItems;
    }
}

void AHBConfig::SetMaxItems(uint32 value)
{
    maxItems = value;
    // CalculatePercents() needs to be called, but only if
    // SetPercentages() has been called at least once already.
}

uint32 AHBConfig::GetMaxItems()
{
    return maxItems;
}

void AHBConfig::SetPercentages(
    uint32 greytg,
    uint32 whitetg,
    uint32 greentg,
    uint32 bluetg,
    uint32 purpletg,
    uint32 orangetg,
    uint32 yellowtg,
    uint32 greyi,
    uint32 whitei,
    uint32 greeni,
    uint32 bluei,
    uint32 purplei,
    uint32 orangei,
    uint32 yellowi)
{
    uint32 totalPercent =
        greytg +
        whitetg +
        greentg +
        bluetg +
        purpletg +
        orangetg +
        yellowtg +
        greyi +
        whitei +
        greeni +
        bluei +
        purplei +
        orangei +
        yellowi;

    if (totalPercent == 0)
    {
        maxItems = 0;
    }
    else if (totalPercent != 100)
    {
        greytg   = 0;
        whitetg  = 27;
        greentg  = 12;
        bluetg   = 10;
        purpletg = 1;
        orangetg = 0;
        yellowtg = 0;

        greyi    = 0;
        whitei   = 10;
        greeni   = 30;
        bluei    = 8;
        purplei  = 2;
        orangei  = 0;
        yellowi  = 0;
    }

    percentGreyTradeGoods   = greytg;
    percentWhiteTradeGoods  = whitetg;
    percentGreenTradeGoods  = greentg;
    percentBlueTradeGoods   = bluetg;
    percentPurpleTradeGoods = purpletg;
    percentOrangeTradeGoods = orangetg;
    percentYellowTradeGoods = yellowtg;
    percentGreyItems        = greyi;
    percentWhiteItems       = whitei;
    percentGreenItems       = greeni;
    percentBlueItems        = bluei;
    percentPurpleItems      = purplei;
    percentOrangeItems      = orangei;
    percentYellowItems      = yellowi;

    CalculatePercents();
}

uint32 AHBConfig::GetPercentages(uint32 color)
{
    switch (color)
    {
    case AHB_GREY_TG:
        return percentGreyTradeGoods;
        break;

    case AHB_WHITE_TG:
        return percentWhiteTradeGoods;
        break;

    case AHB_GREEN_TG:
        return percentGreenTradeGoods;
        break;

    case AHB_BLUE_TG:
        return percentBlueTradeGoods;
        break;

    case AHB_PURPLE_TG:
        return percentPurpleTradeGoods;
        break;

    case AHB_ORANGE_TG:
        return percentOrangeTradeGoods;
        break;

    case AHB_YELLOW_TG:
        return percentYellowTradeGoods;
        break;

    case AHB_GREY_I:
        return percentGreyItems;
        break;

    case AHB_WHITE_I:
        return percentWhiteItems;
        break;

    case AHB_GREEN_I:
        return percentGreenItems;
        break;

    case AHB_BLUE_I:
        return percentBlueItems;
        break;

    case AHB_PURPLE_I:
        return percentPurpleItems;
        break;

    case AHB_ORANGE_I:
        return percentOrangeItems;
        break;

    case AHB_YELLOW_I:
        return percentYellowItems;
        break;

    default:
        return 0;
        break;
    }
}

void AHBConfig::SetMinPrice(uint32 color, uint32 value)
{
    switch (color)
    {
    case AHB_GREY:
        minPriceGrey = value;
        break;

    case AHB_WHITE:
        minPriceWhite = value;
        break;

    case AHB_GREEN:
        minPriceGreen = value;
        break;

    case AHB_BLUE:
        minPriceBlue = value;
        break;

    case AHB_PURPLE:
        minPricePurple = value;
        break;

    case AHB_ORANGE:
        minPriceOrange = value;
        break;

    case AHB_YELLOW:
        minPriceYellow = value;
        break;

    default:
        break;
    }
}

uint32 AHBConfig::GetMinPrice(uint32 color)
{
    switch (color)
    {
    case AHB_GREY:
    {
        if (minPriceGrey == 0)
        {
            return 100;
        }
        else if (minPriceGrey > maxPriceGrey)
        {
            return maxPriceGrey;
        }
        else
        {
            return minPriceGrey;
        }

        break;
    }

    case AHB_WHITE:
    {
        if (minPriceWhite == 0)
        {
            return 150;
        }
        else if (minPriceWhite > maxPriceWhite)
        {
            return maxPriceWhite;
        }
        else
        {
            return minPriceWhite;
        }

        break;
    }

    case AHB_GREEN:
    {
        if (minPriceGreen == 0)
        {
            return 200;
        }
        else if (minPriceGreen > maxPriceGreen)
        {
            return maxPriceGreen;
        }
        else
        {
            return minPriceGreen;
        }

        break;
    }

    case AHB_BLUE:
    {
        if (minPriceBlue == 0)
        {
            return 250;
        }
        else if (minPriceBlue > maxPriceBlue)
        {
            return maxPriceBlue;
        }
        else
        {
            return minPriceBlue;
        }

        break;
    }

    case AHB_PURPLE:
    {
        if (minPricePurple == 0)
        {
            return 300;
        }
        else if (minPricePurple > maxPricePurple)
        {
            return maxPricePurple;
        }
        else
        {
            return minPricePurple;
        }

        break;
    }

    case AHB_ORANGE:
    {
        if (minPriceOrange == 0)
        {
            return 400;
        }
        else if (minPriceOrange > maxPriceOrange)
        {
            return maxPriceOrange;
        }
        else
        {
            return minPriceOrange;
        }

        break;
    }

    case AHB_YELLOW:
    {
        if (minPriceYellow == 0)
        {
            return 500;
        }
        else if (minPriceYellow > maxPriceYellow)
        {
            return maxPriceYellow;
        }
        else
        {
            return minPriceYellow;
        }

        break;
    }

    default:
        return 0;
    }
}

void AHBConfig::SetMaxPrice(uint32 color, uint32 value)
{
    switch (color)
    {
    case AHB_GREY:
        maxPriceGrey = value;
        break;

    case AHB_WHITE:
        maxPriceWhite = value;
        break;

    case AHB_GREEN:
        maxPriceGreen = value;
        break;

    case AHB_BLUE:
        maxPriceBlue = value;
        break;

    case AHB_PURPLE:
        maxPricePurple = value;
        break;

    case AHB_ORANGE:
        maxPriceOrange = value;
        break;

    case AHB_YELLOW:
        maxPriceYellow = value;
        break;

    default:
        break;
    }
}

uint32 AHBConfig::GetMaxPrice(uint32 color)
{
    switch (color)
    {
    case AHB_GREY:
    {
        if (maxPriceGrey == 0)
        {
            return 150;
        }
        else
        {
            return maxPriceGrey;
        }

        break;
    }

    case AHB_WHITE:
    {
        if (maxPriceWhite == 0)
        {
            return 250;
        }
        else
        {
            return maxPriceWhite;
        }

        break;
    }

    case AHB_GREEN:
    {
        if (maxPriceGreen == 0)
        {
            return 300;
        }
        else
        {
            return maxPriceGreen;
        }

        break;
    }

    case AHB_BLUE:
    {
        if (maxPriceBlue == 0)
        {
            return 350;
        }
        else
        {
            return maxPriceBlue;
        }

        break;
    }

    case AHB_PURPLE:
    {
        if (maxPricePurple == 0)
        {
            return 450;
        }
        else
        {
            return maxPricePurple;
        }

        break;
    }

    case AHB_ORANGE:
    {
        if (maxPriceOrange == 0)
        {
            return 550;
        }
        else
        {
            return maxPriceOrange;
        }

        break;
    }

    case AHB_YELLOW:
    {
        if (maxPriceYellow == 0)
        {
            return 650;
        }
        else
        {
            return maxPriceYellow;
        }

        break;
    }

    default:
        return 0;

    }
}

void AHBConfig::SetMinBidPrice(uint32 color, uint32 value)
{
    switch (color)
    {
    case AHB_GREY:
        minBidPriceGrey = value;
        break;

    case AHB_WHITE:
        minBidPriceWhite = value;
        break;

    case AHB_GREEN:
        minBidPriceGreen = value;
        break;

    case AHB_BLUE:
        minBidPriceBlue = value;
        break;

    case AHB_PURPLE:
        minBidPricePurple = value;
        break;

    case AHB_ORANGE:
        minBidPriceOrange = value;
        break;

    case AHB_YELLOW:
        minBidPriceYellow = value;
        break;

    default:
        break;
    }
}

uint32 AHBConfig::GetMinBidPrice(uint32 color)
{
    switch (color)
    {
    case AHB_GREY:
    {
        if (minBidPriceGrey > 100)
        {
            return 100;
        }
        else
        {
            return minBidPriceGrey;
        }

        break;
    }

    case AHB_WHITE:
    {
        if (minBidPriceWhite > 100)
        {
            return 100;
        }
        else
        {
            return minBidPriceWhite;
        }

        break;
    }

    case AHB_GREEN:
    {
        if (minBidPriceGreen > 100)
        {
            return 100;
        }
        else
        {
            return minBidPriceGreen;
        }

        break;
    }

    case AHB_BLUE:
    {
        if (minBidPriceBlue > 100)
        {
            return 100;
        }
        else
        {
            return minBidPriceBlue;
        }

        break;
    }

    case AHB_PURPLE:
    {
        if (minBidPricePurple > 100)
        {
            return 100;
        }
        else
        {
            return minBidPricePurple;
        }

        break;
    }

    case AHB_ORANGE:
    {
        if (minBidPriceOrange > 100)
        {
            return 100;
        }
        else
        {
            return minBidPriceOrange;
        }

        break;
    }

    case AHB_YELLOW:
    {
        if (minBidPriceYellow > 100)
        {
            return 100;
        }
        else
        {
            return minBidPriceYellow;
        }

        break;
    }

    default:
        return 0;
    }
}

void AHBConfig::SetMaxBidPrice(uint32 color, uint32 value)
{
    switch (color)
    {
    case AHB_GREY:
        maxBidPriceGrey = value;
        break;

    case AHB_WHITE:
        maxBidPriceWhite = value;
        break;

    case AHB_GREEN:
        maxBidPriceGreen = value;
        break;

    case AHB_BLUE:
        maxBidPriceBlue = value;
        break;

    case AHB_PURPLE:
        maxBidPricePurple = value;
        break;

    case AHB_ORANGE:
        maxBidPriceOrange = value;
        break;

    case AHB_YELLOW:
        maxBidPriceYellow = value;
        break;

    default:
        break;
    }

}
uint32 AHBConfig::GetMaxBidPrice(uint32 color)
{
    switch (color)
    {
    case AHB_GREY:
    {
        if (maxBidPriceGrey > 100)
        {
            return 100;
        }
        else
        {
            return maxBidPriceGrey;
        }

        break;
    }

    case AHB_WHITE:
    {
        if (maxBidPriceWhite > 100)
        {
            return 100;
        }
        else
        {
            return maxBidPriceWhite;
        }

        break;
    }

    case AHB_GREEN:
    {
        if (maxBidPriceGreen > 100)
        {
            return 100;
        }
        else
        {
            return maxBidPriceGreen;
        }

        break;
    }

    case AHB_BLUE:
    {
        if (maxBidPriceBlue > 100)
        {
            return 100;
        }
        else
        {
            return maxBidPriceBlue;
        }

        break;
    }

    case AHB_PURPLE:
    {
        if (maxBidPricePurple > 100)
        {
            return 100;
        }
        else
        {
            return maxBidPricePurple;
        }

        break;
    }

    case AHB_ORANGE:
    {
        if (maxBidPriceOrange > 100)
        {
            return 100;
        }
        else
        {
            return maxBidPriceOrange;
        }

        break;
    }

    case AHB_YELLOW:
    {
        if (maxBidPriceYellow > 100)
        {
            return 100;
        }
        else
        {
            return maxBidPriceYellow;
        }

        break;
    }

    default:
        return 0;
    }
}

void AHBConfig::SetMaxStack(uint32 color, uint32 value)
{
    switch (color)
    {
    case AHB_GREY:
        maxStackGrey = value;
        break;

    case AHB_WHITE:
        maxStackWhite = value;
        break;

    case AHB_GREEN:
        maxStackGreen = value;
        break;

    case AHB_BLUE:
        maxStackBlue = value;
        break;

    case AHB_PURPLE:
        maxStackPurple = value;
        break;

    case AHB_ORANGE:
        maxStackOrange = value;
        break;

    case AHB_YELLOW:
        maxStackYellow = value;
        break;

    default:
        break;
    }
}

uint32 AHBConfig::GetMaxStack(uint32 color)
{
    switch (color)
    {
    case AHB_GREY:
    {
        return maxStackGrey;
        break;
    }

    case AHB_WHITE:
    {
        return maxStackWhite;
        break;
    }

    case AHB_GREEN:
    {
        return maxStackGreen;
        break;
    }

    case AHB_BLUE:
    {
        return maxStackBlue;
        break;
    }

    case AHB_PURPLE:
    {
        return maxStackPurple;
        break;
    }

    case AHB_ORANGE:
    {
        return maxStackOrange;
        break;
    }

    case AHB_YELLOW:
    {
        return maxStackYellow;
        break;
    }

    default:
        return 0;
    }
}
void AHBConfig::SetBuyerPrice(uint32 color, uint32 value)
{
    switch (color)
    {
    case AHB_GREY:
        buyerPriceGrey = value;
        break;

    case AHB_WHITE:
        buyerPriceWhite = value;
        break;

    case AHB_GREEN:
        buyerPriceGreen = value;
        break;

    case AHB_BLUE:
        buyerPriceBlue = value;
        break;

    case AHB_PURPLE:
        buyerPricePurple = value;
        break;

    case AHB_ORANGE:
        buyerPriceOrange = value;
        break;

    case AHB_YELLOW:
        buyerPriceYellow = value;
        break;

    default:
        break;
    }
}

uint32 AHBConfig::GetBuyerPrice(uint32 color)
{
    switch (color)
    {
    case AHB_GREY:
        return buyerPriceGrey;
        break;

    case AHB_WHITE:
        return buyerPriceWhite;
        break;

    case AHB_GREEN:
        return buyerPriceGreen;
        break;

    case AHB_BLUE:
        return buyerPriceBlue;
        break;

    case AHB_PURPLE:
        return buyerPricePurple;
        break;

    case AHB_ORANGE:
        return buyerPriceOrange;
        break;

    case AHB_YELLOW:
        return buyerPriceYellow;
        break;

    default:
        return 0;
        break;
    }
}

void AHBConfig::SetBiddingInterval(uint32 value)
{
    buyerBiddingInterval = value;
}

uint32 AHBConfig::GetBiddingInterval()
{
    return buyerBiddingInterval;
}

void AHBConfig::CalculatePercents()
{
    //
    // Use the percent values to setup the maximum amount of items per category
    // to be sold in the market
    //

    greytgp   = (uint32)(((double)percentGreyTradeGoods / 100.0) * maxItems);
    whitetgp  = (uint32)(((double)percentWhiteTradeGoods / 100.0) * maxItems);
    greentgp  = (uint32)(((double)percentGreenTradeGoods / 100.0) * maxItems);
    bluetgp   = (uint32)(((double)percentBlueTradeGoods / 100.0) * maxItems);
    purpletgp = (uint32)(((double)percentPurpleTradeGoods / 100.0) * maxItems);
    orangetgp = (uint32)(((double)percentOrangeTradeGoods / 100.0) * maxItems);
    yellowtgp = (uint32)(((double)percentYellowTradeGoods / 100.0) * maxItems);

    greyip    = (uint32)(((double)percentGreyItems / 100.0) * maxItems);
    whiteip   = (uint32)(((double)percentWhiteItems / 100.0) * maxItems);
    greenip   = (uint32)(((double)percentGreenItems / 100.0) * maxItems);
    blueip    = (uint32)(((double)percentBlueItems / 100.0) * maxItems);
    purpleip  = (uint32)(((double)percentPurpleItems / 100.0) * maxItems);
    orangeip  = (uint32)(((double)percentOrangeItems / 100.0) * maxItems);
    yellowip  = (uint32)(((double)percentYellowItems / 100.0) * maxItems);

    uint32 total =
        greytgp +
        whitetgp +
        greentgp +
        bluetgp +
        purpletgp +
        orangetgp +
        yellowtgp +
        greyip +
        whiteip +
        greenip +
        blueip +
        purpleip +
        orangeip +
        yellowip;

    int32 diff = (maxItems - total);

    if (diff < 0)
    {
        if ((whiteip - diff) > 0)
        {
            whiteip -= diff;
        }
        else if ((greenip - diff) > 0)
        {
            greenip -= diff;
        }
    }
}

uint32 AHBConfig::GetMaximum(uint32 color)
{
    switch (color)
    {
    case AHB_GREY_TG:
        return greytgp;
        break;

    case AHB_WHITE_TG:
        return whitetgp;
        break;

    case AHB_GREEN_TG:
        return greentgp;
        break;

    case AHB_BLUE_TG:
        return bluetgp;
        break;

    case AHB_PURPLE_TG:
        return purpletgp;
        break;
    case AHB_ORANGE_TG:
        return orangetgp;
        break;

    case AHB_YELLOW_TG:
        return yellowtgp;
        break;

    case AHB_GREY_I:
        return greyip;
        break;

    case AHB_WHITE_I:
        return whiteip;
        break;

    case AHB_GREEN_I:
        return greenip;
        break;

    case AHB_BLUE_I:
        return blueip;
        break;

    case AHB_PURPLE_I:
        return purpleip;
        break;

    case AHB_ORANGE_I:
        return orangeip;
        break;

    case AHB_YELLOW_I:
        return yellowip;
        break;

    default:
        return 0;
        break;
    }
}

void AHBConfig::DecItemCounts(uint32 Class, uint32 Quality)
{
    switch (Class)
    {
    case ITEM_CLASS_TRADE_GOODS:
        DecItemCounts(Quality);
        break;

    default:
        DecItemCounts(Quality + 7);
        break;
    }
}

void AHBConfig::DecItemCounts(uint32 color)
{
    switch (color)
    {
    case AHB_GREY_TG:
        --greyTGoods;
        break;

    case AHB_WHITE_TG:
        --whiteTGoods;
        break;

    case AHB_GREEN_TG:
        --greenTGoods;
        break;

    case AHB_BLUE_TG:
        --blueTGoods;
        break;

    case AHB_PURPLE_TG:
        --purpleTGoods;
        break;

    case AHB_ORANGE_TG:
        --orangeTGoods;
        break;

    case AHB_YELLOW_TG:
        --yellowTGoods;
        break;

    case AHB_GREY_I:
        --greyItems;
        break;

    case AHB_WHITE_I:
        --whiteItems;
        break;

    case AHB_GREEN_I:
        --greenItems;
        break;

    case AHB_BLUE_I:
        --blueItems;
        break;

    case AHB_PURPLE_I:
        --purpleItems;
        break;

    case AHB_ORANGE_I:
        --orangeItems;
        break;

    case AHB_YELLOW_I:
        --yellowItems;
        break;

    default:
        break;
    }
}

void AHBConfig::IncItemCounts(uint32 Class, uint32 Quality)
{
    switch (Class)
    {
    case ITEM_CLASS_TRADE_GOODS:
        IncItemCounts(Quality);
        break;

    default:
        IncItemCounts(Quality + 7);
        break;
    }
}

void AHBConfig::IncItemCounts(uint32 color)
{
    switch (color)
    {
    case AHB_GREY_TG:
        ++greyTGoods;
        break;

    case AHB_WHITE_TG:
        ++whiteTGoods;
        break;

    case AHB_GREEN_TG:
        ++greenTGoods;
        break;

    case AHB_BLUE_TG:
        ++blueTGoods;
        break;

    case AHB_PURPLE_TG:
        ++purpleTGoods;
        break;

    case AHB_ORANGE_TG:
        ++orangeTGoods;
        break;

    case AHB_YELLOW_TG:
        ++yellowTGoods;
        break;

    case AHB_GREY_I:
        ++greyItems;
        break;

    case AHB_WHITE_I:
        ++whiteItems;
        break;

    case AHB_GREEN_I:
        ++greenItems;
        break;

    case AHB_BLUE_I:
        ++blueItems;
        break;

    case AHB_PURPLE_I:
        ++purpleItems;
        break;

    case AHB_ORANGE_I:
        ++orangeItems;
        break;

    case AHB_YELLOW_I:
        ++yellowItems;
        break;

    default:
        break;
    }
}

void AHBConfig::ResetItemCounts()
{
    greyTGoods   = 0;
    whiteTGoods  = 0;
    greenTGoods  = 0;
    blueTGoods   = 0;
    purpleTGoods = 0;
    orangeTGoods = 0;
    yellowTGoods = 0;

    greyItems    = 0;
    whiteItems   = 0;
    greenItems   = 0;
    blueItems    = 0;
    purpleItems  = 0;
    orangeItems  = 0;
    yellowItems  = 0;
}

uint32 AHBConfig::TotalItemCounts()
{
    return(
        greyTGoods +
        whiteTGoods +
        greenTGoods +
        blueTGoods +
        purpleTGoods +
        orangeTGoods +
        yellowTGoods +

        greyItems +
        whiteItems +
        greenItems +
        blueItems +
        purpleItems +
        orangeItems +
        yellowItems);
}

uint32 AHBConfig::GetItemCounts(uint32 color)
{
    switch (color)
    {
    case AHB_GREY_TG:
        return greyTGoods;
        break;

    case AHB_WHITE_TG:
        return whiteTGoods;
        break;

    case AHB_GREEN_TG:
        return greenTGoods;
        break;

    case AHB_BLUE_TG:
        return blueTGoods;
        break;

    case AHB_PURPLE_TG:
        return purpleTGoods;
        break;

    case AHB_ORANGE_TG:
        return orangeTGoods;
        break;

    case AHB_YELLOW_TG:
        return yellowTGoods;
        break;

    case AHB_GREY_I:
        return greyItems;
        break;

    case AHB_WHITE_I:
        return whiteItems;
        break;

    case AHB_GREEN_I:
        return greenItems;
        break;

    case AHB_BLUE_I:
        return blueItems;
        break;

    case AHB_PURPLE_I:
        return purpleItems;
        break;

    case AHB_ORANGE_I:
        return orangeItems;
        break;

    case AHB_YELLOW_I:
        return yellowItems;
        break;

    default:
        return 0;
        break;
    }
}

void AHBConfig::SetBidsPerInterval(uint32 value)
{
    buyerBidsPerInterval = value;
}

uint32 AHBConfig::GetBidsPerInterval()
{
    return buyerBidsPerInterval;
}
