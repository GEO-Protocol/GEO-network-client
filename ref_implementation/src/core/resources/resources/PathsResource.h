/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_PATHSRESOURCE_H
#define GEO_NETWORK_CLIENT_PATHSRESOURCE_H

#include "BaseResource.h"
#include "../../paths/lib/PathsCollection.h"

class PathsResource : public BaseResource {

public:
    typedef shared_ptr<PathsResource> Shared;

public:
    PathsResource(
        const TransactionUUID &transactionUUID,
        PathsCollection::Shared pathsCollection);

    PathsCollection::Shared pathCollection() const;

private:

    PathsCollection::Shared mPathsCollection;
};


#endif //GEO_NETWORK_CLIENT_PATHSRESOURCE_H
