#include "PathNew.h"

PathNew::PathNew(
    BaseAddress::Shared destination,
    const vector<BaseAddress::Shared>& intermediateNodes) :

    nodes([](
        BaseAddress::Shared _destination,
        const vector<BaseAddress::Shared>& _intermediateNodes) -> const vector<BaseAddress::Shared>
          {
              vector<BaseAddress::Shared> n;
              n.reserve(_intermediateNodes.size() + 1);

              for (const auto &node : _intermediateNodes) {
                  n.push_back(node);
              }
              n.push_back(_destination);

              return n;
          }(destination, intermediateNodes))
{}

PathNew::PathNew(
    BaseAddress::Shared destination,
    const vector<BaseAddress::Shared>&& intermediateNodes) :

    PathNew(destination, intermediateNodes)
{}

BaseAddress::Shared PathNew::destinationUUID() const
{
    if (nodes.empty()) {
        throw IndexError("Path::destinationUUID Path is empty");
    }
    return nodes.at(nodes.size() - 1);
}

vector<BaseAddress::Shared> PathNew::intermediateUUIDs() const
{
    if (nodes.empty()) {
        throw IndexError("Path::intermediateUUIDs Path is empty");
    }
    if (nodes.size() <= 1) {
        return {};
    }
    auto itFirst = nodes.begin();
    auto itLast = nodes.begin() + (nodes.size() - 1);
    return vector<BaseAddress::Shared>(itFirst,  itLast);
}

const size_t PathNew::length() const
{
    return nodes.size();
}

bool PathNew::containsIntermediateNodes() const
{
    return nodes.size() > 2;
}

int PathNew::positionOfNode(
    BaseAddress::Shared nodeAddress) const
{
    for (size_t nodeIdx = 0; nodeIdx < nodes.size(); nodeIdx++) {
        if (nodes.at(nodeIdx) == nodeAddress) {
            return (int)nodeIdx;
        }
    }
    return -1;
}

bool PathNew::containsTrustLine(
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

const string PathNew::toString() const
{
    stringstream s;
    s << "(" << nodes.cbegin()->get()->fullAddress() << ")";
    for (auto it=(++nodes.cbegin()); it != (--nodes.cend()); ++it) {
        s << "-(" << it->get()->fullAddress() << ")";
    }
    s << "-(" << (--nodes.cend())->get()->fullAddress() << ")";
    return s.str();
}

bool operator== (
    const PathNew& p1,
    const PathNew& p2)
{
    // todo : implement if need
    return false;
}