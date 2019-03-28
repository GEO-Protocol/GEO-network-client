#ifndef GEO_NETWORK_CLIENT_CONTRACTOR_H
#define GEO_NETWORK_CLIENT_CONTRACTOR_H

#include "../common/multiprecision/MultiprecisionUtils.h"

class Contractor {
public:
    typedef shared_ptr<Contractor> Shared;

public:
    Contractor(
        ContractorID id,
        vector<BaseAddress::Shared> &addresses,
        uint32_t cryptoKey);

    Contractor(
        ContractorID id,
        ContractorID idOnContractorSide,
        uint32_t cryptoKey,
        bool isConfirmed);

    Contractor(
        vector<BaseAddress::Shared> addresses);

    Contractor(
        byte* buffer);

    const ContractorID getID() const;

    const ContractorID ownIdOnContractorSide() const;

    void setOwnIdOnContractorSide(ContractorID id);

    const uint32_t cryptoKey() const;

    const bool isConfirmed() const;

    void confirm();

    vector<BaseAddress::Shared> addresses() const;

    void setAddresses(
        vector<BaseAddress::Shared> &addresses);

    BaseAddress::Shared mainAddress() const;

    bool containsAddresses(
        vector<BaseAddress::Shared>& addresses) const;

    BytesShared serializeToBytes() const;

    size_t serializedSize() const;

    friend bool operator== (
        Contractor::Shared contractor1,
        Contractor::Shared contractor2);

    friend bool operator!= (
        Contractor::Shared contractor1,
        Contractor::Shared contractor2);

    string toString() const;

    string outputString() const;

private:
    ContractorID mID;
    ContractorID mOwnIdOnContractorSide;
    uint32_t mCryptoKey;
    bool mIsConfirmed;
    vector<BaseAddress::Shared> mAddresses;
};


#endif //GEO_NETWORK_CLIENT_CONTRACTOR_H
