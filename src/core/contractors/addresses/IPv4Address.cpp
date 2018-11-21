#include "IPv4Address.h"

IPv4Address::IPv4Address(
    const string &fullAddress)
{
    size_t addressSeparatorPos = fullAddress.find(
        kAddressSeparator);

    mAddress = fullAddress.substr(
        0,
        addressSeparatorPos);

    auto mPortStr = fullAddress.substr(
        addressSeparatorPos + 1,
        fullAddress.size() - addressSeparatorPos - 1);
    try {
        mPort = (uint16_t)std::stoul(mPortStr);
    } catch (...) {
        throw ValueError(
                "IPv4Address: can't parse address. "
                    "Error occurred while parsing 'port' token.");
    }
}

string IPv4Address::host() const
{
    return mAddress;
}

uint16_t IPv4Address::port() const
{
    return mPort;
}

string IPv4Address::fullAddress() const
{
    stringstream ss;
    ss << mAddress << kAddressSeparator << mPort;
    return ss.str();
}