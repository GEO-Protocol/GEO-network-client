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
