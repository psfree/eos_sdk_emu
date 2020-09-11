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
#include "eossdk_platform.h"
#include "eos_client_api.h"
#include "settings.h"

namespace sdk
{

static const char* ownership_status_to_string(EOS_EOwnershipStatus status)
{
    switch (status)
    {
#define OWNERSHIP_CASE(X) case EOS_EOwnershipStatus::X: return #X
        OWNERSHIP_CASE(EOS_OS_NotOwned);
        OWNERSHIP_CASE(EOS_OS_Owned);
        default: return "Unknown Ownership";
#undef  OWNERSHIP_CASE
    }
}

decltype(EOSSDK_Ecom::catalog_filename)         EOSSDK_Ecom::catalog_filename("catalog.json");
decltype(EOSSDK_Ecom::entitlements_filename)    EOSSDK_Ecom::entitlements_filename("entitlements.json");

EOSSDK_Ecom::EOSSDK_Ecom()
{
    FileManager::load_json(catalog_filename, _catalog);
    FileManager::load_json(entitlements_filename, _entitlements);

    GetCB_Manager().register_callbacks(this);
}

EOSSDK_Ecom::~EOSSDK_Ecom()
{
    GetCB_Manager().unregister_callbacks(this);
}

EOS_EResult EOSSDK_Ecom::copy_entitlement(typename decltype(_queried_entitlements)::iterator it, EOS_Ecom_Entitlement** OutEntitlement)
{
    bool redeemed;
    std::string const* entitlement_id = nullptr;
    std::string const* entitlement_name = nullptr;
    std::string const* catalog_item_id = nullptr;
    bool error = false;

    entitlement_id = &it->first;
    try
    {
        entitlement_name = (*it->second)["entitlement_name"].get_ptr<std::string*>();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "%s \"entitlement_name\" field was not found, it will not be owned", entitlement_id->c_str());
        error = true;
    }
    try
    {
        catalog_item_id  = (*it->second)["catalog_item_id"].get_ptr<std::string*>();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "%s \"catalog_item_id\" field was not found, it will not be owned", entitlement_id->c_str());
        error = true;
    }
    try
    {
        redeemed = (*it->second)["redeemed"];
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "%s \"redeemed\" field was not found, it will not be redeemed", entitlement_id->c_str());
        redeemed = false;
    }

    if (error)
    {
        *OutEntitlement = nullptr;
        return EOS_EResult::EOS_NotFound;
    }

    EOS_Ecom_Entitlement002* entitlement = new EOS_Ecom_Entitlement002;
    entitlement->ApiVersion = EOS_ECOM_ENTITLEMENT_API_002;
    entitlement->EntitlementName = entitlement_name->c_str();
    entitlement->EntitlementId = entitlement_id->c_str();
    entitlement->CatalogItemId = catalog_item_id->c_str();
    entitlement->ServerIndex = -1;
    entitlement->bRedeemed = redeemed;
    entitlement->EndTimestamp = -1;
    *OutEntitlement = reinterpret_cast<decltype(*OutEntitlement)>(entitlement);

    return EOS_EResult::EOS_Success;
}

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
void EOSSDK_Ecom::QueryOwnership(const EOS_Ecom_QueryOwnershipOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOwnershipCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);

    EOS_Ecom_QueryOwnershipCallbackInfo& qoci = res->CreateCallback<EOS_Ecom_QueryOwnershipCallbackInfo>((CallbackFunc)CompletionDelegate);

    int32_t itemownershipversion = EOS_ECOM_ITEMOWNERSHIP_API_LATEST;

    switch (Options->ApiVersion)
    {
        case EOS_ECOM_QUERYOWNERSHIP_API_002:
        {
            auto opts = reinterpret_cast<const EOS_Ecom_QueryOwnershipOptions002*>(Options);
            APP_LOG(Log::LogLevel::INFO, "TODO?: Check the catalog namespace");
            APP_LOG(Log::LogLevel::DEBUG, "CatalogNamespace: %s", (opts->CatalogNamespace == nullptr ? "" : opts->CatalogNamespace));
        }
        case EOS_ECOM_QUERYOWNERSHIP_API_001:
        {
            auto opts = reinterpret_cast<const EOS_Ecom_QueryOwnershipOptions001*>(Options);
            qoci.LocalUserId = opts->LocalUserId;
            
            qoci.ItemOwnershipCount = Options->CatalogItemIdCount;
            APP_LOG(Log::LogLevel::DEBUG, "CatalogItemIdCount: %u", opts->CatalogItemIdCount);
            if (qoci.ItemOwnershipCount > 0)
            {
                EOS_Ecom_ItemOwnership* ownerships = new EOS_Ecom_ItemOwnership[qoci.ItemOwnershipCount];
                for (uint32_t i = 0; i < Options->CatalogItemIdCount; ++i)
                {
                    APP_LOG(Log::LogLevel::DEBUG, "CatalogItemIds[%u]: %s", i, (opts->CatalogItemIds[i] == nullptr ? "" : opts->CatalogItemIds[i]));

                    EOS_EOwnershipStatus owned = EOS_EOwnershipStatus::EOS_OS_NotOwned;
                    char* id;
                    if (opts->CatalogItemIds[i] != nullptr)
                    {
                        size_t idlen = strlen(opts->CatalogItemIds[i])+1;
                        id = new char[idlen];
                        strncpy(id, opts->CatalogItemIds[i], idlen);
                        
                        auto catalog_it = _catalog.find(id);
                        if (catalog_it != _catalog.end())
                        {
                            try
                            {
                                if (catalog_it.value()["owned"].get<bool>())
                                {
                                    owned = EOS_EOwnershipStatus::EOS_OS_Owned;
                                }
                                APP_LOG(Log::LogLevel::INFO, "Catalog Item id %s, %s (from %s)", id, ownership_status_to_string(owned), catalog_filename.c_str());
                            }
                            catch(...)
                            {
                                APP_LOG(Log::LogLevel::ERR, "Catalog Item id %s \"owned\" field was invalid, item not owned", id);
                            }
                        }
                        else
                        {
                            owned = (Settings::Inst().unlock_dlcs ? EOS_EOwnershipStatus::EOS_OS_Owned : EOS_EOwnershipStatus::EOS_OS_NotOwned);
                            APP_LOG(Log::LogLevel::INFO, "Catalog Item id %s, %s (from \"unlock_dlcs\")", id, ownership_status_to_string(owned));
                        }
                    }
                    else
                    {
                        id = new char[1];
                        *id = 0;

                        APP_LOG(Log::LogLevel::WARN, "Empty Catalog Item id, item not owned");
                    }

                    ownerships[i].OwnershipStatus = owned;
                    ownerships[i].ApiVersion = itemownershipversion;
                    ownerships[i].Id = id;
                }
                qoci.ItemOwnership = ownerships;
            }

        }
    }

    qoci.ClientData = ClientData;
    qoci.ResultCode = EOS_EResult::EOS_Success;
    
    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Query the ownership status for a given list of catalog item ids defined with Epic Online Services.
 * The data is return via the callback in the form of a signed JWT that should be verified by an external backend server using a public key for authenticity.
 *
 * @param Options structure containing the account and catalog item ids to retrieve in token form
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 */
void EOSSDK_Ecom::QueryOwnershipToken(const EOS_Ecom_QueryOwnershipTokenOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOwnershipTokenCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    switch (Options->ApiVersion)
    {
        case EOS_ECOM_QUERYOWNERSHIPTOKEN_API_002:
        {
            auto opts = reinterpret_cast<const EOS_Ecom_QueryOwnershipTokenOptions002*>(Options);
            APP_LOG(Log::LogLevel::DEBUG, "CatalogNamespace: %s", (opts->CatalogNamespace == nullptr ? "" : opts->CatalogNamespace));
        }
        case EOS_ECOM_QUERYOWNERSHIPTOKEN_API_001:
        {
            auto opts = reinterpret_cast<const EOS_Ecom_QueryOwnershipTokenOptions001*>(Options);
            APP_LOG(Log::LogLevel::DEBUG, "CatalogItemIdCount: %u", opts->CatalogItemIdCount);
            for (uint32_t i = 0; i < opts->CatalogItemIdCount; ++i)
            {
                APP_LOG(Log::LogLevel::DEBUG, "CatalogItemIds[%u]: %s", i, (opts->CatalogItemIds[i] == nullptr ? "" : opts->CatalogItemIds[i]));
            }
        }
    }

    APP_LOG(Log::LogLevel::INFO, "TODO: Dispatch the callback");

    // TODO: See if this works
    //pFrameResult_t res(new FrameResult);
    //EOS_Ecom_QueryOwnershipTokenCallbackInfo& qotci = res->CreateCallback<EOS_Ecom_QueryOwnershipTokenCallbackInfo>((CallbackFunc)CompletionDelegate);
    //
    //qotci.ClientData = ClientData;
    //qotci.ResultCode = EOS_EResult::EOS_Success;
    //qotci.LocalUserId = Settings::Inst().userid;
    //{
    //    char* str = new char[32];
    //    memset(str, '_', 32);
    //    qotci.OwnershipToken = str;
    //}
    //
    //res->done = true;
    //GetCB_Manager().add_callback(this, res);
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
void EOSSDK_Ecom::QueryEntitlements(const EOS_Ecom_QueryEntitlementsOptions* Options, void* ClientData, const EOS_Ecom_OnQueryEntitlementsCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);

    EOS_Ecom_QueryEntitlementsCallbackInfo& qeci = res->CreateCallback<EOS_Ecom_QueryEntitlementsCallbackInfo>((CallbackFunc)CompletionDelegate);

    qeci.ClientData = ClientData;
    qeci.LocalUserId = Settings::Inst().userid;

    _queried_entitlements.clear();
    if (Options != nullptr)
    {
        switch (Options->ApiVersion)
        {
            case EOS_ECOM_QUERYENTITLEMENTS_API_002:
            {
                auto opts = reinterpret_cast<const EOS_Ecom_QueryEntitlementsOptions002*>(Options);
                APP_LOG(Log::LogLevel::DEBUG, "bIncludeRedeemed: %d", (int)opts->bIncludeRedeemed);
                APP_LOG(Log::LogLevel::DEBUG, "EntitlementNameCount: %u", opts->EntitlementNameCount);
                for (uint32_t i = 0; i < opts->EntitlementNameCount; ++i)
                {
                    auto it = _entitlements.find(opts->EntitlementNames[i]);
                    if (it != _entitlements.end())
                    {
                        bool redeemed;
                        try
                        {
                            redeemed = it.value()["redeemed"];
                        }
                        catch (...)
                        {
                            redeemed = false;
                        }
                        if (!redeemed || Options->bIncludeRedeemed == EOS_TRUE)
                        {
                            APP_LOG(Log::LogLevel::DEBUG, "EntitlementNames[%u]: %s - Found", i, (opts->EntitlementNames[i] == nullptr ? "" : opts->EntitlementNames[i]));
                            _queried_entitlements[opts->EntitlementNames[i]] = &it.value();
                        }
                        else
                        {
                            APP_LOG(Log::LogLevel::DEBUG, "EntitlementNames[%u]: %s - Found but was already redeemed and client asked for non-redeemed", i, (opts->EntitlementNames[i] == nullptr ? "" : opts->EntitlementNames[i]));
                        }
                    }
                    else
                    {
                        APP_LOG(Log::LogLevel::DEBUG, "EntitlementNames[%u]: %s - Not Found", i, (opts->EntitlementNames[i] == nullptr ? "" : opts->EntitlementNames[i]));
                    }
                }
            }
        }
        qeci.ResultCode = EOS_EResult::EOS_Success;
    }
    else
    {
        qeci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Query for a list of catalog offers defined with Epic Online Services.
 * This data will be cached for a limited time and retrieved again from the backend when necessary.
 *
 * @param Options structure containing filter criteria
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 */
void EOSSDK_Ecom::QueryOffers(const EOS_Ecom_QueryOffersOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOffersCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    switch (Options->ApiVersion)
    {
        case EOS_ECOM_QUERYOFFERS_API_001:
        {
            auto opts = reinterpret_cast<const EOS_Ecom_QueryOffersOptions001*>(Options);
            APP_LOG(Log::LogLevel::DEBUG, "OverrideCatalogNamespace: %s", (opts->OverrideCatalogNamespace == nullptr ? "" : opts->OverrideCatalogNamespace));
        }
    }
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
void EOSSDK_Ecom::Checkout(const EOS_Ecom_CheckoutOptions* Options, void* ClientData, const EOS_Ecom_OnCheckoutCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    switch (Options->ApiVersion)
    {
        case EOS_ECOM_CHECKOUT_API_001:
        {
            auto opts = reinterpret_cast<const EOS_Ecom_CheckoutOptions001*>(Options);
            APP_LOG(Log::LogLevel::DEBUG, "EntryCount: %u", opts->EntryCount);
            APP_LOG(Log::LogLevel::DEBUG, "OverrideCatalogNamespace: %s", (opts->OverrideCatalogNamespace == nullptr ? "" : opts->OverrideCatalogNamespace));
            for (uint32_t i = 0; i < opts->EntryCount; ++i)
            {
                auto pEntry = &opts->Entries[i];
                switch (pEntry->ApiVersion)
                {
                    case EOS_ECOM_CHECKOUTENTRY_API_001:
                    {
                        auto entry = reinterpret_cast<const EOS_Ecom_CheckoutEntry001*>(pEntry);
                        APP_LOG(Log::LogLevel::DEBUG, "Entries[%u].OfferId: %s", i, (entry->OfferId == nullptr ? "" : entry->OfferId));
                    }
                }
            }
        }
    }
}

/**
 * Requests that the provided entitlement be marked redeemed.  This will cause that entitlement
 * to no longer be returned from QueryEntitlements unless the include redeemed request flag is set true.
 *
 * @param Options structure containing entitlement to redeem
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 */
void EOSSDK_Ecom::RedeemEntitlements(const EOS_Ecom_RedeemEntitlementsOptions* Options, void* ClientData, const EOS_Ecom_OnRedeemEntitlementsCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();
    APP_LOG(Log::LogLevel::INFO, "TODO");

    if (CompletionDelegate == nullptr)
        return;

    switch (Options->ApiVersion)
    {
        case EOS_ECOM_REDEEMENTITLEMENTS_API_001:
        {
            auto opts = reinterpret_cast<const EOS_Ecom_RedeemEntitlementsOptions001*>(Options);
            APP_LOG(Log::LogLevel::DEBUG, "EntitlementIdCount: %u", opts->EntitlementIdCount);
            for (uint32_t i = 0; i < opts->EntitlementIdCount; ++i)
            {
                APP_LOG(Log::LogLevel::DEBUG, "EntitlementIds[%u]: %s", i, (opts->EntitlementIds[i] == nullptr ? "" : opts->EntitlementIds[i]));
            }
        }
    }
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
uint32_t EOSSDK_Ecom::GetEntitlementsCount(const EOS_Ecom_GetEntitlementsCountOptions* Options)
{
    TRACE_FUNC();
    GLOBAL_LOCK();
    
    if (Options == nullptr)
        return 0;

    return _queried_entitlements.size();
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
uint32_t EOSSDK_Ecom::GetEntitlementsByNameCount(const EOS_Ecom_GetEntitlementsByNameCountOptions* Options)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    APP_LOG(Log::LogLevel::INFO, "EntitlementName: %s", Options->EntitlementName == nullptr ? "<No Name>" : Options->EntitlementName);

    if (Options == nullptr || Options->EntitlementName == nullptr)
        return 0;

    uint32_t count = std::count_if(_queried_entitlements.begin(), _queried_entitlements.end(), [Options]( std::pair<const std::string, fifo_json*> &item) 
    {
        try
        {
            return (*item.second)["entitlement_name"].get_ref<std::string&>() == Options->EntitlementName;
        }
        catch (...)
        {
            return false;
        }
    });

    return count;
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
EOS_EResult EOSSDK_Ecom::CopyEntitlementByIndex(const EOS_Ecom_CopyEntitlementByIndexOptions* Options, EOS_Ecom_Entitlement** OutEntitlement)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (Options == nullptr || Options->EntitlementIndex >= _queried_entitlements.size() || OutEntitlement == nullptr)
    {
        set_nullptr(OutEntitlement);
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto it = _queried_entitlements.begin();
    std::advance(it, Options->EntitlementIndex);

    return copy_entitlement(it, OutEntitlement);
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
EOS_EResult EOSSDK_Ecom::CopyEntitlementByNameAndIndex(const EOS_Ecom_CopyEntitlementByNameAndIndexOptions* Options, EOS_Ecom_Entitlement** OutEntitlement)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (Options == nullptr || Options->EntitlementName == nullptr || OutEntitlement == nullptr)
    {
        set_nullptr(OutEntitlement);
        return EOS_EResult::EOS_InvalidParameters;
    }

    int i = 0;
    auto it = _queried_entitlements.begin();
    for (; it != _queried_entitlements.end(); ++it)
    {
        if ((*it->second)["entitlement_name"].get_ref<std::string&>() == Options->EntitlementName)
        {
            if (i == Options->Index)
            {
                break;
            }
            ++i;
        }
    }
    if (it == _queried_entitlements.end())
    {
        *OutEntitlement = nullptr;
        return EOS_EResult::EOS_NotFound;
    }
    
    return copy_entitlement(it, OutEntitlement);
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
EOS_EResult EOSSDK_Ecom::CopyEntitlementById(const EOS_Ecom_CopyEntitlementByIdOptions* Options, EOS_Ecom_Entitlement** OutEntitlement)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    APP_LOG(Log::LogLevel::INFO, "Entitlement id: %s", Options->EntitlementId == nullptr ? "<no id>" : Options->EntitlementId);

    if (Options == nullptr || Options->EntitlementId == nullptr || OutEntitlement == nullptr)
    {
        set_nullptr(OutEntitlement);
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto it = _queried_entitlements.find(Options->EntitlementId);
    if (it == _queried_entitlements.end())
    {
        *OutEntitlement = nullptr;
        return EOS_EResult::EOS_NotFound;
    }

    return copy_entitlement(it, OutEntitlement);
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
uint32_t EOSSDK_Ecom::GetOfferCount(const EOS_Ecom_GetOfferCountOptions* Options)
{
    TRACE_FUNC();
    APP_LOG(Log::LogLevel::INFO, "TODO");
    GLOBAL_LOCK();

    return 0;
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
EOS_EResult EOSSDK_Ecom::CopyOfferByIndex(const EOS_Ecom_CopyOfferByIndexOptions* Options, EOS_Ecom_CatalogOffer** OutOffer)
{
    TRACE_FUNC();
    APP_LOG(Log::LogLevel::INFO, "TODO");
    GLOBAL_LOCK();

    set_nullptr(OutOffer);
    return EOS_EResult::EOS_NotFound;
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
EOS_EResult EOSSDK_Ecom::CopyOfferById(const EOS_Ecom_CopyOfferByIdOptions* Options, EOS_Ecom_CatalogOffer** OutOffer)
{
    TRACE_FUNC();
    APP_LOG(Log::LogLevel::INFO, "TODO");
    GLOBAL_LOCK();

    set_nullptr(OutOffer);
    return EOS_EResult::EOS_NotFound;
}

/**
 * Fetch the number of items that are associated with a given cached offer for a local user.
 *
 * @return the number of items found.
 */
uint32_t EOSSDK_Ecom::GetOfferItemCount(const EOS_Ecom_GetOfferItemCountOptions* Options)
{
    TRACE_FUNC();
    APP_LOG(Log::LogLevel::INFO, "TODO");
    GLOBAL_LOCK();

    return 0;
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
EOS_EResult EOSSDK_Ecom::CopyOfferItemByIndex(const EOS_Ecom_CopyOfferItemByIndexOptions* Options, EOS_Ecom_CatalogItem** OutItem)
{
    TRACE_FUNC();
    APP_LOG(Log::LogLevel::INFO, "TODO");
    GLOBAL_LOCK();

    set_nullptr(OutItem);
    return EOS_EResult::EOS_NotFound;
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
EOS_EResult EOSSDK_Ecom::CopyItemById(const EOS_Ecom_CopyItemByIdOptions* Options, EOS_Ecom_CatalogItem** OutItem)
{
    TRACE_FUNC();
    APP_LOG(Log::LogLevel::INFO, "TODO");
    GLOBAL_LOCK();

    set_nullptr(OutItem);
    return EOS_EResult::EOS_NotFound;
}

/**
 * Fetch the number of images that are associated with a given cached offer for a local user.
 *
 * @return the number of images found.
 */
uint32_t EOSSDK_Ecom::GetOfferImageInfoCount(const EOS_Ecom_GetOfferImageInfoCountOptions* Options)
{
    TRACE_FUNC();
    APP_LOG(Log::LogLevel::INFO, "TODO");
    GLOBAL_LOCK();

    return 0;
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
EOS_EResult EOSSDK_Ecom::CopyOfferImageInfoByIndex(const EOS_Ecom_CopyOfferImageInfoByIndexOptions* Options, EOS_Ecom_KeyImageInfo** OutImageInfo)
{
    TRACE_FUNC();
    APP_LOG(Log::LogLevel::INFO, "TODO");
    GLOBAL_LOCK();

    set_nullptr(OutImageInfo);
    return EOS_EResult::EOS_NotFound;
}

/**
 * Fetch the number of images that are associated with a given cached item for a local user.
 *
 * @return the number of images found.
 */
uint32_t EOSSDK_Ecom::GetItemImageInfoCount(const EOS_Ecom_GetItemImageInfoCountOptions* Options)
{
    TRACE_FUNC();
    APP_LOG(Log::LogLevel::INFO, "TODO");
    GLOBAL_LOCK();

    return 0;
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
EOS_EResult EOSSDK_Ecom::CopyItemImageInfoByIndex(const EOS_Ecom_CopyItemImageInfoByIndexOptions* Options, EOS_Ecom_KeyImageInfo** OutImageInfo)
{
    TRACE_FUNC();
    APP_LOG(Log::LogLevel::INFO, "TODO");
    GLOBAL_LOCK();

    set_nullptr(OutImageInfo);
    return EOS_EResult::EOS_NotFound;
}

/**
 * Fetch the number of releases that are associated with a given cached item for a local user.
 *
 * @return the number of releases found.
 */
uint32_t EOSSDK_Ecom::GetItemReleaseCount(const EOS_Ecom_GetItemReleaseCountOptions* Options)
{
    TRACE_FUNC();
    APP_LOG(Log::LogLevel::INFO, "TODO");
    GLOBAL_LOCK();

    return 0;
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
EOS_EResult EOSSDK_Ecom::CopyItemReleaseByIndex(const EOS_Ecom_CopyItemReleaseByIndexOptions* Options, EOS_Ecom_CatalogRelease** OutRelease)
{
    TRACE_FUNC();
    APP_LOG(Log::LogLevel::INFO, "TODO");
    GLOBAL_LOCK();

    set_nullptr(OutRelease);
    return EOS_EResult::EOS_NotFound;
}

/**
 * Fetch the number of transactions that are cached for a given local user.
 *
 * @see EOS_Ecom_CheckoutCallbackInfo
 * @see EOS_Ecom_CopyTransactionByIndex
 *
 * @return the number of transactions found.
 */
uint32_t EOSSDK_Ecom::GetTransactionCount(const EOS_Ecom_GetTransactionCountOptions* Options)
{
    TRACE_FUNC();
    APP_LOG(Log::LogLevel::INFO, "TODO");
    GLOBAL_LOCK();
    
    return 0;
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
EOS_EResult EOSSDK_Ecom::CopyTransactionByIndex(const EOS_Ecom_CopyTransactionByIndexOptions* Options, EOS_Ecom_HTransaction* OutTransaction)
{
    TRACE_FUNC();
    APP_LOG(Log::LogLevel::INFO, "TODO");
    GLOBAL_LOCK();

    set_nullptr(OutTransaction);
    return EOS_EResult::EOS_NotFound;
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
EOS_EResult EOSSDK_Ecom::CopyTransactionById(const EOS_Ecom_CopyTransactionByIdOptions* Options, EOS_Ecom_HTransaction* OutTransaction)
{
    TRACE_FUNC();
    APP_LOG(Log::LogLevel::INFO, "TODO");
    GLOBAL_LOCK();

    set_nullptr(OutTransaction);
    return EOS_EResult::EOS_NotFound;
}

///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Ecom::CBRunFrame()
{
    return false;
}

bool EOSSDK_Ecom::RunNetwork(Network_Message_pb const& msg)
{
    return false;
}

bool EOSSDK_Ecom::RunCallbacks(pFrameResult_t res)
{
    GLOBAL_LOCK();

    return res->done;
}

void EOSSDK_Ecom::FreeCallback(pFrameResult_t res)
{
    GLOBAL_LOCK();

    switch (res->ICallback())
    {
        /////////////////////////////
        //        Callbacks        //
        /////////////////////////////
        case EOS_Ecom_QueryOwnershipCallbackInfo::k_iCallback:
        {
            EOS_Ecom_QueryOwnershipCallbackInfo& callback = res->GetCallback<EOS_Ecom_QueryOwnershipCallbackInfo>();
            if (callback.ItemOwnershipCount > 0)
            {
                for (uint32_t i = 0; i < callback.ItemOwnershipCount; ++i)
                    delete[]callback.ItemOwnership[i].Id;

                delete[] callback.ItemOwnership;
            }
        }
        break;

        case EOS_Ecom_QueryOwnershipTokenCallbackInfo::k_iCallback:
        {
            EOS_Ecom_QueryOwnershipTokenCallbackInfo& callback = res->GetCallback<EOS_Ecom_QueryOwnershipTokenCallbackInfo>();
            delete[]callback.OwnershipToken;
        }
        break;
        /////////////////////////////
        //      Notifications      //
        /////////////////////////////
        //case notification_type::k_iCallback:
        //{
        //    notification_type& callback = res->GetCallback<notification_type>();
        //    // Free resources
        //}
        //break;
    }
}

////////////////////////////////////////////////////////////
//                 EOSSDK_Ecom_Transaction                //
////////////////////////////////////////////////////////////

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
EOS_EResult EOSSDK_Ecom_Transaction::GetTransactionId(char* OutBuffer, int32_t* InOutBufferLength)
{
    TRACE_FUNC();

    return EOS_EResult::EOS_NotFound;
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
uint32_t EOSSDK_Ecom_Transaction::GetEntitlementsCount(const EOS_Ecom_Transaction_GetEntitlementsCountOptions* Options)
{
    TRACE_FUNC();

    return 0;
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
EOS_EResult EOSSDK_Ecom_Transaction::CopyEntitlementByIndex(const EOS_Ecom_Transaction_CopyEntitlementByIndexOptions* Options, EOS_Ecom_Entitlement** OutEntitlement)
{
    TRACE_FUNC();

    return EOS_EResult::EOS_NotFound;
}

}