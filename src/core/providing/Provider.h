#ifndef GEO_NETWORK_CLIENT_PROVIDER_H
#define GEO_NETWORK_CLIENT_PROVIDER_H

#include "../contractors/addresses/IPv4WithPortAddress.h"
#include "../crypto/ProviderMsgEncryptor.h"

class Provider {

public:
    typedef shared_ptr<Provider> Shared;

public:
    Provider(
        const string& providerName,
        const string& providerKey,
        const ProviderParticipantID participantID,
        vector<pair<string, string>> providerAddressesStr);

    const string name() const;

    ProviderMsgEncryptor::PublicKey::Shared publicKey() const;

    IPv4WithPortAddress::Shared pingAddress() const;

    IPv4WithPortAddress::Shared lookupAddress() const;

    ProviderParticipantID participantID() const;

    const string info() const;

private:
    string mName;
    ProviderMsgEncryptor::PublicKey::Shared mKey;
    ProviderParticipantID mParticipantID;
    vector<IPv4WithPortAddress::Shared> mAddresses;
};


#endif //GEO_NETWORK_CLIENT_PROVIDER_H
