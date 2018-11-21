#ifndef GEO_NETWORK_CLIENT_CONTRACTORSMANAGER_H
#define GEO_NETWORK_CLIENT_CONTRACTORSMANAGER_H

#include "Contractor.h"
#include "../io/storage/StorageHandler.h"

#include <map>

class ContractorsManager {

public:
    ContractorsManager(
        StorageHandler *storageHandler,
        Logger &logger);

    ContractorID getContractorID(
        IOTransaction::Shared ioTransaction,
        const string &fullAddress,
        const NodeUUID &contractorUUID);

    bool contractorPresent(
        ContractorID contractorID) const;

    Contractor::Shared contractor(
        ContractorID contractorID);

protected:
    const ContractorID nextFreeID(
        IOTransaction::Shared ioTransaction) const;

protected: // log shortcuts
    const string logHeader() const
    noexcept;

    LoggerStream info() const
    noexcept;

    LoggerStream warning() const
    noexcept;

private:
    map<ContractorID, Contractor::Shared> mContractors;

    StorageHandler *mStorageHandler;
    Logger &mLogger;
};


#endif //GEO_NETWORK_CLIENT_CONTRACTORSMANAGER_H
