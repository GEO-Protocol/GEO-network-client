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
        contractor->setAddresses(
            contractorAddresses);
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
    const string &cryptoKey,
    const ContractorID channelIDOnContractorSide)
{
    auto contractorID = contractorIDByAddresses(
        contractorAddresses);
    if (contractorID != kNotFoundContractorID) {
        throw ValueError("Channel for this contractor already present");
    }

    auto id = nextFreeID(ioTransaction);
    info() << "New contractor initializing " << id;
    if (cryptoKey.empty()) {
        auto contractor = make_shared<Contractor>(
            id,
            contractorAddresses,
            MsgEncryptor::generateKeyTrio());
        mContractors[id] = contractor;
        ioTransaction->contractorsHandler()->saveContractor(
            mContractors[id]);
    } else {
        auto contractor = make_shared<Contractor>(
            id,
            contractorAddresses,
            MsgEncryptor::generateKeyTrio(cryptoKey));
        contractor->setOwnIdOnContractorSide(
            channelIDOnContractorSide);
        contractor->confirm();
        mContractors[id] = contractor;
        ioTransaction->contractorsHandler()->saveContractorFull(
            mContractors[id]);
    }
    for (const auto &address : contractorAddresses) {
        ioTransaction->addressHandler()->saveAddress(
            id,
            address);
    }
    return mContractors[id];
}

void ContractorsManager::setConfirmationInfo(
    IOTransaction::Shared ioTransaction,
    ContractorID contractorID,
    ContractorID idOnContractorSide,
    MsgEncryptor::PublicKey::Shared cryptoKey)
{
    if (!contractorPresent(contractorID)) {
        throw NotFoundError(logHeader() + " There is no contractor " + to_string(contractorID));
    }

    auto contractor = mContractors[contractorID];
    if (!contractor->cryptoKey()) {
        throw NotFoundError(logHeader() + " This contractor does not support encryption " + to_string(contractorID));
    }

    contractor->cryptoKey()->contractorPublicKey = cryptoKey;
    contractor->setOwnIdOnContractorSide(idOnContractorSide);
    contractor->confirm();
    ioTransaction->contractorsHandler()->saveConfirmationInfo(
        contractor);
}

void ContractorsManager::updateContractorCryptoKey(
    IOTransaction::Shared ioTransaction,
    ContractorID contractorID,
    const string cryptoKey)
{
    if (!contractorPresent(contractorID)) {
        throw NotFoundError(logHeader() + " There is no contractor " + to_string(contractorID));
    }

    auto contractor = mContractors[contractorID];
    if (!contractor->cryptoKey()) {
        throw NotFoundError(logHeader() + " This contractor does not support encryption " + to_string(contractorID));
    }

    contractor->cryptoKey()->contractorPublicKey = make_shared<MsgEncryptor::PublicKey>(cryptoKey);
    ioTransaction->contractorsHandler()->updateCryptoKey(
        contractor);
}

void ContractorsManager::regenerateCryptoKey(
    IOTransaction::Shared ioTransaction,
    ContractorID contractorID)
{
    if (!contractorPresent(contractorID)) {
        throw NotFoundError(logHeader() + " There is no contractor " + to_string(contractorID));
    }

    auto contractor = mContractors[contractorID];
    if (!contractor->cryptoKey()) {
        throw NotFoundError(logHeader() + " This contractor does not support encryption " + to_string(contractorID));
    }

    auto regeneratedCryptoKey = MsgEncryptor::generateKeyTrio(
        contractor->cryptoKey()->contractorPublicKey);
    contractor->setCryptoKey(
        regeneratedCryptoKey);
    ioTransaction->contractorsHandler()->updateCryptoKey(
        contractor);
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

void ContractorsManager::updateContractorAddresses(
    IOTransaction::Shared ioTransaction,
    ContractorID contractorID,
    vector<BaseAddress::Shared> newAddresses)
{
    if (!contractorPresent(contractorID)) {
        throw NotFoundError(logHeader() + " There is no contractor " + to_string(contractorID));
    }
    ioTransaction->addressHandler()->removeAddresses(contractorID);
    for (const auto &address : newAddresses) {
        ioTransaction->addressHandler()->saveAddress(
            contractorID,
            address);
    }
    mContractors.at(contractorID)->setAddresses(newAddresses);
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