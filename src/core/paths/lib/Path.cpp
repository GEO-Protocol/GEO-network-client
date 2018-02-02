#include "Path.h"

Path::Path(
    const NodeUUID& source,
    const NodeUUID& destination,
    const vector<NodeUUID>& intermediateNodes) :

    nodes([](
        const NodeUUID& _source,
        const NodeUUID& _destination,
        const vector<NodeUUID>& _intermediateNodes) -> const vector<NodeUUID>
        {
            vector<NodeUUID> n;
            n.reserve(_intermediateNodes.size() + 2);

            n.push_back(_source);
            for (const auto &node : _intermediateNodes) {
                n.push_back(node);
            }
            n.push_back(_destination);

            return n;
        }(source, destination, intermediateNodes))
{}

Path::Path(
    const NodeUUID& source,
    const NodeUUID& destination,
    const vector<NodeUUID>&& intermediateNodes) :

    Path(source, destination, intermediateNodes)
{}

const NodeUUID& Path::sourceUUID() const
{
    if (nodes.empty()) {
        throw IndexError("Path::sourceUUID Path is empty");
    }
    return nodes.at(0);
}

const NodeUUID& Path::destinationUUID() const
{
    if (nodes.empty()) {
        throw IndexError("Path::destinationUUID Path is empty");
    }
    return nodes.at(nodes.size() - 1);
}

vector<NodeUUID> Path::intermediateUUIDs() const
{
    if (nodes.empty()) {
        throw IndexError("Path::intermediateUUIDs Path is empty");
    }
    if (nodes.size() <= 2) {
        return {};
    }
    auto itFirst = nodes.begin() + 1;
    auto itLast = nodes.begin() + (nodes.size() - 1);
    return vector<NodeUUID>(itFirst,  itLast);
}

const size_t Path::length() const
{
    return nodes.size();
}

bool Path::containsIntermediateNodes() const
{
    return nodes.size() > 2;
}

int Path::positionOfNode(
    const NodeUUID &nodeUUID) const
{
    for (size_t nodeIdx = 0; nodeIdx < nodes.size(); nodeIdx++) {
        if (nodes.at(nodeIdx) == nodeUUID) {
            return (int)nodeIdx;
        }
    }
    return -1;
}

bool Path::containsTrustLine(
    const NodeUUID &source,
    const NodeUUID &destination) const
{
    for (size_t nodeIdx = 0; nodeIdx < nodes.size() - 1; nodeIdx++) {
        if (nodes.at(nodeIdx) == source && nodes.at(nodeIdx + 1) == destination) {
            return true;
        }
    }
    return false;
}

const string Path::toString() const
{
    stringstream s;
    s << "(" << nodes.cbegin()->stringUUID() << ")";
    for (auto it=(++nodes.cbegin()); it != (--nodes.cend()); ++it) {
        s << "-(" << it->stringUUID() << ")";
    }
    s << "-(" << (--nodes.cend())->stringUUID() << ")";
    return s.str();
}

bool operator== (
    const Path& p1,
    const Path& p2)
{
    return p1.nodes == p2.nodes;
}
