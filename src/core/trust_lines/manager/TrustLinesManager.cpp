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

void TrustLinesManager::loadTrustLinesFromDisk ()
    throw (IOError)
{
    auto ioTransaction = mStorageHandler->beginTransaction();
    const auto kTrustLines = ioTransaction->trustLineHandler()->allTrustLines();
    mTrustLines.reserve(kTrustLines.size());

    for (auto const kTrustLine : kTrustLines) {
        mTrustLines.insert(
            make_pair(
                kTrustLine->contractorNodeUUID(),
                kTrustLine));
    }
}

/**
 * @throws ConflictError
 * @throws bad_alloc
 */
void TrustLinesManager::open(
    IOTransaction::Shared IOTransaction,
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount)
{
    const auto kZeroAmount = TrustLineAmount(0);
    if (amount <= kZeroAmount)
        throw ValueError(
            "TrustLinesManager::open: "
                "can't open trust line with zero amount");

    if (trustLineIsPresent(contractorUUID) and outgoingTrustAmount(contractorUUID) > 0) {
        throw ConflictError(
            "TrustLinesManager::open: "
            "can't open outgoing trust line. There is an already present one.");

    }

    if (not trustLineIsPresent(contractorUUID)) {
        auto trustLine = make_shared<TrustLine>(
            contractorUUID, 0, amount, 0);

        mTrustLines[contractorUUID] = trustLine;
        saveToDisk(
            IOTransaction,
            trustLine);
        return;
    }

    if (outgoingTrustAmount(contractorUUID) == 0) {
        auto trustLine = mTrustLines[contractorUUID];
        trustLine->setOutgoingTrustAmount(amount);
        saveToDisk(
            IOTransaction,
            trustLine);
        return;
    }
}

/**
 * @throws NotFoundError
 * @throws IOError
 */
void TrustLinesManager::close(
    IOTransaction::Shared IOTransaction,
    const NodeUUID &contractorUUID)
{
    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            "TrustLinesManager::close: "
            "trust line doesn't exist.");
    }

    auto trustLine = mTrustLines.find(contractorUUID)->second;
    if (trustLine->outgoingTrustAmount() == TrustLine::kZeroAmount()) {
        throw NotFoundError(
            "TrustLinesManager::close: "
                "can't close outgoing trust line: outgoing amount equals to zero. "
                "It seems that trust line has been already closed. ");
    }


    trustLine->setOutgoingTrustAmount(0);
    if (trustLine->incomingTrustAmount() == 0 and trustLine->balance() == 0) {
        removeTrustLine(
            IOTransaction,
            contractorUUID);

    } else {
        saveToDisk(
            IOTransaction,
            trustLine);
    }
}

/**
 * throw ConflictError - trust line is already exist
 * throw MemoryError - can not allocate memory for trust line instance
 */
void TrustLinesManager::accept(
    IOTransaction::Shared IOTransaction,
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount) {

    const auto kZeroAmount = TrustLineAmount(0);
    if (amount <= kZeroAmount)
        throw ValueError(
            "TrustLinesManager::open: "
                "can't open trust line with zero amount");

    if (trustLineIsPresent(contractorUUID) and incomingTrustAmount(contractorUUID) > 0) {
        throw ConflictError(
            "TrustLinesManager::open: "
                "can't open incoming trust line. There is an already present one.");

    }

    if (not trustLineIsPresent(contractorUUID)) {
        auto trustLine = make_shared<TrustLine>(
            contractorUUID, amount, 0, 0);

        mTrustLines[contractorUUID] = trustLine;
        saveToDisk(
            IOTransaction,
            trustLine);
        return;
    }

    if (incomingTrustAmount(contractorUUID) == 0) {
        auto trustLine = mTrustLines[contractorUUID];
        trustLine->setIncomingTrustAmount(amount);
        saveToDisk(
            IOTransaction,
            trustLine);
        return;
    }
}

/**
 * throw PreconditionFailedError - user already used part of amount
 * throw ValueError - trust amount less or equals by zero
 * throw NotFoundError - trust line does not exist
 */
void TrustLinesManager::reject(
    IOTransaction::Shared IOTransaction,
    const NodeUUID &contractorUUID) {

    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            "TrustLinesManager::close: "
                "trust line doesn't exist.");
    }

    auto trustLine = mTrustLines.find(contractorUUID)->second;
    if (trustLine->incomingTrustAmount() == TrustLine::kZeroAmount()) {
        throw NotFoundError(
            "TrustLinesManager::close: "
                "can't close incoming trust line: incoming amount equals to zero. "
                "It seems that trust line has been already closed. ");
    }

    trustLine->setIncomingTrustAmount(0);
    if (trustLine->outgoingTrustAmount() == 0 and trustLine->balance() == 0) {
        removeTrustLine(
            IOTransaction,
            contractorUUID);

    } else {
        saveToDisk(
            IOTransaction,
            trustLine);
    }
}

void TrustLinesManager::set(
    IOTransaction::Shared IOTransaction,
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount)
{
    const auto kZeroAmount = TrustLineAmount(0);
    if (amount <= kZeroAmount)
        throw ValueError(
            "TrustLinesManager::set: "
                "can't set trust line amount to zero");

    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            "TrustLinesManager::set: "
                "trust line doesn't exist.");
    }

    if (outgoingTrustAmount(contractorUUID) == 0) {
        throw NotFoundError(
            "TrustLinesManager::set: "
                "can't update outgoing trust line. Trust line is closed.");

    }

    auto trustLine = mTrustLines[contractorUUID];
    trustLine->setOutgoingTrustAmount(amount);
    saveToDisk(
        IOTransaction,
        trustLine);
    return;
}

void TrustLinesManager::update(
    IOTransaction::Shared IOTransaction,
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount)
{
    const auto kZeroAmount = TrustLineAmount(0);
    if (amount <= kZeroAmount)
        throw ValueError(
            "TrustLinesManager::update: "
                "can't set trust line amount to zero");

    if (not trustLineIsPresent(contractorUUID)) {
        throw NotFoundError(
            "TrustLinesManager::update: "
                "trust line doesn't exist.");
    }

    if (incomingTrustAmount(contractorUUID) == 0) {
        throw NotFoundError(
            "TrustLinesManager::update: "
                "can't update outgoing trust line. Trust line is closed.");
    }

    auto trustLine = mTrustLines[contractorUUID];
    trustLine->setIncomingTrustAmount(amount);
    saveToDisk(
        IOTransaction,
        trustLine);
    return;
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

ConstSharedTrustLineAmount TrustLinesManager::availableOutgoingCycleAmount(
    const NodeUUID &contractor) const
{
    const auto kTL = trustLineReadOnly(contractor);
    const auto kBalance = kTL->balance();
    if (kBalance <= TrustLine::kZeroBalance()) {
        return make_shared<const TrustLineAmount>(0);
    }

    const auto kAlreadyReservedAmount = mAmountReservationsHandler->totalReserved(
        contractor, AmountReservation::Outgoing);

    if (*kAlreadyReservedAmount.get() == TrustLine::kZeroAmount()) {
        return make_shared<const TrustLineAmount>(kBalance);
    }

    auto kAbsoluteBalance = absoluteBalanceAmount(kBalance);
    if (*kAlreadyReservedAmount.get() > kAbsoluteBalance) {
        return make_shared<const TrustLineAmount>(0);
    } else {
        return make_shared<const TrustLineAmount>(
            kAbsoluteBalance - *kAlreadyReservedAmount.get());
    }
}

ConstSharedTrustLineAmount TrustLinesManager::availableIncomingCycleAmount(
    const NodeUUID& contractor) const
{
    const auto kTL = trustLineReadOnly(contractor);
    const auto kBalance = kTL->balance();
    if (kBalance >= TrustLine::kZeroBalance()) {
        return make_shared<const TrustLineAmount>(0);
    }

    const auto kAlreadyReservedAmount = mAmountReservationsHandler->totalReserved(
        contractor, AmountReservation::Incoming);

    auto kAbsoluteBalance = absoluteBalanceAmount(kBalance);
    if (*kAlreadyReservedAmount.get() == TrustLine::kZeroAmount()) {
        return make_shared<const TrustLineAmount>(kAbsoluteBalance);
    }

    if (*kAlreadyReservedAmount.get() >= kAbsoluteBalance) {
        return make_shared<const TrustLineAmount>(0);
    }
    return make_shared<const TrustLineAmount>(
        kAbsoluteBalance - *kAlreadyReservedAmount.get());
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

    IOTransaction->trustLineHandler()->saveTrustLine(trustLine);
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
    ioTransaction->trustLineHandler()->saveTrustLine(trustLine);
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

    IOTransaction->trustLineHandler()->deleteTrustLine(contractorUUID);
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
        ioTransaction->trustLineHandler()->deleteTrustLine(
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
        debug << trLine.first << " " << *availableIncomingCycleAmount(trLine.first) << endl;
    }
    debug << "print cycle outgoing flows size: " << outgoingFlows().size() << endl;
    for (auto const trLine : mTrustLines) {
        debug << trLine.first << " " << *availableOutgoingCycleAmount(trLine.first) << endl;
    }
}

uint32_t TrustLinesManager::crc32SumFirstLevel(const NodeUUID &contractorUUID) {
    boost::crc_32_type result;
//    set<NodeUUID> firstLevelContractors;
//    stringstream ss;
//    for(const auto kNodeUUIDAndTrustline: mTrustLines)
//        if(kNodeUUIDAndTrustline.first != contractorUUID)
//            firstLevelContractors.insert(kNodeUUIDAndTrustline.first);
//    for(const auto kNodeUUID: firstLevelContractors)
//        ss << kNodeUUID;
//    result.process_bytes(ss.str().data(), ss.str().length());
    return result.checksum();
}

uint32_t TrustLinesManager::crc32SumSecondLevel(const NodeUUID &contractorUUID) {
    boost::crc_32_type result;
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto secondLevelContractors = ioTransaction->routingTablesHandler()->neighborsOfOnRT2(contractorUUID);
    stringstream ss;
    for(const auto kNodeUUID: secondLevelContractors)
        ss << kNodeUUID;
    result.process_bytes(ss.str().data(), ss.str().length());
    return result.checksum();
}

