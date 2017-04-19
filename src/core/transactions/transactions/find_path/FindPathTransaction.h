#ifndef GEO_NETWORK_CLIENT_FINDPATHTRANSACTION_H
#define GEO_NETWORK_CLIENT_FINDPATHTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../paths/PathsManager.h"
#include "../../../resources/manager/ResourcesManager.h"
#include "../../../resources/resources/PathsResource.h"
#include "../../../network/messages/find_path/RequestRoutingTablesMessage.h"
#include "../../../network/messages/find_path/ResultRoutingTable1LevelMessage.h"
#include "../../../network/messages/find_path/ResultRoutingTable2LevelMessage.h"
#include "../../../network/messages/find_path/ResultRoutingTable3LevelMessage.h"
#include "../../../paths/lib/Path.h"
#include "../../../logger/Logger.h"

#include <vector>
#include <unordered_map>
#include <boost/functional/hash.hpp>

class FindPathTransaction : public BaseTransaction {

public:
    typedef shared_ptr<FindPathTransaction> Shared;

    FindPathTransaction(
        NodeUUID &nodeUUID,
        const NodeUUID &contractorUUID,
        const TransactionUUID &requestedTransactionUUID,
        PathsManager *pathsManager,
        ResourcesManager *resourcesManager,
        Logger *logger);

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:

    void sendMessageToRemoteNode();

    void increaseRequestsCounter();

    TransactionResult::SharedConst waitingForResponseState();

    TransactionResult::SharedConst checkTransactionContext();

    TransactionResult::SharedConst buildPaths();

protected:
    enum Stages {
        SendRequestForGettingRoutingTables = 1,
        BuildAllPaths
    };

private:

    const uint32_t kConnectionTimeout = 1500;
    const uint16_t kMaxRequestsCount = 1;

private:

    NodeUUID mContractorUUID;
    TransactionUUID mRequestedTransactionUUID;
    PathsManager *mPathsManager;
    ResourcesManager *mResourcesManager;
    uint16_t mRequestCounter;

    size_t previousContextSize;
    bool isReceiveContractorRT1;
    vector<NodeUUID> mRT1;
    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> mRT2;
    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> mRT3;
};


#endif //GEO_NETWORK_CLIENT_FINDPATHTRANSACTION_H
