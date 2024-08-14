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

#ifndef AUCTION_HOUSE_BOT_CONFIG_H
#define AUCTION_HOUSE_BOT_CONFIG_H

class AHBConfig
{
private:
    uint32 AHID;
    uint32 AHFID;

    uint32 minItems;
    uint32 maxItems;

    uint32 percentGreyTradeGoods;
    uint32 percentWhiteTradeGoods;
    uint32 percentGreenTradeGoods;
    uint32 percentBlueTradeGoods;
    uint32 percentPurpleTradeGoods;
    uint32 percentOrangeTradeGoods;
    uint32 percentYellowTradeGoods;

    uint32 percentGreyItems;
    uint32 percentWhiteItems;
    uint32 percentGreenItems;
    uint32 percentBlueItems;
    uint32 percentPurpleItems;
    uint32 percentOrangeItems;
    uint32 percentYellowItems;

    uint32 minPriceGrey;
    uint32 maxPriceGrey;
    uint32 minBidPriceGrey;
    uint32 maxBidPriceGrey;
    uint32 maxStackGrey;

    uint32 minPriceWhite;
    uint32 maxPriceWhite;
    uint32 minBidPriceWhite;
    uint32 maxBidPriceWhite;
    uint32 maxStackWhite;

    uint32 minPriceGreen;
    uint32 maxPriceGreen;
    uint32 minBidPriceGreen;
    uint32 maxBidPriceGreen;
    uint32 maxStackGreen;

    uint32 minPriceBlue;
    uint32 maxPriceBlue;
    uint32 minBidPriceBlue;
    uint32 maxBidPriceBlue;
    uint32 maxStackBlue;

    uint32 minPricePurple;
    uint32 maxPricePurple;
    uint32 minBidPricePurple;
    uint32 maxBidPricePurple;
    uint32 maxStackPurple;

    uint32 minPriceOrange;
    uint32 maxPriceOrange;
    uint32 minBidPriceOrange;
    uint32 maxBidPriceOrange;
    uint32 maxStackOrange;

    uint32 minPriceYellow;
    uint32 maxPriceYellow;
    uint32 minBidPriceYellow;
    uint32 maxBidPriceYellow;
    uint32 maxStackYellow;

    uint32 buyerPriceGrey;
    uint32 buyerPriceWhite;
    uint32 buyerPriceGreen;
    uint32 buyerPriceBlue;
    uint32 buyerPricePurple;
    uint32 buyerPriceOrange;
    uint32 buyerPriceYellow;
    uint32 buyerBiddingInterval;
    uint32 buyerBidsPerInterval;

    uint32 greytgp;    // Contains the amount of items to be sold in absolute values
    uint32 whitetgp;   // Contains the amount of items to be sold in absolute values
    uint32 greentgp;   // Contains the amount of items to be sold in absolute values
    uint32 bluetgp;    // Contains the amount of items to be sold in absolute values
    uint32 purpletgp;  // Contains the amount of items to be sold in absolute values
    uint32 orangetgp;  // Contains the amount of items to be sold in absolute values
    uint32 yellowtgp;  // Contains the amount of items to be sold in absolute values

    uint32 greyip;     // Contains the amount of items to be sold in absolute values
    uint32 whiteip;    // Contains the amount of items to be sold in absolute values
    uint32 greenip;    // Contains the amount of items to be sold in absolute values
    uint32 blueip;     // Contains the amount of items to be sold in absolute values
    uint32 purpleip;   // Contains the amount of items to be sold in absolute values
    uint32 orangeip;   // Contains the amount of items to be sold in absolute values
    uint32 yellowip;   // Contains the amount of items to be sold in absolute values

    uint32 greyTGoods;
    uint32 whiteTGoods;
    uint32 greenTGoods;
    uint32 blueTGoods;
    uint32 purpleTGoods;
    uint32 orangeTGoods;
    uint32 yellowTGoods;

    uint32 greyItems;
    uint32 whiteItems;
    uint32 greenItems;
    uint32 blueItems;
    uint32 purpleItems;
    uint32 orangeItems;
    uint32 yellowItems;

public:
    AHBConfig(uint32 ahid);
    AHBConfig();
    ~AHBConfig();

    void   Reset();

    uint32 GetAHID();
    uint32 GetAHFID();

    void   SetMinItems       (uint32 value);
    uint32 GetMinItems       ();

    void   SetMaxItems       (uint32 value);
    uint32 GetMaxItems       ();

    void   SetPercentages(
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
        uint32 yellowi);
    uint32 GetPercentages    (uint32 color);

    void   SetMinPrice       (uint32 color, uint32 value);
    uint32 GetMinPrice       (uint32 color);

    void   SetMaxPrice       (uint32 color, uint32 value);
    uint32 GetMaxPrice       (uint32 color);

    void   SetMinBidPrice    (uint32 color, uint32 value);
    uint32 GetMinBidPrice    (uint32 color);

    void   SetMaxBidPrice    (uint32 color, uint32 value);
    uint32 GetMaxBidPrice    (uint32 color);

    void   SetMaxStack       (uint32 color, uint32 value);
    uint32 GetMaxStack       (uint32 color);

    void   SetBuyerPrice     (uint32 color, uint32 value);
    uint32 GetBuyerPrice     (uint32 color);

    void   SetBiddingInterval(uint32 value);
    uint32 GetBiddingInterval();

    void   SetBidsPerInterval(uint32 value);
    uint32 GetBidsPerInterval();

    void   CalculatePercents ();
    uint32 GetMaximum        (uint32 color);

    void   DecItemCounts     (uint32 Class, uint32 Quality);
    void   DecItemCounts     (uint32 color);

    void   IncItemCounts     (uint32 Class, uint32 Quality);
    void   IncItemCounts     (uint32 color);

    void   ResetItemCounts   ();
    uint32 TotalItemCounts   ();

    uint32 GetItemCounts     (uint32 color);
};


#endif // AUCTION_HOUSE_BOT_CONFIG_H
