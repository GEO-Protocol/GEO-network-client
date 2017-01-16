#ifndef GEO_NETWORK_CLIENT_TRUSTLINESMANAGER_H
#define GEO_NETWORK_CLIENT_TRUSTLINESMANAGER_H

#include "../../common/Types.h"

#include "../TrustLine.h"
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
    friend class TrustLinesManagerTests;

private:
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

    ~TrustLinesManager();

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

    TrustLine::Shared trustLineByContractorUUID(
        const NodeUUID &contractorUUID);

    const bool isTrustLineExist(
        const NodeUUID &contractorUUID) const;

protected:
    // Contractor UUID -> trust line to the contractor.
    map<NodeUUID, TrustLine::Shared> mTrustLines;

    // Internal
    TrustLinesStorage *mTrustLinesStorage;

protected:

    void saveTrustLine(
        TrustLine::Shared trustLine);

    void removeTrustLine(
        const NodeUUID &contractorUUID);

    void loadTrustLines();
};


#endif //GEO_NETWORK_CLIENT_TRUSTLINESMANAGER_H
