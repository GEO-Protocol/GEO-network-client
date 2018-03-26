/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_HISTORYPAYMENTSTRANSACTION_H
#define GEO_NETWORK_CLIENT_HISTORYPAYMENTSTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/history/HistoryPaymentsCommand.h"
#include "../../../io/storage/StorageHandler.h"
#include "../../../io/storage/record/payment/PaymentRecord.h"

#include <vector>

class HistoryPaymentsTransaction : public BaseTransaction {

public:
    typedef shared_ptr<HistoryPaymentsTransaction> Shared;

public:
    HistoryPaymentsTransaction(
        NodeUUID &nodeUUID,
        HistoryPaymentsCommand::Shared command,
        StorageHandler *storageHandler,
        Logger &logger);

    HistoryPaymentsCommand::Shared command() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    TransactionResult::SharedConst resultOk(
        const vector<PaymentRecord::Shared> &records);

private:
    HistoryPaymentsCommand::Shared mCommand;
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_HISTORYPAYMENTSTRANSACTION_H
