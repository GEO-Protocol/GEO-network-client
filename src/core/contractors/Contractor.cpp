#include "Contractor.h"

Contractor::Contractor(
    ContractorID id,
    const NodeUUID &uuid,
    BaseAddress::Shared address):
    mID(id),
    mUUID(uuid),
    mAddress(address)
{}

Contractor::Contractor(
    ContractorID id,
    ContractorID idOnContractorSide,
    const NodeUUID &uuid,
    BaseAddress::Shared address) :
    mID(id),
    mOwnIdOnContractorSide(idOnContractorSide),
    mUUID(uuid),
    mAddress(address)
{}

Contractor::Contractor(
    ContractorID id,
    ContractorID idOnContractorSide,
    const NodeUUID &uuid,
    const string &address) :
    mID(id),
    mOwnIdOnContractorSide(idOnContractorSide),
    mUUID(uuid),
    mAddress(make_shared<IPv4WithPortAddress>(address))
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

const BaseAddress::Shared Contractor::getAddress() const
{
    return mAddress;
}

vector<BaseAddress::Shared> Contractor::addresses() const
{
    vector<BaseAddress::Shared> result;
    result.push_back(mAddress);
    return result;
}

BaseAddress::Shared Contractor::mainAddress() const
{
    return mAddress;
}