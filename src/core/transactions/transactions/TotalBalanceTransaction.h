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

    pair<CommandResult::SharedConst, TransactionState::SharedConst> run();

    pair<byte *, size_t> serializeContext();
};


#endif //GEO_NETWORK_CLIENT_TOTALBALANCETRANSACTION_H
