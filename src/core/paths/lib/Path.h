#ifndef GEO_NETWORK_CLIENT_PATH_H
#define GEO_NETWORK_CLIENT_PATH_H

#include "../../contractors/addresses/BaseAddress.h"
#include "../../common/exceptions/IndexError.h"

#include <vector>
#include <sstream>

class Path {

public:
    typedef shared_ptr<Path> Shared;
    typedef shared_ptr<const Path> ConstShared;

public:
    Path(
        vector<BaseAddress::Shared> intermediateNodes);

    // todo : rename intermediate and receiver
    vector<BaseAddress::Shared> intermediates() const;

    int positionOfNode(
        BaseAddress::Shared nodeAddress) const;

    void addReceiver(
        BaseAddress::Shared receiverAddress);

    bool containsTrustLine(
        BaseAddress::Shared source,
        BaseAddress::Shared destination) const;

    const size_t length() const;

    const string toString() const;

    friend bool operator== (
        const Path &p1,
        const Path &p2);

private:
    vector<BaseAddress::Shared> nodes;
};


#endif //GEO_NETWORK_CLIENT_PATH_H
