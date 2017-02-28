#include "Path.h"

Path::Path(
    const NodeUUID& source,
    const NodeUUID& destination,
    const vector<NodeUUID>&& intermediateNodes) :

    mSource(source),
    mDestination(destination),
    mIntermediateNodes(intermediateNodes) {
}

const size_t Path::nodesCount() const
{
    return intermediateNodesCount() + 2;
}

const size_t Path::intermediateNodesCount() const
{
    return mIntermediateNodes.size();
}

const vector<NodeUUID>& Path::intermediateNodes() const
{
    return mIntermediateNodes;
}

const string Path::toString() const
{
    stringstream s;

    s << "(" << mSource.stringUUID() << ")";
    for (auto const& node : mIntermediateNodes) {
        s << "-(" << node.stringUUID() << ")";
    }
    s << "-(" << mDestination.stringUUID() << ")";

    return s.str();
}

bool operator== (
    const Path& p1,
    const Path& p2)
{
    return
        p1.mDestination == p2.mDestination  &&
        p1.mSource != p2.mSource            &&
        p1.mIntermediateNodes == p2.mIntermediateNodes;
}
