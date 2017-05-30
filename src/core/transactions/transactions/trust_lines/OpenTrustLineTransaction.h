#ifndef GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H

#include "../base/BaseTransaction.h"

#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../io/storage/StorageHandler.h"
#include "../../../io/storage/record/trust_line/TrustLineRecord.h"
#include "../../../interface/commands_interface/commands/trust_lines/OpenTrustLineCommand.h"
#include "../routing_tables/TrustLineStatesHandlerTransaction.h"

#include "../../../network/messages/trust_lines/OpenTrustLineMessage.h"
#include "../../../network/messages/trust_lines/AcceptTrustLineMessage.h"
#include "../../../network/messages/response/Response.h"

#include "../../../common/exceptions/ConflictError.h"
#include "../../../common/exceptions/RuntimeError.h"


class OpenTrustLineTransaction:
    public BaseTransaction {

public:
    typedef shared_ptr<OpenTrustLineTransaction> Shared;

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
    inline TransactionResult::SharedConst initOperation();
    inline TransactionResult::SharedConst processResponse();

protected:
    TransactionResult::SharedConst resultOK() const;
    TransactionResult::SharedConst resultTrustLineIsAlreadyPresent() const;
    TransactionResult::SharedConst resultRejected() const;
    TransactionResult::SharedConst resultRemoteNodeIsInaccessible() const;
    TransactionResult::SharedConst resultProtocolError() const;

protected:
    const string logHeader() const
        noexcept;

    void updateHistory(
        IOTransaction::Shared ioTransaction);

protected:
    enum Stages {
        Initialization = 1, // todo: [v1.1] begin counter from 0 instead of 1.
        ResponseProcessing
    };

    OpenTrustLineCommand::Shared mCommand;
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;
};

#endif //GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H
