#include "TrustLinesManager.h"


TrustLinesManager::TrustLinesManager(
    const SerializedEquivalent equivalent,
    StorageHandler *storageHandler,
    Keystore *keyStore,
    Logger &logger):

    mEquivalent(equivalent),
    mStorageHandler(storageHandler),
    mKeysStore(keyStore),
    mLogger(logger),
    mAmountReservationsHandler(
        make_unique<AmountReservationsHandler>())
{
    loadTrustLinesFromStorage();
}

void TrustLinesManager::loadTrustLinesFromStorage()
{
    auto ioTransaction = mStorageHandler->beginTransaction();
    const auto kTrustLines = ioTransaction->trustLinesHandler()->allTrustLinesByEquivalent(mEquivalent);

    mTrustLines.reserve(kTrustLines.size());

    for (auto const &kTrustLine : kTrustLines) {
        try {
            auto auditRecord = ioTransaction->auditHandler()->getActualAudit(
                kTrustLine->trustLineID());
            kTrustLine->setAuditNumber(auditRecord->auditNumber());
            kTrustLine->setIncomingTrustAmount(auditRecord->incomingAmount());
            kTrustLine->setOutgoingTrustAmount(auditRecord->outgoingAmount());

            auto keyChain = mKeysStore->keychain(kTrustLine->trustLineID());
            auto totalIncomingReceiptsAmount = keyChain.incomingCommittedReceiptsAmountsSum(
                ioTransaction,
                auditRecord->auditNumber());
            auto totalOutgoingReceiptsAmount = keyChain.outgoingCommittedReceiptsAmountsSum(
                ioTransaction,
                auditRecord->auditNumber());
            TrustLineBalance balance = auditRecord->balance() + totalIncomingReceiptsAmount - totalOutgoingReceiptsAmount;
            kTrustLine->setBalance(balance);

            if (auditRecord->balance() > TrustLine::kZeroBalance()) {
                totalIncomingReceiptsAmount = totalIncomingReceiptsAmount + TrustLineAmount(auditRecord->balance());
            } else {
                totalOutgoingReceiptsAmount = totalOutgoingReceiptsAmount + TrustLineAmount(abs(auditRecord->balance()));
            }

            kTrustLine->setTotalIncomingReceiptsAmount(
                totalIncomingReceiptsAmount);
            kTrustLine->setTotalOutgoingReceiptsAmount(
                totalOutgoingReceiptsAmount);

        } catch (NotFoundError&) {
            info() << "init TL in storage with contractor " << kTrustLine->contractorNodeUUID();
        }

        mTrustLines.insert(
            make_pair(
                kTrustLine->contractorNodeUUID(),
                kTrustLine));
    }
}

void TrustLinesManager::open(
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount,
    IOTransaction::Shared ioTransaction)
{
    TrustLineID trustLineID = nextFreeID(ioTransaction);
    // In case if trust line to this contractor is absent,
    // and "amount" is greater than 0 - new trust line should be created.
    // contractor is not gateway by default
    auto trustLine = make_shared<TrustLine>(
        contractorUUID,
        trustLineID,
        0,
        amount,
        0,
        false);
    mTrustLines[contractorUUID] = trustLine;

    if (ioTransaction != nullptr) {
        saveToStorage(
            ioTransaction,
            trustLine);
    }
}

void TrustLinesManager::accept(
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount,
    IOTransaction::Shared ioTransaction)
{
    if (trustLineIsPresent(contractorUUID)) {
        throw ValueError(
            logHeader() + "::accept: trust line already present.");
    }
    if (amount == 0) {
        throw ValueError(
            logHeader() + "::accept: "
                "can't establish trust line with zero amount.");
    }

    TrustLineID trustLineID = nextFreeID(ioTransaction);
    // In case if TL to this contractor is absent,
    // and "amount" is greater than 0 - new trust line should be created.
    // contractor is not gateway by default
    auto trustLine = make_shared<TrustLine>(
        contractorUUID,
        trustLineID,
        amount,
        0,
        0,
        false);
    mTrustLines[contractorUUID] = trustLine;

    if (ioTransaction != nullptr) {
        saveToStorage(
            ioTransaction,
            trustLine);
    }
}

TrustLinesManager::TrustLineOperationResult TrustLinesManager::setOutgoing(
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount)
{
    if (outgoingTrustAmount(contractorUUID) == 0) {
        // In case if outgoing TL to this contractor is absent,
        // "amount" can't be 0 (otherwise, trust line set to zero would be opened).
        if (amount == 0) {
            throw ValueError(
                logHeader() + "::setOutgoing: "
                "can't establish trust line with zero amount.");

        } else {
            // In case if "amount" is greater than 0 - outgoing trust line should be created.
            auto trustLine = mTrustLines[contractorUUID];
            trustLine->setOutgoingTrustAmount(amount);
            return TrustLineOperationResult::Opened;
        }
    }

    if (amount == 0) {
        // In case if trust line is already present,
        // but incoming trust amount is 0, and received "amount" is 0 -
        // then it is interpreted as the command to close the outgoing trust line.
        closeOutgoing(
            contractorUUID);
        return TrustLineOperationResult::Closed;
    }

    auto trustLine = mTrustLines[contractorUUID];
    if (trustLine->outgoingTrustAmount() == amount) {
        // There is no reason to write the same data to the disk.
        return TrustLineOperationResult::NoChanges;
    }

    trustLine->setOutgoingTrustAmount(amount);
    return TrustLineOperationResult::Updated;
}

TrustLinesManager::TrustLineOperationResult TrustLinesManager::setIncoming(
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount)
{
    if (incomingTrustAmount(contractorUUID) == 0) {
        // In case if incoming TL amount to this contractor is absent,
        // "amount" can't be 0 (otherwise, trust line with both sides set to zero would be opened).
        if (amount == 0) {
            throw ValueError(
                logHeader() + "::setIncoming: "
                "can't establish trust line with zero amount at both sides.");

        } else {
            // In case if "amount" is greater than 0 - incoming trust line should be created.
            auto trustLine = mTrustLines[contractorUUID];
            trustLine->setIncomingTrustAmount(amount);
            return TrustLineOperationResult::Opened;
        }
    }

    if (amount == 0) {
        // In case if incoming trust line is already present,
        // and received "amount" is 0 -
        // then it is interpreted as the command to close the incoming trust line.
        closeIncoming(
            contractorUUID);
        return TrustLineOperationResult::Closed;
    }

    auto trustLine = mTrustLines[contractorUUID];
    if (trustLine->incomingTrustAmount() == amount) {
        // There is no reason to write the same data to the disk.
        return TrustLineOperationResult::NoChanges;
    }

    trustLine->setIncomingTrustAmount(amount);
    return TrustLineOperationResult::Updated;
}

void TrustLinesManager::closeOutgoing(
    const NodeUUID &contractorUUID)
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            logHeader() + "::closeOutgoing: "
                "No trust line to this contractor is present.");
    }

    auto trustLine = mTrustLines[contractorUUID];
    trustLine->setOutgoingTrustAmount(0);
}

void TrustLinesManager::closeIncoming(
    const NodeUUID &contractorUUID)
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            logHeader() + "::closeIncoming: "
                "No trust line to this contractor is present.");
    }

    auto trustLine = mTrustLines[contractorUUID];
    trustLine->setIncomingTrustAmount(0);
}

void TrustLinesManager::setContractorAsGateway(
    IOTransaction::Shared ioTransaction,
    const NodeUUID &contractorUUID,
    bool contractorIsGateway)
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            logHeader() + "::setContractorAsGateway: "
                "There is no trust line to contractor " + contractorUUID.stringUUID());
    }

    auto trustLine = mTrustLines[contractorUUID];
    trustLine->setContractorAsGateway(contractorIsGateway);
    updateTrustLine(
        ioTransaction,
        trustLine);
}

void TrustLinesManager::setIsOwnKeysPresent(
    const NodeUUID &contractorUUID,
    bool isOwnKeysPresent)
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            logHeader() + "::setIsOwnKeysPresent: "
                "There is no trust line to contractor " + contractorUUID.stringUUID());
    }

    auto trustLine = mTrustLines[contractorUUID];
    trustLine->setIsOwnKeysPresent(isOwnKeysPresent);
}

void TrustLinesManager::setIsContractorKeysPresent(
    const NodeUUID &contractorUUID,
    bool isContractorKeysPresent)
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            logHeader() + "::setIsContractorKeysPresent: "
                "There is no trust line to contractor " + contractorUUID.stringUUID());
    }

    auto trustLine = mTrustLines[contractorUUID];
    trustLine->setIsContractorKeysPresent(isContractorKeysPresent);
}

void TrustLinesManager::setTrustLineState(
    const NodeUUID &contractorUUID,
    TrustLine::TrustLineState state,
    IOTransaction::Shared ioTransaction)
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            logHeader() + "::setTrustLineState: "
                "There is no trust line to contractor " + contractorUUID.stringUUID());
    }

    auto trustLine = mTrustLines[contractorUUID];
    trustLine->setState(state);

    if (ioTransaction != nullptr) {
        updateTrustLine(
            ioTransaction,
            trustLine);
    }
}

void TrustLinesManager::setTrustLineAuditNumberAndMakeActive(
    const NodeUUID &contractorUUID,
    AuditNumber newAuditNumber)
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            logHeader() + "::setTrustLineAuditNumber: "
                "There is no trust line to contractor " + contractorUUID.stringUUID());
    }

    auto trustLine = mTrustLines[contractorUUID];
    trustLine->setAuditNumber(newAuditNumber);
    trustLine->setState(TrustLine::Active);
}

const TrustLineAmount &TrustLinesManager::incomingTrustAmount(
    const NodeUUID &contractorUUID) const
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            logHeader() + "::incomingTrustAmount: "
                "There is no trust line to contractor " + contractorUUID.stringUUID());
    }

    return mTrustLines.at(contractorUUID)->incomingTrustAmount();
}

const TrustLineAmount &TrustLinesManager::outgoingTrustAmount(
    const NodeUUID &contractorUUID) const
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            logHeader() + "::outgoingTrustAmount: "
                "There is no trust line to contractor " + contractorUUID.stringUUID());
    }

    return mTrustLines.at(contractorUUID)->outgoingTrustAmount();
}

const TrustLineBalance &TrustLinesManager::balance(
    const NodeUUID &contractorUUID) const
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            logHeader() + "::balance: "
                "There is no trust line to contractor " + contractorUUID.stringUUID());
    }

    return mTrustLines.at(contractorUUID)->balance();
}

const TrustLineID TrustLinesManager::trustLineID(
    const NodeUUID &contractorUUID) const
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            logHeader() + "::trustLineID: "
                "There is no trust line to contractor " + contractorUUID.stringUUID());
    }

    return mTrustLines.at(contractorUUID)->trustLineID();
}

const AuditNumber TrustLinesManager::auditNumber(
    const NodeUUID &contractorUUID) const
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            logHeader() + "::auditNumber: "
                "There is no trust line to contractor " + contractorUUID.stringUUID());
    }

    return mTrustLines.at(contractorUUID)->currentAuditNumber();
}

const TrustLine::TrustLineState TrustLinesManager::trustLineState(
    const NodeUUID &contractorUUID) const
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            logHeader() + "::trustLineState: "
                "There is no trust line to contractor " + contractorUUID.stringUUID());
    }

    return mTrustLines.at(contractorUUID)->state();
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
            "Trust line has not enough free amount.");
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
    if (kTL->state() != TrustLine::Active) {
        return make_shared<const TrustLineAmount>(0);
    }
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
    if (kTL->state() != TrustLine::Active) {
        return make_shared<const TrustLineAmount>(0);
    }
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
    if (kTL->state() != TrustLine::Active) {
        return make_pair(
            make_shared<const TrustLineAmount>(0),
            make_shared<const TrustLineAmount>(0));
    }
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
    if (kTL->state() != TrustLine::Active) {
        return make_pair(
            make_shared<const TrustLineAmount>(0),
            make_shared<const TrustLineAmount>(0));
    }
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
    const NodeUUID &contractorUUID) const
{
    return mTrustLines.count(contractorUUID) > 0;
}

const bool TrustLinesManager::trustLineIsActive(
    const NodeUUID &contractorUUID) const
{
    if (!trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(logHeader() +
                        " There is no trust line to contractor " + contractorUUID.stringUUID());
    }
    return mTrustLines.at(contractorUUID)->state() == TrustLine::Active;
}

bool TrustLinesManager::isReservationsPresentOnTrustLine(
    const NodeUUID &contractorUUID) const
{
    return mAmountReservationsHandler->isReservationsPresent(
        contractorUUID);
}

void TrustLinesManager::saveToStorage(
    IOTransaction::Shared ioTransaction,
    TrustLine::Shared trustLine)
{
    ioTransaction->trustLinesHandler()->saveTrustLine(
        trustLine,
        mEquivalent);
    try {
        mTrustLines.insert(
            make_pair(
                trustLine->contractorNodeUUID(),
                trustLine));

    } catch (std::bad_alloc&) {
            throw MemoryError(
                logHeader() + "::saveToStorage: "
                "Can not reallocate STL container memory for new trust line instance.");
    }
}

void TrustLinesManager::updateTrustLine(
    IOTransaction::Shared ioTransaction,
    TrustLine::Shared trustLine)
{
    if (not trustLineIsPresent(trustLine->contractorNodeUUID())) {
        throw NotFoundError(
            logHeader() + "::updateTrustLine: "
                "There is no trust line to the contractor.");
    }
    ioTransaction->trustLinesHandler()->updateTrustLine(
        trustLine,
        mEquivalent);
    try {
        mTrustLines[trustLine->contractorNodeUUID()] = trustLine;
    } catch (std::bad_alloc&) {
        throw MemoryError(
            logHeader() + "::updateTrustLine: "
            "Can not reallocate STL container memory for new trust line instance.");
    }
}

/**
 * @throws NotFoundError
 */
void TrustLinesManager::removeTrustLine(
    const NodeUUID &contractorUUID,
    IOTransaction::Shared ioTransaction)
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            logHeader() + "::removeTrustLine: "
            "There is no trust line to the contractor.");
    }

    mTrustLines.erase(contractorUUID);

    if (ioTransaction != nullptr) {
        ioTransaction->trustLinesHandler()->deleteTrustLine(
            contractorUUID,
            mEquivalent);
    }
}

void TrustLinesManager::updateTrustLineFromStorage(
    const NodeUUID &contractorUUID,
    IOTransaction::Shared ioTransaction)
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            logHeader() + "::removeTrustLine: "
                "There is no trust line to the contractor.");
    }

    auto kTrustLine = mTrustLines[contractorUUID];
    try {
        auto auditRecord = ioTransaction->auditHandler()->getActualAudit(
            kTrustLine->trustLineID());
        kTrustLine->setAuditNumber(auditRecord->auditNumber());
        kTrustLine->setIncomingTrustAmount(auditRecord->incomingAmount());
        kTrustLine->setOutgoingTrustAmount(auditRecord->outgoingAmount());
        info() << "audit number " << auditRecord->auditNumber();

        auto keyChain = mKeysStore->keychain(kTrustLine->trustLineID());
        TrustLineAmount totalIncomingReceiptsAmount = keyChain.incomingCommittedReceiptsAmountsSum(
            ioTransaction,
            auditRecord->auditNumber());
        TrustLineAmount totalOutgoingReceiptsAmount = keyChain.outgoingCommittedReceiptsAmountsSum(
            ioTransaction,
            auditRecord->auditNumber());
        TrustLineBalance balance = auditRecord->balance() + totalIncomingReceiptsAmount - totalOutgoingReceiptsAmount;
        kTrustLine->setBalance(balance);

        if (auditRecord->balance() > TrustLine::kZeroBalance()) {
            totalIncomingReceiptsAmount = totalIncomingReceiptsAmount + TrustLineAmount(auditRecord->balance());
        } else {
            totalOutgoingReceiptsAmount = totalOutgoingReceiptsAmount + TrustLineAmount(abs(auditRecord->balance()));
        }

        kTrustLine->setTotalIncomingReceiptsAmount(
            totalIncomingReceiptsAmount);
        kTrustLine->setTotalOutgoingReceiptsAmount(
            totalOutgoingReceiptsAmount);

    } catch (NotFoundError&) {
        info() << "init TL in storage with contractor " << kTrustLine->contractorNodeUUID();
    }

    mTrustLines[kTrustLine->contractorNodeUUID()] = kTrustLine;
}

/**
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

    return (outgoingTrustAmount(contractorUUID) == TrustLine::kZeroAmount()
        and incomingTrustAmount(contractorUUID) == TrustLine::kZeroAmount()
        and balance(contractorUUID) == TrustLine::kZeroBalance());
}

bool TrustLinesManager::isTrustLineOverflowed(
    const NodeUUID &contractorUUID)
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            logHeader() + "::isTrustLineOverflowed: "
                "There is no trust line to the contractor.");
    }
    auto trustLine = mTrustLines[contractorUUID];
    return trustLine->isTrustLineOverflowed();
}

void TrustLinesManager::resetTrustLineTotalReceiptsAmounts(
    const NodeUUID &contractorUUID)
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            logHeader() + "::resetTrustLineTotalReceiptsAmounts: "
                "There is no trust line to the contractor.");
    }
    auto trustLine = mTrustLines[contractorUUID];
    trustLine->resetTotalReceiptsAmounts();
}

vector<NodeUUID> TrustLinesManager::firstLevelNeighborsWithOutgoingFlow() const
{
    vector<NodeUUID> result;
    for (auto const &nodeUUIDAndTrustLine : mTrustLines) {
        if (nodeUUIDAndTrustLine.second->state() != TrustLine::Active) {
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

vector<NodeUUID> TrustLinesManager::firstLevelGatewayNeighborsWithOutgoingFlow() const
{
    vector<NodeUUID> result;
    for (auto const &nodeUUIDAndTrustLine : mTrustLines) {
        if (nodeUUIDAndTrustLine.second->state() != TrustLine::Active) {
            continue;
        }
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
        if (nodeUUIDAndTrustLine.second->state() != TrustLine::Active) {
            continue;
        }
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

vector<NodeUUID> TrustLinesManager::firstLevelNonGatewayNeighborsWithIncomingFlow() const
{
    vector<NodeUUID> result;
    for (auto const &nodeUUIDAndTrustLine : mTrustLines) {
        if (nodeUUIDAndTrustLine.second->state() != TrustLine::Active) {
            continue;
        }
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
        if (nodeUUIDAndTrustLine.second->state() != TrustLine::Active) {
            continue;
        }
        auto trustLineAmountShared = incomingTrustAmountConsideringReservations(
            nodeUUIDAndTrustLine.first);
        result.emplace_back(
            nodeUUIDAndTrustLine.first,
            trustLineAmountShared);
    }
    return result;
}

vector<pair<NodeUUID, ConstSharedTrustLineAmount>> TrustLinesManager::outgoingFlows() const
{
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> result;
    for (auto const &nodeUUIDAndTrustLine : mTrustLines) {
        if (nodeUUIDAndTrustLine.second->state() != TrustLine::Active) {
            continue;
        }
        auto trustLineAmountShared = outgoingTrustAmountConsideringReservations(
            nodeUUIDAndTrustLine.first);
        result.emplace_back(
            nodeUUIDAndTrustLine.first,
            trustLineAmountShared);
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
        if (nodeUUIDAndTrustLine.second->state() != TrustLine::Active) {
            continue;
        }
        if (nodeUUIDAndTrustLine.second->isContractorGateway()) {
            continue;
        }
        auto trustLineAmountShared = incomingTrustAmountConsideringReservations(
            nodeUUIDAndTrustLine.first);
        result.emplace_back(
            nodeUUIDAndTrustLine.first,
            trustLineAmountShared);
    }
    return result;
}

vector<pair<NodeUUID, ConstSharedTrustLineAmount>> TrustLinesManager::outgoingFlowsToGateways() const
{
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> result;
    for (auto const &nodeUUIDAndTrustLine : mTrustLines) {
        if (nodeUUIDAndTrustLine.second->state() != TrustLine::Active) {
            continue;
        }
        if (!nodeUUIDAndTrustLine.second->isContractorGateway()) {
            continue;
        }
        auto trustLineAmountShared = outgoingTrustAmountConsideringReservations(
            nodeUUIDAndTrustLine.first);
        result.emplace_back(
            nodeUUIDAndTrustLine.first,
            trustLineAmountShared);
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
        if (nodeUUIDAndTrustLine.second->state() == TrustLine::Active) {
            result.push_back(
                nodeUUIDAndTrustLine.first);
        }
    }
    return result;
}

ConstSharedTrustLineBalance TrustLinesManager::totalBalance() const
{
    TrustLineBalance result = TrustLine::kZeroBalance();
    for (const auto &trustLine : mTrustLines) {
        if (trustLine.second->state() != TrustLine::Active) {
            continue;
        }
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

vector<NodeUUID> TrustLinesManager::getFirstLevelNodesForCycles(
    TrustLineBalance maxFlow)
{
    vector<NodeUUID> nodes;
    TrustLineBalance stepBalance;
    for (auto const& nodeAndTrustLine : mTrustLines) {
        if (nodeAndTrustLine.second->state() != TrustLine::Active) {
            continue;
        }
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
    for (auto const& nodeAndTrustLine : mTrustLines){
        if (nodeAndTrustLine.second->state() != TrustLine::Active) {
            continue;
        }
        stepBalance = nodeAndTrustLine.second->balance();
        if (stepBalance > TrustLine::kZeroBalance())
            nodes.push_back(nodeAndTrustLine.first);
        }
    return nodes;
}

vector<NodeUUID> TrustLinesManager::firstLevelNeighborsWithNegativeBalance() const
{
    // todo change vector to set
    vector<NodeUUID> nodes;
    TrustLineBalance stepBalance;
    for (const auto &nodeAndTrustLine : mTrustLines){
        if (nodeAndTrustLine.second->state() != TrustLine::Active) {
            continue;
        }
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
        if (nodeAndTrustLine.second->state() != TrustLine::Active) {
            continue;
        }
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
        if (kTrustLine.second->state() != TrustLine::Active) {
            continue;
        }
        const auto kTLAmount = outgoingTrustAmountConsideringReservations(kTrustLine.first);
        *totalAmount += *(kTLAmount);
    }

    return totalAmount;
}

ConstSharedTrustLineAmount TrustLinesManager::totalIncomingAmount() const
{
    auto totalAmount = make_shared<TrustLineAmount>(0);
    for (const auto &kTrustLine : mTrustLines) {
        if (kTrustLine.second->state() != TrustLine::Active) {
            continue;
        }
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

const TrustLineID TrustLinesManager::nextFreeID(
    IOTransaction::Shared ioTransaction) const
{
    vector<TrustLineID> tmpIDs = ioTransaction->trustLinesHandler()->allIDs();
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

TrustLinesManager::TrustLineActionType TrustLinesManager::checkTrustLineAfterTransaction(
    const NodeUUID &contractorUUID,
    bool isActionInitiator)
{
    info() << "checkTrustLineAfterTransaction";
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            logHeader() + "::checkTrustLineAfterTransaction: "
                "No trust line with the contractor is present.");
    }
    auto ioTransaction = mStorageHandler->beginTransaction();
    if (isTrustLineEmpty(contractorUUID)) {
        setTrustLineState(
            contractorUUID,
            TrustLine::AuditPending,
            ioTransaction);
        info() << "TL become empty";
        // if TL become empty, it is necessary to run Audit TA.
        // AuditSource TA run on node which pay
        if (isActionInitiator) {
            info() << "Audit signal";
            return TrustLineActionType::Audit;
        } else {
            return TrustLineActionType::NoActions;
        }
    } else if (isTrustLineOverflowed(contractorUUID)) {
        // todo : add audit rules
        setTrustLineState(
            contractorUUID,
            TrustLine::AuditPending,
            ioTransaction);
        info() << "TL become overflowed";
        // if TL become overflowed, it is necessary to run Audit TA.
        // AuditSource TA run on node which pay
        if (isActionInitiator) {
            info() << "Audit signal";
            return TrustLineActionType::Audit;
        } else {
            return TrustLineActionType::NoActions;
        }
    } else {
        // if both cases AuditSignal and PublicKeysSharingSignal occur simultaneously,
        // AuditSignal run and Audit Transaction and it include changing keys procedure
        auto keyChain = mKeysStore->keychain(
            trustLineID(
                contractorUUID));
        if (keyChain.ownKeysCriticalCount(ioTransaction)) {
            setIsOwnKeysPresent(
                contractorUUID,
                false);
            info() << "Public key sharing signal";
            return TrustLineActionType::KeysSharing;
        }
    }
    return TrustLineActionType::NoActions;
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
