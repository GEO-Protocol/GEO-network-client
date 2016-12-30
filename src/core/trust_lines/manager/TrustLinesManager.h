#ifndef GEO_NETWORK_CLIENT_TRUSTLINESMANAGER_H
#define GEO_NETWORK_CLIENT_TRUSTLINESMANAGER_H

#include "../TrustLine.h"
#include "../../common/NodeUUID.h"
#include "../../io/trust_lines/TrustLinesStorage.h"

#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/ValueError.h"
#include "../../common/exceptions/ConflictError.h"
#include "../../common/exceptions/PreconditionFaultError.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <map>
#include <vector>
#include <malloc.h>


using namespace std;
namespace storage = db::uuid_map_block_storage;

// todo: see Types.h
typedef storage::byte byte;


class TrustLinesManager {
    friend class TrustLinesManagerTest; // todo: rename to TrustLinesManagerTest(s!)

public:
    // todo: why this are public? Who uses them?
    static const size_t kTrustAmountPartSize = 32;
    static const size_t kBalancePartSize = 32;
    static const size_t kSignBytePartSize = 1;
    static const size_t kBucketSize = // todo: why it is called "bucket"? this is the "record"
        + kTrustAmountPartSize
        + kTrustAmountPartSize
        + kBalancePartSize
        + kSignBytePartSize;

public:
    TrustLinesManager();
    ~TrustLinesManager();

    void open(
        const NodeUUID &contractorUUID,
        const trust_amount &amount);

    void close(
        const NodeUUID &contractorUUID);

    void accept(
        const NodeUUID &contractorUUID,
        const trust_amount &amount);

    void reject(
        const NodeUUID &contractorUUID);

    TrustLine::Shared getTrustLineByContractorUUID(
        const NodeUUID &contractorUUID);

protected:
    map<NodeUUID, TrustLine::Shared> mTrustLines; // contractor UUID -> trust line to the contractor.

    // Internal
    TrustLinesStorage *mTrustLinesStorage;

protected:
    // todo: move this method into the TrustLine
    // only the trust line knows how it should be serialized and no other objects.
    // todo: return shared pointer to the buffer
    byte *serializeTrustLine(
        TrustLine::Shared trustLine);

    // todo: move this into the TrustLine
    void deserializeTrustLine(
        const byte *buffer,
        const NodeUUID &contractorUUID);

    // todo: move this into the TrustLine
    vector<byte> trustAmountToBytes(
        const trust_amount &amount);

    // todo: move this into the TrustLine
    vector<byte> balanceToBytes(
            const balance_value &balance);

    // todo: move this into the TrustLine
    trust_amount parseTrustAmount(
            const byte *buffer);

    // todo: move this into the TrustLine
    balance_value parseBalance(
            const byte *buffer);

    void saveTrustLine(
        TrustLine::Shared trustLine);

    void removeTrustLine(
        const NodeUUID &contractorUUID);

    bool isTrustLineExist(
        const NodeUUID &contractorUUID);
    // todo: make it const
//    const bool isTrustLineExist(
//        const NodeUUID &contractorUUID) const;

    void getTrustLinesFromStorage(); // todo: rename to "loadTrustLines"
};


#endif //GEO_NETWORK_CLIENT_TRUSTLINESMANAGER_H
