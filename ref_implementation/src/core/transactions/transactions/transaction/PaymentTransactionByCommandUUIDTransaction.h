/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_PAYMENTTRANSACTIONBYCOMMANDUUIDTRANSACTION_H
#define GEO_NETWORK_CLIENT_PAYMENTTRANSACTIONBYCOMMANDUUIDTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/transactions/PaymentTransactionByCommandUUIDCommand.h"

class PaymentTransactionByCommandUUIDTransaction : public BaseTransaction {

public:
    typedef shared_ptr<PaymentTransactionByCommandUUIDTransaction> Shared;

public:
    PaymentTransactionByCommandUUIDTransaction(
        NodeUUID &nodeUUID,
        PaymentTransactionByCommandUUIDCommand::Shared command,
        BaseTransaction::Shared reuqestedTransaction,
        Logger &logger);

    PaymentTransactionByCommandUUIDCommand::Shared command() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    TransactionResult::SharedConst resultOk(
        string &transactionUUIDStr);

private:
    PaymentTransactionByCommandUUIDCommand::Shared mCommand;
    BaseTransaction::Shared mRequestedPaymentTransaction;
};


#endif //GEO_NETWORK_CLIENT_PAYMENTTRANSACTIONBYCOMMANDUUIDTRANSACTION_H
