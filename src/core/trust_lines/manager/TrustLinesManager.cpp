#include "TrustLinesManager.h"

TrustLinesManager::TrustLinesManager() {
    try {
        mTrustLinesStorage = new TrustLinesStorage(
            "trust_lines.dat");

    } catch (std::bad_alloc &e) {
        throw MemoryError("TrustLinesManager::TrustLinesManager. "
                              "Can not allocate memory for new trust line storage instance.");
    }

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

    vector<byte> trustLineData = trustLine->serializeTrustLine();

    if (isTrustLineExist(trustLine->contractorNodeUUID())) {
        try {
            mTrustLinesStorage->rewrite(
                storage::uuids::uuid(
                    trustLine->contractorNodeUUID()),
                trustLineData.data(),
                kRecordSize);

        } catch (std::exception &e) {
            throw IOError(e.what());
        }

    } else {
        try {
            mTrustLinesStorage->write(
                storage::uuids::uuid(
                    trustLine->contractorNodeUUID()),
                trustLineData.data(),
                kRecordSize);

        } catch (std::exception &e) {
            throw IOError(e.what());
        }
        mTrustLines.insert(make_pair(trustLine->contractorNodeUUID(), trustLine));
    }
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

const bool TrustLinesManager::checkDirection(
    const NodeUUID &contractorUUID,
    const TrustLineDirection direction) const {

    return mTrustLines.at(contractorUUID)->direction() == direction;
}

/**
 * throw IOError - can not read trust line data from file by key
 * throw Exception - unable to create instance of trust line
 */
void TrustLinesManager::loadTrustLines() {

    vector<NodeUUID> contractorsUUIDs = mTrustLinesStorage->getAllContractorsUUIDs();

    if (contractorsUUIDs.size() > 0) {

        for (auto const &item : contractorsUUIDs) {

            storage::Record::Shared record;
            try {
                 record = mTrustLinesStorage->readFromFile(storage::uuids::uuid(item));

            } catch(std::exception &e) {
                throw IOError(e.what());
            }

            try{
                TrustLine *trustLine = new TrustLine(
                    record->data(),
                    item);
                mTrustLines.insert(make_pair(item, TrustLine::Shared(trustLine)));

            } catch (...) {
                throw Exception("TrustLinesManager::loadTrustLine. "
                                    "Unable to create trust line instance from buffer.");
            }
        }
    }
}

/**
 * throw ConflictError - trust line is already exist
 * throw MemoryError - can not allocate memory for trust line instance
 */
void TrustLinesManager::open(
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount) {

    if (isTrustLineExist(contractorUUID)) {
        auto it = mTrustLines.find(contractorUUID);
        TrustLine::Shared trustLine = it->second;
        if (trustLine->outgoingTrustAmount() == 0) {
            trustLine->setOutgoingTrustAmount(
                amount,
                boost::bind(&TrustLinesManager::saveTrustLine, this, trustLine)); //trust line invokes TrustLinesManager's function saveTrustLine() as a callback

        } else {
            throw ConflictError(
                "TrustLinesManager::open. "
                    "Сan not open outgoing trust line. Outgoing trust line to such contractor already exist.");
        }

    } else {
        TrustLine *trustLine = nullptr;
        try{
            trustLine = new TrustLine(
                contractorUUID,
                0,
                amount,
                0);

        } catch (std::bad_alloc &e) {
            throw MemoryError("TrustLinesManager::open. "
                                  "Can not allocate memory for new trust line instance.");
        }
        saveTrustLine(TrustLine::Shared(trustLine));
    }
}

/**
 * throw PreconditionFailedError - contractor already used part of amount
 * throw ValueError - trust amount less or equals by zero
 * throw NotFoundError - trust line does not exist
 */
void TrustLinesManager::close(
    const NodeUUID &contractorUUID) {

    if (isTrustLineExist(contractorUUID)) {
        auto it = mTrustLines.find(contractorUUID);
        TrustLine::Shared trustLine = it->second;
        if (trustLine->outgoingTrustAmount() > TrustLineAmount(0)) {
            if (trustLine->balance() <= TrustLineBalance(0)) {
                if (trustLine->incomingTrustAmount() == TrustLineAmount(0)) {
                    removeTrustLine(contractorUUID);

                } else {
                    trustLine->setOutgoingTrustAmount(
                        0,
                        boost::bind(&TrustLinesManager::saveTrustLine, this, trustLine)); //trust line invokes TrustLinesManager's function saveTrustLine() as a callback
                }

            } else {
                throw PreconditionFailedError(
                    "TrustLinesManager::close. "
                        "Сan not close outgoing trust line. Contractor already used part of amount.");
            }

        } else {
            throw ValueError("TrustLinesManager::close. "
                                 "Сan not close outgoing trust line. Outgoing trust line amount less or equals to zero.");
        }

    } else {
        throw NotFoundError("TrustLinesManager::close. "
                                "Сan not close outgoing trust line. Trust line to such contractor does not exist.");
    }
}

/**
 * throw ConflictError - trust line is already exist
 * throw MemoryError - can not allocate memory for trust line instance
 */
void TrustLinesManager::accept(
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount) {

    if (isTrustLineExist(contractorUUID)) {
        auto it = mTrustLines.find(contractorUUID);
        TrustLine::Shared trustLine = it->second;
        if (trustLine->incomingTrustAmount() == 0) {
            trustLine->setIncomingTrustAmount(
                amount,
                boost::bind(&TrustLinesManager::saveTrustLine, this, trustLine)); //trust line invokes TrustLinesManager's function saveTrustLine() as a callback

        } else {
            throw ConflictError("TrustLinesManager::accept. "
                                    "Сan not accept incoming trust line. Incoming trust line to such contractor already exist.");
        }

    } else {
        TrustLine *trustLine = nullptr;
        try{
            trustLine = new TrustLine(
                contractorUUID,
                amount,
                0,
                0);

        } catch (std::bad_alloc &e) {
            throw MemoryError("TrustLinesManager::accept. "
                                  "Can not allocate memory for new trust line instance.");
        }
        saveTrustLine(TrustLine::Shared(trustLine));
    }
}

/**
 * throw PreconditionFailedError - user already used part of amount
 * throw ValueError - trust amount less or equals by zero
 * throw NotFoundError - trust line does not exist
 */
void TrustLinesManager::reject(
    const NodeUUID &contractorUUID) {

    if (isTrustLineExist(contractorUUID)) {
        auto it = mTrustLines.find(contractorUUID);
        TrustLine::Shared trustLine = it->second;
        if (trustLine->incomingTrustAmount() > TrustLineAmount(0)) {
            if (trustLine->balance() >= TrustLineBalance(0)) {
                if (trustLine->outgoingTrustAmount() == TrustLineAmount(0)) {
                    removeTrustLine(contractorUUID);

                } else {
                    trustLine->setIncomingTrustAmount(
                        0,
                        boost::bind(&TrustLinesManager::saveTrustLine, this, trustLine)); //trust line invokes TrustLinesManager's function saveTrustLine() as a callback
                }

            } else {
                throw PreconditionFailedError("TrustLinesManager::reject. "
                                                  "Сan not reject incoming trust line. User already used part of amount.");
            }

        } else {
            throw ValueError("TrustLinesManager::reject. "
                                 "Сan not reject incoming trust line. Incoming trust line amount less or equals to zero.");
        }

    } else {
        throw NotFoundError("TrustLinesManager::reject. "
                                "Сan not reject incoming trust line. Trust line to such contractor does not exist.");
    }
}

/**
 * throw NotFoundError - trust line by this contractor UUID not found
 */
TrustLine::Shared TrustLinesManager::trustLineByContractorUUID(
    const NodeUUID &contractorUUID) {

    if (isTrustLineExist(contractorUUID)) {
        return mTrustLines.at(contractorUUID);

    } else {
        throw NotFoundError("TrustLinesManager::trustLineByContractorUUID. "
                                "Can't find trust line by such contractor UUID.");
    }
}


