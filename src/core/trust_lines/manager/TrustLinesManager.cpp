#include "TrustLinesManager.h"

TrustLinesManager::TrustLinesManager() {
    mTrustLinesStorage = new TrustLinesStorage("trust_lines_storage.bin");
    getTrustLinesFromStorage();
}

TrustLinesManager::~TrustLinesManager() {
    for (auto const& iterator : mTrustLines) {
        TrustLine::Shared ptr = iterator.second;
        ptr.reset();
    }
    mTrustLines.clear();
    delete mTrustLinesStorage;
}

byte *TrustLinesManager::serializeTrustLine(
        TrustLine::Shared trustLinePtr) {

    byte *buffer = (byte *) malloc(kBucketSize);
    memset(buffer, 0, kBucketSize);

    memcpy(buffer, trustAmountToBytes(trustLinePtr.get()->getIncomingTrustAmount()).data(), kTrustAmountPartSize);
    memcpy(buffer + kTrustAmountPartSize, trustAmountToBytes(trustLinePtr.get()->getOutgoingTrustAmount()).data(), kTrustAmountPartSize);
    memcpy(buffer + kTrustAmountPartSize * 2, balanceToBytes(trustLinePtr.get()->getBalance()).data(), kBalancePartSize + kSignBytePartSize);

    return buffer;
}

vector<byte> TrustLinesManager::trustAmountToBytes(
        const trust_amount &amount) {

    vector<byte> byteSet;
    export_bits(static_cast<boost::multiprecision::checked_uint256_t>(amount), back_inserter(byteSet), 8);

    size_t filledPlace = byteSet.size();
    for (unsigned long i = 0; i < kTrustAmountPartSize - filledPlace; ++i) {
        byteSet.push_back(0);
    }

    return byteSet;
}

vector<byte> TrustLinesManager::balanceToBytes(
        const balance_value &balance) {

    vector<byte> byteSet;
    export_bits(static_cast<boost::multiprecision::int256_t>(balance), back_inserter(byteSet), 8, true);

    size_t filledPlace = byteSet.size();
    for (unsigned long i = 0; i < kBalancePartSize - filledPlace; ++i) {
        byteSet.push_back(0);
    }

    if (balance.sign() == -1) {
        byteSet.push_back(1);

    } else {
        byteSet.push_back(0);
    }

    return byteSet;
}

void TrustLinesManager::deserializeTrustLine(
        const byte *buffer,
        const NodeUUID &contractorUUID) {

    TrustLine *trustLine = new TrustLine(contractorUUID,
                                         parseTrustAmount(buffer),
                                         parseTrustAmount(buffer + kTrustAmountPartSize),
                                         parseBalance(buffer + kTrustAmountPartSize * 2));

    mTrustLines.insert(make_pair(contractorUUID, TrustLine::Shared(trustLine)));
}

trust_amount TrustLinesManager::parseTrustAmount(
        const byte *buffer) {

    trust_amount amount;
    vector<byte> bytesVector(kTrustAmountPartSize);
    vector<byte> notZeroBytesVector;

    copy(buffer, buffer + kTrustAmountPartSize, bytesVector.begin());

    for (unsigned long i = 0; i < bytesVector.size(); ++i) {
        if (bytesVector.at(i) != 0) {
            notZeroBytesVector.push_back(bytesVector.at(i));
        }
    }

    if (notZeroBytesVector.size() > 0) {
        import_bits(amount, notZeroBytesVector.begin(), notZeroBytesVector.end());

    } else if (notZeroBytesVector.size() == 0) {
        import_bits(amount, bytesVector.begin(), bytesVector.end());
    }

    return amount;
}

balance_value TrustLinesManager::parseBalance(
        const byte *buffer) {

    balance_value balance;
    vector<byte> bytesVector(kBalancePartSize);
    vector<byte> notZeroBytesVector;

    byte sign = buffer[kBalancePartSize + kSignBytePartSize - 1];

    copy(buffer, buffer + kBalancePartSize, bytesVector.begin());
    for (unsigned long i = 0; i < bytesVector.size(); ++i) {
        if (bytesVector.at(i) != 0) {
            notZeroBytesVector.push_back(bytesVector.at(i));
        }
    }

    if (notZeroBytesVector.size() > 0) {
        import_bits(balance, notZeroBytesVector.begin(), notZeroBytesVector.end(), 8, true);

    } else if (notZeroBytesVector.size() == 0) {
        import_bits(balance, bytesVector.begin(), bytesVector.end(), 8, true);
    }

    if (sign == 1) {
        balance = balance * -1;
    }

    return balance;
}

void TrustLinesManager::saveTrustLine(
        TrustLine::Shared trustLinePtr) {

    byte *trustLineData = serializeTrustLine(trustLinePtr);

    if (isTrustLineExist(trustLinePtr.get()->getContractorNodeUUID())) {
        try{
            mTrustLinesStorage->rewrite(storage::uuids::uuid(trustLinePtr->getContractorNodeUUID()), trustLineData, kBucketSize);

        }catch (std::exception &e){
            throw IOError(string(string("Can't store existing trust line in file. Message-> ") + string(e.what())).c_str());
        }
        map<NodeUUID, TrustLine::Shared>::iterator it = mTrustLines.find(trustLinePtr->getContractorNodeUUID());
        if (it != mTrustLines.end()){
            it->second = trustLinePtr;
        }

    } else {
        try{
            mTrustLinesStorage->write(storage::uuids::uuid(trustLinePtr.get()->getContractorNodeUUID()), trustLineData, kBucketSize);

        }catch (std::exception &e){
            throw IOError(string(string("Can't store new trust line in file. Message-> ") + string(e.what())).c_str());
        }
        mTrustLines.insert(make_pair(trustLinePtr.get()->getContractorNodeUUID(), trustLinePtr));
    }

    free(trustLineData);
}

void TrustLinesManager::removeTrustLine(
        const NodeUUID &contractorUUID) {

    if (isTrustLineExist(contractorUUID)) {
        try{
            mTrustLinesStorage->erase(storage::uuids::uuid(contractorUUID));

        }catch (std::exception &e){
            throw IOError(string(string("Can't remove trust line from file. Message-> ") + string(e.what())).c_str());
        }
        auto it = mTrustLines.find(contractorUUID);
        TrustLine::Shared ptr = it->second;
        ptr.reset();
        mTrustLines.erase(contractorUUID);

    } else {
        throw ConflictError("Trust line to such contractor does not exist");
    }
}

bool TrustLinesManager::isTrustLineExist(
        const NodeUUID &contractorUUID) {

    return mTrustLines.count(contractorUUID) > 0;
}

void TrustLinesManager::getTrustLinesFromStorage() {

    vector<NodeUUID> contractorsUUIDs = mTrustLinesStorage->getAllContractorsUUIDs();
    if (contractorsUUIDs.size() > 0){
        for (auto const &item : contractorsUUIDs){
            const storage::Block *block = mTrustLinesStorage->readFromFile(storage::uuids::uuid(item));
            deserializeTrustLine(block->data(), item);
            delete block;
        }
    }
}

void TrustLinesManager::open(
        const NodeUUID &contractorUUID,
        const trust_amount &amount) {

    if (amount > trust_amount(0)){
        if (isTrustLineExist(contractorUUID)) {
            auto it = mTrustLines.find(contractorUUID);
            TrustLine::Shared trustLinePtr = it->second;
            if (trustLinePtr.get()->getOutgoingTrustAmount() == 0) {
                trustLinePtr.get()->setOutgoingTrustAmount(boost::bind(&TrustLinesManager::saveTrustLine, this, trustLinePtr), amount);

            } else {
                throw ConflictError("Сan not open outgoing trust line. Outgoing trust line to such contractor already exist.");
            }

        } else {
            TrustLine *trustLine = new TrustLine(contractorUUID, 0, amount, 0);
            saveTrustLine(TrustLine::Shared(trustLine));
        }

    } else {
        throw ValueError("Сan not open outgoing trust line. Outgoing trust line amount less or equals to zero.");
    }
}

void TrustLinesManager::close(
        const NodeUUID &contractorUUID) {

    if (isTrustLineExist(contractorUUID)) {
        auto it = mTrustLines.find(contractorUUID);
        TrustLine::Shared trustLinePtr = it->second;
        if (trustLinePtr.get()->getOutgoingTrustAmount() > trust_amount(0)){
            if (trustLinePtr.get()->getBalance() <= balance_value(0)) {
                if (trustLinePtr.get()->getIncomingTrustAmount() == trust_amount(0)) {
                    trustLinePtr.reset();
                    removeTrustLine(contractorUUID);

                } else {
                    trustLinePtr.get()->setOutgoingTrustAmount(boost::bind(&TrustLinesManager::saveTrustLine, this, trustLinePtr), 0);
                }

            } else {
                throw PreconditionFaultError("Сan not close outgoing trust line. Contractor already used part of amount.");
            }

        } else {
            throw ValueError("Сan not close outgoing trust line. Outgoing trust line amount less or equals to zero.");
        }

    } else {
        throw ConflictError("Сan not close outgoing trust line. Trust line to such contractor does not exist.");
    }
}

void TrustLinesManager::accept(
        const NodeUUID &contractorUUID,
        const trust_amount &amount) {

    if (amount > trust_amount(0)){
        if (isTrustLineExist(contractorUUID)) {
            auto it = mTrustLines.find(contractorUUID);
            TrustLine::Shared trustLinePtr = it->second;
            if (trustLinePtr.get()->getIncomingTrustAmount() == 0){
                trustLinePtr.get()->setIncomingTrustAmount(boost::bind(&TrustLinesManager::saveTrustLine, this, trustLinePtr), amount);
            } else {

                throw ConflictError("Сan not accept incoming trust line. Incoming trust line to such contractor already exist.");
            }

        } else {
            TrustLine *trustLine = new TrustLine(contractorUUID, amount, 0, 0);
            saveTrustLine(TrustLine::Shared(trustLine));
        }

    } else {
        throw ValueError("Сan not accept incoming trust line. Incoming trust line amount less or equals to zero.");
    }
}

void TrustLinesManager::reject(
        const NodeUUID &contractorUUID) {

    if (isTrustLineExist(contractorUUID)) {
        auto it = mTrustLines.find(contractorUUID);
        TrustLine::Shared trustLinePtr = it->second;
        if (trustLinePtr.get()->getIncomingTrustAmount() > trust_amount(0)){
            if (trustLinePtr.get()->getBalance() >= balance_value(0)) {
                if (trustLinePtr.get()->getOutgoingTrustAmount() == trust_amount(0)) {
                    trustLinePtr.reset();
                    removeTrustLine(contractorUUID);
                } else {

                    trustLinePtr.get()->setIncomingTrustAmount(boost::bind(&TrustLinesManager::saveTrustLine, this, trustLinePtr), 0);
                }

            } else {
                throw PreconditionFaultError("Сan not reject incoming trust line. User already used part of amount.");
            }

        } else {
            throw ValueError("Сan not reject incoming trust line. Incoming trust line amount less or equals to zero.");
        }

    } else {
        throw ConflictError("Сan not reject incoming trust line. Trust line to such contractor does not exist.");
    }
}

TrustLine::Shared TrustLinesManager::getTrustLineByContractorUUID(
        const NodeUUID &contractorUUID){

    if (isTrustLineExist(contractorUUID)) {
        return mTrustLines.at(contractorUUID);

    } else {
        throw ConflictError("Can't find trust line by such contractor UUID.");
    }
}


