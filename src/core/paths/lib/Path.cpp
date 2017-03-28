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
        }(source, destination, intermediateNodes)),
    mSourceUUID(source),
    mDestinationUUID(destination),
    mvIntermediateNodes(intermediateNodes)
{}

Path::Path(
    const NodeUUID& source,
    const NodeUUID& destination,
    const vector<NodeUUID>&& intermediateNodes) :

    Path(source, destination, intermediateNodes) {}

/*Path::Path(
    const NodeUUID &source,
    const NodeUUID &destination):

    Path(
        source,
        destination,
        vector<NodeUUID>()::) {}*/

vector<NodeUUID> Path::pathNodes() const {

    vector<NodeUUID> result;
    result.reserve(intermediateUUIDs().size() + 2);
    result.push_back(mSourceUUID);
    result.insert(result.end(), mvIntermediateNodes.begin(), mvIntermediateNodes.end());
    result.push_back(mDestinationUUID);
    return nodes;
}

const NodeUUID& Path::sourceUUID() const {

    return mSourceUUID;
}

const NodeUUID& Path::destinationUUID() const {

    return mDestinationUUID;
}

vector<NodeUUID> Path::intermediateUUIDs() const {

    return mvIntermediateNodes;
}

const size_t Path::length() const
{
    return nodes.size();
}

bool Path::containsIntermediateNodes() const {

    return mvIntermediateNodes.size() != 0;
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
