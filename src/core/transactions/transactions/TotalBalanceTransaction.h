#ifndef GEO_NETWORK_CLIENT_TOTALBALANCETRANSACTION_H
#define GEO_NETWORK_CLIENT_TOTALBALANCETRANSACTION_H

#include "BaseTransaction.h"

#include "../../interface/commands/commands/TotalBalanceCommand.h"


class TotalBalanceTransaction : public BaseTransaction {
    friend class TransactionsScheduler;

private:
    TotalBalanceCommand::Shared mCommand;

public:
    TotalBalanceTransaction(
            TotalBalanceCommand::Shared command);

private:
    TotalBalanceCommand::Shared command() const;

    void setContext(
            Message::Shared message);

    pair<byte *, size_t> serializeContext();

    TransactionResult::Shared run();
};


#endif //GEO_NETWORK_CLIENT_TOTALBALANCETRANSACTION_H
