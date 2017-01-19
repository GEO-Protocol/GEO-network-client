#ifndef GEO_NETWORK_CLIENT_BASETRANSACTION_H
#define GEO_NETWORK_CLIENT_BASETRANSACTION_H

#include "../../common/Types.h"
#include "../../common/NodeUUID.h"
#include "../TransactionUUID.h"

#include "../../network/messages/Message.h"

#include "result/TransactionResult.h"

#include <boost/signals2.hpp>


namespace storage = db::uuid_map_block_storage;
namespace signals = boost::signals2;

class BaseTransaction {
public:
    typedef shared_ptr<BaseTransaction> Shared;

    signals::signal<void(Message::Shared, const NodeUUID&)> sendMessageSignal;

public:
    enum TransactionType {
        OpenTrustLineTransactionType,
        AcceptTrustLineTransactionType,
        UpdateTrustLineTransactionType,
        SetTrustLineTransactionType,
        CloseTrustLineTransactionType,
        RejectTrustLineTransactionType,
        MaximalAmountTransactionType,
        UseCreditTransactionType,
        TotalBalanceTransactionType,
        ContractorsListTransactionType
    };

public:
    const TransactionType transactionType() const;

    const NodeUUID &nodeUUID() const;

    const TransactionUUID &transactionUUID() const;

    void setContext(
        Message::Shared message);

    pair<ConstBytesShared, size_t> serializeContext();

    virtual TransactionResult::Shared run() = 0;

protected:
    BaseTransaction(
        TransactionType type,
        NodeUUID &nodeUUID);

    void increaseStepsCounter();

    void increaseRequestsCounter();

protected:
    TransactionType mType;
    NodeUUID &mNodeUUID;

    TransactionUUID mTransactionUUID;
    Message::Shared mContext;

    uint16_t mStep = 1;
    uint16_t mRequestCounter = 0;
};


#endif //GEO_NETWORK_CLIENT_BASETRANSACTION_H
