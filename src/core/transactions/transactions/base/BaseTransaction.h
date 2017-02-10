#ifndef GEO_NETWORK_CLIENT_BASETRANSACTION_H
#define GEO_NETWORK_CLIENT_BASETRANSACTION_H

#include "TransactionUUID.h"

#include "../../../common/Types.h"
#include "../../../common/NodeUUID.h"

#include "../../../network/messages/Message.hpp"
#include "../../../db/uuid_map_block_storage/UUIDMapBlockStorage.h"

#include "../result/TransactionResult.h"

#include <boost/signals2.hpp>


namespace storage = db::uuid_map_block_storage;
namespace signals = boost::signals2;

class BaseTransaction {
public:
    typedef shared_ptr<BaseTransaction> Shared;
    typedef signals::signal<void(Message::Shared, const NodeUUID&)> SendMessageSignal;
    typedef uint16_t SerialisedTransactionType;

public:
    // todo: (DM): remove "..Type" from enum
    enum TransactionType {
        // Trust lines operations
        OpenTrustLineTransactionType = 1,
        AcceptTrustLineTransactionType,
        SetTrustLineTransactionType,
        CloseTrustLineTransactionType,
        RejectTrustLineTransactionType,
        UpdateTrustLineTransactionType,

        // Routin table operations
        SendRoutingTablesTransactionType,
        AcceptRoutingTablesTransactionType,

        // Payment operations
        CoordinatorPaymentTransaction,
        ReceiverPaymentTransaction,
    };

    mutable SendMessageSignal outgoingMessageIsReadySignal;

public:
    // todo: DEPRECATED
    signals::connection addOnMessageSendSlot(
        const SendMessageSignal::slot_type &slot) const;

    const TransactionType transactionType() const;

    const NodeUUID &nodeUUID() const;

    const TransactionUUID &UUID() const;

    void setContext(
        Message::Shared message);

    // todo: (DM) DEPRECATED
    pair<ConstBytesShared, size_t> serializeContext();

    // todo: (DM) may be const
    virtual pair<BytesShared, size_t> serializeToBytes() = 0;

    virtual TransactionResult::Shared run() = 0;

protected:
    BaseTransaction();

    BaseTransaction(
        TransactionType type,
        NodeUUID &nodeUUID);

    BaseTransaction(
        TransactionType type);

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

    TransactionResult::Shared transactionResultFromCommand (
        CommandResult::SharedConst result) const;

    virtual const string logHeader() const;

protected:
    TransactionType mType;
    NodeUUID mNodeUUID;
    TransactionUUID mTransactionUUID;
    Message::Shared mContext;

    uint16_t mStep = 1;

    // todo: (DM) move this to the trust lines transaction
    uint16_t mRequestCounter = 0;
};


#endif //GEO_NETWORK_CLIENT_BASETRANSACTION_H
