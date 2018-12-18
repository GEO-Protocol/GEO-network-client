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
        BaseAddress::Shared contractorAddress,
        const NodeUUID &contractorUUID);

    ContractorID getContractorID(
        BaseAddress::Shared contractorAddress,
        const NodeUUID &contractorUUID,
        ContractorID idOnContractorSide,
        IOTransaction::Shared ioTransaction = nullptr);

    bool contractorPresent(
        ContractorID contractorID) const;

    Contractor::Shared contractor(
        ContractorID contractorID);

    const ContractorID idOnContractorSide(
        ContractorID contractorID) const;

    void setIDOnContractorSide(
        IOTransaction::Shared ioTransaction,
        ContractorID id,
        ContractorID idOnContractorSide);

    vector<BaseAddress::Shared> ownAddresses() const;

    vector<BaseAddress::Shared> contractorAddresses(
        ContractorID contractorID) const;

    BaseAddress::Shared contractorMainAddress(
        ContractorID contractorID) const;

    // returns id of contractor if address will be founded and kNotFoundContractorID otherwise
    ContractorID contractorIDByAddress(
        BaseAddress::Shared address) const;

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

public:
    static const ContractorID kNotFoundContractorID = std::numeric_limits<ContractorID>::max();

private:
    map<ContractorID, Contractor::Shared> mContractors;
    IPv4WithPortAddress::Shared mOwnIPv4;
    StorageHandler *mStorageHandler;
    Logger &mLogger;
};


#endif //GEO_NETWORK_CLIENT_CONTRACTORSMANAGER_H
