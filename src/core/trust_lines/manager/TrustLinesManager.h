#ifndef GEO_NETWORK_CLIENT_TRUSTLINESMANAGER_H
#define GEO_NETWORK_CLIENT_TRUSTLINESMANAGER_H

#include "../TrustLine.h"

#include "../../common/NodeUUID.h"
#include "../../common/Types.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/ValueError.h"
#include "../../common/exceptions/MemoryError.h"
#include "../../common/exceptions/ConflictError.h"
#include "../../common/exceptions/NotFoundError.h"
#include "../../common/exceptions/PreconditionFailedError.h"
#include "../../logger/Logger.h"
#include "../../payments/reservations/AmountReservationsHandler.h"
#include "../audit_rules/AuditRuleCountPayments.h"

// TODO: remove storage handler include (IO transactions must be transferred as arguments)
#include "../../io/storage/StorageHandler.h"
#include "../../io/storage/IOTransaction.h"
#include "../../crypto/keychain.h"

#include <boost/crc.hpp>
#include <boost/signals2.hpp>
#include <boost/functional/hash.hpp>

#include <unordered_map>
#include <vector>
#include <set>
#include <algorithm>

#ifdef MAC_OS
#include <stdlib.h>
#endif

#ifdef LINUX
#include <malloc.h>
#endif


using namespace std;
namespace signals = boost::signals2;


class TrustLinesManager {
public:
    enum TrustLineOperationResult {
        Opened,
        Updated,
        Closed,
        NoChanges,
    };

    enum TrustLineActionType {
        NoActions,
        Audit,
        KeysSharing,
    };

public:
    typedef signals::signal<void(const NodeUUID&)> PingMessageSignal;

public:
    TrustLinesManager(
        const SerializedEquivalent equivalent,
        StorageHandler *storageHandler,
        Keystore *keyStore,
        Logger &logger);

    void open(
        const NodeUUID &contractorUUID,
        IOTransaction::Shared ioTransaction = nullptr);

    void accept(
        const NodeUUID &contractorUUID,
        IOTransaction::Shared ioTransaction = nullptr);

    /**
     * Creates / Updates / Closes trust line TO the contractor.
     *
     * Creates new trust line in case if no trust line to the contractor is present.
     * Updates trust line with new amount if it's already present.
     * In case if new amount is equal to previously set amount - "NoChanges" result would be returned.
     * Closes the trust line if it's present and received amount = 0.
     *
     * @returns final state of the operation.
     *
     * @throws ValueError in case if amount is 0 and no trust line to this contractor is present.
     * @throws might throw exceptions of the closeOutgoing() method.
     */
    TrustLineOperationResult setOutgoing(
        const NodeUUID &contractorUUID,
        const TrustLineAmount &amount);

    /**
     * Creates / Updates / Closes trust line FROM the contractor.
     *
     * Creates new trust line in case if no trust line from the contractor is present.
     * Updates trust line with new amount if it's already present.
     * In case if ne wamount is equal to previously set amount - "NoChanges" result would be returned.
     * Closes the trust line if it's present and received amount = 0.
     *
     * @returns final state of the operation.
     *
     * @throws ValueError in case if amount is 0 and no trust line to this contractor is present.
     * @throws might throw exceptions of the closeIncoming() method.
     */
    TrustLineOperationResult setIncoming(
        const NodeUUID &contractorUUID,
        const TrustLineAmount &amount);

    /**
     * Closes outgoing trust line TO the contractor.
     *
     * @throws NotFoundError in case if no trust line to this contractor is present.
     * @throws ConflictError in case if there are some reservations on the trust line,
     *         and it can't be removed at this moment.
     */
    void closeOutgoing(
        const NodeUUID &contractorUUID);

    /**
     * Closes incoming trust line FROM the contractor.
     *
     * @throws NotFoundError in case if no trust line to this contractor is present.
     * @throws ConflictError in case if there are some reservations on the trust line,
     *         and it can't be removed at this moment.
     */
    void closeIncoming(
        const NodeUUID &contractorUUID);

    /**
     * Set contractor as Gateway if contractorIsGateway equals true.
     *
     * @throws NotFoundError in case if no trust line to this contractor is present.
     */
    void setContractorAsGateway(
        IOTransaction::Shared ioTransaction,
        const NodeUUID &contractorUUID,
        bool contractorIsGateway);

    void setIsOwnKeysPresent(
        const NodeUUID &contractorUUID,
        bool isOwnKeysPresent);

    void setIsContractorKeysPresent(
        const NodeUUID &contractorUUID,
        bool isContractorKeysPresent);

    void setTrustLineState(
        const NodeUUID &contractorUUID,
        TrustLine::TrustLineState state,
        IOTransaction::Shared ioTransaction = nullptr);

    void setTrustLineAuditNumber(
        const NodeUUID &contractorUUID,
        AuditNumber newAuditNumber);

    /**
     * @returns outgoing trust amount without considering present reservations.
     *
     * @throws NotFoundError in case if no trust line from this contractor is present.
     */
    const TrustLineAmount &outgoingTrustAmount(
        const NodeUUID &contractorUUID) const;

    /**
     * @returns incoming trust amount without considering present reservations.
     *
     * @throws NotFoundError in case if no trust line from this contractor is present.
     */
    const TrustLineAmount &incomingTrustAmount(
        const NodeUUID &contractorUUID) const;

    /**
     * @returns current balance on the trust line.
     *
     * @throws NotFoundError in case if no trust line from this contractor is present.
     */
    const TrustLineBalance &balance(
        const NodeUUID &contractorUUID) const;

    const TrustLineID trustLineID(
        const NodeUUID &contractorUUID) const;

    const AuditNumber auditNumber(
        const NodeUUID &contractorUUID) const;

    const TrustLine::TrustLineState trustLineState(
        const NodeUUID &contractorUUID) const;

    bool trustLineOwnKeysPresent(
        const NodeUUID &contractorUUID) const;

    bool trustLineContractorKeysPresent(
        const NodeUUID &contractorUUID) const;

    /**
     * Reserves payment amount TO the contractor.
     *
     * @param contractor - uuid of the contractor, trust amount to which should be reserved.
     * @param transactionUUID - uuid of the transaction, which reserves the amount.
     * @param amount
     *
     * @throws NotFoundError in case if no trust line is present.
     * @throws ValueError in case, if trust line hasn't enough free amount;
     * @throws ValueError in case, if outgoing trust amount == 0;
     */
    AmountReservation::ConstShared reserveOutgoingAmount(
        const NodeUUID &contractor,
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &amount);

    /**
     * Reserves payment amount FROM the contractor.
     *
     * @param contractor - uuid of the contractor, trust amount from which should be reserved.
     * @param transactionUUID - uuid of the transaction, which reserves the amount.
     * @param amount
     *
     * @throws NotFoundError in case if no trust line is present.
     * @throws ValueError in case, if trust line hasn't enough free amount;
     * @throws ValueError in case, if incoming trust amount == 0;
     */
    AmountReservation::ConstShared reserveIncomingAmount(
        const NodeUUID &contractor,
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &amount);

    /**
     * Updates present reservation with new amount.
     *
     * @throws ValueError in case if trust line has not enough free amount.
     * @throws NotFoundError in case if previous reservations was not found.
     */
    AmountReservation::ConstShared updateAmountReservation(
        const NodeUUID &contractor,
        const AmountReservation::ConstShared reservation,
        const TrustLineAmount &newAmount);

    /**
     * Removes present reservation.
     *
     * @throws NotFoundError in case if previous reservations was not found.
     * @throws IOError in case if attempt to remove trust line (if needed) wasn't successful.
     */
    void dropAmountReservation(
        const NodeUUID &contractor,
        const AmountReservation::ConstShared reservation);

    /**
     * Converts reservation on the trust line to the real used amount.
     *
     * @throws OverflowError in case of attempt to use more, than available amount on the trust line.
     * @throws NotFoundError in case if no trust line with contractor is present.
     */
    void useReservation(
        const NodeUUID &contractor,
        const AmountReservation::ConstShared reservation);

    /**
     * @returns outgoing trust amount with total reserved amount EXCLUDED.
     *
     * @throws NotFoundError in case if no trust line to this contractor is present.
     */
    ConstSharedTrustLineAmount outgoingTrustAmountConsideringReservations(
        const NodeUUID &contractor) const;

    /**
     * @returns incoming trust amount with total reserved amount EXCLUDED.
     *
     * @throws NotFoundError in case if no trust line from this contractor is present.
     */
    ConstSharedTrustLineAmount incomingTrustAmountConsideringReservations(
        const NodeUUID &contractor) const;

    //ToDo: comment this method
    // available outgoing amount considering reservations for cycles
    // returns 2 values: 1) amount considering reservations, 2) amount don't considering reservations
    pair<ConstSharedTrustLineAmount, ConstSharedTrustLineAmount> availableOutgoingCycleAmounts(
        const NodeUUID &contractor) const;

    //ToDo: comment this method
    // available incoming amount considering reservations for cycles
    // returns 2 values: 1) amount considering reservations, 2) amount don't considering reservations
    pair<ConstSharedTrustLineAmount, ConstSharedTrustLineAmount> availableIncomingCycleAmounts(
        const NodeUUID &contractor) const;

    /**
     * @returns total summary of all outgoing possibilities of the node.
     */
    ConstSharedTrustLineAmount totalOutgoingAmount()
        const;

    /**
     * @returns total summary of all incoming possibilities of the node.
     */
    ConstSharedTrustLineAmount totalIncomingAmount()
        const;

    // get all reservations (all transactions) to requested contractor
    vector<AmountReservation::ConstShared> reservationsToContractor(
        const NodeUUID &contractorUUID) const;

    // get all reservations (all transactions) from requested contractor
    vector<AmountReservation::ConstShared> reservationsFromContractor(
        const NodeUUID &contractorUUID) const;

    bool isReservationsPresentOnTrustLine(
        const NodeUUID &contractorUUID) const;

    const bool trustLineIsPresent (
        const NodeUUID &contractorUUID) const;

    const bool trustLineIsActive(
        const NodeUUID &contractorUUID) const;

    void updateTrustLine(
        IOTransaction::Shared ioTransaction,
        TrustLine::Shared trustLine);

    void updateTrustLineFromStorage(
        const NodeUUID &contractorUUID,
        IOTransaction::Shared ioTransaction);

    void removeTrustLine(
        const NodeUUID &contractorUUID,
        IOTransaction::Shared ioTransaction = nullptr);

    bool isTrustLineEmpty(
        const NodeUUID &contractorUUID);

    bool isTrustLineOverflowed(
        const NodeUUID &contractorUUID);

    void resetTrustLineTotalReceiptsAmounts(
        const NodeUUID &contractorUUID);

    vector<NodeUUID> firstLevelNeighborsWithOutgoingFlow() const;

    vector<NodeUUID> firstLevelGatewayNeighborsWithOutgoingFlow() const;

    vector<NodeUUID> firstLevelNeighborsWithIncomingFlow() const;

    vector<NodeUUID> firstLevelNonGatewayNeighborsWithIncomingFlow() const;

    vector<NodeUUID> firstLevelNeighborsWithPositiveBalance() const;

    vector<NodeUUID> firstLevelNeighborsWithNegativeBalance() const;

    vector<NodeUUID> firstLevelNeighborsWithNoneZeroBalance() const;

    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlows() const;

    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlows() const;

    pair<NodeUUID, ConstSharedTrustLineAmount> incomingFlow(
        const NodeUUID &contractorUUID) const;

    pair<NodeUUID, ConstSharedTrustLineAmount> outgoingFlow(
        const NodeUUID &contractorUUID) const;

    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlowsFromNonGateways() const;

    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlowsToGateways() const;

    vector<NodeUUID> gateways() const;

    vector<NodeUUID> firstLevelNeighbors() const;

    // total balance to all 1st level neighbors
    ConstSharedTrustLineBalance totalBalance() const;

    const TrustLine::ConstShared trustLineReadOnly(
        const NodeUUID &contractorUUID) const;

    unordered_map<NodeUUID, TrustLine::Shared, boost::hash<boost::uuids::uuid>>& trustLines();

    vector<NodeUUID> getFirstLevelNodesForCycles(
        TrustLineBalance maxFlow);

    TrustLineActionType checkTrustLineAfterTransaction(
        const NodeUUID &contractorUUID,
        bool isActionInitiator);

    // TODO remove after testing
    void printRTs();

protected:
    void saveToStorage(
        IOTransaction::Shared ioTransaction,
        TrustLine::Shared trustLine);

    /**
     * Reads trust lines info from the internal storage and initialises internal trust lines map.
     * Ignores obsolete trust lines (outgoing 0, incoming 0, and balance 0).
     *
     * @throws IOError in case of storage read error.
     */
    void loadTrustLinesFromStorage();
    
    const TrustLineID nextFreeID(
        IOTransaction::Shared ioTransaction) const;

protected: // log shortcuts
    const string logHeader() const
        noexcept;

    LoggerStream info() const
        noexcept;

    LoggerStream warning() const
        noexcept;

public:
    mutable PingMessageSignal pingMessageSignal;

private:
    unordered_map<NodeUUID, TrustLine::Shared, boost::hash<boost::uuids::uuid>> mTrustLines;
    SerializedEquivalent mEquivalent;

    unordered_map<NodeUUID, BaseAuditRule::Shared, boost::hash<boost::uuids::uuid>> mAuditRules;

    unique_ptr<AmountReservationsHandler> mAmountReservationsHandler;
    StorageHandler *mStorageHandler;
    Keystore *mKeysStore;
    Logger &mLogger;
};

#endif //GEO_NETWORK_CLIENT_TRUSTLINESMANAGER_H
