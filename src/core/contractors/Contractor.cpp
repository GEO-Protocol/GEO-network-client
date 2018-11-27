#include "Contractor.h"

Contractor::Contractor(
    ContractorID id,
    const NodeUUID &uuid,
    const string &ipv4):
    mID(id),
    mUUID(uuid),
    mIPv4(make_shared<IPv4WithPortAddress>(ipv4))
{}

Contractor::Contractor(
    ContractorID id,
    const NodeUUID &uuid,
    IPv4WithPortAddress::Shared ipv4Address) :
    mID(id),
    mUUID(uuid),
    mIPv4(ipv4Address)
{}

const ContractorID Contractor::getID() const
{
    return mID;
}

const NodeUUID& Contractor::getUUID() const
{
    return mUUID;
}

const IPv4WithPortAddress::Shared Contractor::getIPv4() const
{
    return mIPv4;
}

vector<BaseAddress::Shared> Contractor::addresses() const
{
    vector<BaseAddress::Shared> result;
    result.push_back(mIPv4);
    return result;
}