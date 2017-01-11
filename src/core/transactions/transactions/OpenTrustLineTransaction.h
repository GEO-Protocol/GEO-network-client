#ifndef GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H

#include "UniqueTransaction.h"

#include "../../interface/commands/commands/OpenTrustLineCommand.h"

#include "UpdateTrustLineTransaction.h"
#include "CloseTrustLineTransaction.h"

#include "../scheduler/TransactionsScheduler.h"

#include "../../trust_lines/interface/TrustLinesInterface.h"

class OpenTrustLineTransaction : public UniqueTransaction {

public:
    typedef shared_ptr<OpenTrustLineTransaction> Shared;

public:
    OpenTrustLineTransaction(
        OpenTrustLineCommand::Shared command,
        TransactionsScheduler *scheduler,
        TrustLinesInterface *interface);

    OpenTrustLineCommand::Shared command() const;

    void setContext(
        Message::Shared message);

    pair<byte *, size_t> serializeContext();

    pair<CommandResult::SharedConst, TransactionState::SharedConst> run();

private:
    pair<CommandResult::SharedConst, TransactionState::SharedConst> resultOk();

    pair<CommandResult::SharedConst, TransactionState::SharedConst> conflictErrorResult();

    pair<CommandResult::SharedConst, TransactionState::SharedConst> trustLinePresentResult();

private:
    OpenTrustLineCommand::Shared mCommand;

    TrustLinesInterface *mTrustLinesInterface;
};


#endif //GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H
