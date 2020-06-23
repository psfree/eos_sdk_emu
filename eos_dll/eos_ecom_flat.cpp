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

/**
 * The Ecom Interface exposes all catalog, purchasing, and ownership entitlement features available with the Epic Games store
 * All Ecom Interface calls take a handle of type EOS_HEcom as the first parameter.
 * This handle can be retrieved from a EOS_HPlatform handle by using the EOS_Platform_GetEcomInterface function.
 *
 * NOTE: At this time, this feature is only available for products that are part of the Epic Games store.
 *
 * @see EOS_Platform_GetEcomInterface
 */

 /**
  * Query the ownership status for a given list of catalog item ids defined with Epic Online Services.
  * This data will be cached for a limited time and retrieved again from the backend when necessary
  *
  * @param Options structure containing the account and catalog item ids to retrieve
  * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
  * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
  */
EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOwnership(EOS_HEcom Handle, const EOS_Ecom_QueryOwnershipOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOwnershipCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    pInst->QueryOwnership(Options, ClientData, CompletionDelegate);
}

/**
 * Query the ownership status for a given list of catalog item ids defined with Epic Online Services.
 * The data is return via the callback in the form of a signed JWT that should be verified by an external backend server using a public key for authenticity.
 *
 * @param Options structure containing the account and catalog item ids to retrieve in token form
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 */
EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOwnershipToken(EOS_HEcom Handle, const EOS_Ecom_QueryOwnershipTokenOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOwnershipTokenCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    pInst->QueryOwnershipToken(Options, ClientData, CompletionDelegate);
}

/**
 * Query the entitlement information defined with Epic Online Services.
 * A set of entitlement names can be provided to filter the set of entitlements associated with the account.
 * This data will be cached for a limited time and retrieved again from the backend when necessary.
 * Use EOS_Ecom_CopyEntitlementByIndex, EOS_Ecom_CopyEntitlementByNameAndIndex, and EOS_Ecom_CopyEntitlementById to get the entitlement details.
 * Use EOS_Ecom_GetEntitlementsByNameCount to retrieve the number of entitlements with a specific entitlement name.
 *
 * @param Options structure containing the account and entitlement names to retrieve
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 */
EOS_DECLARE_FUNC(void) EOS_Ecom_QueryEntitlements(EOS_HEcom Handle, const EOS_Ecom_QueryEntitlementsOptions* Options, void* ClientData, const EOS_Ecom_OnQueryEntitlementsCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    pInst->QueryEntitlements(Options, ClientData, CompletionDelegate);
}

/**
 * Query for a list of catalog offers defined with Epic Online Services.
 * This data will be cached for a limited time and retrieved again from the backend when necessary.
 *
 * @param Options structure containing filter criteria
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 */
EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOffers(EOS_HEcom Handle, const EOS_Ecom_QueryOffersOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOffersCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    pInst->QueryOffers(Options, ClientData, CompletionDelegate);
}

/**
 * Initiates the purchase flow for a set of offers.  The callback is triggered after the purchase flow.
 * On success, the set of entitlements that were unlocked will be cached.
 * On success, a Transaction Id will be returned. The Transaction Id can be used to obtain a
 * EOS_Ecom_HTransaction handle. The handle can then be used to retrieve the entitlements rewarded by the purchase.
 *
 * @see EOS_Ecom_Transaction_Release
 *
 * @param Options structure containing filter criteria
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 */
EOS_DECLARE_FUNC(void) EOS_Ecom_Checkout(EOS_HEcom Handle, const EOS_Ecom_CheckoutOptions* Options, void* ClientData, const EOS_Ecom_OnCheckoutCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    pInst->Checkout(Options, ClientData, CompletionDelegate);
}

/**
 * Requests that the provided entitlement be marked redeemed.  This will cause that entitlement
 * to no longer be returned from QueryEntitlements unless the include redeemed request flag is set true.
 *
 * @param Options structure containing entitlement to redeem
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 */
EOS_DECLARE_FUNC(void) EOS_Ecom_RedeemEntitlements(EOS_HEcom Handle, const EOS_Ecom_RedeemEntitlementsOptions* Options, void* ClientData, const EOS_Ecom_OnRedeemEntitlementsCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    pInst->RedeemEntitlements(Options, ClientData, CompletionDelegate);
}

/**
 * Fetch the number of entitlements that are cached for a given local user.
 *
 * @param Options structure containing the account id being accessed
 *
 * @see EOS_Ecom_CopyEntitlementByIndex
 *
 * @return the number of entitlements found.
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetEntitlementsCount(EOS_HEcom Handle, const EOS_Ecom_GetEntitlementsCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->GetEntitlementsCount(Options);
}

/**
 * Fetch the number of entitlements with the given Entitlement Name that are cached for a given local user.
 *
 * @param Options structure containing the account id and name being accessed
 *
 * @see EOS_Ecom_CopyEntitlementByNameAndIndex
 *
 * @return the number of entitlements found.
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetEntitlementsByNameCount(EOS_HEcom Handle, const EOS_Ecom_GetEntitlementsByNameCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->GetEntitlementsByNameCount(Options);
}

/**
 * Fetches an entitlement from a given index.
 *
 * @param Options structure containing the account id and index being accessed
 * @param OutEntitlement the entitlement for the given index, if it exists and is valid, use EOS_Ecom_Entitlement_Release when finished
 *
 * @see EOS_Ecom_Entitlement_Release
 *
 * @return EOS_Success if the information is available and passed out in OutEntitlement
 *         EOS_Ecom_EntitlementStale if the entitlement information is stale and passed out in OutEntitlement
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the entitlement is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyEntitlementByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyEntitlementByIndexOptions* Options, EOS_Ecom_Entitlement** OutEntitlement)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyEntitlementByIndex(Options, OutEntitlement);
}

/**
 * Fetches a single entitlement with a given Entitlement Name.  The Index is used to access individual
 * entitlements among those with the same Entitlement Name.  The Index can be a value from 0 to
 * one less than the result from EOS_Ecom_GetEntitlementsByNameCount.
 *
 * @param Options structure containing the account id, entitlement name, and index being accessed
 * @param OutEntitlement the entitlement for the given name index pair, if it exists and is valid, use EOS_Ecom_Entitlement_Release when finished
 *
 * @see EOS_Ecom_Entitlement_Release
 *
 * @return EOS_Success if the information is available and passed out in OutEntitlement
 *         EOS_Ecom_EntitlementStale if the entitlement information is stale and passed out in OutEntitlement
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the entitlement is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyEntitlementByNameAndIndex(EOS_HEcom Handle, const EOS_Ecom_CopyEntitlementByNameAndIndexOptions* Options, EOS_Ecom_Entitlement** OutEntitlement)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyEntitlementByNameAndIndex(Options, OutEntitlement);
}

/**
 * Fetches the entitlement with the given id.
 *
 * @param Options structure containing the account id and entitlement id being accessed
 * @param OutEntitlement the entitlement for the given id, if it exists and is valid, use EOS_Ecom_Entitlement_Release when finished
 *
 * @see EOS_Ecom_CopyEntitlementByNameAndIndex
 * @see EOS_Ecom_Entitlement_Release
 *
 * @return EOS_Success if the information is available and passed out in OutEntitlement
 *         EOS_Ecom_EntitlementStale if the entitlement information is stale and passed out in OutEntitlement
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the entitlement is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyEntitlementById(EOS_HEcom Handle, const EOS_Ecom_CopyEntitlementByIdOptions* Options, EOS_Ecom_Entitlement** OutEntitlement)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyEntitlementById(Options, OutEntitlement);
}

/**
 * Fetch the number of offers that are cached for a given local user.
 *
 * @param Options structure containing the account id being accessed
 *
 * @see EOS_Ecom_CopyOfferByIndex
 *
 * @return the number of offers found.
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetOfferCount(EOS_HEcom Handle, const EOS_Ecom_GetOfferCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->GetOfferCount(Options);
}

/**
 * Fetches an offer from a given index.  The pricing and text are localized to the provided account.
 *
 * @param Options structure containing the account id and index being accessed
 * @param OutOffer the offer for the given index, if it exists and is valid, use EOS_Ecom_CatalogOffer_Release when finished
 *
 * @see EOS_Ecom_CatalogOffer_Release
 * @see EOS_Ecom_GetOfferItemCount
 *
 * @return EOS_Success if the information is available and passed out in OutOffer
 *         EOS_Ecom_CatalogOfferStale if the offer information is stale and passed out in OutOffer
 *         EOS_Ecom_CatalogOfferPriceInvalid if the offer information has an invalid price and passed out in OutOffer
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the offer is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyOfferByIndexOptions* Options, EOS_Ecom_CatalogOffer** OutOffer)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyOfferByIndex(Options, OutOffer);
}

/**
 * Fetches an offer with a given ID.  The pricing and text are localized to the provided account.
 *
 * @param Options structure containing the account id and offer id being accessed
 * @param OutOffer the offer for the given index, if it exists and is valid, use EOS_Ecom_CatalogOffer_Release when finished
 *
 * @see EOS_Ecom_CatalogOffer_Release
 * @see EOS_Ecom_GetOfferItemCount
 *
 * @return EOS_Success if the information is available and passed out in OutOffer
 *         EOS_Ecom_CatalogOfferStale if the offer information is stale and passed out in OutOffer
 *         EOS_Ecom_CatalogOfferPriceInvalid if the offer information has an invalid price and passed out in OutOffer
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the offer is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferById(EOS_HEcom Handle, const EOS_Ecom_CopyOfferByIdOptions* Options, EOS_Ecom_CatalogOffer** OutOffer)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyOfferById(Options, OutOffer);
}

/**
 * Fetch the number of items that are associated with a given cached offer for a local user.
 *
 * @return the number of items found.
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetOfferItemCount(EOS_HEcom Handle, const EOS_Ecom_GetOfferItemCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->GetOfferItemCount(Options);
}

/**
 * Fetches an item from a given index.
 *
 * @param Options structure containing the account id and index being accessed
 * @param OutItem the item for the given index, if it exists and is valid, use EOS_Ecom_CatalogItem_Release when finished
 *
 * @see EOS_Ecom_CatalogItem_Release
 * @see EOS_Ecom_GetItemImageInfoCount
 * @see EOS_Ecom_GetItemReleaseCount
 *
 * @return EOS_Success if the information is available and passed out in OutItem
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_Ecom_CatalogItemState if the item information is stale
 *         EOS_NotFound if the item is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferItemByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyOfferItemByIndexOptions* Options, EOS_Ecom_CatalogItem** OutItem)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyOfferItemByIndex(Options, OutItem);
}

/**
 * Fetches an item with a given ID.
 *
 * @param Options structure containing the item id being accessed
 * @param OutItem the item for the given index, if it exists and is valid, use EOS_Ecom_CatalogItem_Release when finished
 *
 * @see EOS_Ecom_CatalogItem_Release
 * @see EOS_Ecom_GetItemImageInfoCount
 * @see EOS_Ecom_GetItemReleaseCount
 *
 * @return EOS_Success if the information is available and passed out in OutItem
 *         EOS_Ecom_CatalogItemState if the item information is stale and passed out in OutItem
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the offer is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyItemById(EOS_HEcom Handle, const EOS_Ecom_CopyItemByIdOptions* Options, EOS_Ecom_CatalogItem** OutItem)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyItemById(Options, OutItem);
}

/**
 * Fetch the number of images that are associated with a given cached offer for a local user.
 *
 * @return the number of images found.
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetOfferImageInfoCount(EOS_HEcom Handle, const EOS_Ecom_GetOfferImageInfoCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->GetOfferImageInfoCount(Options);
}

/**
 * Fetches an image from a given index.
 *
 * @param Options structure containing the offer id and index being accessed
 * @param OutImageInfo the image for the given index, if it exists and is valid, use EOS_Ecom_KeyImageInfo_Release when finished
 *
 * @see EOS_Ecom_KeyImageInfo_Release
 *
 * @return EOS_Success if the information is available and passed out in OutImageInfo
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_Ecom_CatalogOfferStale if the associated offer information is stale
 *         EOS_NotFound if the image is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferImageInfoByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyOfferImageInfoByIndexOptions* Options, EOS_Ecom_KeyImageInfo** OutImageInfo)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyOfferImageInfoByIndex(Options, OutImageInfo);
}

/**
 * Fetch the number of images that are associated with a given cached item for a local user.
 *
 * @return the number of images found.
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetItemImageInfoCount(EOS_HEcom Handle, const EOS_Ecom_GetItemImageInfoCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->GetItemImageInfoCount(Options);
}

/**
 * Fetches an image from a given index.
 *
 * @param Options structure containing the item id and index being accessed
 * @param OutImageInfo the image for the given index, if it exists and is valid, use EOS_Ecom_KeyImageInfo_Release when finished
 *
 * @see EOS_Ecom_KeyImageInfo_Release
 *
 * @return EOS_Success if the information is available and passed out in OutImageInfo
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_Ecom_CatalogItemState if the associated item information is stale
 *         EOS_NotFound if the image is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyItemImageInfoByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyItemImageInfoByIndexOptions* Options, EOS_Ecom_KeyImageInfo** OutImageInfo)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyItemImageInfoByIndex(Options, OutImageInfo);
}

/**
 * Fetch the number of releases that are associated with a given cached item for a local user.
 *
 * @return the number of releases found.
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetItemReleaseCount(EOS_HEcom Handle, const EOS_Ecom_GetItemReleaseCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->GetItemReleaseCount(Options);
}

/**
 * Fetches a release from a given index.
 *
 * @param Options structure containing the item id and index being accessed
 * @param OutRelease the release for the given index, if it exists and is valid, use EOS_Ecom_CatalogRelease_Release when finished
 *
 * @see EOS_Ecom_CatalogRelease_Release
 *
 * @return EOS_Success if the information is available and passed out in OutRelease
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_Ecom_CatalogItemState if the associated item information is stale
 *         EOS_NotFound if the release is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyItemReleaseByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyItemReleaseByIndexOptions* Options, EOS_Ecom_CatalogRelease** OutRelease)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyItemReleaseByIndex(Options, OutRelease);
}

/**
 * Fetch the number of transactions that are cached for a given local user.
 *
 * @see EOS_Ecom_CheckoutCallbackInfo
 * @see EOS_Ecom_CopyTransactionByIndex
 *
 * @return the number of transactions found.
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetTransactionCount(EOS_HEcom Handle, const EOS_Ecom_GetTransactionCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->GetTransactionCount(Options);
}

/**
 * Fetches the transaction handle at the given index.
 *
 * @param Options structure containing the account id and index being accessed
 *
 * @see EOS_Ecom_CheckoutCallbackInfo
 * @see EOS_Ecom_Transaction_Release
 *
 * @return EOS_Success if the information is available and passed out in OutTransaction
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the transaction is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyTransactionByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyTransactionByIndexOptions* Options, EOS_Ecom_HTransaction* OutTransaction)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyTransactionByIndex(Options, OutTransaction);
}

/**
 * Fetches the transaction handle at the given index.
 *
 * @param Options structure containing the account id and transaction id being accessed
 *
 * @see EOS_Ecom_CheckoutCallbackInfo
 * @see EOS_Ecom_Transaction_Release
 *
 * @return EOS_Success if the information is available and passed out in OutTransaction
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the transaction is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyTransactionById(EOS_HEcom Handle, const EOS_Ecom_CopyTransactionByIdOptions* Options, EOS_Ecom_HTransaction* OutTransaction)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom*>(Handle);
    return pInst->CopyTransactionById(Options, OutTransaction);
}

/**
 * The Ecom Transaction Interface exposes getters for accessing information about a completed transaction.
 * All Ecom Transaction Interface calls take a handle of type EOS_Ecom_HTransaction as the first parameter.
 * A EOS_Ecom_HTransaction handle is originally returned as part of the EOS_Ecom_CheckoutCallbackInfo struct.
 * A EOS_Ecom_HTransaction handle can also be retrieved from a EOS_HEcom handle using EOS_Ecom_CopyTransactionByIndex.
 * It is expected that after a transaction that EOS_Ecom_Transaction_Release is called.
 * When EOS_Platform_Release is called any remaining transactions will also be released.
 *
 * @see EOS_Ecom_CheckoutCallbackInfo
 * @see EOS_Ecom_GetTransactionCount
 * @see EOS_Ecom_CopyTransactionByIndex
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_Transaction_GetTransactionId(EOS_Ecom_HTransaction Handle, char* OutBuffer, int32_t* InOutBufferLength)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Ecom_Transaction*>(Handle);
    return pInst->GetTransactionId(OutBuffer, InOutBufferLength);
}

/**
 * Fetch the number of entitlements that are part of this transaction.
 *
 * @param Options structure containing the account id being accessed
 *
 * @see EOS_Ecom_Transaction_CopyEntitlementByIndex
 *
 * @return the number of entitlements found.
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_Transaction_GetEntitlementsCount(EOS_Ecom_HTransaction Handle, const EOS_Ecom_Transaction_GetEntitlementsCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Ecom_Transaction*>(Handle);
    return pInst->GetEntitlementsCount(Options);
}

/**
 * Fetches an entitlement from a given index.
 *
 * @param Options structure containing the index being accessed
 * @param OutEntitlement the entitlement for the given index, if it exists and is valid, use EOS_Ecom_Entitlement_Release when finished
 *
 * @see EOS_Ecom_Entitlement_Release
 *
 * @return EOS_Success if the information is available and passed out in OutEntitlement
 *         EOS_Ecom_EntitlementStale if the entitlement information is stale and passed out in OutEntitlement
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the entitlement is not found
 */
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
