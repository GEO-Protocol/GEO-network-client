#include "TrustLinesManager.h"

TrustLinesManager::TrustLinesManager(Logger *logger) : mlogger(logger){
    try {
        mTrustLinesStorage = unique_ptr<TrustLinesStorage>(new TrustLinesStorage("trust_lines.dat"));

        mAmountBlocksHandler = unique_ptr<AmountReservationsHandler>(new AmountReservationsHandler());

    } catch (bad_alloc &e) {
        throw MemoryError(
            "TrustLinesManager::TrustLinesManager: "
                "Can not allocate memory for one of the trust lines manager's components.");
    }

    loadTrustLines();
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
                record = mTrustLinesStorage->readByUUID(storage::uuids::uuid(item));

            } catch(std::exception &e) {
                throw IOError(e.what());
            }

            try{
                TrustLine *trustLine = new TrustLine(
                    record->data(),
                    item
                );

                mTrustLines.insert(
                    make_pair(
                        item,
                        TrustLine::Shared(trustLine)
                    )
                );

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
        if (trustLine->outgoingTrustAmount() == TrustLine::kZeroAmount()) {
            trustLine->setOutgoingTrustAmount(amount);
            trustLine->activateOutgoingDirection();
            saveToDisk(trustLine);
        } else {
            throw ConflictError(
                "TrustLinesManager::open: "
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
            trustLine->activateOutgoingDirection();

        } catch (std::bad_alloc &e) {
            throw MemoryError("TrustLinesManager::open: "
                                  "Can not allocate memory for new trust line instance.");
        }
//        mlogger->logTruslineOperationStatus(trustLine->contractorNodeUUID(), amount, "open");
        saveToDisk(TrustLine::Shared(trustLine));
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
        if (trustLine->outgoingTrustAmount() > TrustLine::kZeroAmount()) {
            if (trustLine->balance() <= TrustLine::kZeroBalance()) {
                if (trustLine->incomingTrustAmount() == TrustLine::kZeroAmount()) {
                    removeTrustLine(contractorUUID);
                    mlogger->logTustlineState(
                            contractorUUID,
                            "Both",
                            "Close"
                    );

                } else {
                    trustLine->setOutgoingTrustAmount(0);
                    trustLine->suspendOutgoingDirection();
                    mlogger->logTustlineState(
                            contractorUUID,
                            "Outgoing",
                            "Suspend"
                    );
                    saveToDisk(trustLine);
                }

            } else {
                throw PreconditionFailedError(
                    "TrustLinesManager::close: "
                        "Сan not close outgoing trust line. Contractor already used part of amount.");
            }

        } else {
            throw ValueError("TrustLinesManager::close: "
                                 "Сan not close outgoing trust line. Outgoing trust line amount less or equals to zero.");
        }

    } else {
        throw NotFoundError("TrustLinesManager::close: "
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
        if (trustLine->incomingTrustAmount() == TrustLine::kZeroAmount()) {
            trustLine->setIncomingTrustAmount(amount);
            trustLine->activateIncomingDirection();
            saveToDisk(trustLine);
        } else {
            throw ConflictError("TrustLinesManager::accept: "
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
            trustLine->activateIncomingDirection();

        } catch (std::bad_alloc &e) {
            throw MemoryError("TrustLinesManager::accept: "
                                  "Can not allocate memory for new trust line instance.");
        }
        saveToDisk(TrustLine::Shared(trustLine));
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
        if (trustLine->incomingTrustAmount() > TrustLine::kZeroAmount()) {
            if (trustLine->balance() >= TrustLine::kZeroBalance()) {
                if (trustLine->outgoingTrustAmount() == TrustLine::kZeroAmount()) {
                    removeTrustLine(contractorUUID);
                    mlogger->logTustlineState(
                            contractorUUID,
                            "Both",
                            "Close"
                    );
                } else {
                    trustLine->setIncomingTrustAmount(0);
                    trustLine->suspendIncomingDirection();
                    mlogger->logTustlineState(
                            contractorUUID,
                            "Incoming",
                            "Suspend"
                    );
                    saveToDisk(trustLine);
                }

            } else {
                throw PreconditionFailedError("TrustLinesManager::reject: "
                                                  "Сan not reject incoming trust line. User already used part of amount.");
            }

        } else {
            throw ValueError("TrustLinesManager::reject: "
                                 "Сan not reject incoming trust line. Incoming trust line amount less or equals to zero.");
        }

    } else {
        throw NotFoundError("TrustLinesManager::reject: "
                                "Сan not reject incoming trust line. Trust line to such contractor does not exist.");
    }
}

const bool TrustLinesManager::checkDirection(
    const NodeUUID &contractorUUID,
    const TrustLineDirection direction) const {

    if (isTrustLineExist(contractorUUID)) {
        return mTrustLines.at(contractorUUID)->direction() == direction;
    }

    return false;
}

const BalanceRange TrustLinesManager::balanceRange(
    const NodeUUID &contractorUUID) const {

    return mTrustLines.at(contractorUUID)->balanceRange();
}

void TrustLinesManager::suspendDirection(
    const NodeUUID &contractorUUID,
    const TrustLineDirection direction) {

    switch(direction) {

        case TrustLineDirection::Incoming: {
            mTrustLines.at(contractorUUID)->suspendIncomingDirection();
        }

        case TrustLineDirection::Outgoing: {
            mTrustLines.at(contractorUUID)->suspendOutgoingDirection();
        }

        default: {
            throw ConflictError("TrustLinesManager::suspendDirection: "
                                    "Illegal trust line direction for suspending.");
        }
    }
}

void TrustLinesManager::setIncomingTrustAmount(
    const NodeUUID &contractor,
    const TrustLineAmount &amount) {

    auto iterator = mTrustLines.find(contractor);
    if (iterator == mTrustLines.end()){
        throw NotFoundError(
            "TrustLinesManager::setIncomingTrustAmount: "
                "No trust line found.");
    }

    auto trustLine = iterator->second;
    trustLine->setIncomingTrustAmount(amount);
    saveToDisk(trustLine);

}

void TrustLinesManager::setOutgoingTrustAmount(
    const NodeUUID &contractor,
    const TrustLineAmount &amount) {

    auto iterator = mTrustLines.find(contractor);
    if (iterator == mTrustLines.end()){
        throw NotFoundError("TrustLinesManager::setOutogingTrustAmount: "
                                "no trust line found.");
    }

    auto trustLine = iterator->second;
    trustLine->setOutgoingTrustAmount(amount);
    saveToDisk(trustLine);
}

const TrustLineAmount &TrustLinesManager::incomingTrustAmount(
    const NodeUUID &contractorUUID) {

    return mTrustLines.at(contractorUUID)->incomingTrustAmount();
}

const TrustLineAmount &TrustLinesManager::outgoingTrustAmount(
    const NodeUUID &contractorUUID) {

    return mTrustLines.at(contractorUUID)->outgoingTrustAmount();
}

const TrustLineBalance &TrustLinesManager::balance(
    const NodeUUID &contractorUUID) {

    return mTrustLines.at(contractorUUID)->balance();
}

/*!
 *
 *
 * @param contractor - uuid of the contractor to which the trust line should be reserved.
 * @param transactionUUID - uuid of the transaction, which reserves the amount.
 * @param amount
 *
 *
 * Throws ValueError in case, if trust line hasn't enought free amount;
 * Throws ValueError in case, if trust "amount" == 0;
 * Throws MemoryError;
 */
AmountReservation::ConstShared TrustLinesManager::reserveAmount(
    const NodeUUID &contractor,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount) {

    // todo: ensure reservations

    if (*mTrustLines.at(contractor)->availableAmount() >= amount) {
        return mAmountBlocksHandler->reserve(
            contractor,
            transactionUUID,
            amount
        );
    }
    throw ValueError("TrustLinesManager::reserveAmount: "
                         "Trust line has not enought amount.");
}

AmountReservation::ConstShared TrustLinesManager::updateAmountReservation(
    const NodeUUID &contractor,
    const AmountReservation::ConstShared reservation,
    const TrustLineAmount &newAmount) {

    // todo: ensure reservations

    if ((*mTrustLines.at(contractor)->availableAmount() - reservation->amount()) >= newAmount) {
        return mAmountBlocksHandler->updateReservation(
            contractor,
            reservation,
            newAmount
        );
    }
    throw ValueError("TrustLinesManager::reserveAmount: "
                         "Trust line has not enought amount.");
}

void TrustLinesManager::dropAmountReservation(
    const NodeUUID &contractor,
    const AmountReservation::ConstShared reservation) {

    mAmountBlocksHandler->free(
        contractor,
        reservation
    );
}

const bool TrustLinesManager::isTrustLineExist(
    const NodeUUID &contractorUUID) const {

    return mTrustLines.count(contractorUUID) > 0;
}

/**
 * throws IOError - unable to write or update data in storage
 */
void TrustLinesManager::saveToDisk(
    TrustLine::Shared trustLine) {

    vector<byte> trustLineData = trustLine->serialize();

    if (isTrustLineExist(trustLine->contractorNodeUUID())) {
        try {
            mTrustLinesStorage->rewrite(
                storage::uuids::uuid(trustLine->contractorNodeUUID()),
                trustLineData.data(),
                kRecordSize
            );

        } catch (std::exception &e) {
            throw IOError(e.what());
        }

    } else {
        try {
            mTrustLinesStorage->write(
                storage::uuids::uuid(trustLine->contractorNodeUUID()),
                trustLineData.data(),
                kRecordSize
            );

        } catch (std::exception &e) {
            throw IOError(e.what());
        }

        try {
            mTrustLines.insert(
                make_pair(
                    trustLine->contractorNodeUUID(),
                    trustLine
                )
            );

        } catch (std::bad_alloc&) {
            throw MemoryError("TrustLinesManager::saveToDisk: "
                                  "Can not reallocate STL container memory for new trust line instance.");
        }
    }
    mlogger->logTruslineOperationStatus(
            trustLine->contractorNodeUUID().stringUUID(),
            trustLine->incomingTrustAmount(),
            trustLine->outgoingTrustAmount(),
            trustLine->balance(),
            trustLine->direction()
    );
    trustLineCreatedSignal(
        trustLine->contractorNodeUUID(),
        trustLine->trustLineUUID()
    );
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

const TrustLine::Shared TrustLinesManager::trustLine(
    const NodeUUID &contractorUUID) const {

    if (isTrustLineExist(contractorUUID)) {
        return mTrustLines.at(contractorUUID);

    } else {
        throw NotFoundError(
            "TrustLinesManager::removeTrustLine. "
                "Trust line to such contractor does not exist.");
    }
}

map<NodeUUID, TrustLine::Shared> &TrustLinesManager::trustLines() {

    return mTrustLines;
}

vector<pair<NodeUUID, TrustLineBalance>> TrustLinesManager::getFirstLevelNodesForCycles() {
    vector<pair<NodeUUID, TrustLineBalance>> Nodes;
    for (auto const& x : mTrustLines){
        cout << "TrustLine" << endl;
        cout << x.first << endl;
        cout << x.second->balance() << endl;
        if (x.second->balance() != 0){
            cout << x.second->balance() << endl;
            Nodes.push_back(make_pair(x.first,  x.second->balance()));
        }
    }
    return Nodes;
}

void TrustLinesManager::setSomeBalances() {
//     this is debug method. have to be removed
    NodeUUID contractor1;
    NodeUUID contractor2;
    TrustLine *first_trustline = nullptr;
    TrustLine *second_trustline = nullptr;
    first_trustline = new TrustLine(
            contractor1,
            100,
            100,
            50
    );
    second_trustline = new TrustLine(
            contractor2,
            200,
            200,
            111
    );
    saveToDisk(TrustLine::Shared(first_trustline));
    saveToDisk(TrustLine::Shared(second_trustline));
}
