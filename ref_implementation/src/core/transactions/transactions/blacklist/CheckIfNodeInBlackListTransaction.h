/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_CheckIfNodeInBlackListTransactionTRANSACTION_H
#define GEO_NETWORK_CLIENT_CheckIfNodeInBlackListTransactionTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/blacklist/CheckIfNodeInBlackListCommand.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"


class CheckIfNodeInBlackListTransaction:
    public BaseTransaction {

public:
    typedef shared_ptr<CheckIfNodeInBlackListTransaction> Shared;

public:
    CheckIfNodeInBlackListTransaction(
        NodeUUID &nodeUUID,
        CheckIfNodeInBlackListCommand::Shared command,
        StorageHandler *storageHandler,
        Logger &logger);

    CheckIfNodeInBlackListCommand::Shared command() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    CheckIfNodeInBlackListCommand::Shared mCommand;
    StorageHandler *mStorageHandler;
};

#endif //GEO_NETWORK_CLIENT_CheckIfNodeInBlackListTransactionTRANSACTION_H
