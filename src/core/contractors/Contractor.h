#ifndef GEO_NETWORK_CLIENT_CONTRACTOR_H
#define GEO_NETWORK_CLIENT_CONTRACTOR_H

#include "../common/multiprecision/MultiprecisionUtils.h"
#include "../crypto/MsgEncryptor.h"

class Contractor {
public:
    typedef shared_ptr<Contractor> Shared;

public:
    Contractor(
        ContractorID id,
        vector<BaseAddress::Shared> &addresses,
        const MsgEncryptor::KeyTrio &cryptoKey);

    Contractor(
        ContractorID id,
        ContractorID idOnContractorSide,
        const MsgEncryptor::KeyTrio &cryptoKey,
        bool isConfirmed);

    Contractor(
        vector<BaseAddress::Shared> addresses);

    Contractor(
        byte* buffer);

    const ContractorID getID() const;

    const ContractorID ownIdOnContractorSide() const;

    void setOwnIdOnContractorSide(ContractorID id);

    MsgEncryptor::KeyTrio &cryptoKey();

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
    MsgEncryptor::KeyTrio mCryptoKey;
    bool mIsConfirmed;
    vector<BaseAddress::Shared> mAddresses;
};


#endif //GEO_NETWORK_CLIENT_CONTRACTOR_H
