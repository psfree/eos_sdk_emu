/*
 * Copyright (C) 2020 Nemirtingas
 * This file is part of the Nemirtingas's Epic Emulator
 *
 * The Nemirtingas's Epic Emulator is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * The Nemirtingas's Epic Emulator is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the Nemirtingas's Epic Emulator; if not, see
 * <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "common_includes.h"

namespace sdk
{
    class EOSSDK_Ecom :
        public IRunFrame
    {
    public:
        EOSSDK_Ecom();
        ~EOSSDK_Ecom();

        virtual bool CBRunFrame();
        virtual bool RunNetwork(Network_Message_pb const& msg);
        virtual bool RunCallbacks(pFrameResult_t res);
        virtual void FreeCallback(pFrameResult_t res);

        void        QueryOwnership(const EOS_Ecom_QueryOwnershipOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOwnershipCallback CompletionDelegate);
        void        QueryOwnershipToken(const EOS_Ecom_QueryOwnershipTokenOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOwnershipTokenCallback CompletionDelegate);
        void        QueryEntitlements(const EOS_Ecom_QueryEntitlementsOptions* Options, void* ClientData, const EOS_Ecom_OnQueryEntitlementsCallback CompletionDelegate);
        void        QueryOffers(const EOS_Ecom_QueryOffersOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOffersCallback CompletionDelegate);
        void        Checkout(const EOS_Ecom_CheckoutOptions* Options, void* ClientData, const EOS_Ecom_OnCheckoutCallback CompletionDelegate);
        void        RedeemEntitlements(const EOS_Ecom_RedeemEntitlementsOptions* Options, void* ClientData, const EOS_Ecom_OnRedeemEntitlementsCallback CompletionDelegate);
        uint32_t    GetEntitlementsCount(const EOS_Ecom_GetEntitlementsCountOptions* Options);
        uint32_t    GetEntitlementsByNameCount(const EOS_Ecom_GetEntitlementsByNameCountOptions* Options);
        EOS_EResult CopyEntitlementByIndex(const EOS_Ecom_CopyEntitlementByIndexOptions* Options, EOS_Ecom_Entitlement** OutEntitlement);
        EOS_EResult CopyEntitlementByNameAndIndex(const EOS_Ecom_CopyEntitlementByNameAndIndexOptions* Options, EOS_Ecom_Entitlement** OutEntitlement);
        EOS_EResult CopyEntitlementById(const EOS_Ecom_CopyEntitlementByIdOptions* Options, EOS_Ecom_Entitlement** OutEntitlement);
        uint32_t    GetOfferCount(const EOS_Ecom_GetOfferCountOptions* Options);
        EOS_EResult CopyOfferByIndex(const EOS_Ecom_CopyOfferByIndexOptions* Options, EOS_Ecom_CatalogOffer** OutOffer);
        EOS_EResult CopyOfferById(const EOS_Ecom_CopyOfferByIdOptions* Options, EOS_Ecom_CatalogOffer** OutOffer);
        uint32_t    GetOfferItemCount(const EOS_Ecom_GetOfferItemCountOptions* Options);
        EOS_EResult CopyOfferItemByIndex(const EOS_Ecom_CopyOfferItemByIndexOptions* Options, EOS_Ecom_CatalogItem** OutItem);
        EOS_EResult CopyItemById(const EOS_Ecom_CopyItemByIdOptions* Options, EOS_Ecom_CatalogItem** OutItem);
        uint32_t    GetOfferImageInfoCount(const EOS_Ecom_GetOfferImageInfoCountOptions* Options);
        EOS_EResult CopyOfferImageInfoByIndex(const EOS_Ecom_CopyOfferImageInfoByIndexOptions* Options, EOS_Ecom_KeyImageInfo** OutImageInfo);
        uint32_t    GetItemImageInfoCount(const EOS_Ecom_GetItemImageInfoCountOptions* Options);
        EOS_EResult CopyItemImageInfoByIndex(const EOS_Ecom_CopyItemImageInfoByIndexOptions* Options, EOS_Ecom_KeyImageInfo** OutImageInfo);
        uint32_t    GetItemReleaseCount(const EOS_Ecom_GetItemReleaseCountOptions* Options);
        EOS_EResult CopyItemReleaseByIndex(const EOS_Ecom_CopyItemReleaseByIndexOptions* Options, EOS_Ecom_CatalogRelease** OutRelease);
        uint32_t    GetTransactionCount(const EOS_Ecom_GetTransactionCountOptions* Options);
        EOS_EResult CopyTransactionByIndex(const EOS_Ecom_CopyTransactionByIndexOptions* Options, EOS_Ecom_HTransaction* OutTransaction);
        EOS_EResult CopyTransactionById(const EOS_Ecom_CopyTransactionByIdOptions* Options, EOS_Ecom_HTransaction* OutTransaction);
        EOS_EResult Transaction_GetTransactionId(EOS_Ecom_HTransaction Handle, char* OutBuffer, int32_t* InOutBufferLength);
        uint32_t    Transaction_GetEntitlementsCount(EOS_Ecom_HTransaction Handle, const EOS_Ecom_Transaction_GetEntitlementsCountOptions* Options);
        EOS_EResult Transaction_CopyEntitlementByIndex(EOS_Ecom_HTransaction Handle, const EOS_Ecom_Transaction_CopyEntitlementByIndexOptions* Options, EOS_Ecom_Entitlement** OutEntitlement);
    };
}
