#ifndef GEO_NETWORK_CLIENT_CONTRACTOR_H
#define GEO_NETWORK_CLIENT_CONTRACTOR_H

#include "../common/Types.h"
#include "addresses/IPv4WithPortAddress.h"

class Contractor {
public:
    typedef shared_ptr<Contractor> Shared;

public:
    Contractor(
        ContractorID id,
        BaseAddress::Shared address);

    Contractor(
        ContractorID id,
        ContractorID idOnContractorSide,
        BaseAddress::Shared address);

    Contractor(
        ContractorID id,
        ContractorID idOnContractorSide,
        const string &address);

    const ContractorID getID() const;

    const ContractorID ownIdOnContractorSide() const;

    void setOwnIdOnContractorSide(ContractorID id);

    const BaseAddress::Shared getAddress() const;

    vector<BaseAddress::Shared> addresses() const;

    BaseAddress::Shared mainAddress() const;

private:
    ContractorID mID;
    ContractorID mOwnIdOnContractorSide;
    BaseAddress::Shared mAddress;
};


#endif //GEO_NETWORK_CLIENT_CONTRACTOR_H
