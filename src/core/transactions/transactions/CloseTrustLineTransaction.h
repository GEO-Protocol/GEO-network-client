#ifndef GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H

#include "unique/UniqueTransaction.h"

#include "../../interface/commands/commands/CloseTrustLineCommand.h"

#include "../scheduler/TransactionsScheduler.h"

#include "../../trust_lines/interface/TrustLinesInterface.h"

class CloseTrustLineTransaction : public UniqueTransaction {

public:
    typedef shared_ptr<CloseTrustLineTransaction> Shared;

public:
    CloseTrustLineTransaction(
            CloseTrustLineCommand::Shared command,
            TransactionsScheduler *scheduler,
            TrustLinesInterface *interface);

    CloseTrustLineCommand::Shared command() const;

    void setContext(
            Message::Shared message);

    pair<byte *, size_t> serializeContext();

    TransactionResult::Shared run();

private:
    TransactionResult::Shared conflictErrorResult();

private:
    CloseTrustLineCommand::Shared mCommand;
    TrustLinesInterface *mTrustLinesInterface;

};



#endif //GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H
