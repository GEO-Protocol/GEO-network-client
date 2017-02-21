#ifndef PATH_H
#define PATH_H

#include "../../common/NodeUUID.h"

#include <vector>
#include <sstream>


class Path {
public:
    Path(
        NodeUUID &source,
        NodeUUID &destination,
        vector<NodeUUID> &&intermediateNodes);

    const bool containsIntermediateNodes() const;
    const string toString() const;

protected:
    NodeUUID &mSource;
    NodeUUID &mDestination;
    vector<NodeUUID>& mIntermediateNodes;
};

#endif // PATH_H
