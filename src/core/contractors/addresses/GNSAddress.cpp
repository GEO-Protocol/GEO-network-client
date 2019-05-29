#include "GNSAddress.h"

GNSAddress::GNSAddress(
    const string &fullAddress)
{
    size_t addressSeparatorPos = fullAddress.find(
        kGNSAddressSeparator);
    if (addressSeparatorPos == string::npos) {
        throw ValueError(
            "GNSAddress: can't parse address. There are no separator");
    }

    mName = fullAddress.substr(
        0,
        addressSeparatorPos);

    mProvider = fullAddress.substr(
        addressSeparatorPos + 1,
        fullAddress.size() - addressSeparatorPos - 1);
}

GNSAddress::GNSAddress(
    byte* buffer)
{
    size_t bytesBufferOffset = sizeof(SerializedType);

    auto *addressLength = new (buffer + bytesBufferOffset) uint16_t;
    if (*addressLength == 0) {
        throw ValueError("GNSAddress: can't read 0 length address");
    }
    bytesBufferOffset += sizeof(uint16_t);
    string gnsAddress = string(
        buffer + bytesBufferOffset,
        buffer + bytesBufferOffset + *addressLength);

    size_t addressSeparatorPos = gnsAddress.find(
            kGNSAddressSeparator);
    if (addressSeparatorPos == string::npos) {
        throw ValueError(
                "GNSAddress: can't parse address. There are no separator");
    }

    mName = gnsAddress.substr(
        0,
        addressSeparatorPos);

    mProvider = gnsAddress.substr(
        addressSeparatorPos + 1,
        gnsAddress.size() - addressSeparatorPos - 1);
}

const string GNSAddress::host() const
{
    return mHost;
}

const uint16_t GNSAddress::port() const
{
    return mPort;
}

const string GNSAddress::fullAddress() const
{
    stringstream ss;
    ss << mName << kGNSAddressSeparator << mProvider;
    return ss.str();
}

const BaseAddress::AddressType GNSAddress::typeID() const
{
    return BaseAddress::GNS;
}

BytesShared GNSAddress::serializeToBytes() const
{
    BytesShared dataBytesShared = tryCalloc(serializedSize());
    size_t dataBytesOffset = 0;

    auto addressType = typeID();
    memcpy(
        dataBytesShared.get(),
        &addressType,
        sizeof(SerializedType));
    dataBytesOffset += sizeof(SerializedType);

    const string fullAddress = this->fullAddress();
    auto addressLength = (uint16_t)fullAddress.size();
    memcpy(
        dataBytesShared.get(),
        &addressLength,
        sizeof(uint16_t));
    dataBytesOffset += sizeof(uint16_t);

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        fullAddress.c_str(),
        addressLength);

    return dataBytesShared;
}

size_t GNSAddress::serializedSize() const
{
    // 1 bytes - address type
    // 2 bytes - address length
    // rest bytes - address
    return sizeof(byte) + sizeof(uint16_t) + mName.size() + 1 + mProvider.size();
}

void GNSAddress::setIPAndPort(
    const string &providerData)
{
    size_t addressSeparatorPos = providerData.find(
        kAddressSeparator);
    if (addressSeparatorPos == string::npos) {
        throw ValueError(
            "GNSAddress: can't parse provider data. There are no separator");
    }

    mHost = providerData.substr(
        0,
        addressSeparatorPos);

    auto mPortStr = providerData.substr(
        addressSeparatorPos + 1,
        providerData.size() - addressSeparatorPos - 1);
    try {
        mPort = (uint16_t)std::stoul(mPortStr);
    } catch (...) {
        throw ValueError(
                "GNSAddress: can't parse address. "
                "Error occurred while parsing 'port' token.");
    }
}

const string GNSAddress::provider() const
{
    return mProvider;
}