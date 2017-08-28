#ifndef GEO_NETWORK_CLIENT_REQUESTMESSAGEWITHRESERVATIONS_H
#define GEO_NETWORK_CLIENT_REQUESTMESSAGEWITHRESERVATIONS_H

#include "../../base/transaction/TransactionMessage.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

#include <vector>

class RequestMessageWithReservations : public TransactionMessage {

public:
    typedef shared_ptr<RequestMessageWithReservations> Shared;

public:
    RequestMessageWithReservations(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const vector<pair<PathID, ConstSharedTrustLineAmount>> &finalAmountsConfig);

    RequestMessageWithReservations(
        BytesShared buffer);

    const vector<pair<PathID, ConstSharedTrustLineAmount>> &finalAmountsConfiguration() const;

protected:
    virtual pair<BytesShared, size_t> serializeToBytes() const
    throw (bad_alloc);

protected:
    typedef uint32_t RecordNumber;
    typedef RecordNumber RecordCount;

private:
    vector<pair<PathID, ConstSharedTrustLineAmount>> mFinalAmountsConfiguration;
};


#endif //GEO_NETWORK_CLIENT_REQUESTMESSAGEWITHRESERVATIONS_H
