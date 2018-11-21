#include "ContractorsManager.h"

ContractorsManager::ContractorsManager(
    StorageHandler *storageHandler,
    Logger &logger):
    mStorageHandler(storageHandler),
    mLogger(logger)
{
    auto ioTransaction = mStorageHandler->beginTransaction();
    for (const auto &contractor : ioTransaction->contractorsHandler()->allContractors()) {
        mContractors.insert(
            make_pair(
                contractor->getID(),
                contractor));
    }
    for (const auto &contractor : mContractors) {
        info() << contractor.second->getID() << " " << contractor.second->getUUID() << " " << contractor.second->getIPv4();
    }
}

ContractorID ContractorsManager::getContractorID(
    IOTransaction::Shared ioTransaction,
    const string &fullAddress,
    const NodeUUID &contractorUUID)
{
    for (const auto &contractor : mContractors) {
        if (contractor.second->getIPv4()->fullAddress() == fullAddress) {
            return contractor.second->getID();
        }
    }
    auto id = nextFreeID(ioTransaction);
    info() << "New contractor initializing " << id;
    mContractors[id] = make_shared<Contractor>(
        id,
        contractorUUID,
        fullAddress);
    ioTransaction->contractorsHandler()->saveContractor(
        mContractors[id]);
    return id;
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