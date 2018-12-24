#include "Contractor.h"

Contractor::Contractor(
    ContractorID id,
    BaseAddress::Shared address):
    mID(id),
    mAddress(address)
{}

Contractor::Contractor(
    ContractorID id,
    ContractorID idOnContractorSide,
    BaseAddress::Shared address) :
    mID(id),
    mOwnIdOnContractorSide(idOnContractorSide),
    mAddress(address)
{}

Contractor::Contractor(
    ContractorID id,
    ContractorID idOnContractorSide,
    const string &address) :
    mID(id),
    mOwnIdOnContractorSide(idOnContractorSide),
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