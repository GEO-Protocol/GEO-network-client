#ifndef OUTGOINGNODESHANDLER_H
#define OUTGOINGNODESHANDLER_H

#include "OutgoingRemoteNode.h"
#include "OutgoingRemoteNodeNew.h"
#include "OutgoingRemoteAddressNode.h"
#include "../../../../contractors/ContractorsManager.h"

#include <boost/unordered/unordered_map.hpp>
#include <forward_list>

using namespace std;


class OutgoingNodesHandler {
public:
    OutgoingNodesHandler (
        IOService &ioService,
        UDPSocket &socket,
        UUID2Address &UUID2AddressService,
        ContractorsManager *contractorsManager,
        Logger &logger)
        noexcept;

    OutgoingRemoteNode* handler(
        const NodeUUID &nodeUUID)
        noexcept;

    OutgoingRemoteNodeNew* handler(
        const ContractorID contractorID)
        noexcept;

    OutgoingRemoteAddressNode* handler(
        const BaseAddress::Shared address)
        noexcept;

protected:
    static chrono::seconds kHandlersTTL()
        noexcept;

protected:
    void rescheduleCleaning()
        noexcept;

    void removeOutdatedHandlers()
        noexcept;

    void removeOutdatedHandlersNew()
        noexcept;

    void removeOutdatedAddressHandlers()
        noexcept;

    static string logHeader()
        noexcept;

    LoggerStream debug() const
        noexcept;

protected:
    boost::unordered_map<NodeUUID, OutgoingRemoteNode::Unique> mNodes;
    boost::unordered_map<ContractorID, OutgoingRemoteNodeNew::Unique> mNodesNew;
    boost::unordered_map<string, OutgoingRemoteAddressNode::Unique> mAddressNodes;
    boost::unordered_map<NodeUUID, DateTime> mLastAccessDateTimes;
    boost::unordered_map<ContractorID, DateTime> mLastAccessDateTimesNew;
    boost::unordered_map<string, DateTime> mLastAccessDateTimesAddress;

    boost::asio::steady_timer mCleaningTimer;

    IOService &mIOService;
    UDPSocket &mSocket;
    UUID2Address &mUUID2AddressService;
    ContractorsManager *mContractorsManager;
    Logger &mLog;
};

#endif // OUTGOINGNODESHANDLER_H
