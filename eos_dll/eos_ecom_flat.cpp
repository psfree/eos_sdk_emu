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

#include "eossdk_ecom.h"

using namespace sdk;

EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOwnership(EOS_HEcom Handle, const EOS_Ecom_QueryOwnershipOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOwnershipCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    pInst->QueryOwnership(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOwnershipToken(EOS_HEcom Handle, const EOS_Ecom_QueryOwnershipTokenOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOwnershipTokenCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    pInst->QueryOwnershipToken(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_QueryEntitlements(EOS_HEcom Handle, const EOS_Ecom_QueryEntitlementsOptions* Options, void* ClientData, const EOS_Ecom_OnQueryEntitlementsCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    pInst->QueryEntitlements(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOffers(EOS_HEcom Handle, const EOS_Ecom_QueryOffersOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOffersCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    pInst->QueryOffers(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_Checkout(EOS_HEcom Handle, const EOS_Ecom_CheckoutOptions* Options, void* ClientData, const EOS_Ecom_OnCheckoutCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    pInst->Checkout(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Ecom_RedeemEntitlements(EOS_HEcom Handle, const EOS_Ecom_RedeemEntitlementsOptions* Options, void* ClientData, const EOS_Ecom_OnRedeemEntitlementsCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    pInst->RedeemEntitlements(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetEntitlementsCount(EOS_HEcom Handle, const EOS_Ecom_GetEntitlementsCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->GetEntitlementsCount(Options);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetEntitlementsByNameCount(EOS_HEcom Handle, const EOS_Ecom_GetEntitlementsByNameCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->GetEntitlementsByNameCount(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyEntitlementByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyEntitlementByIndexOptions* Options, EOS_Ecom_Entitlement** OutEntitlement)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyEntitlementByIndex(Options, OutEntitlement);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyEntitlementByNameAndIndex(EOS_HEcom Handle, const EOS_Ecom_CopyEntitlementByNameAndIndexOptions* Options, EOS_Ecom_Entitlement** OutEntitlement)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyEntitlementByNameAndIndex(Options, OutEntitlement);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyEntitlementById(EOS_HEcom Handle, const EOS_Ecom_CopyEntitlementByIdOptions* Options, EOS_Ecom_Entitlement** OutEntitlement)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyEntitlementById(Options, OutEntitlement);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetOfferCount(EOS_HEcom Handle, const EOS_Ecom_GetOfferCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->GetOfferCount(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyOfferByIndexOptions* Options, EOS_Ecom_CatalogOffer** OutOffer)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyOfferByIndex(Options, OutOffer);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferById(EOS_HEcom Handle, const EOS_Ecom_CopyOfferByIdOptions* Options, EOS_Ecom_CatalogOffer** OutOffer)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyOfferById(Options, OutOffer);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetOfferItemCount(EOS_HEcom Handle, const EOS_Ecom_GetOfferItemCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->GetOfferItemCount(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferItemByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyOfferItemByIndexOptions* Options, EOS_Ecom_CatalogItem** OutItem)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyOfferItemByIndex(Options, OutItem);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyItemById(EOS_HEcom Handle, const EOS_Ecom_CopyItemByIdOptions* Options, EOS_Ecom_CatalogItem** OutItem)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyItemById(Options, OutItem);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetOfferImageInfoCount(EOS_HEcom Handle, const EOS_Ecom_GetOfferImageInfoCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->GetOfferImageInfoCount(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferImageInfoByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyOfferImageInfoByIndexOptions* Options, EOS_Ecom_KeyImageInfo** OutImageInfo)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyOfferImageInfoByIndex(Options, OutImageInfo);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetItemImageInfoCount(EOS_HEcom Handle, const EOS_Ecom_GetItemImageInfoCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->GetItemImageInfoCount(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyItemImageInfoByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyItemImageInfoByIndexOptions* Options, EOS_Ecom_KeyImageInfo** OutImageInfo)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyItemImageInfoByIndex(Options, OutImageInfo);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetItemReleaseCount(EOS_HEcom Handle, const EOS_Ecom_GetItemReleaseCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->GetItemReleaseCount(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyItemReleaseByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyItemReleaseByIndexOptions* Options, EOS_Ecom_CatalogRelease** OutRelease)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyItemReleaseByIndex(Options, OutRelease);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetTransactionCount(EOS_HEcom Handle, const EOS_Ecom_GetTransactionCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->GetTransactionCount(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyTransactionByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyTransactionByIndexOptions* Options, EOS_Ecom_HTransaction* OutTransaction)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyTransactionByIndex(Options, OutTransaction);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyTransactionById(EOS_HEcom Handle, const EOS_Ecom_CopyTransactionByIdOptions* Options, EOS_Ecom_HTransaction* OutTransaction)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyTransactionById(Options, OutTransaction);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_Transaction_GetTransactionId(EOS_Ecom_HTransaction Handle, char* OutBuffer, int32_t* InOutBufferLength)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom_Transaction*>(Handle);
    return pInst->GetTransactionId(OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_Transaction_GetEntitlementsCount(EOS_Ecom_HTransaction Handle, const EOS_Ecom_Transaction_GetEntitlementsCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Ecom_Transaction*>(Handle);
    return pInst->GetEntitlementsCount(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_Transaction_CopyEntitlementByIndex(EOS_Ecom_HTransaction Handle, const EOS_Ecom_Transaction_CopyEntitlementByIndexOptions* Options, EOS_Ecom_Entitlement** OutEntitlement)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom_Transaction*>(Handle);
    return pInst->CopyEntitlementByIndex(Options, OutEntitlement);
}

/**
 * Release the memory associated with an EOS_Ecom_Entitlement structure. This must be called on data
 * retrieved from EOS_Ecom_CopyEntitlementByIndex and EOS_Ecom_CopyEntitlementById.
 *
 * @param Entitlement - The entitlement structure to be released
 *
 * @see EOS_Ecom_Entitlement
 * @see EOS_Ecom_CopyEntitlementByIndex
 * @see EOS_Ecom_CopyEntitlementById
 */
EOS_DECLARE_FUNC(void) EOS_Ecom_Entitlement_Release(EOS_Ecom_Entitlement* Entitlement)
{
    TRACE_FUNC();
    if (Entitlement == nullptr)
        return;

    switch (Entitlement->ApiVersion)
    {
        case EOS_ECOM_ENTITLEMENT_API_002:
        {
            EOS_Ecom_Entitlement002* entitlement = reinterpret_cast<EOS_Ecom_Entitlement002*>(Entitlement);
            delete entitlement;
        }
        break;

        case EOS_ECOM_ENTITLEMENT_API_001:
        {
            EOS_Ecom_Entitlement001* entitlement = reinterpret_cast<EOS_Ecom_Entitlement001*>(Entitlement);
            delete entitlement;
        }
    }
}

/**
 * Release the memory associated with an EOS_Ecom_CatalogItem structure. This must be called on data
 * retrieved from EOS_Ecom_CopyOfferItemByIndex.
 *
 * @param CatalogItem - The catalog item structure to be released
 *
 * @see EOS_Ecom_CatalogItem
 * @see EOS_Ecom_CopyOfferItemByIndex
 */
EOS_DECLARE_FUNC(void) EOS_Ecom_CatalogItem_Release(EOS_Ecom_CatalogItem* CatalogItem)
{
    TRACE_FUNC();

    if (CatalogItem == nullptr)
        return;

    delete CatalogItem;
}

/**
 * Release the memory associated with an EOS_Ecom_CatalogOffer structure. This must be called on data
 * retrieved from EOS_Ecom_CopyOfferByIndex.
 *
 * @param CatalogOffer - The catalog offer structure to be released
 *
 * @see EOS_Ecom_CatalogOffer
 * @see EOS_Ecom_CopyOfferByIndex
 */
EOS_DECLARE_FUNC(void) EOS_Ecom_CatalogOffer_Release(EOS_Ecom_CatalogOffer* CatalogOffer)
{
    TRACE_FUNC();
    if (CatalogOffer == nullptr)
        return;

    delete CatalogOffer;
}

/**
 * Release the memory associated with an EOS_Ecom_KeyImageInfo structure. This must be called on data
 * retrieved from EOS_Ecom_CopyItemImageInfoByIndex.
 *
 * @param KeyImageInfo - The key image info structure to be released
 *
 * @see EOS_Ecom_KeyImageInfo
 * @see EOS_Ecom_CopyItemImageInfoByIndex
 */
EOS_DECLARE_FUNC(void) EOS_Ecom_KeyImageInfo_Release(EOS_Ecom_KeyImageInfo* KeyImageInfo)
{
    TRACE_FUNC();
    if (KeyImageInfo == nullptr)
        return;

    delete KeyImageInfo;
}

/**
 * Release the memory associated with an EOS_Ecom_CatalogRelease structure. This must be called on
 * data retrieved from EOS_Ecom_CopyItemReleaseByIndex.
 *
 * @param CatalogRelease - The catalog release structure to be released
 *
 * @see EOS_Ecom_CatalogRelease
 * @see EOS_Ecom_CopyItemReleaseByIndex
 */
EOS_DECLARE_FUNC(void) EOS_Ecom_CatalogRelease_Release(EOS_Ecom_CatalogRelease* CatalogRelease)
{
    TRACE_FUNC();
    if (CatalogRelease == nullptr)
        return;

    delete CatalogRelease;
}

/**
 * Release the memory associated with an EOS_Ecom_HTransaction.  Is is expected to be called after
 * being received from a EOS_Ecom_CheckoutCallbackInfo.
 *
 * @param Transaction A handle to a transaction.
 *
 * @see EOS_Ecom_CheckoutCallbackInfo
 * @see EOS_Ecom_GetTransactionCount
 * @see EOS_Ecom_CopyTransactionByIndex
 */
EOS_DECLARE_FUNC(void) EOS_Ecom_Transaction_Release(EOS_Ecom_HTransaction Transaction)
{
    TRACE_FUNC();
    if (Transaction == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Ecom_Transaction*>(Transaction);
    delete pInst;
}
