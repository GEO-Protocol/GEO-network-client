#include "Path.h"

Path::Path(NodeUUID& source,
    NodeUUID& destination,
    vector<NodeUUID>&& intermediateNodes) :

    mSource(source),
    mDestination(destination),
    mIntermediateNodes(intermediateNodes) {
}

const bool Path::containsIntermediateNodes() const {
    return mIntermediateNodes.size() != 0;
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
