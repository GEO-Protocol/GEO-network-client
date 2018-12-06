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
    ContractorID idOnContractorSide,
    const NodeUUID &uuid,
    IPv4WithPortAddress::Shared ipv4Address) :
    mID(id),
    mOwnIdOnContractorSide(idOnContractorSide),
    mUUID(uuid),
    mIPv4(ipv4Address)
{}

Contractor::Contractor(
    ContractorID id,
    ContractorID idOnContractorSide,
    const NodeUUID &uuid,
    const string &ipv4) :
    mID(id),
    mOwnIdOnContractorSide(idOnContractorSide),
    mUUID(uuid),
    mIPv4(make_shared<IPv4WithPortAddress>(ipv4))
{}

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

BaseAddress::Shared Contractor::mainAddress() const
{
    return mIPv4;
}