#ifndef PATH_H
#define PATH_H

#include "../../common/NodeUUID.h"

#include <vector>
#include <sstream>
#include <algorithm>

class Path {
public:
    Path(
        const NodeUUID &source,
        const NodeUUID &destination,
        const vector<NodeUUID> &&intermediateNodes);

    const size_t nodesCount() const;
    const size_t intermediateNodesCount() const;

    const vector<NodeUUID>& intermediateNodes() const;

    const string toString() const;

    friend bool operator== (
        const Path &p1,
        const Path &p2);

protected:
    const NodeUUID mSource;
    const NodeUUID mDestination;
    const vector<NodeUUID> mIntermediateNodes;
};

#endif // PATH_H
