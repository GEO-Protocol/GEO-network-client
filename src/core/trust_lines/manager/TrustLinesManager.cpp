#include "TrustLinesManager.h"


TrustLinesManager::TrustLinesManager(
    StorageHandler *storageHandler,
    Logger &logger):

    mStorageHandler(storageHandler),
    mLogger(logger),
    mAmountReservationsHandler(
        make_unique<AmountReservationsHandler>())
{
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
    if (outgoingTrustAmountDespiteReservations(contractorUUID) == 0) {
        // In case if outgoing TL to this contractor is absent,
        // "amount" can't be 0 (otherwise, trust line set to zero would be opened).
        if (amount == 0) {
            throw ValueError(
                "TrustLinesManager::setOutgoing: "
                "can't establish trust line with zero amount.");

        } else {
            if (not trustLineIsPresent(contractorUUID)) {
                // In case if trust line to this contractor is absent,
                // and "amount" is greater than 0 - new trust line should be created.
                auto trustLine = make_shared<TrustLine>(
                    contractorUUID, 0, amount, 0);
                mTrustLines[contractorUUID] = trustLine;
                saveToDisk(IOTransaction, trustLine);
            } else {
                // In case if trust line to this contractor is present,
                // and "amount" is greater than 0 - outgoing trust line should be created.
                auto trustLine = mTrustLines[contractorUUID];
                trustLine->setOutgoingTrustAmount(amount);
                saveToDisk(
                    IOTransaction,
                    trustLine);
            }
            return TrustLineOperationResult::Opened;
        }

    } else if (amount == 0) {
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
    if (incomingTrustAmountDespiteResevations(contractorUUID) == 0) {
        // In case if incoming TL amount to this contractor is absent,
        // "amount" can't be 0 (otherwise, trust line with both sides set to zero would be opened).
        if (amount == 0) {
            throw ValueError(
                "TrustLinesManager::setIncoming: "
                "can't establish trust line with zero amount at both sides.");

        } else {
            if (not trustLineIsPresent(contractorUUID)) {
                // In case if TL to this contractor is absent,
                // and "amount" is greater than 0 - new trust line should be created.
                auto trustLine = make_shared<TrustLine>(
                    contractorUUID, amount, 0, 0);
                mTrustLines[contractorUUID] = trustLine;
                saveToDisk(IOTransaction, trustLine);
            } else {
                // In case if TL to this contractor is present,
                // and "amount" is greater than 0 - incoming trust line should be created.
                auto trustLine = mTrustLines[contractorUUID];
                trustLine->setIncomingTrustAmount(amount);
                saveToDisk(
                    IOTransaction,
                    trustLine);
            }
            return TrustLineOperationResult::Opened;
        }

    } else if (amount == 0) {
        // In case if incoming trust line is already present,
        // and received "amount" is 0 -
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
            and trustLine->balance() == 0
            and not reservationIsPresent(contractorUUID)) {

        removeTrustLine(
            IOTransaction,
            contractorUUID);

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
            and trustLine->balance() == 0
            and not reservationIsPresent(contractorUUID)) {

        removeTrustLine(
            IOTransaction,
            contractorUUID);

    } else {
        // Trust line was modified, and wasn't removed.
        // Now it must be saved.
        saveToDisk(
            IOTransaction,
            trustLine);
    }
}

//const bool TrustLinesManager::checkDirection(
//    const NodeUUID &contractorUUID,
//    const TrustLineDirection direction) const {

//    if (trustLineIsPresent(contractorUUID)) {
//        return mTrustLines.at(contractorUUID)->direction() == direction;
//    }

//    return false;
//}

//const BalanceRange TrustLinesManager::balanceRange(
//    const NodeUUID &contractorUUID) const {

//    return mTrustLines.at(contractorUUID)->balanceRange();
//}

const TrustLineAmount &TrustLinesManager::incomingTrustAmountDespiteResevations(
    const NodeUUID &contractorUUID) const
{
    if (not trustLineIsPresent(contractorUUID)) {
        return TrustLine::kZeroAmount();
    }

    return mTrustLines.at(contractorUUID)->incomingTrustAmount();
}

const TrustLineAmount &TrustLinesManager::outgoingTrustAmountDespiteReservations(
    const NodeUUID &contractorUUID) const
{
    if (not trustLineIsPresent(contractorUUID)) {
        return TrustLine::kZeroAmount();
    }

    return mTrustLines.at(contractorUUID)->outgoingTrustAmount();
}

const TrustLineBalance &TrustLinesManager::balance(
    const NodeUUID &contractorUUID) const
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            "TrustLinesManager::outgoingTrustAmountDespiteResevations: "
            "There is no trust line to this contractor.");
    }

    return mTrustLines.at(contractorUUID)->balance();
}

AmountReservation::ConstShared TrustLinesManager::reserveOutgoingAmount(
    const NodeUUID &contractor,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount)
{
    const auto kAvailableAmount = outgoingTrustAmountConsideringReservations(contractor);
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

AmountReservation::ConstShared TrustLinesManager::reserveIncomingAmount(
    const NodeUUID& contractor,
    const TransactionUUID& transactionUUID,
    const TrustLineAmount& amount)
{
    const auto kAvailableAmount = incomingTrustAmountConsideringReservations(contractor);
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
    const auto kAvailableAmount = *(outgoingTrustAmountConsideringReservations(contractor));

    // Previous reservation would be removed (updated),
    // so it's amount must be added to the the available amount on the trust line.
    if (kAvailableAmount + reservation->amount() >= newAmount)
        return mAmountReservationsHandler->updateReservation(
            contractor,
            reservation,
            newAmount);

    throw ValueError(
        "TrustLinesManager::reserveOutgoingAmount: "
        "Trust line has not enought free amount.");
}

void TrustLinesManager::dropAmountReservation(
    const NodeUUID &contractor,
    const AmountReservation::ConstShared reservation)
{
    mAmountReservationsHandler->free(
        contractor,
        reservation);
}

ConstSharedTrustLineAmount TrustLinesManager::outgoingTrustAmountConsideringReservations(
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

ConstSharedTrustLineAmount TrustLinesManager::incomingTrustAmountConsideringReservations(
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
 * @throws IOError
 * @throws NotFoundError
 */
bool TrustLinesManager::isTrustLineEmpty(
    const NodeUUID &contractorUUID)
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            "TrustLinesManager::isTrustLineEmpty: "
                    "There is no trust line to the contractor.");
    }

    auto outgoingAmountShared = outgoingTrustAmountConsideringReservations(contractorUUID);
    auto incomingAmountShared = incomingTrustAmountConsideringReservations(contractorUUID);
    return (*outgoingAmountShared.get() == 0
        and *incomingAmountShared.get() == 0
        and balance(contractorUUID) == 0);
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
        auto trustLineAmountShared = outgoingTrustAmountConsideringReservations(nodeUUIDAndTrustLine.first);
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
        auto trustLineAmountShared = incomingTrustAmountConsideringReservations(nodeUUIDAndTrustLine.first);
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
        auto trustLineAmountShared = incomingTrustAmountConsideringReservations(nodeUUIDAndTrustLine.first);
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
        auto trustLineAmountShared = outgoingTrustAmountConsideringReservations(nodeUUIDAndTrustLine.first);
        auto trustLineAmountPtr = trustLineAmountShared.get();
            result.push_back(
                make_pair(
                    nodeUUIDAndTrustLine.first,
                    make_shared<const TrustLineAmount>(
                        *trustLineAmountPtr)));
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
    if (not trustLineIsPresent(contractor)) {
        throw NotFoundError(
            "TrustLinesManager::useReservation: "
            "No trust line with the contractor is present.");
    }

    switch (reservation->direction()) {
    case AmountReservation::Outgoing: {
        mTrustLines[contractor]->pay(reservation->amount());
        return;
    }

    case AmountReservation::Incoming: {
        mTrustLines[contractor]->acceptPayment(reservation->amount());
        return;
    }

    default: {
        throw ValueError(
            "TrustLinesManager::useReservation: "
            "Unexpected trust line direction occurred.");
    }
    }
}

ConstSharedTrustLineAmount TrustLinesManager::totalOutgoingAmount () const
{
    auto totalAmount = make_shared<TrustLineAmount>(0);
    for (const auto kTrustLine : mTrustLines) {
        const auto kTLAmount = outgoingTrustAmountConsideringReservations(kTrustLine.first);
        *totalAmount += *(kTLAmount);
    }

    return totalAmount;
}

ConstSharedTrustLineAmount TrustLinesManager::totalIncomingAmount () const
{
    auto totalAmount = make_shared<TrustLineAmount>(0);
    for (const auto kTrustLine : mTrustLines) {
        const auto kTLAmount = incomingTrustAmountConsideringReservations(kTrustLine.first);
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
               << itTrustLine.second->incomingTrustAmount() << " "
               << itTrustLine.second->outgoingTrustAmount() << " "
               << itTrustLine.second->balance() << endl;
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
        auto const availableIncomingCycleAmounts = this->availableIncomingCycleAmounts(trLine.first);
        debug << trLine.first << " " << *(availableIncomingCycleAmounts.first)
              << " " << *(availableIncomingCycleAmounts.second) << endl;
    }
    debug << "print cycle outgoing flows size: " << outgoingFlows().size() << endl;
    for (auto const trLine : mTrustLines) {
        auto const availableOutgoingCycleAmounts = this->availableOutgoingCycleAmounts(trLine.first);
        debug << trLine.first << " " << *(availableOutgoingCycleAmounts.first)
              << " " << *(availableOutgoingCycleAmounts.second) << endl;
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
