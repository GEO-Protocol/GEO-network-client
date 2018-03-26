/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_BASERESOURCE_H
#define GEO_NETWORK_CLIENT_BASERESOURCE_H

#include "../../common/Types.h"
#include "../../transactions/transactions/base/TransactionUUID.h"

#include <memory>

class BaseResource {
public:
    typedef shared_ptr<BaseResource> Shared;

public:
    enum ResourceType {
        Paths = 1
    };

public:
    BaseResource(
        const ResourceType type,
        const TransactionUUID &transactionUUID);

    const ResourceType type() const;

    const TransactionUUID &transactionUUID() const;

private:
    ResourceType mType;
    TransactionUUID mTransactionUUID;

};


#endif //GEO_NETWORK_CLIENT_BASERESOURCE_H
