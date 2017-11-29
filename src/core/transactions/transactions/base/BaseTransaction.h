#ifndef GEO_NETWORK_CLIENT_BASETRANSACTION_H
#define GEO_NETWORK_CLIENT_BASETRANSACTION_H

#include "TransactionUUID.h"

#include "../../../common/Types.h"
#include "../../../common/NodeUUID.h"
#include "../../../common/memory/MemoryUtils.h"

#include "../../../network/messages/Message.hpp"
#include "../result/TransactionResult.h"
#include "../result/state/TransactionState.h"
#include "../../../network/messages/result/MessageResult.h"
#include "../../../interface/results_interface/result/CommandResult.h"
#include "../../../resources/resources/BaseResource.h"

#include "../../../common/exceptions/RuntimeError.h"

#include "../../../logger/Logger.h"

#include <boost/signals2.hpp>

#include <vector>
#include <deque>
#include <utility>
#include <cstdint>
#include <sstream>


namespace signals = boost::signals2;

class BaseTransaction {
public:
    typedef shared_ptr<BaseTransaction> Shared;
    typedef uint16_t SerializedTransactionType;

    typedef signals::signal<void(Message::Shared, const NodeUUID&)> SendMessageSignal;
    typedef signals::signal<void(BaseTransaction::Shared)> LaunchSubsidiaryTransactionSignal;

public:
    // TODO: add other states shortcuts here
    TransactionResult::Shared resultDone () const;
    TransactionResult::Shared resultFlushAndContinue() const;
    TransactionResult::Shared resultWaitForMessageTypes(
        vector<Message::MessageType> &&requiredMessagesTypes,
        uint32_t noLongerThanMilliseconds) const;
    TransactionResult::Shared resultAwaikAfterMilliseconds(
        uint32_t responseWaitTime) const ;
    TransactionResult::Shared resultContinuePreviousState() const;

public:
    virtual ~BaseTransaction() = default;

    enum TransactionType {
        SetOutgoingTrustLineTransaction = 100,
        SetIncomingTrustLineTransaction = 101,

        // Cycles
        Cycles_ThreeNodesInitTransaction = 200,
        Cycles_ThreeNodesReceiverTransaction = 201,
        Cycles_FourNodesInitTransaction = 202,
        Cycles_FourNodesReceiverTransaction = 203,
        Cycles_FiveNodesInitTransaction = 204,
        Cycles_FiveNodesReceiverTransaction = 205,
        Cycles_SixNodesInitTransaction = 206,
        Cycles_SixNodesReceiverTransaction = 207,

        // Payments
        CoordinatorPaymentTransaction = 301,
        ReceiverPaymentTransaction = 302,
        IntermediateNodePaymentTransaction = 303,
        VoutesStatusResponsePaymentTransaction = 304,
        Payments_CycleCloserInitiatorTransaction = 305,
        Payments_CycleCloserIntermediateNodeTransaction = 306,

        // Max flow calculation
        CollectTopologyTransactionType = 401,
        InitiateMaxFlowCalculationTransactionType = 402,
        ReceiveMaxFlowCalculationOnTargetTransactionType = 403,
        MaxFlowCalculationSourceFstLevelTransactionType = 404,
        MaxFlowCalculationTargetFstLevelTransactionType = 405,
        MaxFlowCalculationSourceSndLevelTransactionType = 406,
        MaxFlowCalculationTargetSndLevelTransactionType = 407,
        ReceiveResultMaxFlowCalculationTransactionType = 408,
        MaxFlowCalculationCacheUpdateTransactionType = 409,

        // Contractors
        ContractorsList = 500,
        TrustlinesList = 501,
        TrustLineOne = 502,

        // TotalBalances
        TotalBalancesTransactionType = 600,
        InitiateTotalBalancesFromRemoutNodeTransactionType = 601,

        // History
        HistoryPaymentsTransactionType = 700,
        HistoryTrustLinesTransactionType = 701,
        HistoryWithContractorTransactionType = 702,
        HistoryAdditionalPaymentsTransactionType = 703,

        // FindPath
        FindPathByMaxFlowTransactionType = 800,

        //RoutingTable
        RoutingTableInitTransactionType = 900,
        RoutingTableResponceTransactionType = 901,

        // Transactions
        TransactionByCommandUUIDType = 1000
    };

public:
    virtual const TransactionType transactionType() const;

    const TransactionUUID &currentTransactionUUID () const;

    const NodeUUID &currentNodeUUID () const;

    const int currentStep() const;


    void recreateTransactionUUID();

    void pushContext(
        Message::Shared message);

    void pushResource(
        BaseResource::Shared resource);

    virtual pair<BytesShared, size_t> serializeToBytes() const;

    virtual TransactionResult::SharedConst run() = 0;

protected:
    // TODO: make logger REQUIRED
    BaseTransaction(
        const TransactionType type,
        Logger &log);

    // TODO: make logger REQUIRED
    BaseTransaction(
        const TransactionType type,
        const TransactionUUID &transactionUUID,
        Logger &log);

    // TODO: make logger REQUIRED
    BaseTransaction(
        const TransactionType type,
        const NodeUUID &nodeUUID,
        Logger &log);

    // TODO: make logger REQUIRED
    BaseTransaction(
        const TransactionType type,
        const TransactionUUID &transactionUUID,
        const NodeUUID &nodeUUID,
        Logger &log);

    BaseTransaction(
        BytesShared buffer,
        const NodeUUID &nodeUUID,
        Logger &log);

    [[deprecated("Use sendMessage() instead.")]]
    void addMessage(
        Message::Shared message,
        const NodeUUID &nodeUUID);

    // TODO: convert to hpp?
    template <typename ContextMessageType>
    inline shared_ptr<ContextMessageType> popNextMessage()
    {
        const auto message = static_pointer_cast<ContextMessageType>(mContext.front());
        mContext.pop_front();
        return message;
    }

    template<typename ResourceType>
    inline shared_ptr<ResourceType> popNextResource() {

        const auto resource = static_pointer_cast<ResourceType>(mResources.front());
        mResources.pop_front();
        return resource;
    }

    // TODO: convert to hpp?
    template <typename MessageType, typename... Args>
    inline void sendMessage(
        const NodeUUID &addressee,
        Args&&... args) const
    {
        const auto message = make_shared<MessageType>(args...);
        outgoingMessageIsReadySignal(
            message,
            addressee);
    }

    inline void sendMessage(
        const NodeUUID &addressee,
        const Message::Shared message) const
    {
        outgoingMessageIsReadySignal(
            message,
            addressee);
    }

    void launchSubsidiaryTransaction(
      BaseTransaction::Shared transaction);

    [[deprecated("Use stages enum instead. See payment operations as example")]]
    void increaseStepsCounter();

    [[deprecated("Use stages enum instead. See payment operations as example")]]
    void resetStepsCounter();

    [[deprecated]]
    void setExpectationResponsesCounter(
        uint16_t count);

    void resetExpectationResponsesCounter();

    void clearContext();

    virtual void deserializeFromBytes(
        BytesShared buffer);

    static const size_t kOffsetToInheritedBytes();

    TransactionResult::SharedConst transactionResultFromCommand(
        CommandResult::SharedConst result) const;

    TransactionResult::SharedConst transactionResultFromMessage(
        MessageResult::SharedConst result) const;

    TransactionResult::SharedConst transactionResultFromState(
        TransactionState::SharedConst state) const;

    [[deprecated]]
    TransactionResult::SharedConst finishTransaction();

    virtual const string logHeader() const;
    LoggerStream info() const;
    LoggerStream warning() const;
    LoggerStream error() const;
    LoggerStream debug() const;

public:
    mutable SendMessageSignal outgoingMessageIsReadySignal;
    mutable LaunchSubsidiaryTransactionSignal runSubsidiaryTransactionSignal;

protected:
    uint16_t mkStandardConnectionTimeout = 1500; //miliseconds
    uint16_t mkExpectationResponsesCount = 0;
    uint16_t mkWaitingForResponseTime = 3000;
    uint16_t mStep = 1;
    uint8_t mVotesRecoveryStep = 0;

protected:
    TransactionType mType;
    TransactionUUID mTransactionUUID;
    NodeUUID mNodeUUID;
    deque<Message::Shared> mContext;
    deque<BaseResource::Shared> mResources;

    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_BASETRANSACTION_H
