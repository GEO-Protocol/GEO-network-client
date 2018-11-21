#include "Contractor.h"

Contractor::Contractor(
    ContractorID id,
    const NodeUUID &uuid,
    const string &ipv4):
    mID(id),
    mUUID(uuid),
    mIPv4(make_shared<IPv4Address>(ipv4))
{}

const ContractorID Contractor::getID() const
{
    return mID;
}

const NodeUUID& Contractor::getUUID() const
{
    return mUUID;
}

const IPv4Address::Shared Contractor::getIPv4() const
{
    return mIPv4;
}