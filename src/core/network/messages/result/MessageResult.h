#ifndef GEO_NETWORK_CLIENT_MESSAGEREUSLT_H
#define GEO_NETWORK_CLIENT_MESSAGEREUSLT_H

#include "../../../common/Types.h"
#include "../../../common/NodeUUID.h"
#include "../../../common/time/TimeUtils.h"

#include "../../../transactions/transactions/base/TransactionUUID.h"

#include <memory>
#include <stdint.h>

using namespace std;

class MessageResult {
public:
    typedef shared_ptr<MessageResult> Shared;
    typedef shared_ptr<const MessageResult> SharedConst;

public:
    MessageResult(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const uint16_t resultCode
    );

    const NodeUUID &senderUUID() const;

    const TransactionUUID transactionUUID() const;

    const uint16_t resultCode() const;

    const DateTime &timestampCompleted() const;

    const string serialize() const;

private:
    NodeUUID mSenderUUID;
    TransactionUUID mTransactionUUID;
    uint16_t mResultCode;
    DateTime mTimestampCompleted;
};


#endif //GEO_NETWORK_CLIENT_MESSAGEREUSLT_H
