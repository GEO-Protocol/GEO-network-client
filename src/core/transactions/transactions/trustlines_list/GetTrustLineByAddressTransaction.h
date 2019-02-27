#ifndef GEO_NETWORK_CLIENT_GETTRUSTLINEBYADDRESSTRANSACTION_H_H
#define GEO_NETWORK_CLIENT_GETTRUSTLINEBYADDRESSTRANSACTION_H_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/trust_lines_list/GetTrustLineByAddressCommand.h"
#include "../../../contractors/ContractorsManager.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"

class GetTrustLineByAddressTransaction :
    public BaseTransaction {

public:
    typedef shared_ptr<GetTrustLineByAddressTransaction> Shared;

public:
    GetTrustLineByAddressTransaction(
        GetTrustLineByAddressCommand::Shared command,
        ContractorsManager *contractorsManager,
        TrustLinesManager *trustLinesManager,
        Logger &logger)
        noexcept;

    TransactionResult::SharedConst run();

    TransactionResult::SharedConst resultTrustLineIsAbsent();

protected:
    const string logHeader() const;

private:
    GetTrustLineByAddressCommand::Shared mCommand;
    ContractorsManager *mContractorsManager;
    TrustLinesManager *mTrustLinesManager;
};

#endif //GEO_NETWORK_CLIENT_GETTRUSTLINEBYADDRESSTRANSACTION_H_H
