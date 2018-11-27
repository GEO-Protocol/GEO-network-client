#ifndef GEO_NETWORK_CLIENT_CONTRACTORSMANAGER_H
#define GEO_NETWORK_CLIENT_CONTRACTORSMANAGER_H

#include "Contractor.h"
#include "../io/storage/StorageHandler.h"
#include "../common/exceptions/NotFoundError.h"

#include <map>

class ContractorsManager {

public:
    ContractorsManager(
        const Host &interface,
        const Port port,
        StorageHandler *storageHandler,
        Logger &logger);

    ContractorID getContractorID(
        IOTransaction::Shared ioTransaction,
        const string &fullAddress,
        const NodeUUID &contractorUUID);

    ContractorID getContractorID(
        IPv4WithPortAddress::Shared ipv4Address,
        const NodeUUID &contractorUUID,
        IOTransaction::Shared ioTransaction = nullptr);

    bool contractorPresent(
        ContractorID contractorID) const;

    Contractor::Shared contractor(
        ContractorID contractorID);

    vector<BaseAddress::Shared> ownAddresses() const;

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
    IPv4WithPortAddress::Shared mOwnIPv4;
    StorageHandler *mStorageHandler;
    Logger &mLogger;
};


#endif //GEO_NETWORK_CLIENT_CONTRACTORSMANAGER_H
