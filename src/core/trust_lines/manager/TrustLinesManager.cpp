#include "TrustLinesManager.h"

TrustLinesManager::TrustLinesManager(
    TrustLineHandler *trustLineHandler,
    Logger *logger) :

    mTrustLineHandler(trustLineHandler),
    mlogger(logger){
    try {

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
 */
void TrustLinesManager::loadTrustLines() {

    for (auto const itTrustLine : mTrustLineHandler->trustLines()) {
        mTrustLines.insert(
            make_pair(
                itTrustLine->contractorNodeUUID(),
                itTrustLine));
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
//                    mlogger->logTustlineState(
//                            contractorUUID,
//                            "Both",
//                            "Close"
//                    );

                } else {
                    trustLine->setOutgoingTrustAmount(0);
                    trustLine->suspendOutgoingDirection();
//                    mlogger->logTustlineState(
//                            contractorUUID,
//                            "Outgoing",
//                            "Suspend"
//                    );
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
//                    mlogger->logTustlineState(
//                            contractorUUID,
//                            "Both",
//                            "Close"
//                    );
                } else {
                    trustLine->setIncomingTrustAmount(0);
                    trustLine->suspendIncomingDirection();
//                    mlogger->logTustlineState(
//                            contractorUUID,
//                            "Incoming",
//                            "Suspend"
//                    );
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
 * Reserves payment amount FROM this node TO the contractor.
 *
 * @param contractor - uuid of the contractor to which the trust line should be reserved.
 * @param transactionUUID - uuid of the transaction, which reserves the amount.
 * @param amount
 *
 *
 * @throws NotFoundError in case if no trust line against contractor UUID is present.
 * @throws ValueError in case, if trust line hasn't enought free amount;
 * @throws ValueError in case, if outgoing trust amount == 0;
 * @throws MemoryError;
 */
AmountReservation::ConstShared TrustLinesManager::reserveAmount(
    const NodeUUID &contractor,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount)
{
    const auto kTL = trustLineReadOnly(contractor);
    const auto kAvailableAmount = kTL->availableAmount();
    const auto kAlreadyReservedAmount =
        mAmountBlocksHandler->totalReserved(
            contractor, AmountReservation::Outgoing);

    if (*kAlreadyReservedAmount > *kAvailableAmount) {
        throw ValueError(
            "TrustLinesManager::reserveAmount: amount overflow prevented.");
    }

    if (*kAvailableAmount >= amount) {
        return mAmountBlocksHandler->reserve(
            contractor,
            transactionUUID,
            amount,
            AmountReservation::Outgoing);
    }
    throw ValueError(
        "TrustLinesManager::reserveAmount: "
        "there is no enough amount on the trust line.");
}

/*!
 * Reserves payment amount TO this node FROM the contractor.
 *
 * @param contractor - uuid of the contractor to which the trust line should be reserved.
 * @param transactionUUID - uuid of the transaction, which reserves the amount.
 * @param amount
 *
 *
 * @throws NotFoundError in case if no trust line against contractor UUID is present.
 * @throws ValueError in case, if trust line hasn't enought free amount;
 * @throws ValueError in case, if outgoing trust amount == 0;
 * @throws MemoryError;
 */
AmountReservation::ConstShared TrustLinesManager::reserveIncomingAmount(
    const NodeUUID& contractor,
    const TransactionUUID& transactionUUID,
    const TrustLineAmount& amount)
{
    const auto kTL = trustLineReadOnly(contractor);
    const auto kAvailableAmount = kTL->availableIncomingAmount();
    const auto kAlreadyReservedAmount =
        mAmountBlocksHandler->totalReserved(
            contractor, AmountReservation::Incoming);

    if (*kAlreadyReservedAmount > *kAvailableAmount) {
        throw ValueError(
            "TrustLinesManager::reserveAmount: amount overflow prevented.");
    }

    if (*kAvailableAmount >= amount) {
        return mAmountBlocksHandler->reserve(
            contractor,
            transactionUUID,
            amount,
            AmountReservation::Incoming);
    }
    throw ValueError(
        "TrustLinesManager::reserveAmount: "
        "there is no enough amount on the trust line.");
}

AmountReservation::ConstShared TrustLinesManager::updateAmountReservation(
    const NodeUUID &contractor,
    const AmountReservation::ConstShared reservation,
    const TrustLineAmount &newAmount) {

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(newAmount > TrustLineAmount(0));
#endif

    // Note: copy of shared ptr is required
    const auto kTL = mTrustLines[contractor];
    const auto kAvailableAmount = *(kTL->availableAmount());

    // Previous reservation would be removed (updated),
    // so it's amount must be added to the the available maount on the trust line.
    if (kAvailableAmount + reservation->amount() >= newAmount)
        return mAmountBlocksHandler->updateReservation(
            contractor,
            reservation,
            newAmount);

    throw ValueError("TrustLinesManager::reserveAmount: trust line has not enough amount.");
}

void TrustLinesManager::dropAmountReservation(
    const NodeUUID &contractor,
    const AmountReservation::ConstShared reservation) {

    mAmountBlocksHandler->free(
        contractor,
        reservation);
}

ConstSharedTrustLineAmount TrustLinesManager::availableOutgoingAmount(
    const NodeUUID& contractor)
{
    const auto kTL = trustLineReadOnly(contractor);
    const auto kAvailableAmount = kTL->availableAmount();

    const auto kAlreadyReservedAmount = mAmountBlocksHandler->totalReserved(
        contractor, AmountReservation::Outgoing);

    if (*kAlreadyReservedAmount >= *kAvailableAmount) {
        return make_shared<const TrustLineAmount>(0);
    }
    return make_shared<const TrustLineAmount>(
        *kAvailableAmount - *kAlreadyReservedAmount);
}

ConstSharedTrustLineAmount TrustLinesManager::availableIncomingAmount(
    const NodeUUID& contractor)
{
    const auto kTL = trustLineReadOnly(contractor);
    const auto kAvailableAmount = kTL->availableIncomingAmount();
    const auto kAlreadyReservedAmount = mAmountBlocksHandler->totalReserved(
        contractor, AmountReservation::Incoming);

    if (*kAlreadyReservedAmount >= *kAvailableAmount) {
        return make_shared<const TrustLineAmount>(0);
    }
    return make_shared<const TrustLineAmount>(
        *kAvailableAmount - *kAlreadyReservedAmount);
}

const bool TrustLinesManager::isTrustLineExist(
    const NodeUUID &contractorUUID) const {

    return mTrustLines.count(contractorUUID) > 0;
}

void TrustLinesManager::saveToDisk(
    TrustLine::Shared trustLine) {

    bool alreadyExisted = false;

    if (isTrustLineExist(trustLine->contractorNodeUUID())) {
        alreadyExisted = true;
    }
    mTrustLineHandler->saveTrustLine(trustLine);
    mTrustLineHandler->commit();
    try {
        mTrustLines.insert(
            make_pair(
                trustLine->contractorNodeUUID(),
                trustLine));

        } catch (std::bad_alloc&) {
            throw MemoryError("TrustLinesManager::saveToDisk: "
                                  "Can not reallocate STL container memory for new trust line instance.");
        }

    if (alreadyExisted) {
        trustLineStateModifiedSignal(
            trustLine->contractorNodeUUID(),
            trustLine->direction());

    } else {
        trustLineCreatedSignal(
            trustLine->contractorNodeUUID(),
            trustLine->direction());
    }
}

/**
 * throw IOError - unable to remove data from storage
 * throw NotFoundError - operation with not existing trust line
 */
void TrustLinesManager::removeTrustLine(
    const NodeUUID &contractorUUID) {

    if (isTrustLineExist(contractorUUID)) {
        mTrustLineHandler->deleteTrustLine(
            contractorUUID);
        mTrustLines.erase(contractorUUID);

        trustLineStateModifiedSignal(
            contractorUUID,
            TrustLineDirection::Nowhere);

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


vector<NodeUUID> TrustLinesManager::firstLevelNeighborsWithOutgoingFlow() const {
    vector<NodeUUID> result;
    for (auto const &nodeUUIDAndTrustLine : mTrustLines) {
        auto trustLineAmountShared = nodeUUIDAndTrustLine.second->availableAmount();
        auto trustLineAmountPtr = trustLineAmountShared.get();
        if (*trustLineAmountPtr > TrustLine::kZeroAmount()) {
            result.push_back(nodeUUIDAndTrustLine.first);
        }
    }
    return result;
}

vector<NodeUUID> TrustLinesManager::firstLevelNeighborsWithIncomingFlow() const {
    vector<NodeUUID> result;
    for (auto const &nodeUUIDAndTrustLine : mTrustLines) {
        auto trustLineAmountShared = nodeUUIDAndTrustLine.second->availableIncomingAmount();
        auto trustLineAmountPtr = trustLineAmountShared.get();

        if (*trustLineAmountPtr > TrustLine::kZeroAmount()) {
            result.push_back(nodeUUIDAndTrustLine.first);
        }
    }
    return result;
}

vector<pair<NodeUUID, ConstSharedTrustLineAmount>> TrustLinesManager::incomingFlows() const {
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> result;
    for (auto const &nodeUUIDAndTrustLine : mTrustLines) {
        auto trustLineAmountShared = nodeUUIDAndTrustLine.second->availableIncomingAmount();
        auto trustLineAmountPtr = trustLineAmountShared.get();
        if (*trustLineAmountPtr > TrustLine::kZeroAmount()) {
            result.push_back(
                make_pair(
                    nodeUUIDAndTrustLine.first,
                    make_shared<const TrustLineAmount>(
                        *trustLineAmountPtr)));
        }
    }
    return result;
}

vector<pair<NodeUUID, ConstSharedTrustLineAmount>> TrustLinesManager::outgoingFlows() const {
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> result;
    for (auto const &nodeUUIDAndTrustLine : mTrustLines) {
        auto trustLineAmountShared = nodeUUIDAndTrustLine.second->availableAmount();
        auto trustLineAmountPtr = trustLineAmountShared.get();
        if (*trustLineAmountPtr > TrustLine::kZeroAmount()) {
            result.push_back(
                make_pair(
                    nodeUUIDAndTrustLine.first,
                    make_shared<const TrustLineAmount>(
                        *trustLineAmountPtr)));
        }
    }
    return result;
}

vector<pair<const NodeUUID, const TrustLineDirection>> TrustLinesManager::rt1WithDirections() const {

    vector<pair<const NodeUUID, const TrustLineDirection >> result;
    result.reserve(mTrustLines.size());
    for (auto &nodeUUIDAndTrustLine : mTrustLines) {
        result.push_back(
            make_pair(
                nodeUUIDAndTrustLine.first,
                nodeUUIDAndTrustLine.second->direction()));
    }
    return result;
}

vector<NodeUUID> TrustLinesManager::rt1() const {

    vector<NodeUUID> result;
    result.reserve(mTrustLines.size());
    for (auto &nodeUUIDAndTrustLine : mTrustLines) {
        result.push_back(
            nodeUUIDAndTrustLine.first);
    }
    return result;
}

/**
 *
 * @throws NotFoundError - in case if no trust line with exact contractor.
 */
const TrustLine::ConstShared TrustLinesManager::trustLineReadOnly(
    const NodeUUID& contractorUUID) const
{
    if (isTrustLineExist(contractorUUID)) {
        // Since c++11, a return value is an rvalue.
        //
        // -> mTrustLines.at(contractorUUID)
        //
        // In this case, there will be no shared_ptr copy done due to RVO.
        // But thi copy is strongly needed here:
        // othervise, moved shared_ptr would try to free the memory,
        // that is also used by the shared_ptr in the map.
        // As a result - map corruption has place.
        const auto temp = const_pointer_cast<const TrustLine>(
            mTrustLines.at(contractorUUID));
        return temp;

    } else {
        throw NotFoundError(
            "TrustLinesManager::trustLineReadOnly: "
            "Trust line to such an contractor does not exists.");
    }
}

map<NodeUUID, TrustLine::Shared> &TrustLinesManager::trustLines() {

    return mTrustLines;
}

/**
 * @returns "true" if "node" is listed into the trustlines,
 * otherwise - returns "false".
 */
const bool TrustLinesManager::isNeighbor(
    const NodeUUID& node) const
{
    return mTrustLines.count(node) == 1;
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

vector<pair<NodeUUID, TrustLineBalance>> TrustLinesManager::getFirstLevelNodesForCycles(TrustLineBalance maxFlow) {
    vector<pair<NodeUUID, TrustLineBalance>> Nodes;
    TrustLineBalance zerobalance = 0;
    TrustLineBalance stepbalance;
    for (auto const& x : mTrustLines){
        stepbalance = x.second->balance();
        if (maxFlow == zerobalance) {
            if (stepbalance != zerobalance) {
                Nodes.push_back(make_pair(x.first, stepbalance));
                }
        } else if(maxFlow < zerobalance){
            if (stepbalance < zerobalance) {
                Nodes.push_back(make_pair(x.first, min(maxFlow, stepbalance)));
            }
        } else {
            if (stepbalance > zerobalance) {
                Nodes.push_back(make_pair(x.first, min(maxFlow, stepbalance)));
            }
        }
    }
    return Nodes;
}

void TrustLinesManager::useReservation(
    const NodeUUID &contractor,
    const AmountReservation::ConstShared reservation)
{
    if (mTrustLines.count(contractor) != 1)
        throw NotFoundError(
            "TrustLinesManager::useReservation: no trust line to the contractor.");

    if (reservation->direction() == AmountReservation::Outgoing)
        mTrustLines[contractor]->pay(reservation->amount());
    else if (reservation->direction() == AmountReservation::Incoming)
        mTrustLines[contractor]->acceptPayment(reservation->amount());
    else
        throw ValueError(
            "TrustLinesManager::useReservation: invalid trust line direction occurred.");
}

/**
 * @returns total summary of all outgoing possibilities of the node.
 */
ConstSharedTrustLineAmount TrustLinesManager::totalOutgoingAmount () const
    throw (bad_alloc)
{
    auto totalAmount = make_shared<TrustLineAmount>(0);
    for (const auto kTrustLine : mTrustLines) {
        const auto kTLAmount = kTrustLine.second->availableAmount();
        *totalAmount += *(kTLAmount);
    }

    return totalAmount;
}

/**
 * @returns total summary of all incoming possibilities of the node.
 */
ConstSharedTrustLineAmount TrustLinesManager::totalIncomingAmount () const
    throw (bad_alloc)
{
    auto totalAmount = make_shared<TrustLineAmount>(0);
    for (const auto kTrustLine : mTrustLines) {
        const auto kTLAmount = kTrustLine.second->availableIncomingAmount();
        *totalAmount += *(kTLAmount);
    }

    return totalAmount;
}
