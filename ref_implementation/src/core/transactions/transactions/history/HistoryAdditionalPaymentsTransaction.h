/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_HISTORYADDITIONALPAYMENTSTRANSACTION_H
#define GEO_NETWORK_CLIENT_HISTORYADDITIONALPAYMENTSTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/history/HistoryAdditionalPaymentsCommand.h"
#include "../../../io/storage/StorageHandler.h"
#include "../../../io/storage/record/payment/PaymentRecord.h"

#include <vector>

class HistoryAdditionalPaymentsTransaction : public BaseTransaction {

public:
    typedef shared_ptr<HistoryAdditionalPaymentsTransaction> Shared;

public:
    HistoryAdditionalPaymentsTransaction(
        NodeUUID &nodeUUID,
        HistoryAdditionalPaymentsCommand::Shared command,
        StorageHandler *storageHandler,
        Logger &logger);

    HistoryAdditionalPaymentsCommand::Shared command() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    TransactionResult::SharedConst resultOk(
        const vector<PaymentRecord::Shared> &records);

private:
    HistoryAdditionalPaymentsCommand::Shared mCommand;
    StorageHandler *mStorageHandler;
};

#endif //GEO_NETWORK_CLIENT_HISTORYADDITIONALPAYMENTSTRANSACTION_H
