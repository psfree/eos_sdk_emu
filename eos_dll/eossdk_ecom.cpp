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

EOSSDK_Ecom::EOSSDK_Ecom()
{
    GetCB_Manager().register_callbacks(this);
}

EOSSDK_Ecom::~EOSSDK_Ecom()
{
    GetCB_Manager().unregister_callbacks(this);
}

void EOSSDK_Ecom::QueryOwnership(const EOS_Ecom_QueryOwnershipOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOwnershipCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    pFrameResult_t res(new FrameResult);

    EOS_Ecom_QueryOwnershipCallbackInfo& qoci = res->CreateCallback<EOS_Ecom_QueryOwnershipCallbackInfo>((CallbackFunc)CompletionDelegate);

    int32_t itemownershipversion = EOS_ECOM_ITEMOWNERSHIP_API_LATEST;

    switch (Options->ApiVersion)
    {
        case EOS_ECOM_QUERYOWNERSHIP_API_002:
        {
            auto po = reinterpret_cast<const EOS_Ecom_QueryOwnershipOptions002*>(Options);
            //po->CatalogNamespace;
        }
        case EOS_ECOM_QUERYOWNERSHIP_API_001:
        {
            auto po = reinterpret_cast<const EOS_Ecom_QueryOwnershipOptions001*>(Options);
            qoci.LocalUserId = po->LocalUserId;
            
            qoci.ItemOwnershipCount = Options->CatalogItemIdCount;
            if (qoci.ItemOwnershipCount > 0)
            {
                EOS_Ecom_ItemOwnership* ownerships = new EOS_Ecom_ItemOwnership[qoci.ItemOwnershipCount];
                for (int i = 0; i < Options->CatalogItemIdCount; ++i)
                {
                    char* id;
                    if (po->CatalogItemIds[i] != nullptr)
                    {
                        size_t idlen = strlen(po->CatalogItemIds[i])+1;
                        id = new char[idlen];
                        strncpy(id, po->CatalogItemIds[i], idlen);
                        
                        LOG(Log::LogLevel::INFO, "Owning Catalog Item: %s", id);
                        ownerships[i].OwnershipStatus = EOS_EOwnershipStatus::EOS_OS_Owned;
                    }
                    else
                    {
                        id = new char[1];
                        *id = 0;

                        LOG(Log::LogLevel::INFO, "Empty Catalog Item id, not owned");
                        ownerships[i].OwnershipStatus = EOS_EOwnershipStatus::EOS_OS_NotOwned;
                    }

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

void EOSSDK_Ecom::QueryOwnershipToken(const EOS_Ecom_QueryOwnershipTokenOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOwnershipTokenCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

}

void EOSSDK_Ecom::QueryEntitlements(const EOS_Ecom_QueryEntitlementsOptions* Options, void* ClientData, const EOS_Ecom_OnQueryEntitlementsCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

}

void EOSSDK_Ecom::QueryOffers(const EOS_Ecom_QueryOffersOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOffersCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

}

void EOSSDK_Ecom::Checkout(const EOS_Ecom_CheckoutOptions* Options, void* ClientData, const EOS_Ecom_OnCheckoutCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

}

void EOSSDK_Ecom::RedeemEntitlements(const EOS_Ecom_RedeemEntitlementsOptions* Options, void* ClientData, const EOS_Ecom_OnRedeemEntitlementsCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

}

uint32_t EOSSDK_Ecom::GetEntitlementsCount(const EOS_Ecom_GetEntitlementsCountOptions* Options)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return 0;
}

uint32_t EOSSDK_Ecom::GetEntitlementsByNameCount(const EOS_Ecom_GetEntitlementsByNameCountOptions* Options)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return 0;
}

EOS_EResult EOSSDK_Ecom::CopyEntitlementByIndex(const EOS_Ecom_CopyEntitlementByIndexOptions* Options, EOS_Ecom_Entitlement** OutEntitlement)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return EOS_EResult::EOS_Success;
}

EOS_EResult EOSSDK_Ecom::CopyEntitlementByNameAndIndex(const EOS_Ecom_CopyEntitlementByNameAndIndexOptions* Options, EOS_Ecom_Entitlement** OutEntitlement)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return EOS_EResult::EOS_Success;
}

EOS_EResult EOSSDK_Ecom::CopyEntitlementById(const EOS_Ecom_CopyEntitlementByIdOptions* Options, EOS_Ecom_Entitlement** OutEntitlement)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return EOS_EResult::EOS_Success;
}

uint32_t EOSSDK_Ecom::GetOfferCount(const EOS_Ecom_GetOfferCountOptions* Options)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return 0;
}

EOS_EResult EOSSDK_Ecom::CopyOfferByIndex(const EOS_Ecom_CopyOfferByIndexOptions* Options, EOS_Ecom_CatalogOffer** OutOffer)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return EOS_EResult::EOS_Success;
}

EOS_EResult EOSSDK_Ecom::CopyOfferById(const EOS_Ecom_CopyOfferByIdOptions* Options, EOS_Ecom_CatalogOffer** OutOffer)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return EOS_EResult::EOS_Success;
}

uint32_t EOSSDK_Ecom::GetOfferItemCount(const EOS_Ecom_GetOfferItemCountOptions* Options)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return 0;
}

EOS_EResult EOSSDK_Ecom::CopyOfferItemByIndex(const EOS_Ecom_CopyOfferItemByIndexOptions* Options, EOS_Ecom_CatalogItem** OutItem)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return EOS_EResult::EOS_Success;
}

EOS_EResult EOSSDK_Ecom::CopyItemById(const EOS_Ecom_CopyItemByIdOptions* Options, EOS_Ecom_CatalogItem** OutItem)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return EOS_EResult::EOS_Success;
}

uint32_t EOSSDK_Ecom::GetOfferImageInfoCount(const EOS_Ecom_GetOfferImageInfoCountOptions* Options)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return 0;
}

EOS_EResult EOSSDK_Ecom::CopyOfferImageInfoByIndex(const EOS_Ecom_CopyOfferImageInfoByIndexOptions* Options, EOS_Ecom_KeyImageInfo** OutImageInfo)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return EOS_EResult::EOS_Success;
}

uint32_t EOSSDK_Ecom::GetItemImageInfoCount(const EOS_Ecom_GetItemImageInfoCountOptions* Options)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return 0;
}

EOS_EResult EOSSDK_Ecom::CopyItemImageInfoByIndex(const EOS_Ecom_CopyItemImageInfoByIndexOptions* Options, EOS_Ecom_KeyImageInfo** OutImageInfo)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return EOS_EResult::EOS_Success;
}

uint32_t EOSSDK_Ecom::GetItemReleaseCount(const EOS_Ecom_GetItemReleaseCountOptions* Options)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return 0;
}

EOS_EResult EOSSDK_Ecom::CopyItemReleaseByIndex(const EOS_Ecom_CopyItemReleaseByIndexOptions* Options, EOS_Ecom_CatalogRelease** OutRelease)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return EOS_EResult::EOS_Success;
}

uint32_t EOSSDK_Ecom::GetTransactionCount(const EOS_Ecom_GetTransactionCountOptions* Options)
{
    TRACE_FUNC();
    GLOBAL_LOCK();
    
    return 0;
}

EOS_EResult EOSSDK_Ecom::CopyTransactionByIndex(const EOS_Ecom_CopyTransactionByIndexOptions* Options, EOS_Ecom_HTransaction* OutTransaction)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return EOS_EResult::EOS_Success;
}

EOS_EResult EOSSDK_Ecom::CopyTransactionById(const EOS_Ecom_CopyTransactionByIdOptions* Options, EOS_Ecom_HTransaction* OutTransaction)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return EOS_EResult::EOS_Success;
}

EOS_EResult EOSSDK_Ecom::Transaction_GetTransactionId(EOS_Ecom_HTransaction Handle, char* OutBuffer, int32_t* InOutBufferLength)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return EOS_EResult::EOS_Success;
}

uint32_t EOSSDK_Ecom::Transaction_GetEntitlementsCount(EOS_Ecom_HTransaction Handle, const EOS_Ecom_Transaction_GetEntitlementsCountOptions* Options)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return 0;
}

EOS_EResult EOSSDK_Ecom::Transaction_CopyEntitlementByIndex(EOS_Ecom_HTransaction Handle, const EOS_Ecom_Transaction_CopyEntitlementByIndexOptions* Options, EOS_Ecom_Entitlement** OutEntitlement)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return EOS_EResult::EOS_Success;
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

    switch (res->res.m_iCallback)
    {
        /////////////////////////////
        //        Callbacks        //
        /////////////////////////////
        case EOS_Ecom_QueryOwnershipCallbackInfo::k_iCallback:
        {
            EOS_Ecom_QueryOwnershipCallbackInfo& qoci = res->GetCallback<EOS_Ecom_QueryOwnershipCallbackInfo>();
            if (qoci.ItemOwnershipCount > 0)
            {
                for (int i = 0; i < qoci.ItemOwnershipCount; ++i)
                    delete[]qoci.ItemOwnership[i].Id;

                delete[] qoci.ItemOwnership;
            }
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

}