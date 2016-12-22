#ifndef GEO_NETWORK_CLIENT_BASETRANSACTION_H
#define GEO_NETWORK_CLIENT_BASETRANSACTION_H

#include "../../network/messages/Message.h"
#include "state/TransactionState.h"
#include "../../interface/results/result/CommandResult.h"

class BaseTransaction {

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

private:
    TransactionType mType;

protected:
    Message::Shared mContext;

protected:
    BaseTransaction(
            TransactionType type);

public:
    const TransactionType transactionType() const;

protected:
    virtual void setContext(
            Message::Shared message) = 0;

    virtual pair<CommandResult::SharedConst, TransactionState::SharedConst> run() = 0;

};


#endif //GEO_NETWORK_CLIENT_BASETRANSACTION_H
