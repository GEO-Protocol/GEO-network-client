#ifndef GEO_NETWORK_CLIENT_GETTRUSTLINEBYIDTRANSACTION_H
#define GEO_NETWORK_CLIENT_GETTRUSTLINEBYIDTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/trust_lines_list/GetTrustLineByIDCommand.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"

class GetTrustLineByIDTransaction : public BaseTransaction {

public:
    typedef shared_ptr<GetTrustLineByIDTransaction> Shared;

public:
    GetTrustLineByIDTransaction(
        GetTrustLineByIDCommand::Shared command,
        TrustLinesManager *trustLinesManager,
        Logger &logger)
        noexcept;

    TransactionResult::SharedConst run();

    TransactionResult::SharedConst resultTrustLineIsAbsent();

protected:
    const string logHeader() const;

private:
    GetTrustLineByIDCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;
};


#endif //GEO_NETWORK_CLIENT_GETTRUSTLINEBYIDTRANSACTION_H
