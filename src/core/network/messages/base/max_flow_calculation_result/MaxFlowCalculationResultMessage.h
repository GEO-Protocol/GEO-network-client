#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONRESULTMESSAGE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONRESULTMESSAGE_H

#include "../transaction/TransactionMessage.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"

#include "../../../../transactions/transactions/base/TransactionUUID.h"

#include <memory>
#include <utility>
#include <stdint.h>

using namespace std;

//TODO:: (M.I.) Please, choose or this class is need for you in future. Now, he is
//TODO:: completely duplicate logic of TransactionMessage class. If no - remove him and his directory too.
class MaxFlowCalculationResultMessage : public TransactionMessage {
public:
    typedef shared_ptr<MaxFlowCalculationResultMessage> Shared;

protected:
    MaxFlowCalculationResultMessage();

    MaxFlowCalculationResultMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID);

    virtual const MessageType typeID() const = 0;

    virtual pair<BytesShared, size_t> serializeToBytes();

    virtual void deserializeFromBytes(
        BytesShared buffer);

    static const size_t kOffsetToInheritedBytes();

};

#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONRESULTMESSAGE_H
