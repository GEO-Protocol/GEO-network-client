#ifndef GEO_NETWORK_CLIENT_CONTRACTOR_H
#define GEO_NETWORK_CLIENT_CONTRACTOR_H

#include "../common/Types.h"
#include "../common/NodeUUID.h"
#include "addresses/IPv4WithPortAddress.h"

class Contractor {
public:
    typedef shared_ptr<Contractor> Shared;

public:
    Contractor(
        ContractorID id,
        const NodeUUID &uuid,
        const string &ipv4);

    const ContractorID getID() const;

    const NodeUUID& getUUID() const;

    const IPv4WithPortAddress::Shared getIPv4() const;

    vector<BaseAddress::Shared> addresses() const;

private:
    ContractorID mID;
    NodeUUID mUUID;
    IPv4WithPortAddress::Shared mIPv4;
};


#endif //GEO_NETWORK_CLIENT_CONTRACTOR_H
