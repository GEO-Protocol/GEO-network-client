#include "Provider.h"

Provider::Provider(
    const string &providerName,
    const string &providerKey,
    vector<pair<string, string>> providersAddressesStr) :
    mName(providerName),
    mKey(providerKey)
{
    if (providersAddressesStr.empty()) {
        throw ValueError("Provider: " + providerName + " has no addresses");
    }
    for (const auto &addressStr : providersAddressesStr) {
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

BaseAddress::Shared Provider::mainAddress() const
{
    return mAddresses.at(0);
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