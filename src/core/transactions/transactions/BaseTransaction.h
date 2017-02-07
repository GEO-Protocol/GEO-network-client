#ifndef GEO_NETWORK_CLIENT_BASETRANSACTION_H
#define GEO_NETWORK_CLIENT_BASETRANSACTION_H

#include "../../common/Types.h"

#include "../../common/NodeUUID.h"
#include "../TransactionUUID.h"

#include "../../network/messages/Message.h"

#include "result/TransactionResult.h"

#include "../../db/uuid_map_block_storage/UUIDMapBlockStorage.h"

#include <boost/signals2.hpp>


namespace storage = db::uuid_map_block_storage;
namespace signals = boost::signals2;

class BaseTransaction {
public:
    typedef shared_ptr<BaseTransaction> Shared;
    typedef signals::signal<void(Message::Shared, const NodeUUID&)> SendMessageSignal;
    typedef uint16_t SerialisedTransactionType;

public:
    enum TransactionType {
        OpenTrustLineTransactionType = 1,
        AcceptTrustLineTransactionType = 2,
        SetTrustLineTransactionType = 3,
        CloseTrustLineTransactionType = 4,
        RejectTrustLineTransactionType = 5,
        UpdateTrustLineTransactionType = 6,
        SendRoutingTablesTransactionType = 7,
        AcceptRoutingTablesTransactionType = 8,
    };

    mutable SendMessageSignal outgoingMessageIsReadySignal;

public:
    signals::connection addOnMessageSendSlot(
        const SendMessageSignal::slot_type &slot) const;

    const TransactionType transactionType() const;

    const NodeUUID &nodeUUID() const;

    const TransactionUUID &UUID() const;

    void setContext(
        Message::Shared message);

    pair<ConstBytesShared, size_t> serializeContext();

    virtual pair<BytesShared, size_t> serializeToBytes() = 0;

    virtual TransactionResult::Shared run() = 0;

protected:
    BaseTransaction(
        TransactionType type,
        NodeUUID &nodeUUID);

    BaseTransaction();

    void addMessage(
        Message::Shared message,
        const NodeUUID &nodeUUID);

    void increaseStepsCounter();

    void increaseRequestsCounter();

    void resetRequestsCounter();

    pair<BytesShared, size_t> serializeParentToBytes();

    void deserializeParentFromBytes(
        BytesShared buffer);

    virtual void deserializeFromBytes(
        BytesShared buffer) = 0;

    const size_t kOffsetToDataBytes();

    TransactionResult::Shared transactionResultFromCommand(
        CommandResult::SharedConst result);

protected:
    TransactionType mType;
    NodeUUID mNodeUUID;

    TransactionUUID mTransactionUUID;
    Message::Shared mContext;

    uint16_t mStep = 1;
    uint16_t mRequestCounter = 0;
};


#endif //GEO_NETWORK_CLIENT_BASETRANSACTION_H
