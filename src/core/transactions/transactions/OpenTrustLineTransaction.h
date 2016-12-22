#ifndef GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H

#include "BaseTransaction.h"
#include "../../interface/commands/commands/OpenTrustLineCommand.h"

class TransactionsScheduler;

class OpenTrustLineTransaction : public BaseTransaction {
    friend class TransactionsScheduler;

private:
    OpenTrustLineCommand::Shared mCommand;

public:
    OpenTrustLineTransaction(
            OpenTrustLineCommand::Shared command);

private:
    OpenTrustLineCommand::Shared command() const;

    void setContext(
            Message::Shared message);

    pair<CommandResult::SharedConst, TransactionState::SharedConst> run();
};


#endif //GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H
