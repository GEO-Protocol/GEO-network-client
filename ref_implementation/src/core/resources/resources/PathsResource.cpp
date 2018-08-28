/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "PathsResource.h"

PathsResource::PathsResource(
    const TransactionUUID &transactionUUID,
    PathsCollection::Shared pathsCollection):

    BaseResource(
        BaseResource::ResourceType::Paths,
        transactionUUID),

    mPathsCollection(pathsCollection){}

PathsCollection::Shared PathsResource::pathCollection() const {

    return mPathsCollection;
}
