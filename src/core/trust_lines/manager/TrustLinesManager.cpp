#include "TrustLinesManager.h"


TrustLinesManager::TrustLinesManager(
    const SerializedEquivalent equivalent,
    StorageHandler *storageHandler,
    Keystore *keyStore,
    ContractorsManager *contractorsManager,
    Logger &logger):

    mEquivalent(equivalent),
    mStorageHandler(storageHandler),
    mKeysStore(keyStore),
    mContractorsManager(contractorsManager),
    mLogger(logger),
    mAmountReservationsHandler(
        make_unique<AmountReservationsHandler>())
{
    loadTrustLinesFromStorage();
#if defined(DEBUG) || defined(TESTS)
    printTLs();
#endif
}

void TrustLinesManager::loadTrustLinesFromStorage()
{
    auto ioTransaction = mStorageHandler->beginTransaction();
    const auto kTrustLines = ioTransaction->trustLinesHandler()->allTrustLinesByEquivalent(mEquivalent);

    mTrustLines.reserve(kTrustLines.size());

    for (auto const &kTrustLine : kTrustLines) {
        auto keyChain = mKeysStore->keychain(kTrustLine->trustLineID());
        try {
            auto auditRecord = ioTransaction->auditHandler()->getActualAudit(
                kTrustLine->trustLineID());
            kTrustLine->setAuditNumber(auditRecord->auditNumber());
            kTrustLine->setIncomingTrustAmount(auditRecord->incomingAmount());
            kTrustLine->setOutgoingTrustAmount(auditRecord->outgoingAmount());

            if (!auditRecord->isPendingState()) {
                auto totalIncomingReceiptsAmount = keyChain.incomingCommittedReceiptsAmountsSum(
                    ioTransaction,
                    auditRecord->auditNumber());
                auto totalOutgoingReceiptsAmount = keyChain.outgoingCommittedReceiptsAmountsSum(
                    ioTransaction,
                    auditRecord->auditNumber());
                TrustLineBalance balance =
                    auditRecord->balance() + totalIncomingReceiptsAmount - totalOutgoingReceiptsAmount;
                kTrustLine->setBalance(balance);

                if (auditRecord->balance() > TrustLine::kZeroBalance()) {
                    totalIncomingReceiptsAmount =
                        totalIncomingReceiptsAmount + TrustLineAmount(auditRecord->balance());
                } else {
                    totalOutgoingReceiptsAmount =
                        totalOutgoingReceiptsAmount + TrustLineAmount(abs(auditRecord->balance()));
                }

                kTrustLine->setTotalIncomingReceiptsAmount(
                    totalIncomingReceiptsAmount);
                kTrustLine->setTotalOutgoingReceiptsAmount(
                    totalOutgoingReceiptsAmount);
            } else {
                info() << "audit pending TL in storage with contractor " << kTrustLine->contractorID();
                kTrustLine->setState(TrustLine::AuditPending);
                mContractorsShouldBePinged.push_back(
                    kTrustLine->contractorID());
            }

            if (keyChain.ownKeysPresent(ioTransaction)) {
                kTrustLine->setIsOwnKeysPresent(true);
            }

            if (keyChain.contractorKeysPresent(ioTransaction)) {
                kTrustLine->setIsContractorKeysPresent(true);
            }

        } catch (NotFoundError&) {
            info() << "init TL in storage with contractor " << kTrustLine->contractorID();
            kTrustLine->setState(
                TrustLine::Init);
            if (keyChain.ownKeysPresent(ioTransaction)) {
                warning() << "Something wrong, because TL contains own valid keys";
            }
            if (keyChain.contractorKeysPresent(ioTransaction)) {
                warning() << "Something wrong, because TL contains contractor's valid keys";
            }
            mContractorsShouldBePinged.push_back(
                kTrustLine->contractorID());
        }

        mTrustLines.insert(
            make_pair(
                kTrustLine->contractorID(),
                kTrustLine));
    }
}

void TrustLinesManager::open(
    ContractorID contractorID,
    IOTransaction::Shared ioTransaction)
{
    if (trustLineIsPresent(contractorID)) {
        throw ValueError(
            logHeader() + "::open: trust line already present.");
    }
    TrustLineID trustLineID = nextFreeID(ioTransaction);
    // In case if TL to this contractor is absent,
    // new trust line should be created.
    // contractor is not gateway by default
    auto trustLine = make_shared<TrustLine>(
        contractorID,
        trustLineID);
    mTrustLines[contractorID] = trustLine;

    if (ioTransaction != nullptr) {
        ioTransaction->trustLinesHandler()->saveTrustLine(
            trustLine,
            mEquivalent);
    }
}

void TrustLinesManager::accept(
    ContractorID contractorID,
    IOTransaction::Shared ioTransaction)
{
    if (trustLineIsPresent(contractorID)) {
        throw ValueError(
            logHeader() + "::accept: trust line already present.");
    }

    TrustLineID trustLineID = nextFreeID(ioTransaction);
    // In case if TL to this contractor is absent,
    // new trust line should be created.
    // contractor is not gateway by default
    auto trustLine = make_shared<TrustLine>(
        contractorID,
        trustLineID);
    mTrustLines[contractorID] = trustLine;

    if (ioTransaction != nullptr) {
        ioTransaction->trustLinesHandler()->saveTrustLine(
            trustLine,
            mEquivalent);
    }
}

TrustLinesManager::TrustLineOperationResult TrustLinesManager::setOutgoing(
    ContractorID contractorID,
    const TrustLineAmount &amount)
{
    if (outgoingTrustAmount(contractorID) == 0) {
        // In case if outgoing TL to this contractor is absent,
        // "amount" can't be 0 (otherwise, trust line set to zero would be opened).
        if (amount == 0) {
            throw ValueError(
                logHeader() + "::setOutgoing: "
                    "can't establish trust line with zero amount.");

        } else {
            // In case if "amount" is greater than 0 - outgoing trust line should be created.
            auto trustLine = mTrustLines[contractorID];
            trustLine->setOutgoingTrustAmount(amount);
            return TrustLineOperationResult::Updated;
        }
    }

    if (amount == 0) {
        // In case if trust line is already present,
        // but incoming trust amount is 0, and received "amount" is 0 -
        // then it is interpreted as the command to close the outgoing trust line.
        closeOutgoing(
            contractorID);
        return TrustLineOperationResult::Closed;
    }

    auto trustLine = mTrustLines[contractorID];
    if (trustLine->outgoingTrustAmount() == amount) {
        // There is no reason to write the same data to the disk.
        return TrustLineOperationResult::NoChanges;
    }

    trustLine->setOutgoingTrustAmount(amount);
    return TrustLineOperationResult::Updated;
}

TrustLinesManager::TrustLineOperationResult TrustLinesManager::setIncoming(
    ContractorID contractorID,
    const TrustLineAmount &amount)
{
    if (incomingTrustAmount(contractorID) == 0) {
        // In case if incoming TL amount to this contractor is absent,
        // "amount" can't be 0 (otherwise, trust line with both sides set to zero would be opened).
        if (amount == 0) {
            throw ValueError(
                logHeader() + "::setIncoming: "
                    "can't establish trust line with zero amount at both sides.");

        } else {
            // In case if "amount" is greater than 0 - incoming trust line should be created.
            auto trustLine = mTrustLines[contractorID];
            trustLine->setIncomingTrustAmount(amount);
            return TrustLineOperationResult::Updated;
        }
    }

    if (amount == 0) {
        // In case if incoming trust line is already present,
        // and received "amount" is 0 -
        // then it is interpreted as the command to close the incoming trust line.
        closeIncoming(
            contractorID);
        return TrustLineOperationResult::Closed;
    }

    auto trustLine = mTrustLines[contractorID];
    if (trustLine->incomingTrustAmount() == amount) {
        // There is no reason to write the same data to the disk.
        return TrustLineOperationResult::NoChanges;
    }

    trustLine->setIncomingTrustAmount(amount);
    return TrustLineOperationResult::Updated;
}

void TrustLinesManager::closeOutgoing(
    ContractorID contractorID)
{
    if (outgoingTrustAmount(contractorID) == 0) {
        throw ValueError(logHeader() + "::closeOutgoing: "
            "can't close outgoing trust line with zero amount.");
    }

    auto trustLine = mTrustLines[contractorID];
    trustLine->setOutgoingTrustAmount(0);
}

void TrustLinesManager::closeIncoming(
    ContractorID contractorID)
{
    if (incomingTrustAmount(contractorID) == 0) {
        throw ValueError(logHeader() + "::closeIncoming: "
            "can't close incoming trust line with zero amount.");
    }

    auto trustLine = mTrustLines[contractorID];
    trustLine->setIncomingTrustAmount(0);
}

void TrustLinesManager::setContractorAsGateway(
    IOTransaction::Shared ioTransaction,
    ContractorID contractorID,
    bool contractorIsGateway)
{
    if (not trustLineIsPresent(contractorID)) {
        throw NotFoundError(
            logHeader() + "::setContractorAsGateway: "
                "There is no trust line to contractor " + to_string(contractorID));
    }

    auto trustLine = mTrustLines[contractorID];
    trustLine->setContractorAsGateway(contractorIsGateway);
    ioTransaction->trustLinesHandler()->updateTrustLineIsContractorGateway(
        trustLine,
        mEquivalent);
}

void TrustLinesManager::setIsOwnKeysPresent(
    ContractorID contractorID,
    bool isOwnKeysPresent)
{
    if (not trustLineIsPresent(contractorID)) {
        throw NotFoundError(
            logHeader() + "::setIsOwnKeysPresent: "
                "There is no trust line to contractor " + to_string(contractorID));
    }

    auto trustLine = mTrustLines[contractorID];
    trustLine->setIsOwnKeysPresent(isOwnKeysPresent);
}

void TrustLinesManager::setIsContractorKeysPresent(
    ContractorID contractorID,
    bool isContractorKeysPresent)
{
    if (not trustLineIsPresent(contractorID)) {
        throw NotFoundError(
            logHeader() + "::setIsContractorKeysPresent: "
                "There is no trust line to contractor " + to_string(contractorID));
    }

    auto trustLine = mTrustLines[contractorID];
    trustLine->setIsContractorKeysPresent(isContractorKeysPresent);
}

void TrustLinesManager::setTrustLineState(
    ContractorID contractorID,
    TrustLine::TrustLineState state,
    IOTransaction::Shared ioTransaction)
{
    if (not trustLineIsPresent(contractorID)) {
        throw NotFoundError(
            logHeader() + "::setTrustLineState: "
                "There is no trust line to contractor " + to_string(contractorID));
    }

    auto trustLine = mTrustLines[contractorID];
    trustLine->setState(state);

    if (ioTransaction != nullptr) {
        ioTransaction->trustLinesHandler()->updateTrustLineState(
            trustLine,
            mEquivalent);
    }
}

void TrustLinesManager::setTrustLineAuditNumber(
    ContractorID contractorID,
    AuditNumber newAuditNumber)
{
    if (not trustLineIsPresent(contractorID)) {
        throw NotFoundError(
            logHeader() + "::setTrustLineAuditNumber: "
                "There is no trust line to contractor " + to_string(contractorID));
    }

    auto trustLine = mTrustLines[contractorID];
    trustLine->setAuditNumber(newAuditNumber);
}

const bool TrustLinesManager::isContractorGateway(
    ContractorID contractorID) const
{
    if (not trustLineIsPresent(contractorID)) {
        throw NotFoundError(
            logHeader() + "::isContractorGateway: "
                "There is no trust line to this contractor.");
    }

    return mTrustLines.at(contractorID)->isContractorGateway();
}

const TrustLineAmount &TrustLinesManager::incomingTrustAmount(
    ContractorID contractorID) const
{
    if (not trustLineIsPresent(contractorID)) {
        throw NotFoundError(
            logHeader() + "::incomingTrustAmount: "
                "There is no trust line to contractor " + to_string(contractorID));
    }

    return mTrustLines.at(contractorID)->incomingTrustAmount();
}

const TrustLineAmount &TrustLinesManager::outgoingTrustAmount(
    ContractorID contractorID) const
{
    if (not trustLineIsPresent(contractorID)) {
        throw NotFoundError(
            logHeader() + "::outgoingTrustAmount: "
                "There is no trust line to contractor " + to_string(contractorID));
    }

    return mTrustLines.at(contractorID)->outgoingTrustAmount();
}

const TrustLineBalance &TrustLinesManager::balance(
    ContractorID contractorID) const
{
    if (not trustLineIsPresent(contractorID)) {
        throw NotFoundError(
            logHeader() + "::balance: "
                "There is no trust line to contractor " + to_string(contractorID));
    }

    return mTrustLines.at(contractorID)->balance();
}

const TrustLineID TrustLinesManager::trustLineID(
    ContractorID contractorID) const
{
    if (not trustLineIsPresent(contractorID)) {
        throw NotFoundError(
            logHeader() + "::trustLineID: "
                "There is no trust line to contractor " + to_string(contractorID));
    }

    return mTrustLines.at(contractorID)->trustLineID();
}

const AuditNumber TrustLinesManager::auditNumber(
    ContractorID contractorID) const
{
    if (not trustLineIsPresent(contractorID)) {
        throw NotFoundError(
            logHeader() + "::auditNumber: "
                "There is no trust line to contractor " + to_string(contractorID));
    }

    return mTrustLines.at(contractorID)->currentAuditNumber();
}

const TrustLine::TrustLineState TrustLinesManager::trustLineState(
    ContractorID contractorID) const
{
    if (not trustLineIsPresent(contractorID)) {
        throw NotFoundError(
            logHeader() + "::trustLineState: "
                "There is no trust line to contractor " + to_string(contractorID));
    }

    return mTrustLines.at(contractorID)->state();
}

bool TrustLinesManager::trustLineOwnKeysPresent(
    ContractorID contractorID) const
{
    if (not trustLineIsPresent(contractorID)) {
        throw NotFoundError(
            logHeader() + "::trustLineOwnKeysPresent: "
                "There is no trust line to contractor " + to_string(contractorID));
    }

    return mTrustLines.at(contractorID)->isOwnKeysPresent();
}

bool TrustLinesManager::trustLineContractorKeysPresent(
    ContractorID contractorID) const
{
    if (not trustLineIsPresent(contractorID)) {
        throw NotFoundError(
            logHeader() + "::trustLineContractorKeysPresent: "
                "There is no trust line to contractor " + to_string(contractorID));
    }

    return mTrustLines.at(contractorID)->isContractorKeysPresent();
}

AmountReservation::ConstShared TrustLinesManager::reserveOutgoingAmount(
    ContractorID contractor,
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
    ContractorID contractor,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount)
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
    ContractorID contractor,
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

AmountReservation::ConstShared TrustLinesManager::getAmountReservation(
    ContractorID contractor,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount,
    AmountReservation::ReservationDirection direction)
{
    return mAmountReservationsHandler->getReservation(
        contractor,
        transactionUUID,
        amount,
        direction);
}

void TrustLinesManager::dropAmountReservation(
    ContractorID contractor,
    const AmountReservation::ConstShared reservation)
{
    mAmountReservationsHandler->free(
        contractor,
        reservation);
}

ConstSharedTrustLineAmount TrustLinesManager::outgoingTrustAmountConsideringReservations(
    ContractorID contractorID) const
{
    const auto kTL = trustLineReadOnly(contractorID);
    if (kTL->state() != TrustLine::Active) {
        return make_shared<const TrustLineAmount>(0);
    }
    const auto kAvailableAmount = kTL->availableOutgoingAmount();
    const auto kAlreadyReservedAmount = mAmountReservationsHandler->totalReserved(
        contractorID, AmountReservation::Outgoing);

    if (*kAlreadyReservedAmount >= *kAvailableAmount) {
        return make_shared<const TrustLineAmount>(0);
    }
    return make_shared<const TrustLineAmount>(
        *kAvailableAmount - *kAlreadyReservedAmount);
}

ConstSharedTrustLineAmount TrustLinesManager::incomingTrustAmountConsideringReservations(
    ContractorID contractorID) const
{
    const auto kTL = trustLineReadOnly(contractorID);
    if (kTL->state() != TrustLine::Active) {
        return make_shared<const TrustLineAmount>(0);
    }
    const auto kAvailableAmount = kTL->availableIncomingAmount();
    const auto kAlreadyReservedAmount = mAmountReservationsHandler->totalReserved(
        contractorID, AmountReservation::Incoming);

    if (*kAlreadyReservedAmount >= *kAvailableAmount) {
        return make_shared<const TrustLineAmount>(0);
    }
    return make_shared<const TrustLineAmount>(
        *kAvailableAmount - *kAlreadyReservedAmount);
}

pair<ConstSharedTrustLineAmount, ConstSharedTrustLineAmount> TrustLinesManager::availableOutgoingCycleAmounts(
    ContractorID contractorID) const
{
    const auto kTL = trustLineReadOnly(contractorID);
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
        contractorID, AmountReservation::Outgoing);

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
    ContractorID contractorID) const
{
    const auto kTL = trustLineReadOnly(contractorID);
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
        contractorID, AmountReservation::Incoming);

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

const bool TrustLinesManager::trustLineIsPresent(
    ContractorID contractorID) const
{
    return mTrustLines.count(contractorID) > 0;
}

const bool TrustLinesManager::trustLineIsActive(
    ContractorID contractorID) const
{
    if (!trustLineIsPresent(contractorID)) {
        throw NotFoundError(logHeader() +
            " There is no trust line to contractor " + to_string(contractorID));
    }
    return mTrustLines.at(contractorID)->state() == TrustLine::Active;
}

bool TrustLinesManager::isReservationsPresentOnTrustLine(
    ContractorID contractorID) const
{
    return mAmountReservationsHandler->isReservationsPresentWithContractor(
        contractorID);
}

bool TrustLinesManager::isReservationsPresentConsiderTransaction(
    const TransactionUUID &transactionUUID) const
{
    return mAmountReservationsHandler->isTransactionReservationsPresent(
        transactionUUID);
}

/**
 * @throws NotFoundError
 */
void TrustLinesManager::removeTrustLine(
    ContractorID contractorID,
    IOTransaction::Shared ioTransaction)
{
    if (not trustLineIsPresent(contractorID)) {
        throw NotFoundError(
            logHeader() + "::removeTrustLine: "
                "There is no trust line to the contractor " + to_string(contractorID));
    }

    mTrustLines.erase(contractorID);

    if (ioTransaction != nullptr) {
        ioTransaction->trustLinesHandler()->deleteTrustLine(
            contractorID,
            mEquivalent);
    }

    // todo remove contractor if need
}

void TrustLinesManager::updateTrustLineFromStorage(
    ContractorID contractorID,
    IOTransaction::Shared ioTransaction)
{
    if (not trustLineIsPresent(contractorID)) {
        throw NotFoundError(
            logHeader() + "::removeTrustLine: "
                "There is no trust line to the contractor.");
    }

    auto kTrustLine = mTrustLines[contractorID];
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


        kTrustLine->setIsOwnKeysPresent(
            keyChain.ownKeysPresent(ioTransaction));
        kTrustLine->setIsContractorKeysPresent(
            keyChain.contractorKeysPresent(ioTransaction));

    } catch (NotFoundError&) {
        info() << "init TL in storage with contractor " << kTrustLine->contractorID();
    }

    mTrustLines[kTrustLine->contractorID()] = kTrustLine;
}

/**
 * @throws NotFoundError
 */
bool TrustLinesManager::isTrustLineEmpty(
    ContractorID contractorID)
{
    if (not trustLineIsPresent(contractorID)) {
        throw NotFoundError(
           logHeader() + "::isTrustLineEmpty: "
                "There is no trust line to the contractor " + to_string(contractorID));
    }

    return (outgoingTrustAmount(contractorID) == TrustLine::kZeroAmount()
        and incomingTrustAmount(contractorID) == TrustLine::kZeroAmount()
        and balance(contractorID) == TrustLine::kZeroBalance());
}

bool TrustLinesManager::isTrustLineOverflowed(
    ContractorID contractorID)
{
    if (not trustLineIsPresent(contractorID)) {
        throw NotFoundError(
            logHeader() + "::isTrustLineOverflowed: "
                "There is no trust line to the contractor " + to_string(contractorID));
    }
    auto trustLine = mTrustLines[contractorID];
    return trustLine->isTrustLineOverflowed();
}

void TrustLinesManager::resetTrustLineTotalReceiptsAmounts(
    ContractorID contractorID)
{
    if (not trustLineIsPresent(contractorID)) {
        throw NotFoundError(
            logHeader() + "::resetTrustLineTotalReceiptsAmounts: "
                "There is no trust line to the contractor " + to_string(contractorID));
    }
    auto trustLine = mTrustLines[contractorID];
    trustLine->resetTotalReceiptsAmounts();
}

vector<ContractorID> TrustLinesManager::firstLevelNeighborsWithOutgoingFlow() const
{
    vector<ContractorID> result;
    for (auto const &nodeIDAndTrustLine : mTrustLines) {
        if (nodeIDAndTrustLine.second->state() != TrustLine::Active) {
            continue;
        }
        auto trustLineAmountShared = outgoingTrustAmountConsideringReservations(
            nodeIDAndTrustLine.first);
        auto trustLineAmountPtr = trustLineAmountShared.get();
        if (*trustLineAmountPtr > TrustLine::kZeroAmount()) {
            result.push_back(
                nodeIDAndTrustLine.first);
        }
    }
    return result;
}

vector<ContractorID> TrustLinesManager::firstLevelGatewayNeighborsWithOutgoingFlow() const
{
    vector<ContractorID> result;
    for (auto const &nodeIDAndTrustLine : mTrustLines) {
        if (nodeIDAndTrustLine.second->state() != TrustLine::Active) {
            continue;
        }
        if (!nodeIDAndTrustLine.second->isContractorGateway()) {
            continue;
        }
        auto trustLineAmountShared = outgoingTrustAmountConsideringReservations(
            nodeIDAndTrustLine.first);
        auto trustLineAmountPtr = trustLineAmountShared.get();
        if (*trustLineAmountPtr > TrustLine::kZeroAmount()) {
            result.push_back(
                nodeIDAndTrustLine.first);
        }
    }
    return result;
}

vector<ContractorID> TrustLinesManager::firstLevelNeighborsWithIncomingFlow() const
{
    vector<ContractorID> result;
    for (auto const &nodeIDAndTrustLine : mTrustLines) {
        if (nodeIDAndTrustLine.second->state() != TrustLine::Active) {
            continue;
        }
        auto trustLineAmountShared = incomingTrustAmountConsideringReservations(
            nodeIDAndTrustLine.first);
        auto trustLineAmountPtr = trustLineAmountShared.get();

        if (*trustLineAmountPtr > TrustLine::kZeroAmount()) {
            result.push_back(
                nodeIDAndTrustLine.first);
        }
    }
    return result;
}

vector<ContractorID> TrustLinesManager::firstLevelGatewayNeighborsWithIncomingFlow() const
{
    vector<ContractorID> result;
    for (auto const &nodeIDAndTrustLine : mTrustLines) {
        if (!nodeIDAndTrustLine.second->isContractorGateway()) {
            continue;
        }
        auto trustLineAmountShared = incomingTrustAmountConsideringReservations(
            nodeIDAndTrustLine.first);
        auto trustLineAmountPtr = trustLineAmountShared.get();

        if (*trustLineAmountPtr > TrustLine::kZeroAmount()) {
            result.push_back(nodeIDAndTrustLine.first);
        }
    }
    return result;
}

vector<ContractorID> TrustLinesManager::firstLevelNonGatewayNeighborsWithIncomingFlow() const
{
    vector<ContractorID> result;
    for (auto const &nodeIDAndTrustLine : mTrustLines) {
        if (nodeIDAndTrustLine.second->state() != TrustLine::Active) {
            continue;
        }
        if (nodeIDAndTrustLine.second->isContractorGateway()) {
            continue;
        }
        auto trustLineAmountShared = incomingTrustAmountConsideringReservations(
            nodeIDAndTrustLine.first);
        auto trustLineAmountPtr = trustLineAmountShared.get();

        if (*trustLineAmountPtr > TrustLine::kZeroAmount()) {
            result.push_back(nodeIDAndTrustLine.first);
        }
    }
    return result;
}

vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> TrustLinesManager::incomingFlows() const
{
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> result;
    for (auto const &nodeIDAndTrustLine : mTrustLines) {
        if (nodeIDAndTrustLine.second->state() != TrustLine::Active) {
            continue;
        }
        auto trustLineAmountShared = incomingTrustAmountConsideringReservations(
            nodeIDAndTrustLine.first);
        result.emplace_back(
            mContractorsManager->contractorMainAddress(nodeIDAndTrustLine.first),
            trustLineAmountShared);
    }
    return result;
}

vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> TrustLinesManager::outgoingFlows() const
{
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> result;
    for (auto const &nodeIDAndTrustLine : mTrustLines) {
        if (nodeIDAndTrustLine.second->state() != TrustLine::Active) {
            continue;
        }
        auto trustLineAmountShared = outgoingTrustAmountConsideringReservations(
            nodeIDAndTrustLine.first);
        result.emplace_back(
            mContractorsManager->contractorMainAddress(nodeIDAndTrustLine.first),
            trustLineAmountShared);
    }
    return result;
}

pair<BaseAddress::Shared, ConstSharedTrustLineAmount> TrustLinesManager::incomingFlow(
    ContractorID contractorID) const
{
    return make_pair(
        mContractorsManager->contractorMainAddress(contractorID),
        incomingTrustAmountConsideringReservations(
            contractorID));
}

pair<BaseAddress::Shared, ConstSharedTrustLineAmount> TrustLinesManager::outgoingFlow(
    ContractorID contractorID) const
{
    return make_pair(
        mContractorsManager->contractorMainAddress(contractorID),
        outgoingTrustAmountConsideringReservations(
            contractorID));
}

vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> TrustLinesManager::incomingFlowsFromNonGateways() const
{
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> result;
    for (auto const &nodeIDAndTrustLine : mTrustLines) {
        if (nodeIDAndTrustLine.second->state() != TrustLine::Active) {
            continue;
        }
        if (nodeIDAndTrustLine.second->isContractorGateway()) {
            continue;
        }
        auto trustLineAmountShared = incomingTrustAmountConsideringReservations(
            nodeIDAndTrustLine.first);
        result.emplace_back(
            mContractorsManager->contractorMainAddress(nodeIDAndTrustLine.first),
            trustLineAmountShared);
    }
    return result;
}

vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> TrustLinesManager::incomingFlowsFromGateways() const
{
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> result;
    for (auto const &nodeIDAndTrustLine : mTrustLines) {
        if (!nodeIDAndTrustLine.second->isContractorGateway()) {
            continue;
        }
        auto trustLineAmountShared = incomingTrustAmountConsideringReservations(
            nodeIDAndTrustLine.first);
        result.emplace_back(
            mContractorsManager->contractorMainAddress(nodeIDAndTrustLine.first),
            trustLineAmountShared);
    }
    return result;
}

vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> TrustLinesManager::outgoingFlowsToGateways() const
{
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> result;
    for (auto const &nodeIDAndTrustLine : mTrustLines) {
        if (nodeIDAndTrustLine.second->state() != TrustLine::Active) {
            continue;
        }
        if (!nodeIDAndTrustLine.second->isContractorGateway()) {
            continue;
        }
        auto trustLineAmountShared = outgoingTrustAmountConsideringReservations(
            nodeIDAndTrustLine.first);
        result.emplace_back(
            mContractorsManager->contractorMainAddress(nodeIDAndTrustLine.first),
            trustLineAmountShared);
    }
    return result;
}

vector<ContractorID> TrustLinesManager::gateways() const
{
    vector<ContractorID> result;
    for (auto const &nodeIDAndTrustLine : mTrustLines) {
        if (nodeIDAndTrustLine.second->isContractorGateway()) {
            result.push_back(nodeIDAndTrustLine.first);
        }
    }
    return result;
}

vector<ContractorID> TrustLinesManager::firstLevelNeighbors() const
{
    vector<ContractorID> result;
    result.reserve(mTrustLines.size());
    for (auto &nodeIDAndTrustLine : mTrustLines) {
        if (nodeIDAndTrustLine.second->state() == TrustLine::Active) {
            result.push_back(
                nodeIDAndTrustLine.first);
        }
    }
    return result;
}

vector<BaseAddress::Shared> TrustLinesManager::firstLevelNeighborsAddresses() const
{
    vector<BaseAddress::Shared> result;
    result.reserve(mTrustLines.size());
    for (auto &nodeIDAndTrustLine : mTrustLines) {
        if (nodeIDAndTrustLine.second->state() != TrustLine::Archived) {
            result.push_back(
                mContractorsManager->contractorMainAddress(
                    nodeIDAndTrustLine.first));
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
    ContractorID contractorID) const
{
    if (trustLineIsPresent(contractorID)) {
        // Since c++11, a return value is an rvalue.
        //
        // -> mTrustLines.at(contractorID)
        //
        // In this case, there will be no shared_ptr copy done due to RVO.
        // But the copy is strongly needed here. Otherwise, moved shared_ptr would
        // try to free the memory, that is also used by the shared_ptr in the map.
        // As a result - map would be corrupted.
        const auto temp = const_pointer_cast<const TrustLine>(
            mTrustLines.at(contractorID));
        return temp;

    } else {
        throw NotFoundError(
            logHeader() + "::trustLineReadOnly: " + to_string(contractorID) +
                " Trust line to such an contractor does not exists.");
    }
}

unordered_map<ContractorID, TrustLine::Shared> &TrustLinesManager::trustLines()
{
    return mTrustLines;
}

vector<ContractorID> TrustLinesManager::getFirstLevelNodesForCycles(
    bool isCreditorBranch)
{
    vector<ContractorID> nodes;
    TrustLineBalance stepBalance;
    for (auto const& nodeAndTrustLine : mTrustLines) {
        if (nodeAndTrustLine.second->state() != TrustLine::Active) {
            continue;
        }
        stepBalance = nodeAndTrustLine.second->balance();
        if (isCreditorBranch and stepBalance > TrustLine::kZeroBalance()) {
            nodes.push_back(nodeAndTrustLine.first);
        } else if (!isCreditorBranch and stepBalance < TrustLine::kZeroBalance()) {
            nodes.push_back(nodeAndTrustLine.first);
        }
    }
    return nodes;
}

vector<ContractorID> TrustLinesManager::firstLevelNeighborsWithPositiveBalance() const
{
    vector<ContractorID> nodes;
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

vector<ContractorID> TrustLinesManager::firstLevelNeighborsWithNegativeBalance() const
{
    // todo change vector to set
    vector<ContractorID> nodes;
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

vector<ContractorID> TrustLinesManager::firstLevelNeighborsWithNoneZeroBalance() const
{
    vector<ContractorID> nodes;
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
    ContractorID contractorID,
    const AmountReservation::ConstShared reservation)
{
    if (not trustLineIsPresent(contractorID)) {
        throw NotFoundError(
            logHeader() + "::useReservation: "
                "No trust line is present with the contractor " + to_string(contractorID));
    }

    switch (reservation->direction()) {
        case AmountReservation::Outgoing: {
            mTrustLines[contractorID]->pay(reservation->amount());
            return;
        }

        case AmountReservation::Incoming: {
            mTrustLines[contractorID]->acceptPayment(reservation->amount());
            return;
        }

        default: {
            throw ValueError(
                logHeader() + "::useReservation: "
                    "Unexpected trust line direction occurred.");
        }
    }
}

ConstSharedTrustLineAmount TrustLinesManager::totalOutgoingAmount() const
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
    ContractorID contractorID) const
{
    return mAmountReservationsHandler->contractorReservations(
        contractorID,
        AmountReservation::ReservationDirection::Outgoing);
}

vector<AmountReservation::ConstShared> TrustLinesManager::reservationsFromContractor(
    ContractorID contractorID) const
{
    return mAmountReservationsHandler->contractorReservations(
        contractorID,
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
    ContractorID contractorID,
    bool isActionInitiator)
{
    info() << "checkTrustLineAfterTransaction";
    if (not trustLineIsPresent(contractorID)) {
        throw NotFoundError(
            logHeader() + "::checkTrustLineAfterTransaction: "
                "No trust line with the contractor is present " + to_string(contractorID));
    }
    auto ioTransaction = mStorageHandler->beginTransaction();
    if (isTrustLineEmpty(contractorID)) {
        info() << "TL become empty";
        // if TL become empty, it is necessary to run Audit TA.
        // AuditSource TA run on node which pay
        if (isActionInitiator) {
            info() << "Audit signal";
            return TrustLineActionType::Audit;
        } else {
            return TrustLineActionType::NoActions;
        }
    } else if (isTrustLineOverflowed(contractorID)) {
        // todo : add audit rules
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
                contractorID));
        if (keyChain.ownKeysCriticalCount(ioTransaction)) {
            setIsOwnKeysPresent(
                contractorID,
                false);
            info() << "Public key sharing signal";
            return TrustLineActionType::KeysSharing;
        }
    }
    return TrustLineActionType::NoActions;
}

vector<ContractorID> TrustLinesManager::contractorsShouldBePinged() const
{
    return mContractorsShouldBePinged;
}

void TrustLinesManager::clearContractorsShouldBePinged()
{
    mContractorsShouldBePinged.clear();
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

LoggerStream TrustLinesManager::warning() const
    noexcept
{
    return mLogger.warning(logHeader());
}

void TrustLinesManager::printTLs()
{
    LoggerStream debug = mLogger.debug("TrustLinesManager::printTLs");
    auto ioTransaction = mStorageHandler->beginTransaction();
    debug << "printTLs\t size: " << trustLines().size() << endl;
    for (const auto &itTrustLine : trustLines()) {
        debug << itTrustLine.second->contractorID() << " "
              << itTrustLine.second->state() << " "
              << itTrustLine.second->incomingTrustAmount() << " "
              << itTrustLine.second->outgoingTrustAmount() << " "
              << itTrustLine.second->balance() << " "
              << itTrustLine.second->isContractorGateway() << endl;
    }
}

void TrustLinesManager::printTLFlows()
{
    LoggerStream debug = mLogger.debug("TrustLinesManager::printTLFlows");
    auto ioTransaction = mStorageHandler->beginTransaction();
    debug << "print payment incoming flows size: " << incomingFlows().size() << endl;
    for (auto const &itIncomingFlow : incomingFlows()) {
        debug << itIncomingFlow.first->fullAddress() << " " << *itIncomingFlow.second.get() << endl;
    }
    debug << "print payment outgoing flows size: " << outgoingFlows().size() << endl;
    for (auto const &itOutgoingFlow : outgoingFlows()) {
        debug << itOutgoingFlow.first->fullAddress() << " " << *itOutgoingFlow.second.get() << endl;
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
