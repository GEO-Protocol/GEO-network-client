#include "TrustLinesManager.h"

TrustLinesManager::TrustLinesManager() {
    // todo: wrap into the try catch (std::bad_alloc)
    mTrustLinesStorage = new TrustLinesStorage(
        "trust_lines_storage.bin"); // todo: move it to the io/trust_lines/trust_lines.dat

}

TrustLinesManager::~TrustLinesManager() {
    mTrustLines.clear();
    delete mTrustLinesStorage;
}

/**
 * throws IOError - unable to write or update data in storage
 */
void TrustLinesManager::saveTrustLine(
    TrustLine::Shared trustLine) {

    vector<byte> *trustLineData = trustLine->serializeTrustLine();

    if (isTrustLineExist(trustLine->contractorNodeUUID())) {
        try {
            mTrustLinesStorage->rewrite(
                storage::uuids::uuid(
                    trustLine->contractorNodeUUID()),
                trustLineData->data(),
                kRecordSize);

        } catch (std::exception &e) {
            delete trustLineData;
            throw IOError(e.what());
        }

    } else {
        try {
            mTrustLinesStorage->write(
                storage::uuids::uuid(
                    trustLine->contractorNodeUUID()),
                trustLineData->data(),
                kRecordSize);

        } catch (std::exception &e) {
            delete trustLineData;
            throw IOError(e.what());
        }
        mTrustLines.insert(make_pair(trustLine->contractorNodeUUID(), trustLine));
    }

    delete trustLineData;
}

/**
 * throw IOError - unable to remove data from storage
 * throw NotFoundError - operation with not existing trust line
 */
void TrustLinesManager::removeTrustLine(
    const NodeUUID &contractorUUID) {

    if (isTrustLineExist(contractorUUID)) {
        try {
            mTrustLinesStorage->erase(storage::uuids::uuid(contractorUUID));

        } catch (std::exception &e) {
            throw IOError("TrustLinesManager::removeTrustLine. "
                              "Can't remove trust line from file.");
        }
        mTrustLines.erase(contractorUUID);

    } else {
        throw NotFoundError(
            "TrustLinesManager::removeTrustLine. "
                "Trust line to such contractor does not exist.");
    }
}

const bool TrustLinesManager::isTrustLineExist(
    const NodeUUID &contractorUUID) const {

    return mTrustLines.count(contractorUUID) > 0;
}

/**
 * throw Exception - unable to create instance of trust line
 */
void TrustLinesManager::loadTrustLines() {

    vector<NodeUUID> contractorsUUIDs = mTrustLinesStorage->getAllContractorsUUIDs();

    if (contractorsUUIDs.size() > 0) {

        for (auto const &item : contractorsUUIDs) {

            storage::Record *record = nullptr;
            try {
                 record = mTrustLinesStorage->readFromFile(storage::uuids::uuid(item));

            } catch(std::exception &e) {
                if (record != nullptr) {
                    delete record;
                }
                throw IOError(e.what());
            }

            try{
                TrustLine *trustLine = new TrustLine(record->data(), item);
                mTrustLines.insert(make_pair(item, TrustLine::Shared(trustLine)));

            } catch (...) {
                if (record != nullptr) {
                    delete record;
                }
                throw Exception("TrustLinesManager::loadTrustLine. "
                                    "Unable to create trust line instance from buffer.");
            }

            delete record;
        }
    }
}


void TrustLinesManager::open(
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount) {

    if (isTrustLineExist(contractorUUID)) {
        auto it = mTrustLines.find(contractorUUID);
        TrustLine::Shared trustLinePtr = it->second;
        if (trustLinePtr->outgoingTrustAmount() == 0) {
            trustLinePtr->setOutgoingTrustAmount(
                amount,
                boost::bind(&TrustLinesManager::saveTrustLine, this, trustLinePtr));

        } else {
            throw ConflictError(
                "TrustLinesManager::open. "
                    "Сan not open outgoing trust line. Outgoing trust line to such contractor already exist.");
        }

    } else {
        TrustLine *trustLine = new TrustLine(
            contractorUUID, 
            0, 
            amount, 
            0);
        saveTrustLine(TrustLine::Shared(trustLine));
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
        if (trustLinePtr->outgoingTrustAmount() > TrustLineAmount(0)) { // todo: replace ".get()" with simple  ->
            if (trustLinePtr->balance() <= TrustLineBalance(0)) { // todo: replace ".get()" with simple  ->
                if (trustLinePtr->incomingTrustAmount() ==
                    TrustLineAmount(0)) { // todo: replace ".get()" with simple  ->
                    trustLinePtr.reset(); // todo: wtf?!!
                    removeTrustLine(contractorUUID);

                } else {
                    // todo: this line is too long
                    trustLinePtr->setOutgoingTrustAmount(0, boost::bind(&TrustLinesManager::saveTrustLine, this,
                                                                              trustLinePtr)); // todo: replace ".get()" with simple  ->
                }

            } else {
                // todo: exception must inform about method that throws the exception to be able to find it via log file
                throw PreconditionFaultError(
                    "Сan not close outgoing trust line. Contractor already used part of amount.");
            }

        } else {
            // todo: exception must inform about method that throws the exception to be able to find it via log file
            throw ValueError("Сan not close outgoing trust line. Outgoing trust line amount less or equals to zero.");
        }

    } else {
        // todo: exception must inform about method that throws the exception to be able to find it via log file
        throw ConflictError(
            "Сan not close outgoing trust line. Trust line to such contractor does not exist."); // todo: NotFoundError
    }
}

// todo: add comment about possible ConflictError
// todo: add comment about possible MemoryError
// todo: add comment about possible ValueError
void TrustLinesManager::accept(
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount) {

    if (amount > TrustLineAmount(0)) { // todo: trust amount can't be less that 0!
        if (isTrustLineExist(contractorUUID)) {
            auto it = mTrustLines.find(contractorUUID);
            TrustLine::Shared trustLinePtr = it->second;
            if (trustLinePtr->incomingTrustAmount() == 0) { // todo: replace ".get()" with simple  ->
                trustLinePtr->setIncomingTrustAmount(amount, boost::bind(&TrustLinesManager::saveTrustLine,
                                                                               this, // todo: replace ".get()" with simple  ->
                                                                               trustLinePtr));
            } else {

                // todo: exception must inform about method that throws the exception to be able to find it via log file
                throw ConflictError(
                    "Сan not accept incoming trust line. Incoming trust line to such contractor already exist.");
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
        if (trustLinePtr->incomingTrustAmount() > TrustLineAmount(0)) { // todo: replace ".get()" with simple  ->
            if (trustLinePtr->balance() >= TrustLineBalance(0)) { // todo: replace ".get()" with simple  ->
                if (trustLinePtr->outgoingTrustAmount() == TrustLineAmount(0)) {
                    trustLinePtr.reset(); //todo: wtf!!?
                    removeTrustLine(contractorUUID);
                } else {

                    // todo: add comment that TL would be saved by the SaveTrustLineCallback
                    // it's not obviously at all.
                    trustLinePtr->setIncomingTrustAmount(0, boost::bind(&TrustLinesManager::saveTrustLine, this,
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
        throw ConflictError(
            "Сan not reject incoming trust line. Trust line to such contractor does not exist."); // todo: NotFoundError
    }
}

// todo: add comment about possible NotFoundError
TrustLine::Shared TrustLinesManager::getTrustLineByContractorUUID(
    const NodeUUID &contractorUUID) {

    if (isTrustLineExist(contractorUUID)) {
        return mTrustLines.at(contractorUUID);

    } else {
        // todo: exception must inform about method that throws the exception to be able to find it via log file
        throw ConflictError("Can't find trust line by such contractor UUID."); // todo: NotFoundError
    }
}


