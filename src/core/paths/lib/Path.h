#ifndef PATH_H
#define PATH_H

#include "../../common/NodeUUID.h"
#include "../../common/exceptions/IndexError.h"

#include <vector>
#include <sstream>


class Path {
public:
    typedef shared_ptr<Path> Shared;
    typedef shared_ptr<const Path> ConstShared;

public:
    Path(
        const NodeUUID &source,
        const NodeUUID &destination,
        const vector<NodeUUID> &intermediateNodes = vector<NodeUUID>());

    Path(
        const NodeUUID &source,
        const NodeUUID &destination,
        const vector<NodeUUID> &&intermediateNodes);

    const NodeUUID &sourceUUID() const;

    const NodeUUID &destinationUUID() const;

    vector<NodeUUID> intermediateUUIDs() const;

    bool containsIntermediateNodes() const;

    int positionOfNode(
        const NodeUUID &nodeUUID) const;

    bool containsTrustLine(
        const NodeUUID &source,
        const NodeUUID &destination) const;

    const size_t length() const;

    const string toString() const;

    friend bool operator== (
        const Path &p1,
        const Path &p2);

public:
    const vector<NodeUUID> nodes;
};

#endif // PATH_H
