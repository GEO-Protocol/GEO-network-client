#ifndef GEO_NETWORK_CLIENT_CONTRACTORSMANAGER_H
#define GEO_NETWORK_CLIENT_CONTRACTORSMANAGER_H

#include "Contractor.h"
#include "../io/storage/StorageHandler.h"
#include "../common/exceptions/ValueError.h"
#include "../common/exceptions/NotFoundError.h"

#include <map>

class ContractorsManager {

public:
    ContractorsManager(
        vector<pair<string, string>> ownAddresses,
        StorageHandler *storageHandler,
        Logger &logger);

    vector<Contractor::Shared> allContractors() const;

    Contractor::Shared createContractor(
        IOTransaction::Shared ioTransaction,
        vector<BaseAddress::Shared> contractorAddresses,
        const string &cryptoKey = "",
        const ContractorID channelIDOnContractorSide = 0);

    bool contractorPresent(
        ContractorID contractorID) const;

    bool channelConfirmed(
        ContractorID contractorID) const;

    Contractor::Shared contractor(
        ContractorID contractorID);

    const ContractorID idOnContractorSide(
        ContractorID contractorID) const;

    void setConfirmationInfo(
        IOTransaction::Shared ioTransaction,
        ContractorID contractorID,
        ContractorID idOnContractorSide,
        MsgEncryptor::PublicKey::Shared cryptoKey);

    void updateContractorCryptoKey(
        IOTransaction::Shared ioTransaction,
        ContractorID contractorID,
        const string cryptoKey);

    void updateChannelIDOnContractorSide(
        IOTransaction::Shared ioTransaction,
        ContractorID contractorID,
        ContractorID channelIdOnContractorSide);

    void regenerateCryptoKey(
        IOTransaction::Shared ioTransaction,
        ContractorID contractorID);

    vector<BaseAddress::Shared> ownAddresses() const;

    vector<BaseAddress::Shared> contractorAddresses(
        ContractorID contractorID) const;

    BaseAddress::Shared contractorMainAddress(
        ContractorID contractorID) const;

    // returns id of contractor if address will be founded and kNotFoundContractorID otherwise
    ContractorID contractorIDByAddress(
        BaseAddress::Shared address) const;

    Contractor::Shared selfContractor() const;

    // returns id of contractor if addresses will be founded and kNotFoundContractorID otherwise
    ContractorID contractorIDByAddresses(
        vector<BaseAddress::Shared> &checkedAddresses) const;

    void updateContractorAddresses(
        IOTransaction::Shared ioTransaction,
        ContractorID contractorID,
        vector<BaseAddress::Shared> newAddresses);

protected:
    const ContractorID nextFreeID(
        IOTransaction::Shared ioTransaction) const;

protected: // log shortcuts
    const string logHeader() const
    noexcept;

    LoggerStream info() const
    noexcept;

    LoggerStream debug() const
    noexcept;

    LoggerStream warning() const
    noexcept;

public:
    static const ContractorID kNotFoundContractorID = std::numeric_limits<ContractorID>::max();

private:
    map<ContractorID, Contractor::Shared> mContractors;
    Contractor::Shared mSelf;
    StorageHandler *mStorageHandler;
    Logger &mLogger;
};


#endif //GEO_NETWORK_CLIENT_CONTRACTORSMANAGER_H
