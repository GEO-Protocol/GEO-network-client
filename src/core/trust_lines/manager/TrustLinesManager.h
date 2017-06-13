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
#include "../../common/NodeUUID.h"

#include "../../logger/Logger.h"
#include "../../payments/amount_blocks/AmountReservationsHandler.h"

// TODO: remove storage handler include (IO transactions must be transferred as arguments)
#include "../../io/storage/StorageHandler.h"
#include "../../io/storage/IOTransaction.h"

#include <boost/crc.hpp>
#include <boost/signals2.hpp>
#include <boost/functional/hash.hpp>

#include <unordered_map>
#include <vector>

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
    // todo: rename signals so tehm would start with "Signal", i.e. SignalTrustLineCreated
    signals::signal<void(const NodeUUID&, const TrustLineDirection)> trustLineCreatedSignal;
    signals::signal<void(const NodeUUID&, const TrustLineDirection)> trustLineStateModifiedSignal;

public:
    TrustLinesManager(
        StorageHandler *storageHandler,
        Logger &logger)
        throw (bad_alloc, IOError);

    void open(
        IOTransaction::Shared IOTransaction,
        const NodeUUID &contractorUUID,
        const TrustLineAmount &amount);

    void close(
        IOTransaction::Shared IOTransaction,
        const NodeUUID &contractorUUID);

    void accept(
        IOTransaction::Shared IOTransaction,
        const NodeUUID &contractorUUID,
        const TrustLineAmount &amount);

    void reject(
        IOTransaction::Shared IOTransaction,
        const NodeUUID &contractorUUID);

    void set(
        IOTransaction::Shared IOTransaction,
        const NodeUUID &contractorUUID,
        const TrustLineAmount &amount);

    void update(
        IOTransaction::Shared IOTransaction,
        const NodeUUID &contractorUUID,
        const TrustLineAmount &amount);

    const bool checkDirection(
        const NodeUUID &contractorUUID,
        const TrustLineDirection direction) const;

    const BalanceRange balanceRange(
        const NodeUUID &contractorUUID) const;

    void setIncomingTrustAmount(
        const NodeUUID &contractor,
        const TrustLineAmount &amount);

    void setOutgoingTrustAmount(
        const NodeUUID &contractor,
        const TrustLineAmount &amount);

    const TrustLineAmount &incomingTrustAmount(
        const NodeUUID &contractorUUID);

    const TrustLineAmount &outgoingTrustAmount(
        const NodeUUID &contractorUUID);

    const TrustLineBalance &balance(
        const NodeUUID &contractorUUID);

    AmountReservation::ConstShared reserveOutgoingAmount(
        const NodeUUID &contractor,
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &amount);

    AmountReservation::ConstShared reserveIncomingAmount(
        const NodeUUID &contractor,
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &amount);

    AmountReservation::ConstShared updateAmountReservation(
        const NodeUUID &contractor,
        const AmountReservation::ConstShared reservation,
        const TrustLineAmount &newAmount);

    void dropAmountReservation(
        const NodeUUID &contractor,
        const AmountReservation::ConstShared reservation);

    void useReservation(
        const NodeUUID &contractor,
        const AmountReservation::ConstShared reservation);

    // available outgoing amount considering reservations
    ConstSharedTrustLineAmount availableOutgoingAmount(
        const NodeUUID &contractor) const;

    // available incoming amount considering reservations
    ConstSharedTrustLineAmount availableIncomingAmount(
        const NodeUUID &contractor) const;

    // available outgoing amount considering reservations for cycles
    // returns 2 values: 1) amount considering reservations, 2) amount don't considering reservations
    pair<ConstSharedTrustLineAmount, ConstSharedTrustLineAmount> availableOutgoingCycleAmounts(
        const NodeUUID &contractor) const;

    // available incoming amount considering reservations for cycles
    // returns 2 values: 1) amount considering reservations, 2) amount don't considering reservations
    pair<ConstSharedTrustLineAmount, ConstSharedTrustLineAmount> availableIncomingCycleAmounts(
        const NodeUUID &contractor) const;

    ConstSharedTrustLineAmount totalOutgoingAmount()
        const throw (bad_alloc);

    ConstSharedTrustLineAmount totalIncomingAmount()
        const throw (bad_alloc);

    // get all reservations (all transactions) to requested contractor
    vector<AmountReservation::ConstShared> reservationsToContractor(
        const NodeUUID &contractorUUID) const;

    // get all reservations (all transactions) from requested contractor
    vector<AmountReservation::ConstShared> reservationsFromContractor(
        const NodeUUID &contractorUUID) const;

    const bool trustLineIsPresent (
        const NodeUUID &contractorUUID) const;

    const bool reservationIsPresent(
        const NodeUUID &contractorUUID) const;

    const bool isNeighbor(
        const NodeUUID &node) const;

    void saveToDisk(
        IOTransaction::Shared IOTransaction,
        TrustLine::Shared trustLine);

    [[deprecated]]
    void saveToDisk(
        TrustLine::Shared trustLine);

    void removeTrustLine(
        IOTransaction::Shared IOTransaction,
        const NodeUUID &contractorUUID);

    [[deprecated]]
    void removeTrustLine(
        const NodeUUID &contractorUUID);

    uint32_t crc32SumFirstLevel(const NodeUUID &contractorUUID);

    uint32_t crc32SumSecondLevel(const NodeUUID &contractorUUID);

    vector<NodeUUID> firstLevelNeighborsWithOutgoingFlow() const;

    vector<NodeUUID> firstLevelNeighborsWithIncomingFlow() const;

    vector<NodeUUID> firstLevelNeighborsWithPositiveBalance() const;

    vector<NodeUUID> firstLevelNeighborsWithNegativeBalance() const;

    vector<NodeUUID> firstLevelNeighborsWithNoneZeroBalance() const;

    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlows() const;

    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlows() const;

    vector<pair<const NodeUUID, const TrustLineDirection>> rt1WithDirections() const;

    vector<NodeUUID> rt1() const;

    // total balance to all 1st level neighbors
    ConstSharedTrustLineBalance totalBalance() const;

    [[deprecated("Buggy function. Use trustLineReadOnly instead")]]
    const TrustLine::Shared trustLine(
        const NodeUUID &contractorUUID) const;

    const TrustLine::ConstShared trustLineReadOnly(
        const NodeUUID &contractorUUID) const;

    unordered_map<NodeUUID, TrustLine::Shared, boost::hash<boost::uuids::uuid>>& trustLines();

    vector<NodeUUID> getFirstLevelNodesForCycles(
            TrustLineBalance maxFlow);

    // TODO remove after testing
    void printRTs();

    // TODO remove after testing
    // this method is used for testing closing cycles
    pair<TrustLineBalance, TrustLineBalance> debtAndCredit();

protected:
    void loadTrustLinesFromDisk ()
        throw (IOError);

private:
    static const size_t kTrustAmountPartSize = 32;
    static const size_t kBalancePartSize = 32;
    static const size_t kSignBytePartSize = 1;
    static const size_t kTrustStatePartSize = 1;
    static const size_t kRecordSize =
        + kTrustAmountPartSize
        + kTrustAmountPartSize
        + kBalancePartSize
        + kSignBytePartSize
        + kTrustStatePartSize
        + kTrustStatePartSize;


    unordered_map<NodeUUID, TrustLine::Shared, boost::hash<boost::uuids::uuid>> mTrustLines;

    unique_ptr<AmountReservationsHandler> mAmountReservationsHandler;
    StorageHandler *mStorageHandler;
    Logger &mLogger;
};

#endif //GEO_NETWORK_CLIENT_TRUSTLINESMANAGER_H
