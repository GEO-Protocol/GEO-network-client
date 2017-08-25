#include "TrustLinesManager.h"


TrustLinesManager::TrustLinesManager(
    StorageHandler *storageHandler,
    Logger &logger)
    throw (bad_alloc, IOError) :

    mStorageHandler(storageHandler),
    mLogger(logger)
{
    mAmountReservationsHandler = make_unique<AmountReservationsHandler>();

    loadTrustLinesFromDisk();
}

void TrustLinesManager::loadTrustLinesFromDisk()
{
    auto ioTransaction = mStorageHandler->beginTransaction();
    const auto kTrustLines = ioTransaction->trustLinesHandler()->allTrustLines();

    mTrustLines.reserve(kTrustLines.size());

    for (auto const kTrustLine : kTrustLines) {
        if (kTrustLine->outgoingTrustAmount() == 0
                and kTrustLine->incomingTrustAmount() == 0
                and kTrustLine->balance() == 0) {

            // Empty trust line occured.
            // This might occure in case if trust line wasn't deleted properly when it was closed by both sides.
            // Now it must be removed.
            ioTransaction->trustLinesHandler()->deleteTrustLine(kTrustLine->contractorNodeUUID());
            info() << "Trust line to the node " << kTrustLine->contractorNodeUUID()
                   << " is empty (outgoing trust amount = 0, incoming trust amount = 0, balane = 0). Removed.";
            continue;

        } else {
            mTrustLines.insert(
                make_pair(
                    kTrustLine->contractorNodeUUID(),
                    kTrustLine));
        }
    }
}

const string TrustLinesManager::logHeader()
    noexcept
{
    return "[TrustLinesManager]";
}

LoggerStream TrustLinesManager::info() const
    noexcept
{
    return mLogger.info(logHeader());
}

TrustLinesManager::TrustLineOperationResult TrustLinesManager::setOutgoing(
    IOTransaction::Shared IOTransaction,
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount)
{
    if (not trustLineIsPresent(contractorUUID)) {
        // In case if TL to this contractor is absent,
        // "amount" can't be 0 (otherwise, trust line with both sides set to zero would be opened).
        if (amount == 0) {
            throw ValueError(
                "TrustLinesManager::setOutgoing: "
                "can't establish trust line with zero amount at both sides.");

        } else {
            // In case if trust line to this contractor is absent,
            // and "amount" is greater than 0 - new outgoing trust line should be created.
            auto trustLine = make_shared<TrustLine>(
                contractorUUID, 0, amount, 0);

            mTrustLines[contractorUUID] = trustLine;
            saveToDisk(IOTransaction, trustLine);
            return TrustLineOperationResult::Opened;
        }

    } else if (amount == 0 and incomingTrustAmount(contractorUUID) == 0) {
        // In case if trust line is already present,
        // but incoming trust amount is 0, and received "amount" is 0 -
        // then it is interpreted as the command to close the outgoing trust line.
        closeOutgoing(
            IOTransaction,
            contractorUUID);
        return TrustLineOperationResult::Closed;
    }

    auto trustLine = mTrustLines[contractorUUID];
    if (trustLine->outgoingTrustAmount() == amount) {
        // There is no reason to write the same data to the disk.
        return TrustLineOperationResult::NoChanges;
    }

    trustLine->setOutgoingTrustAmount(amount);
    saveToDisk(
        IOTransaction,
        trustLine);
    return TrustLineOperationResult::Updated;
}

TrustLinesManager::TrustLineOperationResult TrustLinesManager::setIncoming(
    IOTransaction::Shared IOTransaction,
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount)
{
    if (not trustLineIsPresent(contractorUUID)) {
        // In case if TL to this contractor is absent,
        // "amount" can't be 0 (otherwise, trust line with both sides set to zero would be opened).
        if (amount == 0) {
            throw ValueError(
                "TrustLinesManager::setIncoming: "
                "can't establish trust line with zero amount at both sides.");

        } else {
            // In case if TL to this contractor is absent,
            // and "amount" is greater than 0 - new incoming trust line should be created.
            auto trustLine = make_shared<TrustLine>(
                contractorUUID, amount, 0, 0);

            mTrustLines[contractorUUID] = trustLine;
            saveToDisk(IOTransaction, trustLine);
            return TrustLineOperationResult::Opened;
        }

    } else if (amount == 0 and outgoingTrustAmount(contractorUUID) == 0) {
        // In case if trust line is already present,
        // but outgoing trust amount is 0, and received "amount" is 0 -
        // then it is interpreted as the command to close the incoming trust line.
        closeIncoming(
            IOTransaction,
            contractorUUID);
        return TrustLineOperationResult::Closed;
    }

    auto trustLine = mTrustLines[contractorUUID];
    if (trustLine->incomingTrustAmount() == amount) {
        // There is no reason to write the same data to the disk.
        return TrustLineOperationResult::NoChanges;
    }

    trustLine->setIncomingTrustAmount(amount);
    saveToDisk(
        IOTransaction,
        trustLine);
    return TrustLineOperationResult::Updated;
}

void TrustLinesManager::closeOutgoing(
    IOTransaction::Shared IOTransaction,
    const NodeUUID &contractorUUID)
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            "TrustLinesManager::closeOutgoing: "
            "Can't close outgoing trust line. No trust line to this contractor is present.");
    }

    auto trustLine = mTrustLines[contractorUUID];
    trustLine->setOutgoingTrustAmount(0);

    // In case if incoming trust amount is also 0, trust line might be removed at all,
    // but only if no reservations are present.
    // Note: totalReservedOnTrustLine() returns pointer, and it's comparison with 0 is always false.
    if (trustLine->incomingTrustAmount() == 0
        and *mAmountReservationsHandler->totalReservedOnTrustLine(contractorUUID) == 0) {

        removeTrustLine(
            IOTransaction,
            contractorUUID);
        return;

    } else {
        // Trust line was modified, and wasn't removed.
        // Now it must be saved.
        saveToDisk(
            IOTransaction,
            trustLine);
    }
}

void TrustLinesManager::closeIncoming(
    IOTransaction::Shared IOTransaction,
    const NodeUUID &contractorUUID)
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            "TrustLinesManager::closeIncoming: "
            "Can't close incoming trust line. No trust line to this contractor is present.");
    }

    auto trustLine = mTrustLines[contractorUUID];
    trustLine->setIncomingTrustAmount(0);

    // In case if outgoing trust amount is also 0, trust line might be removed at all,
    // but only if no reservations are present.
    // Note: totalReservedOnTrustLine() returns pointer, and it's comparison with 0 is always false.
    if (trustLine->outgoingTrustAmount() == 0
        and *mAmountReservationsHandler->totalReservedOnTrustLine(contractorUUID) == 0) {
        removeTrustLine(
            IOTransaction,
            contractorUUID);
        return;

    } else {
        // Trust line was modified, and wasn't removed.
        // Now it must be saved.
        saveToDisk(
            IOTransaction,
            trustLine);
    }
}

const bool TrustLinesManager::checkDirection(
    const NodeUUID &contractorUUID,
    const TrustLineDirection direction) const {

    if (trustLineIsPresent(contractorUUID)) {
        return mTrustLines.at(contractorUUID)->direction() == direction;
    }

    return false;
}

const BalanceRange TrustLinesManager::balanceRange(
    const NodeUUID &contractorUUID) const {

    return mTrustLines.at(contractorUUID)->balanceRange();
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
AmountReservation::ConstShared TrustLinesManager::reserveOutgoingAmount(
    const NodeUUID &contractor,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount)
{
    const auto kAvailableAmount = availableOutgoingAmount(contractor);
    if (*kAvailableAmount >= amount) {
        return mAmountReservationsHandler->reserve(
            contractor,
            transactionUUID,
            amount,
            AmountReservation::Outgoing);
    }
    throw ValueError(
        "TrustLinesManager::reserveOutgoingAmount: "
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
    const auto kAvailableAmount = availableIncomingAmount(contractor);
    if (*kAvailableAmount >= amount) {
        return mAmountReservationsHandler->reserve(
            contractor,
            transactionUUID,
            amount,
            AmountReservation::Incoming);
    }
    throw ValueError(
        "TrustLinesManager::reserveOutgoingAmount: "
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
    const auto kAvailableAmount = *(availableOutgoingAmount(contractor));

    // Previous reservation would be removed (updated),
    // so it's amount must be added to the the available amount on the trust line.
    if (kAvailableAmount + reservation->amount() >= newAmount)
        return mAmountReservationsHandler->updateReservation(
            contractor,
            reservation,
            newAmount);

    throw ValueError("TrustLinesManager::reserveOutgoingAmount: trust line has not enough amount.");
}

void TrustLinesManager::dropAmountReservation(
    const NodeUUID &contractor,
    const AmountReservation::ConstShared reservation) {

    mAmountReservationsHandler->free(
        contractor,
        reservation);
}

ConstSharedTrustLineAmount TrustLinesManager::availableOutgoingAmount(
    const NodeUUID& contractor) const
{
    const auto kTL = trustLineReadOnly(contractor);
    const auto kAvailableAmount = kTL->availableOutgoingAmount();

    const auto kAlreadyReservedAmount = mAmountReservationsHandler->totalReserved(
        contractor, AmountReservation::Outgoing);

    if (*kAlreadyReservedAmount >= *kAvailableAmount) {
        return make_shared<const TrustLineAmount>(0);
    }
    return make_shared<const TrustLineAmount>(
        *kAvailableAmount - *kAlreadyReservedAmount);
}

ConstSharedTrustLineAmount TrustLinesManager::availableIncomingAmount(
    const NodeUUID& contractor) const
{
    const auto kTL = trustLineReadOnly(contractor);
    const auto kAvailableAmount = kTL->availableIncomingAmount();
    const auto kAlreadyReservedAmount = mAmountReservationsHandler->totalReserved(
        contractor, AmountReservation::Incoming);

    if (*kAlreadyReservedAmount >= *kAvailableAmount) {
        return make_shared<const TrustLineAmount>(0);
    }
    return make_shared<const TrustLineAmount>(
        *kAvailableAmount - *kAlreadyReservedAmount);
}

pair<ConstSharedTrustLineAmount, ConstSharedTrustLineAmount> TrustLinesManager::availableOutgoingCycleAmounts(
    const NodeUUID &contractor) const
{
    const auto kTL = trustLineReadOnly(contractor);
    const auto kBalance = kTL->balance();
    if (kBalance <= TrustLine::kZeroBalance()) {
        return make_pair(
            make_shared<const TrustLineAmount>(0),
            make_shared<const TrustLineAmount>(0));
    }

    const auto kAlreadyReservedAmount = mAmountReservationsHandler->totalReserved(
        contractor, AmountReservation::Outgoing);

    if (*kAlreadyReservedAmount.get() == TrustLine::kZeroAmount()) {
        return make_pair(
            make_shared<const TrustLineAmount>(kBalance),
            make_shared<const TrustLineAmount>(kBalance));
    }

    auto kAbsoluteBalance = absoluteBalanceAmount(kBalance);
    if (*kAlreadyReservedAmount.get() > kAbsoluteBalance) {
        return make_pair(
            make_shared<const TrustLineAmount>(0),
            make_shared<const TrustLineAmount>(kBalance));
    } else {
        return make_pair(
            make_shared<const TrustLineAmount>(
                kAbsoluteBalance - *kAlreadyReservedAmount.get()),
            make_shared<const TrustLineAmount>(kBalance));
    }
}

pair<ConstSharedTrustLineAmount, ConstSharedTrustLineAmount> TrustLinesManager::availableIncomingCycleAmounts(
    const NodeUUID &contractor) const
{
    const auto kTL = trustLineReadOnly(contractor);
    const auto kBalance = kTL->balance();
    if (kBalance >= TrustLine::kZeroBalance()) {
        return make_pair(
            make_shared<const TrustLineAmount>(0),
            make_shared<const TrustLineAmount>(0));
    }

    const auto kAlreadyReservedAmount = mAmountReservationsHandler->totalReserved(
        contractor, AmountReservation::Incoming);

    auto kAbsoluteBalance = absoluteBalanceAmount(kBalance);
    if (*kAlreadyReservedAmount.get() == TrustLine::kZeroAmount()) {
        return make_pair(
            make_shared<const TrustLineAmount>(kAbsoluteBalance),
            make_shared<const TrustLineAmount>(kAbsoluteBalance));
    }

    if (*kAlreadyReservedAmount.get() >= kAbsoluteBalance) {
        return make_pair(
            make_shared<const TrustLineAmount>(0),
            make_shared<const TrustLineAmount>(kAbsoluteBalance));
    }
    return make_pair(
        make_shared<const TrustLineAmount>(
            kAbsoluteBalance - *kAlreadyReservedAmount.get()),
        make_shared<const TrustLineAmount>(kAbsoluteBalance));
}

const bool TrustLinesManager::trustLineIsPresent (
    const NodeUUID &contractorUUID) const {

    return mTrustLines.count(contractorUUID) > 0;
}

const bool TrustLinesManager::reservationIsPresent(
    const NodeUUID &contractorUUID) const {
    return mAmountReservationsHandler->isReservationPresent(contractorUUID);
}

void TrustLinesManager::saveToDisk(
    IOTransaction::Shared IOTransaction,
    TrustLine::Shared trustLine) {

    bool alreadyExisted = false;
    if (trustLineIsPresent(trustLine->contractorNodeUUID())) {
        alreadyExisted = true;
    }

    IOTransaction->trustLinesHandler()->saveTrustLine(trustLine);
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

void TrustLinesManager::saveToDisk(TrustLine::Shared trustLine)
{
    bool alreadyExisted = false;
    if (trustLineIsPresent(trustLine->contractorNodeUUID())) {
        alreadyExisted = true;
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    ioTransaction->trustLinesHandler()->saveTrustLine(trustLine);
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
 * @throws IOError
 * @throws NotFoundError
 */
void TrustLinesManager::removeTrustLine(
    IOTransaction::Shared IOTransaction,
    const NodeUUID &contractorUUID)
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            "TrustLinesManager::removeTrustLine: "
            "There is no trust line to the contractor.");
    }

    IOTransaction->trustLinesHandler()->deleteTrustLine(contractorUUID);
    mTrustLines.erase(contractorUUID);
}

/**
 * throw IOError - unable to remove data from storage
 * throw NotFoundError - operation with not existing trust line
 */
void TrustLinesManager::removeTrustLine(
    const NodeUUID &contractorUUID) {

    if (trustLineIsPresent(contractorUUID)) {
        auto ioTransaction = mStorageHandler->beginTransaction();
        ioTransaction->trustLinesHandler()->deleteTrustLine(
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
    if (trustLineIsPresent(contractorUUID)) {
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
        auto trustLineAmountShared = availableOutgoingAmount(nodeUUIDAndTrustLine.first);
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
        auto trustLineAmountShared = availableIncomingAmount(nodeUUIDAndTrustLine.first);
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
        auto trustLineAmountShared = availableIncomingAmount(nodeUUIDAndTrustLine.first);
        auto trustLineAmountPtr = trustLineAmountShared.get();
            result.push_back(
                make_pair(
                    nodeUUIDAndTrustLine.first,
                    make_shared<const TrustLineAmount>(
                        *trustLineAmountPtr)));
    }
    return result;
}

vector<pair<NodeUUID, ConstSharedTrustLineAmount>> TrustLinesManager::outgoingFlows() const {
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> result;
    for (auto const &nodeUUIDAndTrustLine : mTrustLines) {
        auto trustLineAmountShared = availableOutgoingAmount(nodeUUIDAndTrustLine.first);
        auto trustLineAmountPtr = trustLineAmountShared.get();
            result.push_back(
                make_pair(
                    nodeUUIDAndTrustLine.first,
                    make_shared<const TrustLineAmount>(
                        *trustLineAmountPtr)));
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

ConstSharedTrustLineBalance TrustLinesManager::totalBalance() const
{
    TrustLineBalance result = TrustLine::kZeroBalance();
    for (const auto trustLine : mTrustLines) {
        result += trustLine.second->balance();
    }
    return make_shared<const TrustLineBalance>(result);
}

/**
 *
 * @throws NotFoundError - in case if no trust line with exact contractor.
 */
const TrustLine::ConstShared TrustLinesManager::trustLineReadOnly(
    const NodeUUID& contractorUUID) const
{
    if (trustLineIsPresent(contractorUUID)) {
        // Since c++11, a return value is an rvalue.
        //
        // -> mTrustLines.at(contractorUUID)
        //
        // In this case, there will be no shared_ptr copy done due to RVO.
        // But the copy is strongly needed here. Otherwise, moved shared_ptr would
        // try to free the memory, that is also used by the shared_ptr in the map.
        // As a result - map would be corrupted.
        const auto temp = const_pointer_cast<const TrustLine>(
            mTrustLines.at(contractorUUID));
        return temp;

    } else {
        throw NotFoundError(
            "TrustLinesManager::trustLineReadOnly: "
            "Trust line to such an contractor does not exists.");
    }
}

unordered_map<NodeUUID, TrustLine::Shared, boost::hash<boost::uuids::uuid>> &TrustLinesManager::trustLines()
{
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

vector<NodeUUID> TrustLinesManager::getFirstLevelNodesForCycles(TrustLineBalance maxFlow) {
    vector<NodeUUID> Nodes;
    TrustLineBalance zerobalance = 0;
    TrustLineBalance stepbalance;
    for (auto const& x : mTrustLines){
        stepbalance = x.second->balance();
        if (maxFlow == zerobalance) {
            if (stepbalance != zerobalance) {
                Nodes.push_back(x.first);
                }
        } else if(maxFlow < zerobalance){
            if (stepbalance < zerobalance) {
                Nodes.push_back(x.first);
            }
        } else {
            if (stepbalance > zerobalance) {
                Nodes.push_back(x.first);
            }
        }
    }
    return Nodes;
}

vector<NodeUUID> TrustLinesManager::firstLevelNeighborsWithPositiveBalance() const {
    vector<NodeUUID> Nodes;
    TrustLineBalance zerobalance = 0;
    TrustLineBalance stepbalance;
    for (auto const& x : mTrustLines){
        stepbalance = x.second->balance();
        if (stepbalance > zerobalance)
            Nodes.push_back(x.first);
        }
    return Nodes;
}

vector<NodeUUID> TrustLinesManager::firstLevelNeighborsWithNegativeBalance() const {
    // todo change vector to set
    vector<NodeUUID> Nodes;
    TrustLineBalance zerobalance = 0;
    TrustLineBalance stepbalance;
    for (const auto &x : mTrustLines){
        stepbalance = x.second->balance();
        if (stepbalance < zerobalance)
            Nodes.push_back(x.first);
    }
    return Nodes;
}

vector<NodeUUID> TrustLinesManager::firstLevelNeighborsWithNoneZeroBalance() const {
    vector<NodeUUID> Nodes;
    TrustLineBalance zerobalance = 0;
    TrustLineBalance stepbalance;
    for (auto const &x : mTrustLines) {
        stepbalance = x.second->balance();
        if (stepbalance != zerobalance)
            Nodes.push_back(x.first);
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
        const auto kTLAmount = availableOutgoingAmount(kTrustLine.first);
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
        const auto kTLAmount = availableIncomingAmount(kTrustLine.first);
        *totalAmount += *(kTLAmount);
    }

    return totalAmount;
}

vector<AmountReservation::ConstShared> TrustLinesManager::reservationsToContractor(
    const NodeUUID &contractorUUID) const
{
    return mAmountReservationsHandler->contractorReservations(
        contractorUUID,
        AmountReservation::ReservationDirection::Outgoing);
}

vector<AmountReservation::ConstShared> TrustLinesManager::reservationsFromContractor(
    const NodeUUID &contractorUUID) const
{
    return mAmountReservationsHandler->contractorReservations(
        contractorUUID,
        AmountReservation::ReservationDirection::Incoming);
}

void TrustLinesManager::printRTs()
{
    LoggerStream debug = mLogger.debug("TrustLinesManager::printRts");
    auto ioTransaction = mStorageHandler->beginTransaction();
    debug << "printRTs\tRT1 size: " << trustLines().size() << endl;
    for (const auto itTrustLine : trustLines()) {
        debug << "printRTs\t" << itTrustLine.second->contractorNodeUUID() << " "
               << (int)itTrustLine.second->incomingTrustAmount() << " "
               << (int)itTrustLine.second->outgoingTrustAmount() << " "
               << (int)itTrustLine.second->balance() << endl;
    }
    debug << "printRTs\tRT2 size: " << ioTransaction->routingTablesHandler()->rt2Records().size() << endl;
    for (auto const itRT2 : ioTransaction->routingTablesHandler()->rt2Records()) {
        debug << itRT2.first << " " << itRT2.second << endl;
    }
    debug << "printRTs\tRT3 size: " << ioTransaction->routingTablesHandler()->rt3Records().size() << endl;
    for (auto const itRT3 : ioTransaction->routingTablesHandler()->rt3Records()) {
        debug << itRT3.first << " " << itRT3.second << endl;
    }
    debug << "print payment incoming flows size: " << incomingFlows().size() << endl;
    for (auto const itIncomingFlow : incomingFlows()) {
        debug << itIncomingFlow.first << " " << *itIncomingFlow.second.get() << endl;
    }
    debug << "print payment outgoing flows size: " << outgoingFlows().size() << endl;
    for (auto const itOutgoingFlow : outgoingFlows()) {
        debug << itOutgoingFlow.first << " " << *itOutgoingFlow.second.get() << endl;
    }
    debug << "print cycle incoming flows size: " << incomingFlows().size() << endl;
    for (auto const trLine : mTrustLines) {
//        const auto availableIncomingCycleAmounts = availableIncomingCycleAmounts(trLine.first);
//        debug << trLine.first << " " << *(availableIncomingCycleAmounts.first)
//              << " " << *(availableIncomingCycleAmounts.second) << endl;
    }
    debug << "print cycle outgoing flows size: " << outgoingFlows().size() << endl;
    for (auto const trLine : mTrustLines) {
//        auto const availableOutgoingCycleAmounts = availableOutgoingCycleAmounts(trLine.first);
//        debug << trLine.first << " " << *(availableOutgoingCycleAmounts.first)
//              << " " << *(availableOutgoingCycleAmounts.second) << endl;
    }
}

pair<TrustLineBalance, TrustLineBalance> TrustLinesManager::debtAndCredit()
{
    TrustLineBalance debt = TrustLine::kZeroBalance();
    TrustLineBalance credit = TrustLine::kZeroBalance();
    for (auto const trLine : mTrustLines) {
        if (trLine.second->balance() > TrustLine::kZeroBalance()) {
            debt += trLine.second->balance();
        } else {
            credit += trLine.second->balance();
        }
    }
    return make_pair(debt, credit);
}

uint32_t TrustLinesManager::crc32SumAllFirstLevelNeighbors(
    const NodeUUID &firstLevelContractorUUID)
{
    // Calculate crc32 sum for all rt1 without node that equal firstLevelContractorUUID
    // crc32(sorted(NodeUUID1;NodeUUID2;...NodeUUIDn))
    boost::crc_32_type result;
    std::set<NodeUUID> firstLevelContractors;
    stringstream ss;

    for(const auto kNodeUUIDAndTrustLine: mTrustLines)
        if(kNodeUUIDAndTrustLine.first != firstLevelContractorUUID)
            firstLevelContractors.insert(kNodeUUIDAndTrustLine.first);

    copy(firstLevelContractors.begin(), firstLevelContractors.end(), ostream_iterator<NodeUUID>(ss, ";"));
    result.process_bytes(ss.str().data(), ss.str().length());
    return result.checksum();
}

uint32_t TrustLinesManager::crc32SumSecondLevelForNeighbor(const NodeUUID &firstLevelContractorUUID)
{
    // Calculate crc32 sum for all destination values from rt2 where sourse value equal firstLevelContractorUUID
    // crc32(NodeUUID1;NodeUUID2;...NodeUUIDn)
    boost::crc_32_type result;
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto secondLevelContractors = ioTransaction->routingTablesHandler()->neighborsOfOnRT2(firstLevelContractorUUID);

    stringstream ss;
    copy(secondLevelContractors.begin(), secondLevelContractors.end(), ostream_iterator<NodeUUID>(ss, ";"));
    result.process_bytes(ss.str().data(), ss.str().length());
    return result.checksum();
}

uint32_t TrustLinesManager::crc32SumFirstAndSecondLevel(const NodeUUID &requestNodeUUID)
{
    // Calculate crc32 sum for rt1 and rt2 without nodes that are inerhits for requestNodeUUID
    boost::crc_32_type result;
    stringstream firstLevelCRC32Sum;
    auto ioTransaction = mStorageHandler->beginTransaction();
    // Get all firstLevel neighbors
    auto firstLevelContractors = rt1();
    // To prevent different CRC32 sum on equal vector of NodeUUID - sort it
    sort(firstLevelContractors.begin(), firstLevelContractors.end());
    for(const auto kNodeUUIDFirstLevel: firstLevelContractors){
        if(kNodeUUIDFirstLevel == requestNodeUUID)
            // Initiator Node will not calculate itself to calculate second level crc32
            continue;
        boost::crc_32_type stepCRC32;
        stringstream secondLevelCRC32Sum;
        auto stepSecondLevelContractors = ioTransaction->routingTablesHandler()->neighborsOfOnRT2(kNodeUUIDFirstLevel);
        copy(
            stepSecondLevelContractors.begin(),
            stepSecondLevelContractors.end(),
            ostream_iterator<NodeUUID>(secondLevelCRC32Sum, ";"));
        secondLevelCRC32Sum << kNodeUUIDFirstLevel;
        stepCRC32.process_bytes(secondLevelCRC32Sum.str().data(), secondLevelCRC32Sum.str().length());
        firstLevelCRC32Sum << stepCRC32.checksum();
    }
    result.process_bytes(firstLevelCRC32Sum.str().data(), firstLevelCRC32Sum.str().length());
    return result.checksum();
}

uint32_t TrustLinesManager::crc32SumSecondAndThirdLevelForNeighbor(const NodeUUID &firstLevelContractorUUID)
{
    boost::crc_32_type result;
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto secondLevelContractors = ioTransaction->routingTablesHandler()->neighborsOfOnRT2(firstLevelContractorUUID);
    stringstream secondLevelCRC32Sum;
    for(const auto kNodeUUIDSecondLevel: secondLevelContractors){
        boost::crc_32_type stepCRC32;
        stringstream thirdLevelCRC32Sum;
        auto stepThirdLevelContractors = ioTransaction->routingTablesHandler()->neighborsOfOnRT3(kNodeUUIDSecondLevel);
        copy(stepThirdLevelContractors.begin(), stepThirdLevelContractors.end(), ostream_iterator<NodeUUID>(thirdLevelCRC32Sum, ";"));
        thirdLevelCRC32Sum << kNodeUUIDSecondLevel;
        stepCRC32.process_bytes(thirdLevelCRC32Sum.str().data(), thirdLevelCRC32Sum.str().length());
        secondLevelCRC32Sum << stepCRC32.checksum();
    }
    result.process_bytes(secondLevelCRC32Sum.str().data(), secondLevelCRC32Sum.str().length());
    return result.checksum();
}

uint32_t TrustLinesManager::crc32SumThirdLevelForNeighbor(
        const NodeUUID &secondLevelContractorUUID,
        const NodeUUID &firstLevelNodeUUID)
{
    boost::crc_32_type result;
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto FourthLevelContractors = ioTransaction->routingTablesHandler()->neighborsOfOnRT3(secondLevelContractorUUID);
    FourthLevelContractors.insert(firstLevelNodeUUID);
    stringstream ss;
    copy(FourthLevelContractors.begin(), FourthLevelContractors.end(), ostream_iterator<NodeUUID>(ss, ";"));
    result.process_bytes(ss.str().data(), ss.str().length());
    return result.checksum();
}
