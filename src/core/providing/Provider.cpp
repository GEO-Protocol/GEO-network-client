#include "Provider.h"

Provider::Provider(
    const string &providerName,
    const string &providerKey,
    const ProviderParticipantID participantID,
    vector<pair<string, string>> providerAddressesStr) :
    mName(providerName),
    mKey(make_shared<ProviderMsgEncryptor::PublicKey>(providerKey)),
    mParticipantID(participantID)
{
    if (providerAddressesStr.size() < 2) {
        throw ValueError("Provider: " + providerName + " has no enough addresses");
    }
    for (const auto &addressStr : providerAddressesStr) {
        if (addressStr.first == "ipv4") {
            try {
                mAddresses.push_back(
                    make_shared<IPv4WithPortAddress>(
                        addressStr.second));
            } catch (...) {
                throw ValueError("Provider: " + providerName +
                    " can't create provider address of type " + addressStr.first);
            }

        } else {
            throw ValueError("Provider: " + providerName + " can't create provider address. "
                             "Wrong address type " + addressStr.first);
        }
    }
}

const string Provider::name() const
{
    return mName;
}

ProviderMsgEncryptor::PublicKey::Shared Provider::publicKey() const
{
    return mKey;
}

IPv4WithPortAddress::Shared Provider::pingAddress() const
{
    return mAddresses.at(0);
}

IPv4WithPortAddress::Shared Provider::lookupAddress() const
{
    return mAddresses.at(1);
}

ProviderParticipantID Provider::participantID() const
{
    return mParticipantID;
}

const string Provider::info() const
{
    stringstream ss;
    ss << mName << " " << mKey;
    for (const auto &address : mAddresses) {
        ss << " " << address->fullAddress();
    }
    return ss.str();
}