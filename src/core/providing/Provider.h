#ifndef GEO_NETWORK_CLIENT_PROVIDER_H
#define GEO_NETWORK_CLIENT_PROVIDER_H

#include "../contractors/addresses/IPv4WithPortAddress.h"

class Provider {

public:
    typedef shared_ptr<Provider> Shared;

public:
    Provider(
        const string& providerName,
        const string& providerKey,
        vector<pair<string, string>> observersAddressesStr);

    const string name() const;

    BaseAddress::Shared mainAddress() const;

    ProviderParticipantID participantID() const;

    const string info() const;

private:
    string mName;
    string mKey;
    ProviderParticipantID mParticipantID;
    vector<IPv4WithPortAddress::Shared> mAddresses;
};


#endif //GEO_NETWORK_CLIENT_PROVIDER_H
