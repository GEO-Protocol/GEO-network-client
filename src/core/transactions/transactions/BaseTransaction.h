#ifndef GEO_NETWORK_CLIENT_BASETRANSACTION_H
#define GEO_NETWORK_CLIENT_BASETRANSACTION_H

#include "../TransactionUUID.h"
#include "../../network/messages/Message.h"
#include "state/TransactionState.h"
#include "../../interface/results/result/CommandResult.h"
#include "../../db/UUIDMapBlockStorage.h"

namespace storage = db::uuid_map_block_storage;

typedef storage::byte byte;

class BaseTransaction {
    friend class TransactionsScheduler;

public:
    typedef shared_ptr<BaseTransaction> Shared;

public:
    enum TransactionType {
        OpenTrustLineTransaction,
        UpdateTrustLineTransaction,
        CloseTrustLineTransaction,
        MaximalAmountTransaction,
        UseCreditTransaction,
        TotalBalanceTransaction,
        ContractorsListTransaction
    };
    typedef uint8_t SerializedTransactionType;

protected:
    TransactionType mType;
    TransactionUUID mTransactionUUID;
    Message::Shared mContext;

protected:
    BaseTransaction(
            TransactionType type);

public:
    const TransactionType transactionType() const;

    const TransactionUUID uuid() const;

protected:
    virtual void setContext(
            Message::Shared message) = 0;

    virtual pair<CommandResult::SharedConst, TransactionState::SharedConst> run() = 0;

    virtual pair<byte *, size_t> serializeContext() = 0;

};


#endif //GEO_NETWORK_CLIENT_BASETRANSACTION_H
