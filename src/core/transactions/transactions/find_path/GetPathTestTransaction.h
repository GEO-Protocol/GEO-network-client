#ifndef GEO_NETWORK_CLIENT_GETPATHTESTTRANSACTION_H
#define GEO_NETWORK_CLIENT_GETPATHTESTTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/find_path/FindPathCommand.h"
#include "../../../resources/manager/ResourcesManager.h"
#include "../../../resources/resources/BaseResource.h"
#include "../../../resources/resources/PathsResource.h"
#include "../../../logger/Logger.h"

// TODO : it should be removed after testing of getting paths from FindPathTransaction
class GetPathTestTransaction : public BaseTransaction {

public:
    typedef shared_ptr<GetPathTestTransaction> Shared;

public:
    GetPathTestTransaction(
        NodeUUID &nodeUUID,
        FindPathCommand::Shared command,
        ResourcesManager *resourcesManager,
        Logger *logger);

    FindPathCommand::Shared command() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    TransactionResult::SharedConst waitingForResourcesState();

    TransactionResult::SharedConst checkResourcesContext();

private:
    const uint32_t kConnectionTimeout = 4000;
    const uint16_t kMaxRequestsCount = 1;

private:
    FindPathCommand::Shared mCommand;
    ResourcesManager *mResourcesManager;
    uint16_t mRequestCounter;
};


#endif //GEO_NETWORK_CLIENT_GETPATHTESTTRANSACTION_H
