/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_PATHSCOLLECTION_H
#define GEO_NETWORK_CLIENT_PATHSCOLLECTION_H

#include "Path.h"
#include "../../common/exceptions/ValueError.h"
#include "../../common/exceptions/IndexError.h"

#include <vector>

class PathsCollection {

public:
    typedef shared_ptr<PathsCollection> Shared;

public:
    PathsCollection(
        const NodeUUID &sourceUUID,
        const NodeUUID &destinationUUID);

    void add(
        Path &path);

    void resetCurrentPath();

    Path::Shared nextPath();

    bool hasNextPath();

    size_t count() const;

private:
    NodeUUID mSourceNode;
    NodeUUID mDestinationNode;
    vector<vector<NodeUUID>> mPaths;
    size_t mCurrentPath;
};


#endif //GEO_NETWORK_CLIENT_PATHSCOLLECTION_H
