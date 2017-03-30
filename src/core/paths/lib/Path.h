#ifndef PATH_H
#define PATH_H

#include "../../common/NodeUUID.h"

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
        const vector<NodeUUID> &intermediateNodes);

    Path(
        const NodeUUID &source,
        const NodeUUID &destination,
        const vector<NodeUUID> &&intermediateNodes);

    Path(
        const NodeUUID &source,
        const NodeUUID &destination);

    vector<NodeUUID> pathNodes() const;

    const NodeUUID &sourceUUID() const;

    const NodeUUID &destinationUUID() const;

    vector<NodeUUID> intermediateUUIDs() const;

    bool containsIntermediateNodes() const;

    const size_t length() const;
    const string toString() const;

    friend bool operator== (
        const Path &p1,
        const Path &p2);

public:
    const vector<NodeUUID> nodes;
    NodeUUID mSourceUUID;
    NodeUUID mDestinationUUID;
    vector<NodeUUID> mvIntermediateNodes;
};

#endif // PATH_H
