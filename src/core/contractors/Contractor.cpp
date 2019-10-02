#include "Contractor.h"

Contractor::Contractor(
    ContractorID id,
    vector<BaseAddress::Shared> &addresses,
    const MsgEncryptor::KeyTrio::Shared &cryptoKey) :
    mID(id),
    mCryptoKey(cryptoKey),
    mAddresses(addresses),
    mIsConfirmed(false)
{}

Contractor::Contractor(
    ContractorID id,
    ContractorID idOnContractorSide,
    const MsgEncryptor::KeyTrio::Shared &cryptoKey,
    bool isConfirmed) :
    mID(id),
    mOwnIdOnContractorSide(idOnContractorSide),
    mCryptoKey(cryptoKey),
    mIsConfirmed(isConfirmed)
{}

Contractor::Contractor(
    vector<BaseAddress::Shared> addresses):
    mAddresses(addresses)
{}

Contractor::Contractor(
    byte* buffer)
{
    size_t bufferDataOffset = 0;

    byte addressesCount;
    memcpy(
        &addressesCount,
        buffer + bufferDataOffset,
        sizeof(byte));
    bufferDataOffset += sizeof(byte);
    mAddresses.reserve(addressesCount);

    for (size_t idx = 0; idx < addressesCount; idx++) {
        auto address = deserializeAddress(
            buffer + bufferDataOffset);
        mAddresses.push_back(address);
        bufferDataOffset += address->serializedSize();
    }
}

const ContractorID Contractor::getID() const
{
    return mID;
}

const ContractorID Contractor::ownIdOnContractorSide() const
{
    return mOwnIdOnContractorSide;
}

void Contractor::setOwnIdOnContractorSide(
    ContractorID id)
{
    mOwnIdOnContractorSide = id;
}

MsgEncryptor::KeyTrio::Shared Contractor::cryptoKey()
{
    return mCryptoKey;
}

void Contractor::setCryptoKey(
    MsgEncryptor::KeyTrio::Shared cryptoKey)
{
    mCryptoKey = cryptoKey;
}

const bool Contractor::isConfirmed() const
{
    return mIsConfirmed;
}

void Contractor::confirm()
{
    mIsConfirmed = true;
}

vector<BaseAddress::Shared> Contractor::addresses() const
{
    return mAddresses;
}

void Contractor::setAddresses(
    vector<BaseAddress::Shared> &addresses)
{
    mAddresses = addresses;
}

BaseAddress::Shared Contractor::mainAddress() const
{
    return mAddresses.at(0);
}

bool Contractor::containsAddresses(
    vector<BaseAddress::Shared> &addresses) const
{
    for (const auto &address : addresses) {
        bool containsAddress = false;
        for (const auto &contractorAddress : mAddresses) {
            if (contractorAddress == address) {
                containsAddress = true;
                break;
            }
        }
        if (!containsAddress) {
            return false;
        }
    }
    return true;
}

bool Contractor::containsAtLeastOneAddress(
    vector<BaseAddress::Shared> addresses) const
{
    for (const auto &address : addresses) {
        for (const auto &contractorAddress : mAddresses) {
            if (contractorAddress == address) {
                return true;
            }
        }
    }
    return false;
}

BytesShared Contractor::serializeToBytes() const
{
    BytesShared dataBytesShared = tryCalloc(serializedSize());
    size_t dataBytesOffset = 0;

    auto addressesCount = mAddresses.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &addressesCount,
        sizeof(byte));
    dataBytesOffset += sizeof(byte);

    for (const auto &address : mAddresses) {
        auto serializedAddress = address->serializeToBytes();
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            serializedAddress.get(),
            address->serializedSize());
        dataBytesOffset += address->serializedSize();
    }
    return dataBytesShared;
}

size_t Contractor::serializedSize() const
{
    size_t result = sizeof(byte);
    for (const auto &address : mAddresses) {
        result += address->serializedSize();
    }
    return result;
}

string Contractor::toString() const
{
    stringstream ss;
    ss << mID << " " << mOwnIdOnContractorSide << " " << " " << mIsConfirmed << " ";
    for (const auto &address : mAddresses) {
        ss << address->fullAddress() << " ";
    }
    return ss.str();
}

string Contractor::outputString() const
{
    stringstream ss;
    auto first = true;
    for (const auto &address : mAddresses) {
        if (first) {
            ss << address->fullAddress();
            first = false;
        } else {
            ss << " " << address->fullAddress();
        }
    }
    return ss.str();
}

bool operator== (
    Contractor::Shared contractor1,
    Contractor::Shared contractor2)
{
    if (contractor1->addresses().size() != contractor2->addresses().size()) {
        return false;
    }
    for (size_t idx = 0; idx < contractor1->addresses().size(); idx++) {
        if (contractor1->addresses().at(idx) != contractor2->addresses().at(idx)) {
            return false;
        }
    }
    return true;
}

bool operator!= (
    Contractor::Shared contractor1,
    Contractor::Shared contractor2)
{
    if (contractor1->addresses().size() != contractor2->addresses().size()) {
        return true;
    }
    for (size_t idx = 0; idx < contractor1->addresses().size(); idx++) {
        if (contractor1->addresses().at(idx) != contractor2->addresses().at(idx)) {
            return true;
        }
    }
    return false;
}