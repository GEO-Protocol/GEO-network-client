#ifndef GEO_NETWORK_CLIENT_BASETRANSACTION_H
#define GEO_NETWORK_CLIENT_BASETRANSACTION_H

#include "TransactionUUID.h"
#include "../result/TransactionResult.h"

#include "../../../common/NodeUUID.h"
#include "../../../common/Types.h"
#include "../../../common/memory/MemoryUtils.h"

#include "../../../network/messages/Message.hpp"
#include "../../../db/uuid_map_block_storage/UUIDMapBlockStorage.h"

#include <boost/signals2.hpp>

#include <vector>
#include <memory>
#include <utility>
#include <cstdint>

namespace storage = db::uuid_map_block_storage;
namespace signals = boost::signals2;

class BaseTransaction {
public:
    typedef shared_ptr<BaseTransaction> Shared;
    typedef uint16_t SerializedTransactionType;
    typedef signals::signal<void(Message::Shared, const NodeUUID&)> SendMessageSignal;

public:
    enum TransactionType {
        OpenTrustLineTransactionType = 1,
        AcceptTrustLineTransactionType,
        SetTrustLineTransactionType,
        UpdateTrustLineTransactionType,
        CloseTrustLineTransactionType,
        RejectTrustLineTransactionType,
        PropagationRoutingTablesTransactionType,
        AcceptRoutingTablesTransactionType,

        // Payments
        CoordinatorPaymentTransaction,
        ReceiverPaymentTransaction
    };

public:
    const TransactionType transactionType() const;

    const TransactionUUID &UUID() const;

    const NodeUUID &nodeUUID() const;

    void setContext(
        Message::Shared message);

    virtual pair<BytesShared, size_t> serializeToBytes() const;

    virtual TransactionResult::SharedConst run() = 0;

protected:
    BaseTransaction();

    BaseTransaction(
        TransactionType type);

    BaseTransaction(
        TransactionType type,
        NodeUUID &nodeUUID);

    void addMessage(
        Message::Shared message,
        const NodeUUID &nodeUUID);

    void increaseStepsCounter();

    void setExpectationResponsesCounter(
        uint16_t count);

    void resetExpectationResponsesCounter();

    virtual void deserializeFromBytes(
        BytesShared buffer);

    static const size_t kOffsetToInheritedBytes();

    TransactionResult::SharedConst transactionResultFromCommand(
        CommandResult::SharedConst result);

    TransactionResult::SharedConst transactionResultFromMessage(
        MessageResult::SharedConst result);

public:
    mutable SendMessageSignal outgoingMessageIsReadySignal;

protected:
    TransactionType mType;
    TransactionUUID mTransactionUUID;
    NodeUUID mNodeUUID;

    uint16_t mExpectationResponsesCount = 0;
    vector<Message::Shared> mContext;

    uint16_t mStep = 1;
};


#endif //GEO_NETWORK_CLIENT_BASETRANSACTION_H
