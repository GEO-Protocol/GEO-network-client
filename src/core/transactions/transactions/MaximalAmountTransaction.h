#ifndef GEO_NETWORK_CLIENT_MAXIMALAMOUNTTRANSACTION_H
#define GEO_NETWORK_CLIENT_MAXIMALAMOUNTTRANSACTION_H

#include "BaseTransaction.h"

#include "../../interface/commands/commands/MaximalTransactionAmountCommand.h"

class MaximalAmountTransaction : public BaseTransaction {
    friend class TransactionsScheduler;

private:
    MaximalTransactionAmountCommand::Shared mCommand;

public:
    MaximalAmountTransaction(
            MaximalTransactionAmountCommand::Shared command);

private:
    MaximalTransactionAmountCommand::Shared command() const;

    void setContext(
            Message::Shared message);

    pair<byte *, size_t> serializeContext();

    TransactionResult::Shared run();
};


#endif //GEO_NETWORK_CLIENT_MAXIMALAMOUNTTRANSACTION_H
