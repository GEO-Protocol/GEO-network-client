#include "ContractorsManager.h"

ContractorsManager::ContractorsManager(
    vector<pair<string, string>> ownAddressesStr,
    StorageHandler *storageHandler,
    Logger &logger):
    mStorageHandler(storageHandler),
    mLogger(logger)
{
    vector<BaseAddress::Shared> selfAddresses;
    for (const auto &addressStr : ownAddressesStr) {
        if (addressStr.first == "ipv4") {
            try {
                selfAddresses.push_back(
                    make_shared<IPv4WithPortAddress>(
                        addressStr.second));
            } catch (...) {
                throw ValueError("ContractorsManager: can't create own address of type " + addressStr.first);
            }

        } else {
            throw ValueError("ContractorsManager: can't create own address. "
                                 "Wrong address type " + addressStr.first);
        }
    }
    mSelf = make_shared<Contractor>(selfAddresses);

    auto ioTransaction = mStorageHandler->beginTransaction();
    for (const auto &contractor : ioTransaction->contractorsHandler()->allContractors()) {
        auto contractorAddresses = ioTransaction->addressHandler()->contractorAddresses(
            contractor->getID());
        contractor->setAddresses(contractorAddresses);
        mContractors.insert(
            make_pair(
                contractor->getID(),
                contractor));
    }
#ifdef DEBUG_LOG_TRUST_LINES_PROCESSING
    for (const auto &contractor : mContractors) {
        debug() << contractor.second->toString();
    }
    debug() << "Own address " << mSelf->mainAddress()->fullAddress();
#endif
}

vector<Contractor::Shared> ContractorsManager::allContractors() const
{
    vector<Contractor::Shared> result;
    result.reserve(mContractors.size());
    for (const auto &contractor : mContractors) {
        result.push_back(
            contractor.second);
    }
    return result;
}

Contractor::Shared ContractorsManager::createContractor(
    IOTransaction::Shared ioTransaction,
    vector<BaseAddress::Shared> contractorAddresses,
    const string &cryptoKey)
{
    auto contractorID = contractorIDByAddresses(
        contractorAddresses);
    if (contractorID != kNotFoundContractorID) {
        throw ValueError("Channel for this contractor already present");
    }

    auto id = nextFreeID(ioTransaction);
    info() << "New contractor initializing " << id;
    if (cryptoKey == "") {
        // todo : generate new crypto key
        mContractors[id] = make_shared<Contractor>(
            id,
            contractorAddresses,
            MsgEncryptor::generateKeyTrio());
    } else {
        mContractors[id] = make_shared<Contractor>(
            id,
            contractorAddresses,
            MsgEncryptor::generateKeyTrio(cryptoKey));
    }
    ioTransaction->contractorsHandler()->saveContractor(
        mContractors[id]);
    for (const auto &address : contractorAddresses) {
        ioTransaction->addressHandler()->saveAddress(
            id,
            address);
    }
    return mContractors[id];
}

void ContractorsManager::setCryptoKey(
    IOTransaction::Shared ioTransaction,
    ContractorID contractorID,
    MsgEncryptor::PublicKeyShared cryptoKey)
{
    if (!contractorPresent(contractorID)) {
        throw NotFoundError(logHeader() + " There is no contractor " + to_string(contractorID));
    }

    auto contractor = mContractors[contractorID];
    contractor->cryptoKey().outputKey = cryptoKey;
    ioTransaction->contractorsHandler()->saveCryptoKey(contractor);
}

bool ContractorsManager::contractorPresent(
    ContractorID contractorID) const
{
    return mContractors.find(contractorID) != mContractors.end();
}

bool ContractorsManager::channelConfirmed(
    ContractorID contractorID) const
{
    if (!contractorPresent(contractorID)) {
        throw NotFoundError(logHeader() + " There is no contractor " + to_string(contractorID));
    }
    return mContractors.at(contractorID)->isConfirmed();
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
    contractor->confirm();
}

ContractorID ContractorsManager::contractorIDByAddresses(
    vector<BaseAddress::Shared> &checkedAddresses) const
{
    // todo : throw error if contractor is present but only with partial addresses
    for (const auto &contractor : mContractors) {
        bool contractorMatches = true;
        for (const auto &contractorAddress : contractor.second->addresses()) {
            bool addressMatches = false;
            for (const auto &checkedAddress : checkedAddresses) {
                if (checkedAddress == contractorAddress) {
                    addressMatches = true;
                    break;
                }
            }
            if (!addressMatches) {
                contractorMatches = false;
                break;
            }
        }
        if (contractorMatches) {
            return contractor.first;
        }
    }
    return kNotFoundContractorID;
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
    return mSelf->addresses();
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

Contractor::Shared ContractorsManager::selfContractor() const
{
    return mSelf;
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

LoggerStream ContractorsManager::debug() const
    noexcept
{
    return mLogger.debug(logHeader());
}

LoggerStream ContractorsManager::warning() const
    noexcept
{
    return mLogger.warning(logHeader());
}