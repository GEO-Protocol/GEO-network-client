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

#include <map>
#include <vector>
#include <malloc.h>


using namespace std;
namespace storage = db::uuid_map_block_storage;


class TrustLinesManager {
    // todo: deprecated; Tests subclass should be used.
    friend class TrustLinesManagerTests;

private:
    // todo: serizlisation/deserialisation of the TrstLine is the responsiblity of the TrustLine.
    // todo: move this into the TrustLine.

    // todo: this code is duplicated into the TrustLine.
    static const size_t kTrustAmountPartSize = 32;
    static const size_t kBalancePartSize = 32;
    static const size_t kSignBytePartSize = 1;
    static const size_t kRecordSize =
        + kTrustAmountPartSize
        + kTrustAmountPartSize
        + kBalancePartSize
        + kSignBytePartSize;

public:
    TrustLinesManager();

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

    void setIncomingTrustAmount(
        const NodeUUID &contractor,
        const TrustLineAmount &amount);

    void setOutgoingTrustAmount(
        const NodeUUID &contractor,
        const TrustLineAmount &amount);

    AmountReservation::ConstShared reserveAmount(
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

    const bool isTrustLineExist(
        const NodeUUID &contractorUUID) const;

    const bool checkDirection(
        const NodeUUID &contractorUUID,
        const TrustLineDirection direction) const;

protected:
    // Contractor UUID -> trust line to the contractor.
    map<NodeUUID, TrustLine::Shared> mTrustLines;

    unique_ptr<TrustLinesStorage> mTrustLinesStorage;
    unique_ptr<AmountReservationsHandler> mAmountBlocksHandler;

protected:

    void saveToDisk(
        TrustLine::Shared trustLine);

    void removeTrustLine(
        const NodeUUID &contractorUUID);

    void loadTrustLines();
};


#endif //GEO_NETWORK_CLIENT_TRUSTLINESMANAGER_H
