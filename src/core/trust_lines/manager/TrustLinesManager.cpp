#include "TrustLinesManager.h"

TrustLinesManager::TrustLinesManager() {
    // todo: wrap into the try catch (std::bad_alloc)
    mTrustLinesStorage = new TrustLinesStorage("trust_lines_storage.bin"); // todo: move it to the io/trust_lines/trust_lines.dat

    // todo: this method should not be called into the constructor
    // otherwise delete for the previous instruction would not be called,
    // because constructor will not complete and destructor will not be called.
    getTrustLinesFromStorage();
}

TrustLinesManager::~TrustLinesManager() {
    // todo: critical error!
    // trust lines would be remove in case when all records of the map would be removed.
    for (auto const& iterator : mTrustLines) {
        TrustLine::Shared ptr = iterator.second;
        ptr.reset();
    }
    mTrustLines.clear();
    delete mTrustLinesStorage;
}

byte *TrustLinesManager::serializeTrustLine(
    TrustLine::Shared trustLine) {

    byte *buffer = (byte *) malloc(kBucketSize);
    memset(buffer, 0, kBucketSize); // todo: wtf?
    // todo: in case of exception buffer will remain in memory
    // todo: wrap logic into the try except
    // todo: wrap buffer into shared pointer right after creation

    memcpy(buffer, trustAmountToBytes(trustLine.get()->getIncomingTrustAmount()).data(), kTrustAmountPartSize);
    memcpy(buffer + kTrustAmountPartSize, trustAmountToBytes(trustLine.get()->getOutgoingTrustAmount()).data(), kTrustAmountPartSize);
    memcpy(buffer + kTrustAmountPartSize * 2, balanceToBytes(trustLine.get()->getBalance()).data(), kBalancePartSize + kSignBytePartSize);
    // todo: (hsc) check if this serializes the sign

    return buffer;
}

vector<byte> TrustLinesManager::trustAmountToBytes(
    const trust_amount &amount) {

    vector<byte> byteSet; // todo: add resize, prevent 256 memory reallocations
    export_bits(static_cast<boost::multiprecision::checked_uint256_t>(amount), back_inserter(byteSet), 8); // todo: can boost::multiprecision::checked_uint256_t be simplified?

    size_t filledPlace = byteSet.size();
    for (unsigned long i = 0; i < kTrustAmountPartSize - filledPlace; ++i) { // todo: for (size_t i = 0; ...
        byteSet.push_back(0);
    }

    return byteSet; // todo: use shared ptr, and create byteSet on the heap.
                    // prevent memory copying.
}

// todo: write a comment that this method returns 33 bytes instead of 32
// sign addition is too hidden.
vector<byte> TrustLinesManager::balanceToBytes(
        const balance_value &balance) {

    vector<byte> byteSet; // todo: resize, prevent 256 memory reallocations
    export_bits(static_cast<boost::multiprecision::int256_t>(balance), back_inserter(byteSet), 8, true); // todo: can boost::multiprecision::checked_uint256_t be simplified?

    size_t filledPlace = byteSet.size();
    for (unsigned long i = 0; i < kBalancePartSize - filledPlace; ++i) { // todo: for (size_t i = 0; ...
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

    TrustLine *trustLine = new TrustLine(
        contractorUUID,
        parseTrustAmount(buffer),
        parseTrustAmount(buffer + kTrustAmountPartSize),
        parseBalance(buffer + kTrustAmountPartSize * 2));

    mTrustLines.insert(make_pair(contractorUUID, TrustLine::Shared(trustLine)));
}

trust_amount TrustLinesManager::parseTrustAmount(
        const byte *buffer) {

    trust_amount amount;
    vector<byte> bytesVector(kTrustAmountPartSize); // todo: is reserve possible?
    copy(buffer, buffer + kTrustAmountPartSize, bytesVector.begin());

    vector<byte> notZeroBytesVector; // todo: is reserve possible?
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
    vector<byte> bytesVector(kBalancePartSize); // todo: is reserve possible?
    vector<byte> notZeroBytesVector; // todo: is reserve possible?

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
    TrustLine::Shared trustLine) {

    byte *trustLineData = serializeTrustLine(trustLine); // todo: use shared buffer

    // todo: internal storage should do the check if index is present.
    // this method only must call save() and it's all.
    // no presence check here.

    if (isTrustLineExist(trustLine.get()->getContractorNodeUUID())) { // todo: replace ".get()" with simple  ->
        try{
            mTrustLinesStorage->rewrite(storage::uuids::uuid(trustLine->getContractorNodeUUID()), trustLineData, kBucketSize);

        }catch (std::exception &e){
            // todo: memory leak: trustLineData remains in memory
            // todo: exception must inform about method that throws the exception to be able to find it via log file
            // todo: Exception constructor already accepts string AND char sequences. No cast is needed.
            throw IOError(string(string("Can't store existing trust line in file. Message-> ") + string(e.what())).c_str());
        }

        // todo: map<NodeUUID, TrustLine::Shared>::iterator it -> auto it
        // todo: trust line is always present. the check is several lines of code above this one.
        // todo: shared pointer already points to CHANGED trustline. what the logic of this operation? why are you change it one more time?
        map<NodeUUID, TrustLine::Shared>::iterator it = mTrustLines.find(trustLine->getContractorNodeUUID());
        if (it != mTrustLines.end()){
            it->second = trustLine;
        }

    } else {
        try{
            mTrustLinesStorage->write(storage::uuids::uuid(trustLine.get()->getContractorNodeUUID()), trustLineData, kBucketSize); // todo: replace ".get()" with simple  ->

        }catch (std::exception &e){
            // todo: memory leak: trustLineData remains in memory
            // todo: exception must inform about method that throws the exception to be able to find it via log file
            // todo: Exception constructor already accepts string AND char sequences. No cast is needed.
            throw IOError(string(string("Can't store new trust line in file. Message-> ") + string(e.what())).c_str());
        }
        mTrustLines.insert(make_pair(trustLine.get()->getContractorNodeUUID(), trustLine)); // todo: replace ".get()" with simple  ->
    }

    // todo: exception may be thrown and this instruction will be never called.
    free(trustLineData);
}

// todo: write comment about possible IOError throwed from this method.
// todo: write comment about possible NotFoundError throwed from this method.
void TrustLinesManager::removeTrustLine(
    const NodeUUID &contractorUUID) {

    if (isTrustLineExist(contractorUUID)) {
        try {
            mTrustLinesStorage->erase(storage::uuids::uuid(contractorUUID));

        }catch (std::exception &e){
            // todo: exception must inform about method that throws the exception to be able to find it via log file
            // todo: Exception constructor already accepts string AND char sequences. No cast is needed.

            // todo: storage should throws IOError
            throw IOError(string(string("Can't remove trust line from file. Message-> ") + string(e.what())).c_str());
        }
        auto it = mTrustLines.find(contractorUUID);
        TrustLine::Shared ptr = it->second; // shared ptr is alredy points to the trust line. what's the logic of this line?
        ptr.reset(); // todo: wtf??! critical data corruption is possible with such an approach!
        mTrustLines.erase(contractorUUID);

    } else {
        // todo: exception must inform about method that throws the exception to be able to find it via log file
        throw ConflictError("Trust line to such contractor does not exist"); // todo: use NotFoundError. it is alredy present
    }
}

bool TrustLinesManager::isTrustLineExist(
    const NodeUUID &contractorUUID) {

    return mTrustLines.count(contractorUUID) > 0;
}

// todo: write comment about possible IOError
// todo: write comment about possible MemoryError
void TrustLinesManager::getTrustLinesFromStorage() {

    vector<NodeUUID> contractorsUUIDs = mTrustLinesStorage->getAllContractorsUUIDs();
    if (contractorsUUIDs.size() > 0){
        for (auto const &item : contractorsUUIDs){
            const storage::Block *block = mTrustLinesStorage->readFromFile(storage::uuids::uuid(item)); // todo: rename storage::Block to the Record
            deserializeTrustLine(block->data(), item); // todo: TrustLine must have deserialization constructor
            delete block; // todo: shared ptr?
        }
    }
}

// todo: write comment about possible ConflictError
// todo: write comment about possible ValueError
void TrustLinesManager::open(
    const NodeUUID &contractorUUID,
    const trust_amount &amount) {

    if (amount > trust_amount(0)){ // todo: trust amount can't be less that 0!
        if (isTrustLineExist(contractorUUID)) {
            auto it = mTrustLines.find(contractorUUID);
            TrustLine::Shared trustLinePtr = it->second;
            if (trustLinePtr.get()->getOutgoingTrustAmount() == 0) { // todo: replace ".get()" with simple  ->
                trustLinePtr.get()->setOutgoingTrustAmount(amount, boost::bind(&TrustLinesManager::saveTrustLine, this, trustLinePtr)); // todo: replace ".get()" with simple  ->

            } else {
                // todo: exception must inform about method that throws the exception to be able to find it via log file
                throw ConflictError("Сan not open outgoing trust line. Outgoing trust line to such contractor already exist.");
            }

        } else {
            // todo: possible memory error
            TrustLine *trustLine = new TrustLine(contractorUUID, 0, amount, 0);
            saveTrustLine(TrustLine::Shared(trustLine));
        }

    } else {
        // todo: trust amount can't be less that 0!
        // todo: exception must inform about method that throws the exception to be able to find it via log file
        throw ValueError("Сan not open outgoing trust line. Outgoing trust line amount less or equals to zero.");
    }
}

// todo: add comment about possible PreconditionFaultError
// todo: add comment about possible ValueError
// todo: add comment about possible NotFoundError
void TrustLinesManager::close(
    const NodeUUID &contractorUUID) {

    if (isTrustLineExist(contractorUUID)) {
        auto it = mTrustLines.find(contractorUUID);
        TrustLine::Shared trustLinePtr = it->second;
        if (trustLinePtr.get()->getOutgoingTrustAmount() > trust_amount(0)){ // todo: replace ".get()" with simple  ->
            if (trustLinePtr.get()->getBalance() <= balance_value(0)) { // todo: replace ".get()" with simple  ->
                if (trustLinePtr.get()->getIncomingTrustAmount() == trust_amount(0)) { // todo: replace ".get()" with simple  ->
                    trustLinePtr.reset(); // todo: wtf?!!
                    removeTrustLine(contractorUUID);

                } else {
                    // todo: this line is too long
                    trustLinePtr.get()->setOutgoingTrustAmount(0, boost::bind(&TrustLinesManager::saveTrustLine, this, trustLinePtr)); // todo: replace ".get()" with simple  ->
                }

            } else {
                // todo: exception must inform about method that throws the exception to be able to find it via log file
                throw PreconditionFaultError("Сan not close outgoing trust line. Contractor already used part of amount.");
            }

        } else {
            // todo: exception must inform about method that throws the exception to be able to find it via log file
            throw ValueError("Сan not close outgoing trust line. Outgoing trust line amount less or equals to zero.");
        }

    } else {
        // todo: exception must inform about method that throws the exception to be able to find it via log file
        throw ConflictError("Сan not close outgoing trust line. Trust line to such contractor does not exist."); // todo: NotFoundError
    }
}

// todo: add comment about possible ConflictError
// todo: add comment about possible MemoryError
// todo: add comment about possible ValueError
void TrustLinesManager::accept(
    const NodeUUID &contractorUUID,
    const trust_amount &amount) {

    if (amount > trust_amount(0)){ // todo: trust amount can't be less that 0!
        if (isTrustLineExist(contractorUUID)) {
            auto it = mTrustLines.find(contractorUUID);
            TrustLine::Shared trustLinePtr = it->second;
            if (trustLinePtr.get()->getIncomingTrustAmount() == 0){ // todo: replace ".get()" with simple  ->
                trustLinePtr.get()->setIncomingTrustAmount(amount, boost::bind(&TrustLinesManager::saveTrustLine, this, // todo: replace ".get()" with simple  ->
                                                                               trustLinePtr));
            } else {

                // todo: exception must inform about method that throws the exception to be able to find it via log file
                throw ConflictError("Сan not accept incoming trust line. Incoming trust line to such contractor already exist.");
            }

        } else {
            // todo: memory error
            TrustLine *trustLine = new TrustLine(contractorUUID, amount, 0, 0);
            saveTrustLine(TrustLine::Shared(trustLine));
        }

    } else {
        // todo: trust amount can't be less that 0!
        throw ValueError("Сan not accept incoming trust line. Incoming trust line amount less or equals to zero.");
    }
}

// todo: add comment about possible PreconditionFailedError
// todo: add comment about possible ValueError
// todo: add comment about possible NotFoundError
void TrustLinesManager::reject(
        const NodeUUID &contractorUUID) {

    if (isTrustLineExist(contractorUUID)) {
        auto it = mTrustLines.find(contractorUUID);
        TrustLine::Shared trustLinePtr = it->second; // todo: trustLinePtr -> trustLine
        if (trustLinePtr.get()->getIncomingTrustAmount() > trust_amount(0)){ // todo: replace ".get()" with simple  ->
            if (trustLinePtr.get()->getBalance() >= balance_value(0)) { // todo: replace ".get()" with simple  ->
                if (trustLinePtr.get()->getOutgoingTrustAmount() == trust_amount(0)) {
                    trustLinePtr.reset(); //todo: wtf!!?
                    removeTrustLine(contractorUUID);
                } else {

                    // todo: add comment that TL would be saved by the callback
                    // it's not obviously at all.
                    trustLinePtr.get()->setIncomingTrustAmount(0, boost::bind(&TrustLinesManager::saveTrustLine, this,
                                                                              trustLinePtr));
                }

            } else {
                // todo: exception must inform about method that throws the exception to be able to find it via log file
                throw PreconditionFaultError("Сan not reject incoming trust line. User already used part of amount.");
            }

        } else {
            // todo: exception must inform about method that throws the exception to be able to find it via log file
            throw ValueError("Сan not reject incoming trust line. Incoming trust line amount less or equals to zero.");
        }

    } else {
        // todo: exception must inform about method that throws the exception to be able to find it via log file
        throw ConflictError("Сan not reject incoming trust line. Trust line to such contractor does not exist."); // todo: NotFoundError
    }
}

// todo: add comment about possible NotFoundError
TrustLine::Shared TrustLinesManager::getTrustLineByContractorUUID(
    const NodeUUID &contractorUUID){

    if (isTrustLineExist(contractorUUID)) {
        return mTrustLines.at(contractorUUID);

    } else {
        // todo: exception must inform about method that throws the exception to be able to find it via log file
        throw ConflictError("Can't find trust line by such contractor UUID."); // todo: NotFoundError
    }
}


