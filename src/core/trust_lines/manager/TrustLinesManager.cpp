#include "TrustLinesManager.h"


TrustLinesManager::TrustLinesManager(
    const SerializedEquivalent equivalent,
    StorageHandler *storageHandler,
    Logger &logger):

    mEquivalent(equivalent),
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
    const auto kTrustLines = ioTransaction->trustLinesHandler()->allTrustLinesByEquivalent(mEquivalent);

    mTrustLines.reserve(kTrustLines.size());

    for (auto const &kTrustLine : kTrustLines) {
        if (kTrustLine->outgoingTrustAmount() == 0
                and kTrustLine->incomingTrustAmount() == 0
                and kTrustLine->balance() == 0) {

            // Empty trust line occurred.
            // This might occurre in case if trust line wasn't deleted properly when it was closed by both sides.
            // Now it must be removed.
            ioTransaction->trustLinesHandler()->deleteTrustLine(
                kTrustLine->contractorNodeUUID(),
                mEquivalent);
            info() << "Trust line to the node " << kTrustLine->contractorNodeUUID()
                   << " is empty (outgoing trust amount = 0, incoming trust amount = 0, balance = 0). Removed.";
            continue;

        } else {
            mTrustLines.insert(
                make_pair(
                    kTrustLine->contractorNodeUUID(),
                    kTrustLine));
        }
    }
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
                logHeader() + "::setOutgoing: "
                "can't establish trust line with zero amount.");

        } else {
            if (not trustLineIsPresent(contractorUUID)) {
                // In case if trust line to this contractor is absent,
                // and "amount" is greater than 0 - new trust line should be created.
                // todo : contractor is not gateway by default
                auto trustLine = make_shared<TrustLine>(
                    contractorUUID, 0, amount, 0, false);
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
                logHeader() + "::setIncoming: "
                "can't establish trust line with zero amount at both sides.");

        } else {
            if (not trustLineIsPresent(contractorUUID)) {
                // In case if TL to this contractor is absent,
                // and "amount" is greater than 0 - new trust line should be created.
                // todo : contractor is not gateway by default
                auto trustLine = make_shared<TrustLine>(
                    contractorUUID, amount, 0, 0, false);
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
            logHeader() + "::closeOutgoing: "
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
            logHeader() + "::closeIncoming: "
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

void TrustLinesManager::setContractorAsGateway(
    IOTransaction::Shared IOTransaction,
    const NodeUUID &contractorUUID,
    bool contractorIsGateway)
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            logHeader() + "::setContractorAsGateway: "
            "Can't set contractor as gateway. No trust line to this contractor is present.");
    }

    auto trustLine = mTrustLines[contractorUUID];
    trustLine->setContractorAsGateway(contractorIsGateway);
    saveToDisk(
        IOTransaction,
        trustLine);
}

const bool TrustLinesManager::isContractorGateway(
        const NodeUUID &contractorUUID) const
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            logHeader() + "::isContractorGateway: "
                "There is no trust line to this contractor.");
    }

    return mTrustLines.at(contractorUUID)->isContractorGateway();
}

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
            logHeader() + "::outgoingTrustAmountDespiteResevations: "
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
        logHeader() + "::reserveOutgoingAmount: "
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
        logHeader() + "::reserveOutgoingAmount: "
        "there is no enough amount on the trust line.");
}

AmountReservation::ConstShared TrustLinesManager::updateAmountReservation(
    const NodeUUID &contractor,
    const AmountReservation::ConstShared reservation,
    const TrustLineAmount &newAmount)
{
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
        logHeader() + "::reserveOutgoingAmount: "
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
    TrustLine::Shared trustLine)
{
    IOTransaction->trustLinesHandler()->saveTrustLine(
        trustLine,
        mEquivalent);
    try {
        mTrustLines.insert(
            make_pair(
                trustLine->contractorNodeUUID(),
                trustLine));

        } catch (std::bad_alloc&) {
            throw MemoryError(
                logHeader() + "::saveToDisk: "
                "Can not reallocate STL container memory for new trust line instance.");
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
            logHeader() + "::removeTrustLine: "
            "There is no trust line to the contractor.");
    }

    IOTransaction->trustLinesHandler()->deleteTrustLine(
        contractorUUID,
        mEquivalent);
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
            logHeader() + "::isTrustLineEmpty: "
            "There is no trust line to the contractor.");
    }

    auto outgoingAmountShared = outgoingTrustAmountConsideringReservations(contractorUUID);
    auto incomingAmountShared = incomingTrustAmountConsideringReservations(contractorUUID);
    return (*outgoingAmountShared.get() == 0
        and *incomingAmountShared.get() == 0
        and balance(contractorUUID) == 0);
}

vector<NodeUUID> TrustLinesManager::firstLevelNeighborsWithOutgoingFlow() const
{
    vector<NodeUUID> result;
    for (auto const &nodeUUIDAndTrustLine : mTrustLines) {
        auto trustLineAmountShared = outgoingTrustAmountConsideringReservations(
            nodeUUIDAndTrustLine.first);
        auto trustLineAmountPtr = trustLineAmountShared.get();
        if (*trustLineAmountPtr > TrustLine::kZeroAmount()) {
            result.push_back(
                nodeUUIDAndTrustLine.first);
        }
    }
    return result;
}

vector<NodeUUID> TrustLinesManager::firstLevelGatewayNeighborsWithOutgoingFlow() const
{
    vector<NodeUUID> result;
    for (auto const &nodeUUIDAndTrustLine : mTrustLines) {
        if (!nodeUUIDAndTrustLine.second->isContractorGateway()) {
            continue;
        }
        auto trustLineAmountShared = outgoingTrustAmountConsideringReservations(
            nodeUUIDAndTrustLine.first);
        auto trustLineAmountPtr = trustLineAmountShared.get();
        if (*trustLineAmountPtr > TrustLine::kZeroAmount()) {
            result.push_back(
                nodeUUIDAndTrustLine.first);
        }
    }
    return result;
}

vector<NodeUUID> TrustLinesManager::firstLevelNeighborsWithIncomingFlow() const
{
    vector<NodeUUID> result;
    for (auto const &nodeUUIDAndTrustLine : mTrustLines) {
        auto trustLineAmountShared = incomingTrustAmountConsideringReservations(
            nodeUUIDAndTrustLine.first);
        auto trustLineAmountPtr = trustLineAmountShared.get();

        if (*trustLineAmountPtr > TrustLine::kZeroAmount()) {
            result.push_back(
                nodeUUIDAndTrustLine.first);
        }
    }
    return result;
}

vector<NodeUUID> TrustLinesManager::firstLevelGatewayNeighborsWithIncomingFlow() const
{
    vector<NodeUUID> result;
    for (auto const &nodeUUIDAndTrustLine : mTrustLines) {
        if (!nodeUUIDAndTrustLine.second->isContractorGateway()) {
            continue;
        }
        auto trustLineAmountShared = incomingTrustAmountConsideringReservations(
                nodeUUIDAndTrustLine.first);
        auto trustLineAmountPtr = trustLineAmountShared.get();

        if (*trustLineAmountPtr > TrustLine::kZeroAmount()) {
            result.push_back(nodeUUIDAndTrustLine.first);
        }
    }
    return result;
}

vector<NodeUUID> TrustLinesManager::firstLevelNonGatewayNeighborsWithIncomingFlow() const
{
    vector<NodeUUID> result;
    for (auto const &nodeUUIDAndTrustLine : mTrustLines) {
        if (nodeUUIDAndTrustLine.second->isContractorGateway()) {
            continue;
        }
        auto trustLineAmountShared = incomingTrustAmountConsideringReservations(
            nodeUUIDAndTrustLine.first);
        auto trustLineAmountPtr = trustLineAmountShared.get();

        if (*trustLineAmountPtr > TrustLine::kZeroAmount()) {
            result.push_back(nodeUUIDAndTrustLine.first);
        }
    }
    return result;
}

vector<pair<NodeUUID, ConstSharedTrustLineAmount>> TrustLinesManager::incomingFlows() const
{
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> result;
    for (auto const &nodeUUIDAndTrustLine : mTrustLines) {
        auto trustLineAmountShared = incomingTrustAmountConsideringReservations(
            nodeUUIDAndTrustLine.first);
        result.push_back(
            make_pair(
                nodeUUIDAndTrustLine.first,
                trustLineAmountShared));
    }
    return result;
}

vector<pair<NodeUUID, ConstSharedTrustLineAmount>> TrustLinesManager::outgoingFlows() const
{
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> result;
    for (auto const &nodeUUIDAndTrustLine : mTrustLines) {
        auto trustLineAmountShared = outgoingTrustAmountConsideringReservations(
            nodeUUIDAndTrustLine.first);
        result.push_back(
            make_pair(
                nodeUUIDAndTrustLine.first,
                trustLineAmountShared));
    }
    return result;
}

pair<NodeUUID, ConstSharedTrustLineAmount> TrustLinesManager::incomingFlow(
    const NodeUUID &contractorUUID) const
{
    return make_pair(
        contractorUUID,
        incomingTrustAmountConsideringReservations(
            contractorUUID));
}

pair<NodeUUID, ConstSharedTrustLineAmount> TrustLinesManager::outgoingFlow(
    const NodeUUID &contractorUUID) const
{
    return make_pair(
        contractorUUID,
        outgoingTrustAmountConsideringReservations(
            contractorUUID));
}

vector<pair<NodeUUID, ConstSharedTrustLineAmount>> TrustLinesManager::incomingFlowsFromNonGateways() const
{
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> result;
    for (auto const &nodeUUIDAndTrustLine : mTrustLines) {
        if (nodeUUIDAndTrustLine.second->isContractorGateway()) {
            continue;
        }
        auto trustLineAmountShared = incomingTrustAmountConsideringReservations(
            nodeUUIDAndTrustLine.first);
        result.push_back(
            make_pair(
                nodeUUIDAndTrustLine.first,
                trustLineAmountShared));
    }
    return result;
}

vector<pair<NodeUUID, ConstSharedTrustLineAmount>> TrustLinesManager::incomingFlowsFromGateways() const
{
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> result;
    for (auto const &nodeUUIDAndTrustLine : mTrustLines) {
        if (!nodeUUIDAndTrustLine.second->isContractorGateway()) {
            continue;
        }
        auto trustLineAmountShared = incomingTrustAmountConsideringReservations(
            nodeUUIDAndTrustLine.first);
        result.push_back(
            make_pair(
                nodeUUIDAndTrustLine.first,
                trustLineAmountShared));
    }
    return result;
}

vector<pair<NodeUUID, ConstSharedTrustLineAmount>> TrustLinesManager::outgoingFlowsToGateways() const
{
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> result;
    for (auto const &nodeUUIDAndTrustLine : mTrustLines) {
        if (!nodeUUIDAndTrustLine.second->isContractorGateway()) {
            continue;
        }
        auto trustLineAmountShared = outgoingTrustAmountConsideringReservations(
            nodeUUIDAndTrustLine.first);
        result.push_back(
            make_pair(
                nodeUUIDAndTrustLine.first,
                trustLineAmountShared));
    }
    return result;
}

vector<NodeUUID> TrustLinesManager::gateways() const
{
    vector<NodeUUID> result;
    for (auto const &nodeUUIDAndTrustLine : mTrustLines) {
        if (nodeUUIDAndTrustLine.second->isContractorGateway()) {
            result.push_back(nodeUUIDAndTrustLine.first);
        }
    }
    return result;
}

vector<NodeUUID> TrustLinesManager::firstLevelNeighbors() const
{
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
    for (const auto &trustLine : mTrustLines) {
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
            logHeader() + "::trustLineReadOnly: " + contractorUUID.stringUUID() +
                    " Trust line to such an contractor does not exists.");
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

vector<NodeUUID> TrustLinesManager::getFirstLevelNodesForCycles(
    TrustLineBalance maxFlow)
{
    vector<NodeUUID> nodes;
    TrustLineBalance stepBalance;
    for (auto const& nodeAndTrustLine : mTrustLines) {
        stepBalance = nodeAndTrustLine.second->balance();
        if (maxFlow == TrustLine::kZeroBalance()) {
            if (stepBalance != TrustLine::kZeroBalance()) {
                nodes.push_back(nodeAndTrustLine.first);
            }
        } else if(maxFlow < TrustLine::kZeroBalance()){
            if (stepBalance < TrustLine::kZeroBalance()) {
                nodes.push_back(nodeAndTrustLine.first);
            }
        } else {
            if (stepBalance > TrustLine::kZeroBalance()) {
                nodes.push_back(nodeAndTrustLine.first);
            }
        }
    }
    return nodes;
}

vector<NodeUUID> TrustLinesManager::firstLevelNeighborsWithPositiveBalance() const
{
    vector<NodeUUID> nodes;
    TrustLineBalance stepBalance;
    for (auto const& x : mTrustLines){
        stepBalance = x.second->balance();
        if (stepBalance > TrustLine::kZeroBalance())
            nodes.push_back(x.first);
        }
    return nodes;
}

vector<NodeUUID> TrustLinesManager::firstLevelNeighborsWithNegativeBalance() const
{
    // todo change vector to set
    vector<NodeUUID> nodes;
    TrustLineBalance stepBalance;
    for (const auto &nodeAndTrustLine : mTrustLines){
        stepBalance = nodeAndTrustLine.second->balance();
        if (stepBalance < TrustLine::kZeroBalance())
            nodes.push_back(nodeAndTrustLine.first);
    }
    return nodes;
}

vector<NodeUUID> TrustLinesManager::firstLevelNeighborsWithNoneZeroBalance() const
{
    vector<NodeUUID> nodes;
    TrustLineBalance stepBalance;
    for (auto const &nodeAndTrustLine : mTrustLines) {
        stepBalance = nodeAndTrustLine.second->balance();
        if (stepBalance != TrustLine::kZeroBalance())
            nodes.push_back(nodeAndTrustLine.first);
    }
    return nodes;
}

void TrustLinesManager::useReservation(
    const NodeUUID &contractor,
    const AmountReservation::ConstShared reservation)
{
    if (not trustLineIsPresent(contractor)) {
        throw NotFoundError(
            logHeader() + "::useReservation: "
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
            logHeader() + "::useReservation: "
            "Unexpected trust line direction occurred.");
    }
    }
}

ConstSharedTrustLineAmount TrustLinesManager::totalOutgoingAmount () const
{
    auto totalAmount = make_shared<TrustLineAmount>(0);
    for (const auto &kTrustLine : mTrustLines) {
        const auto kTLAmount = outgoingTrustAmountConsideringReservations(kTrustLine.first);
        *totalAmount += *(kTLAmount);
    }

    return totalAmount;
}

ConstSharedTrustLineAmount TrustLinesManager::totalIncomingAmount() const
{
    auto totalAmount = make_shared<TrustLineAmount>(0);
    for (const auto &kTrustLine : mTrustLines) {
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

const string TrustLinesManager::logHeader() const
    noexcept
{
    stringstream s;
    s << "[TrustLinesManager: " << mEquivalent << "] ";
    return s.str();
}

LoggerStream TrustLinesManager::info() const
    noexcept
{
    return mLogger.info(logHeader());
}

void TrustLinesManager::printRTs()
{
    LoggerStream debug = mLogger.debug("TrustLinesManager::printRts");
    auto ioTransaction = mStorageHandler->beginTransaction();
    debug << "printRTs\tRT1 size: " << trustLines().size() << endl;
    for (const auto &itTrustLine : trustLines()) {
        debug << "printRTs\t" << itTrustLine.second->contractorNodeUUID() << " "
               << itTrustLine.second->incomingTrustAmount() << " "
               << itTrustLine.second->outgoingTrustAmount() << " "
               << itTrustLine.second->balance() << " "
               << itTrustLine.second->isContractorGateway() << endl;
    }
    debug << "print payment incoming flows size: " << incomingFlows().size() << endl;
    for (auto const &itIncomingFlow : incomingFlows()) {
        debug << itIncomingFlow.first << " " << *itIncomingFlow.second.get() << endl;
    }
    debug << "print payment outgoing flows size: " << outgoingFlows().size() << endl;
    for (auto const &itOutgoingFlow : outgoingFlows()) {
        debug << itOutgoingFlow.first << " " << *itOutgoingFlow.second.get() << endl;
    }
    debug << "print cycle incoming flows size: " << incomingFlows().size() << endl;
    for (auto const &trLine : mTrustLines) {
        auto const availableIncomingCycleAmounts = this->availableIncomingCycleAmounts(trLine.first);
        debug << trLine.first << " " << *(availableIncomingCycleAmounts.first)
              << " " << *(availableIncomingCycleAmounts.second) << endl;
    }
    debug << "print cycle outgoing flows size: " << outgoingFlows().size() << endl;
    for (auto const &trLine : mTrustLines) {
        auto const availableOutgoingCycleAmounts = this->availableOutgoingCycleAmounts(trLine.first);
        debug << trLine.first << " " << *(availableOutgoingCycleAmounts.first)
              << " " << *(availableOutgoingCycleAmounts.second) << endl;
    }
}
