/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "BaseResource.h"

BaseResource::BaseResource(
    const BaseResource::ResourceType type,
    const TransactionUUID &transactionUUID) :

    mType(type),
    mTransactionUUID(transactionUUID) {}

const BaseResource::ResourceType BaseResource::type() const {

    return mType;
}

const TransactionUUID &BaseResource::transactionUUID() const {

    return mTransactionUUID;
}




