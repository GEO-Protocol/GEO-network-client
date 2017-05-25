#ifndef GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H

#include "TrustLineTransaction.h"

#include "../../../../io/storage/StorageHandler.h"
#include "../../../../io/storage/record/trust_line/TrustLineRecord.h"

#include "../../../../interface/commands_interface/commands/trust_lines/OpenTrustLineCommand.h"

#include "../../../../network/messages/trust_lines/OpenTrustLineMessage.h"
#include "../../../../network/messages/trust_lines/AcceptTrustLineMessage.h"
#include "../../../../network/messages/response/Response.h"

#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../common/exceptions/ConflictError.h"
#include "../../../../common/exceptions/RuntimeError.h"

#include "../../../../transactions/transactions/routing_tables/TrustLineStatesHandlerTransaction.h"


class OpenTrustLineTransaction:
    public TrustLineTransaction {

public:
    typedef shared_ptr<OpenTrustLineTransaction> Shared;

private:
    enum Stages {
        Initial,
        ResponseProcessing
    };

public:
    OpenTrustLineTransaction(
        const NodeUUID &nodeUUID,
        OpenTrustLineCommand::Shared command,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Logger &logger)
        noexcept;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

    TransactionResult::SharedConst initOperation();

    TransactionResult::SharedConst processResponse();

    void updateHistory(
        IOTransaction::Shared ioTransaction);

    TransactionResult::SharedConst resultOK();

    TransactionResult::SharedConst resultTrustLineIsAlreadyPresent();

    TransactionResult::SharedConst resultRejected();

    TransactionResult::SharedConst resultRemoteNodeIsInaccessible();

    TransactionResult::SharedConst resultProtocolError();

private:
    OpenTrustLineCommand::Shared mCommand;
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;
};

#endif //GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H
