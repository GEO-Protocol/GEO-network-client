#ifndef GEO_NETWORK_CLIENT_PATHSCOLLECTION_H
#define GEO_NETWORK_CLIENT_PATHSCOLLECTION_H

#include "../../common/NodeUUID.h"
#include "../../common/exceptions/IndexError.h"

#include "../../common/memory/MemoryUtils.h"

#include <vector>


class PathsCollection {
public:
    typedef vector<NodeUUID> IntermediateNodes;
    typedef shared_ptr<IntermediateNodes> SharedIntermediateNodes;

public:
    class Path {
        friend class PathsCollection;

    public:
        typedef shared_ptr<Path> Shared;
        typedef shared_ptr<const Path> ConstShared;

    public:
        BytesShared serialize() const;

    private:
        // Note:
        // Paths must be created only from PathsCollection instances;
        Path(
            const NodeUUID &sourceNode,
            const NodeUUID &destinationNode,
            const PathsCollection::SharedIntermediateNodes intermediateNodes);

    private:
        const NodeUUID &mSourceNode;
        const NodeUUID &mDestinationNode;
        const PathsCollection::SharedIntermediateNodes mIntermediateNodes;
    };

    class Iterator {
    public:
        Iterator(
            NodeUUID &sourceNode,
            NodeUUID &destinationNode,
            const vector<SharedIntermediateNodes> &paths);

        Path next();

    private:
        const NodeUUID &mSourceNode;
        const NodeUUID &mDestinationNode;
        const vector<SharedIntermediateNodes> &mPaths;
        size_t mCurrentPathIndex;
    };

public:
    PathsCollection(
        const NodeUUID &sourceNode,
        const NodeUUID &destinationNode,
        bool isDirectPathPresent=false);

    void addPath(
        SharedIntermediateNodes intermediateNodes);

private:
    NodeUUID mSourceNode;
    NodeUUID mDestinationNode;
    bool mIsDirectPathPresent;
    vector<SharedIntermediateNodes> mPaths;
};


#endif //GEO_NETWORK_CLIENT_PATHSCOLLECTION_H
