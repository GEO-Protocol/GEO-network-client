#ifndef OUTGOINGNODESHANDLER_H
#define OUTGOINGNODESHANDLER_H

#include "OutgoingRemoteNode.h"
#include "OutgoingRemoteAddressNode.h"
#include "OutgoingRemoteProvider.h"

#include <boost/unordered/unordered_map.hpp>
#include <forward_list>

using namespace std;


class OutgoingNodesHandler {
public:
    OutgoingNodesHandler (
        IOService &ioService,
        UDPSocket &socket,
        ContractorsManager *contractorsManager,
        Logger &logger)
        noexcept;

    OutgoingRemoteNode* handler(
        const ContractorID contractorID)
        noexcept;

    OutgoingRemoteAddressNode* handler(
        const BaseAddress::Shared address)
        noexcept;

    OutgoingRemoteProvider* handler(
        const Provider::Shared provider)
        noexcept;

protected:
    static chrono::seconds kHandlersTTL()
        noexcept;

protected:
    void rescheduleCleaning()
        noexcept;

    void removeOutdatedHandlers()
        noexcept;

    void removeOutdatedAddressHandlers()
        noexcept;

    void removeOutdatedProviderHandlers()
        noexcept;

    static string logHeader()
        noexcept;

    LoggerStream debug() const
        noexcept;

protected:
    boost::unordered_map<ContractorID, OutgoingRemoteNode::Unique> mNodes;
    boost::unordered_map<string, OutgoingRemoteAddressNode::Unique> mAddressNodes;
    boost::unordered_map<string, OutgoingRemoteProvider::Unique> mProviders;
    boost::unordered_map<ContractorID, DateTime> mLastAccessDateTimes;
    boost::unordered_map<string, DateTime> mLastAccessDateTimesAddress;
    boost::unordered_map<string, DateTime> mLastAccessDateTimesProvider;

    boost::asio::steady_timer mCleaningTimer;

    IOService &mIOService;
    UDPSocket &mSocket;
    ContractorsManager *mContractorsManager;
    Logger &mLog;
};

#endif // OUTGOINGNODESHANDLER_H
