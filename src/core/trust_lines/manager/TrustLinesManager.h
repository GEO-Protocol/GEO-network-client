#ifndef GEO_NETWORK_CLIENT_TRUSTLINESMANAGER_H
#define GEO_NETWORK_CLIENT_TRUSTLINESMANAGER_H


#include "../../common/Types.h"

#include "../TrustLine.h"
#include "../../payments/amount_blocks/AmountReservationsHandler.h"
#include "../../io/trust_lines/TrustLinesStorage.h"

#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/ValueError.h"
#include "../../common/exceptions/MemoryError.h"
#include "../../common/exceptions/ConflictError.h"
#include "../../common/exceptions/NotFoundError.h"
#include "../../common/exceptions/PreconditionFailedError.h"
#include "../../logger/Logger.h"

#include <boost/signals2.hpp>

#include <map>
#include <vector>
#include <malloc.h>

// TODO: remove me
#include <iostream>


using namespace std;
namespace storage = db::uuid_map_block_storage;
namespace signals = boost::signals2;


class TrustLinesManager {
    // todo: deprecated; Tests subclass should be used.
    friend class TrustLinesManagerTests;

public:
    signals::signal<void(const NodeUUID&, const TrustLineDirection)> trustLineCreatedSignal;
    signals::signal<void(const NodeUUID&, const TrustLineDirection)> trustLineStateModifiedSignal;

public:
    TrustLinesManager(Logger *logger);

    void loadTrustLines();

    void open(
        const NodeUUID &contractorUUID,
        const TrustLineAmount &amount);

    void close(
        const NodeUUID &contractorUUID);

    void accept(
        const NodeUUID &contractorUUID,
        const TrustLineAmount &amount);

    void reject(
        const NodeUUID &contractorUUID);

    const bool checkDirection(
        const NodeUUID &contractorUUID,
        const TrustLineDirection direction) const;

    const BalanceRange balanceRange(
        const NodeUUID &contractorUUID) const;

    void suspendDirection(
        const NodeUUID &contractorUUID,
        const TrustLineDirection direction);

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

    // TODO: rename to "reserveOutgoingAmount"
    AmountReservation::ConstShared reserveAmount(
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

    ConstSharedTrustLineAmount availableOutgoingAmount(
        const NodeUUID &contractor);

    ConstSharedTrustLineAmount availableIncomingAmount(
        const NodeUUID &contractor);

    const bool isTrustLineExist(
        const NodeUUID &contractorUUID) const;

    const bool isNeighbor(
        const NodeUUID &node) const;

    void saveToDisk(
        TrustLine::Shared trustLine);

    void removeTrustLine(
        const NodeUUID &contractorUUID);

    vector<NodeUUID> firstLevelNeighborsWithOutgoingFlow() const;

    vector<NodeUUID> firstLevelNeighborsWithIncomingFlow() const;

    vector<NodeUUID> firstLevelNeighborsWithPositiveBalance() const;

    vector<NodeUUID> firstLevelNeighborsWithNegativeBalance() const;

    vector<NodeUUID> firstLevelNeighborsWithNoneZeroBalance() const;

    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlows() const;

    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlows() const;

    vector<pair<const NodeUUID, const TrustLineDirection>> rt1WithDirections() const;

    vector<NodeUUID> rt1() const;

    [[deprecated("Buggy function. Use trustLineReadOnly instead")]]
    const TrustLine::Shared trustLine(
        const NodeUUID &contractorUUID) const;

    const TrustLine::ConstShared trustLineReadOnly(
        const NodeUUID &contractorUUID) const;

    // todo: return const shared
    map<NodeUUID, TrustLine::Shared>& trustLines();

    vector<NodeUUID> getFirstLevelNodesForCycles(
            TrustLineBalance maxFlow);

    void setSomeBalances();

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

    // Contractor UUID -> trust line to the contractor.
    map<NodeUUID, TrustLine::Shared> mTrustLines;

    unique_ptr<TrustLinesStorage> mTrustLinesStorage;
    unique_ptr<AmountReservationsHandler> mAmountBlocksHandler;
    Logger *mlogger;
};

#endif //GEO_NETWORK_CLIENT_TRUSTLINESMANAGER_H
