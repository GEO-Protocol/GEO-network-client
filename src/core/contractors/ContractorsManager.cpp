#include "ContractorsManager.h"

ContractorsManager::ContractorsManager(
    const Host &interface,
    const Port port,
    StorageHandler *storageHandler,
    Logger &logger):
    mStorageHandler(storageHandler),
    mLogger(logger)
{
    mOwnIPv4 = make_shared<IPv4WithPortAddress>(
        interface,
        port);
    auto ioTransaction = mStorageHandler->beginTransaction();
    for (const auto &contractor : ioTransaction->contractorsHandler()->allContractors()) {
        mContractors.insert(
            make_pair(
                contractor->getID(),
                contractor));
    }
    for (const auto &contractor : mContractors) {
        info() << contractor.second->getID() << " " << contractor.second->ownIdOnContractorSide() << " "
               << " " << contractor.second->getAddress()->fullAddress();
    }
    info() << "Own ip " << mOwnIPv4->fullAddress();
}

ContractorID ContractorsManager::getContractorID(
    IOTransaction::Shared ioTransaction,
    BaseAddress::Shared contractorAddress)
{
    for (const auto &contractor : mContractors) {
        if (contractor.second->getAddress() == contractorAddress) {
            return contractor.second->getID();
        }
    }
    auto id = nextFreeID(ioTransaction);
    info() << "New contractor initializing " << id;
    mContractors[id] = make_shared<Contractor>(
        id,
        contractorAddress);
    ioTransaction->contractorsHandler()->saveContractor(
        mContractors[id]);
    return id;
}

ContractorID ContractorsManager::getContractorID(
    BaseAddress::Shared contractorAddress,
    ContractorID idOnContractorSide,
    IOTransaction::Shared ioTransaction)
{
    for (const auto &contractor : mContractors) {
        if (contractor.second->getAddress() == contractorAddress) {
            return contractor.second->getID();
        }
    }

    if (ioTransaction != nullptr) {
        auto id = nextFreeID(ioTransaction);
        info() << "New contractor initializing " << id;
        mContractors[id] = make_shared<Contractor>(
            id,
            idOnContractorSide,
            contractorAddress);
        ioTransaction->contractorsHandler()->saveContractorFull(
            mContractors[id]);
        return id;
    } else {
        throw NotFoundError("There is no contractor with requested address");
    }
}

bool ContractorsManager::contractorPresent(
    ContractorID contractorID) const
{
    return mContractors.find(contractorID) != mContractors.end();
}

Contractor::Shared ContractorsManager::contractor(
    ContractorID contractorID)
{
    if (!contractorPresent(contractorID)) {
        throw NotFoundError(logHeader() + " There is no contractor " + to_string(contractorID));
    }
    return mContractors.at(contractorID);
}

const ContractorID ContractorsManager::idOnContractorSide(
    ContractorID contractorID) const
{
    if (!contractorPresent(contractorID)) {
        throw NotFoundError(logHeader() + " There is no contractor " + to_string(contractorID));
    }
    return mContractors.at(contractorID)->ownIdOnContractorSide();
}

void ContractorsManager::setIDOnContractorSide(
    IOTransaction::Shared ioTransaction,
    ContractorID contractorID,
    ContractorID idOnContractorSide)
{
    if (!contractorPresent(contractorID)) {
        throw NotFoundError(logHeader() + " There is no contractor " + to_string(contractorID));
    }

    auto contractor = mContractors[contractorID];
    contractor->setOwnIdOnContractorSide(idOnContractorSide);
    ioTransaction->contractorsHandler()->saveIdOnContractorSide(contractor);
}

const ContractorID ContractorsManager::nextFreeID(
    IOTransaction::Shared ioTransaction) const
{
    vector<TrustLineID> tmpIDs = ioTransaction->contractorsHandler()->allIDs();
    if (tmpIDs.empty()) {
        return 0;
    }
    sort(tmpIDs.begin(), tmpIDs.end());
    auto prevElement = *tmpIDs.begin();
    if (prevElement != 0) {
        return 0;
    }
    for (auto it = tmpIDs.begin() + 1; it != tmpIDs.end(); it++) {
        if (*it - prevElement > 1) {
            return prevElement + 1;
        }
        prevElement = *it;
    }
    return prevElement + 1;
}

vector<BaseAddress::Shared> ContractorsManager::ownAddresses() const
{
    vector<BaseAddress::Shared> result;
    result.push_back(mOwnIPv4);
    return result;
}

vector<BaseAddress::Shared> ContractorsManager::contractorAddresses(
    ContractorID contractorID) const
{
    if (!contractorPresent(contractorID)) {
        throw NotFoundError(logHeader() + " There is no contractor " + to_string(contractorID));
    }
    return mContractors.at(contractorID)->addresses();
}

BaseAddress::Shared ContractorsManager::contractorMainAddress(
    ContractorID contractorID) const
{
    if (!contractorPresent(contractorID)) {
        throw NotFoundError(logHeader() + " There is no contractor " + to_string(contractorID));
    }
    return mContractors.at(contractorID)->mainAddress();
}

ContractorID ContractorsManager::contractorIDByAddress(
    BaseAddress::Shared address) const
{
    for (const auto &contractor : mContractors) {
        for (const auto &contractorAddress : contractor.second->addresses()) {
            if (contractorAddress == address) {
                return contractor.first;
            }
        }
    }
    return kNotFoundContractorID;
}

const string ContractorsManager::logHeader() const
    noexcept
{
    stringstream s;
    s << "[ContractorsManager] ";
    return s.str();
}

LoggerStream ContractorsManager::info() const
    noexcept
{
    return mLogger.info(logHeader());
}

LoggerStream ContractorsManager::warning() const
    noexcept
{
    return mLogger.warning(logHeader());
}