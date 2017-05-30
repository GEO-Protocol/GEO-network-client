#ifndef GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H

#include "../base/BaseTransaction.h"

#include "../../../interface/commands_interface/commands/trust_lines/CloseTrustLineCommand.h"

#include "../../../io/storage/StorageHandler.h"
#include "../../../io/storage/record/trust_line/TrustLineRecord.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../network/messages/trust_lines/CloseTrustLineMessage.h"


class CloseTrustLineTransaction:
    public BaseTransaction {

public:
    typedef shared_ptr<CloseTrustLineTransaction> Shared;

public:
    CloseTrustLineTransaction(
        const NodeUUID &nodeUUID,
        CloseTrustLineCommand::Shared command,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Logger &logger)
        noexcept;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const
        noexcept;

    void updateHistory(
        IOTransaction::Shared ioTransaction);

    TransactionResult::SharedConst resultOK();

    TransactionResult::SharedConst resultTrustLineIsAbsent();

    TransactionResult::SharedConst resultProtocolError();

private:
    CloseTrustLineCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H
