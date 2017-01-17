#ifndef GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H

#include "UniqueTransaction.h"

#include "../../interface/commands/commands/OpenTrustLineCommand.h"

#include "AcceptTrustLineTransaction.h"
#include "UpdateTrustLineTransaction.h"
#include "CloseTrustLineTransaction.h"

#include "../../network/communicator/Communicator.h"
#include "../../network/messages/Message.h"
#include "../../network/messages/outgoing/OpenTrustLineMessage.h"

#include "../scheduler/TransactionsScheduler.h"

#include "../../trust_lines/interface/TrustLinesInterface.h"

class Communicator;
class OpenTrustLineTransaction : public UniqueTransaction {

public:
    typedef shared_ptr<OpenTrustLineTransaction> Shared;

public:
    OpenTrustLineTransaction(
        OpenTrustLineCommand::Shared command,
        Communicator *communicator,
        TransactionsScheduler *scheduler,
        TrustLinesInterface *interface);

    OpenTrustLineCommand::Shared command() const;

    void setContext(
        Message::Shared message);

    pair<byte *, size_t> serializeContext();

    TransactionResult::Shared run();

private:
    bool checkSameTypeTransactions();

    bool checkTrustLineDirection();

    void sendMessageToRemoteNode();

    void increaseStepsCounter();

    TransactionResult::Shared resultOk();

    TransactionResult::Shared trustLinePresentResult();

    TransactionResult::Shared conflictErrorResult();

private:
    OpenTrustLineCommand::Shared mCommand;
    Communicator *mCommunicator;
    TrustLinesInterface *mTrustLinesInterface;

    uint16_t mStep;
};


#endif //GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H
