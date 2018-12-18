#include "Path.h"

Path::Path(
    vector<BaseAddress::Shared> intermediateNodes) :
    nodes(intermediateNodes)
{}

vector<BaseAddress::Shared> Path::intermediates() const
{
    return nodes;
}

const size_t Path::length() const
{
    return nodes.size();
}

int Path::positionOfNode(
    BaseAddress::Shared nodeAddress) const
{
    for (size_t nodeIdx = 0; nodeIdx < nodes.size(); nodeIdx++) {
        if (nodes.at(nodeIdx) == nodeAddress) {
            return (int)nodeIdx;
        }
    }
    return -1;
}

void Path::addReceiver(
    BaseAddress::Shared receiverAddress)
{
    nodes.push_back(receiverAddress);
}

bool Path::containsTrustLine(
    BaseAddress::Shared source,
    BaseAddress::Shared destination) const
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
    if (nodes.empty()) {
        return "direct path";
    }
    stringstream s;
    s << "(" << nodes.cbegin()->get()->fullAddress() << ")";
    for (auto it=(++nodes.cbegin()); it != (nodes.cend()); ++it) {
        s << "-(" << it->get()->fullAddress() << ")";
    }
    return s.str();
}

bool operator== (
    const Path& p1,
    const Path& p2)
{
    // todo : implement if need
    return false;
}