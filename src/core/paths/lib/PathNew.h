#ifndef GEO_NETWORK_CLIENT_PATHNEW_H
#define GEO_NETWORK_CLIENT_PATHNEW_H

#include "../../contractors/addresses/BaseAddress.h"
#include "../../common/exceptions/IndexError.h"

#include <vector>
#include <sstream>

class PathNew {

public:
    typedef shared_ptr<PathNew> Shared;
    typedef shared_ptr<const PathNew> ConstShared;

public:
    PathNew(
        BaseAddress::Shared destination,
        const vector<BaseAddress::Shared> &intermediateNodes = vector<BaseAddress::Shared>());

    PathNew(
        BaseAddress::Shared destination,
        const vector<BaseAddress::Shared> &&intermediateNodes);

    BaseAddress::Shared destinationUUID() const;

    vector<BaseAddress::Shared> intermediateUUIDs() const;

    bool containsIntermediateNodes() const;

    int positionOfNode(
        BaseAddress::Shared nodeAddress) const;

    bool containsTrustLine(
        BaseAddress::Shared source,
        BaseAddress::Shared destination) const;

    const size_t length() const;

    const string toString() const;

    friend bool operator== (
        const PathNew &p1,
        const PathNew &p2);

public:
    const vector<BaseAddress::Shared> nodes;

};


#endif //GEO_NETWORK_CLIENT_PATHNEW_H
