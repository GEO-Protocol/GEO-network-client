#ifndef GEO_NETWORK_CLIENT_TRUSTLINESMANAGER_H
#define GEO_NETWORK_CLIENT_TRUSTLINESMANAGER_H

#include <map>
#include <vector>
#include <malloc.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "../TrustLine.h"
#include "../../common/NodeUUID.h"
#include "../../io/trust_lines/TrustLinesStorage.h"

#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/ValueError.h"
#include "../../common/exceptions/ConflictError.h"
#include "../../common/exceptions/PreconditionFaultError.h"

using namespace std;

namespace storage = db::uuid_map_block_storage;

typedef storage::byte byte;


class TrustLinesManager {
    friend class TrustLinesInterface;
    friend class TrustLinesManagerTest;

public:
    static const size_t kTrustAmountPartSize = 32;
    static const size_t kBalancePartSize = 32;
    static const size_t kSignBytePartSize = 1;
    static const size_t kBucketSize = kTrustAmountPartSize + kTrustAmountPartSize + kBalancePartSize + kSignBytePartSize;

private:
    map<NodeUUID, TrustLine::Shared> mTrustLines;
    TrustLinesStorage *mTrustLinesStorage;

private:
    byte *serializeTrustLine(
            TrustLine::Shared trustLinePtr);

    vector<byte> trustAmountToBytes(
            const trust_amount &amount);

    vector<byte> balanceToBytes(
            const balance_value &balance);

    void deserializeTrustLine(
            const byte *buffer,
            const NodeUUID &contractorUUID);

    trust_amount parseTrustAmount(
            const byte *buffer);

    balance_value parseBalance(
            const byte *buffer);

    void saveTrustLine(
            TrustLine::Shared trustLinePtr);

    void removeTrustLine(
            const NodeUUID &contractorUUID);

    bool isTrustLineExist(
            const NodeUUID &contractorUUID);

    void getTrustLinesFromStorage();

public:
    TrustLinesManager();

    ~TrustLinesManager();

private:
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
};


#endif //GEO_NETWORK_CLIENT_TRUSTLINESMANAGER_H
