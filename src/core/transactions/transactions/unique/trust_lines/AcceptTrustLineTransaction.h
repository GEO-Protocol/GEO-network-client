#ifndef GEO_NETWORK_CLIENT_ACCEPTTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_ACCEPTTRUSTLINETRANSACTION_H

#include "TrustLineTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"

#include "../../../../io/storage/HistoryStorage.h"
#include "../../../../io/storage/record/trust_line/TrustLineRecord.h"

#include "../../../../network/messages/Message.hpp"
#include "../../../../network/messages/trust_lines/AcceptTrustLineMessage.h"
#include "../../../../network/messages/response/Response.h"

#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../common/exceptions/ConflictError.h"

#include <memory>
#include <utility>
#include <cstdint>

class AcceptTrustLineTransaction : public TrustLineTransaction {
public:
    typedef shared_ptr<AcceptTrustLineTransaction> Shared;

private:
    enum Stages{
        CheckJournal = 1,
        CheckUnicity,
        CheckIncomingDirection
    };

public:
    AcceptTrustLineTransaction(
        const NodeUUID &nodeUUID,
        AcceptTrustLineMessage::Shared message,
        TrustLinesManager *manager,
        HistoryStorage *historyStorage);

    AcceptTrustLineTransaction(
        BytesShared buffer,
        TrustLinesManager *manager,
        HistoryStorage *historyStorage);

    AcceptTrustLineMessage::Shared message() const;

    TransactionResult::SharedConst run();

private:
    bool checkJournal();

    bool isTransactionToContractorUnique();

    bool isIncomingTrustLineDirectionExisting();

    bool isIncomingTrustLineAlreadyAccepted();

    void acceptTrustLine();

    void logAcceptingTrustLineOperation();

    void sendResponseCodeToContractor(
        const uint16_t code);

private:
    AcceptTrustLineMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    HistoryStorage *mHistoryStorage;
};


#endif //GEO_NETWORK_CLIENT_ACCEPTTRUSTLINETRANSACTION_H
