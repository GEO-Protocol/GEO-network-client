#include "IPv4WithPortAddress.h"

IPv4WithPortAddress::IPv4WithPortAddress(
    const string &fullAddress)
{
    size_t addressSeparatorPos = fullAddress.find(
        kAddressSeparator);

    auto addressStr = fullAddress.substr(
        0,
        addressSeparatorPos);
    addressStr = addressStr + ".";

    size_t byteStartPos = 0;
    for (int idx = 0; idx < 4; idx++) {
        size_t bytesSeparatorPos = addressStr.find('.', byteStartPos);
        auto byteStr = addressStr.substr(byteStartPos, bytesSeparatorPos - byteStartPos);
        mAddress[idx] = (byte) std::stoul(byteStr);
        byteStartPos = bytesSeparatorPos + 1;
    }

    auto mPortStr = fullAddress.substr(
        addressSeparatorPos + 1,
        fullAddress.size() - addressSeparatorPos - 1);
    try {
        mPort = (uint16_t)std::stoul(mPortStr);
    } catch (...) {
        throw ValueError(
                "IPv4WithPortAddress: can't parse address. "
                    "Error occurred while parsing 'port' token.");
    }
}

IPv4WithPortAddress::IPv4WithPortAddress(
    const Host &host,
    const Port port) :
    mPort(port)
{
    auto addressStr = host + ".";
    size_t byteStartPos = 0;
    for (int idx = 0; idx < 4; idx++) {
        size_t bytesSeparatorPos = addressStr.find('.', byteStartPos);
        auto byteStr = addressStr.substr(byteStartPos, bytesSeparatorPos - byteStartPos);
        mAddress[idx] = (byte) std::stoul(byteStr);
        byteStartPos = bytesSeparatorPos + 1;
    }
}

IPv4WithPortAddress::IPv4WithPortAddress(
    byte* buffer)
{
    size_t bytesBufferOffset = sizeof(SerializedType);

    for (int idx = 0; idx < 4; idx++) {
        uint16_t nextIPByte;
        memcpy(
            &nextIPByte,
            buffer + bytesBufferOffset,
            sizeof(byte));
        bytesBufferOffset += sizeof(byte);
        mAddress[idx] = (byte)nextIPByte;
    }

    memcpy(
        &mPort,
        buffer + bytesBufferOffset,
        sizeof(Port));
}

const string IPv4WithPortAddress::host() const
{
    stringstream ss;
    ss << (int)mAddress[0] << "." << (int)mAddress[1] << "." << (int)mAddress[2] << "." << (int)mAddress[3];
    return ss.str();
}

const uint16_t IPv4WithPortAddress::port() const
{
    return mPort;
}

const string IPv4WithPortAddress::fullAddress() const
{
    stringstream ss;
    ss << (int)mAddress[0] << "." << (int)mAddress[1] << "." << (int)mAddress[2] << "."
       << (int)mAddress[3] << kAddressSeparator << mPort;
    return ss.str();
}

const BaseAddress::AddressType IPv4WithPortAddress::typeID() const
{
    return BaseAddress::IPv4_IncludingPort;
}

BytesShared IPv4WithPortAddress::serializeToBytes() const
{
    BytesShared dataBytesShared = tryCalloc(serializedSize());
    size_t dataBytesOffset = 0;

    auto addressType = typeID();
    memcpy(
        dataBytesShared.get(),
        &addressType,
        sizeof(SerializedType));
    dataBytesOffset += sizeof(SerializedType);

    for (int idx = 0; idx < 4; idx++) {
        auto nextByte = mAddress[idx];
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &nextByte,
            sizeof(byte));
        dataBytesOffset += sizeof(byte);
    }

    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mPort,
        sizeof(Port));

    return dataBytesShared;
}

size_t IPv4WithPortAddress::serializedSize() const
{
    // 1 bytes - address type
    // 4 bytes - ip
    // 2 bytes - port
    return 7;
}