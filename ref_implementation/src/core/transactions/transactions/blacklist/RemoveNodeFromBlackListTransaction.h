/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_REMOVENODEFROMBLACKLISTTRANSACTION_H
#define GEO_NETWORK_CLIENT_REMOVENODEFROMBLACKLISTTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/blacklist/RemoveNodeFromBlackListCommand.h"
#include "../../../io/storage/StorageHandler.h"

class RemoveNodeFromBlackListTransaction:
    public BaseTransaction {

public:
    typedef shared_ptr<RemoveNodeFromBlackListTransaction> Shared;

public:
    RemoveNodeFromBlackListTransaction(
        NodeUUID &nodeUUID,
        RemoveNodeFromBlackListCommand::Shared command,
        StorageHandler *storageHandler,
        Logger &logger);

    RemoveNodeFromBlackListCommand::Shared command() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;
protected:
    RemoveNodeFromBlackListCommand::Shared mCommand;
    StorageHandler *mStorageHandler;

};

#endif //GEO_NETWORK_CLIENT_REMOVENODEFROMBLACKLISTTRANSACTION_H
