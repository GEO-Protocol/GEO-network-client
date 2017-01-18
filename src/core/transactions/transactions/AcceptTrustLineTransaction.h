#ifndef GEO_NETWORK_CLIENT_ACCEPTTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_ACCEPTTRUSTLINETRANSACTION_H

#include "UniqueTransaction.h"

#include "../../network/messages/incoming/AcceptTrustLineMessage.h"

#include "../../network/communicator/Communicator.h"
#include "../../network/messages/Message.h"
#include "../../network/messages/response/Response.h"

#include "../scheduler/TransactionsScheduler.h"

#include "../../trust_lines/interface/TrustLinesInterface.h"

class Communicator;
class AcceptTrustLineTransaction : public UniqueTransaction {

public:
    typedef shared_ptr<AcceptTrustLineTransaction> Shared;

public:
    AcceptTrustLineTransaction(
        AcceptTrustLineMessage::Shared message,
        Communicator *communicator,
        TransactionsScheduler *scheduler,
        TrustLinesInterface *interface);

    AcceptTrustLineMessage::Shared message() const;

    void setContext(
        Message::Shared message);

    pair<byte *, size_t> serializeContext();

    TransactionResult::Shared run();

private:
    bool checkJournal();

    bool checkSameTypeTransactions();

    bool checkTrustLineDirection();

    bool checkTrustLineAmount();

    void sendResponse(
        uint16_t code);

    void createTrustLine();

    void increaseStepsCounter();

    TransactionResult::Shared makeResult(
        MessageResult::Shared messageResult);


private:
    AcceptTrustLineMessage::Shared mMessage;
    Communicator *mCommunicator;
    TrustLinesInterface *mTrustLinesInterface;

    uint16_t mStep;
};


#endif //GEO_NETWORK_CLIENT_ACCEPTTRUSTLINETRANSACTION_H
