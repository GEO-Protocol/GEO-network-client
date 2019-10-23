#ifndef GEO_NETWORK_CLIENT_BASETRANSACTION_H
#define GEO_NETWORK_CLIENT_BASETRANSACTION_H

#include "TransactionUUID.h"

#include "../../../common/Types.h"
#include "../../../common/memory/MemoryUtils.h"

#include "../../../network/messages/base/transaction/ConfirmationMessage.h"
#include "../../../network/communicator/internal/incoming/TailManager.h"
#include "../result/TransactionResult.h"
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
    typedef uint16_t SerializedStep;

    typedef signals::signal<void(Message::Shared, const ContractorID)> SendMessageSignal;
    typedef signals::signal<void(Message::Shared, BaseAddress::Shared)> SendMessageToAddressSignal;
    typedef signals::signal<void(
            TransactionMessage::Shared,
            ContractorID,
            Message::MessageType,
            uint32_t)> SendMessageWithCachingSignal;
    typedef signals::signal<void(BaseTransaction::Shared)> LaunchSubsidiaryTransactionSignal;
    typedef signals::signal<void(ConfirmationMessage::Shared)> ProcessConfirmationMessageSignal;
    typedef signals::signal<void(ContractorID, const SerializedEquivalent, bool)> TrustLineActionSignal;
    typedef signals::signal<void(ContractorID, const SerializedEquivalent)> PublicKeysSharingSignal;
    typedef signals::signal<void(ContractorID, const SerializedEquivalent)> AuditSignal;
    typedef signals::signal<void(ContractorID)> ProcessPongMessageSignal;

public:
    virtual ~BaseTransaction() = default;

    enum TransactionType {
        // Trust lines
        OpenTrustLineTransaction = 100,
        AcceptTrustLineTransaction = 101,
        SetOutgoingTrustLineTransaction = 102,
        CloseIncomingTrustLineTransactionType = 103,
        PublicKeysSharingSourceTransactionType = 104,
        PublicKeysSharingTargetTransactionType = 105,
        AuditSourceTransactionType = 106,
        AuditTargetTransactionType = 107,
        ConflictResolverInitiatorTransactionType = 108,
        ConflictResolverContractorTransactionType = 109,
        CheckTrustLineAfterPaymentTransactionType = 110,
        RemoveTrustLineTransactionType = 111,
        ResetTrustLineSourceTransactionType = 112,
        ResetTrustLineDestinationTransactionType = 113,

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
        MaxFlowCalculationStepTwoTransactionType = 408,
        MaxFlowCalculationFullyTransactionType = 409,

        // TrustLine list
        TrustLinesList = 500,
        TrustLineOneByAddress = 501,
        TrustLineOneByID = 502,
        EquivalentsList = 503,

        // TotalBalances
        TotalBalancesTransactionType = 600,

        // History
        HistoryPaymentsTransactionType = 700,
        HistoryTrustLinesTransactionType = 701,
        HistoryWithContractorTransactionType = 702,
        HistoryAdditionalPaymentsTransactionType = 703,

        // FindPath
        FindPathByMaxFlowTransactionType = 800,

        // Channels
        OpenChannelTransaction = 900,
        ConfirmChannelTransaction = 901,
        ContractorsList = 902,
        ChannelInfo = 903,
        ChannelInfoByAddresses = 904,
        UpdateChannelAddressesInitiator = 905,
        UpdateChannelAddressesTarget = 906,
        SetChannelContractorAddresses = 907,
        SetChannelContractorCryptoKey = 908,
        RegenerateChannelCryptoKey = 909,

        // Transactions
        TransactionByCommandUUIDType = 1000,

        // Gateway notification
        GatewayNotificationSenderType = 1100,
        GatewayNotificationReceiverType = 1101,

        // General
        NoEquivalentType = 1200,
        PongReactionType = 1201
    };

public:
    virtual const TransactionType transactionType() const;

    const TransactionUUID &currentTransactionUUID () const;

    const SerializedEquivalent equivalent() const;

    const int currentStep() const;

    const DateTime timeStarted() const;

    void recreateTransactionUUID();

    void pushContext(
        Message::Shared message);

    void pushResource(
        BaseResource::Shared resource);

    virtual pair<BytesShared, size_t> serializeToBytes() const;

    static const size_t kOffsetToInheritedBytes();

    virtual TransactionResult::SharedConst run() = 0;

protected:
    BaseTransaction(
        const TransactionType type,
        Logger &log);

    BaseTransaction(
        const TransactionType type,
        const TransactionUUID &transactionUUID,
        Logger &log);

    BaseTransaction(
        const TransactionType type,
        const SerializedEquivalent equivalent,
        Logger &log);

    BaseTransaction(
        const TransactionType type,
        const TransactionUUID &transactionUUID,
        const SerializedEquivalent equivalent,
        Logger &log);

    BaseTransaction(
        BytesShared buffer,
        Logger &log);

    // TODO: convert to hpp?
    template <typename ContextMessageType>
    inline shared_ptr<ContextMessageType> popNextMessage()
    {
        const auto message = static_pointer_cast<ContextMessageType>(mContext.front());
        mContext.pop_front();
        return message;
    }

    // TODO: convert to hpp?
    template <typename ContextMessageType>
    inline shared_ptr<ContextMessageType> popNextMessage(TailManager::Tail &tail)
    {
        const auto message = static_pointer_cast<ContextMessageType>(tail.front());
        tail.pop_front();
        return message;
    }

    template<typename ResourceType>
    inline shared_ptr<ResourceType> popNextResource()
    {
        const auto resource = static_pointer_cast<ResourceType>(mResources.front());
        mResources.pop_front();
        return resource;
    }

    // TODO: convert to hpp?
    template <typename MessageType, typename... Args>
    inline void sendMessage(
        const ContractorID addressee,
        Args&&... args) const
    {
        const auto message = make_shared<MessageType>(args...);
        outgoingMessageIsReadySignal(
            message,
            addressee);
    }

    template <typename MessageType, typename... Args>
    inline void sendMessage(
        BaseAddress::Shared addressee,
        Args&&... args) const
    {
        const auto message = make_shared<MessageType>(args...);
        outgoingMessageToAddressReadySignal(
            message,
            addressee);
    }

    inline void sendMessage(
        const ContractorID addressee,
        const Message::Shared message) const
    {
        outgoingMessageIsReadySignal(
            message,
            addressee);
    }

    inline void sendMessage(
        BaseAddress::Shared addressee,
        const Message::Shared message) const
    {
        outgoingMessageToAddressReadySignal(
            message,
            addressee);
    }

    template <typename MessageType, typename... Args>
    inline void sendMessageWithTemporaryCaching(
        ContractorID addressee,
        Message::MessageType incomingMessageTypeFilter,
        uint32_t cacheLivingSecondsTime,
        Args&&... args) const
    {
        const auto message = make_shared<MessageType>(args...);
        sendMessageWithCachingSignal(
            message,
            addressee,
            incomingMessageTypeFilter,
            cacheLivingSecondsTime);
    }

    void processConfirmationMessage(
        const ConfirmationMessage::Shared confirmationMessage)
    {
        processConfirmationMessageSignal(
            confirmationMessage);
    }

    void processPongMessage(
        ContractorID contractorID)
    {
        processPongMessageSignal(
            contractorID);
    }

    void launchSubsidiaryTransaction(
        BaseTransaction::Shared transaction);

    void clearContext();

    // TODO: add other states shortcuts here
    TransactionResult::Shared resultDone () const;

    TransactionResult::Shared resultFlushAndContinue() const;

    TransactionResult::Shared resultWaitForMessageTypes(
        vector<Message::MessageType> &&requiredMessagesTypes,
        uint32_t noLongerThanMilliseconds) const;

    TransactionResult::Shared resultWaitForResourceTypes(
        vector<BaseResource::ResourceType> &&requiredResourcesType,
        uint32_t noLongerThanMilliseconds) const;

    TransactionResult::Shared resultWaitForResourceAndMessagesTypes(
        vector<BaseResource::ResourceType> &&requiredResourcesType,
        vector<Message::MessageType> &&requiredMessagesTypes,
        uint32_t noLongerThanMilliseconds) const;

    TransactionResult::Shared resultAwakeAfterMilliseconds(
        uint32_t responseWaitTime) const ;

    TransactionResult::Shared resultContinuePreviousState() const;

    TransactionResult::Shared resultWaitForMessageTypesAndAwakeAfterMilliseconds(
        vector<Message::MessageType> &&requiredMessagesTypes,
        uint32_t noLongerThanMilliseconds) const;

    TransactionResult::Shared resultAwakeAsFastAsPossible() const;

    TransactionResult::Shared transactionResultFromCommand(
        CommandResult::SharedConst result) const;

    TransactionResult::Shared transactionResultFromCommandAndWaitForMessageTypes(
        CommandResult::SharedConst result,
        vector<Message::MessageType> &&requiredMessagesTypes,
        uint32_t noLongerThanMilliseconds) const;

    TransactionResult::Shared transactionResultFromCommandAndAwakeAfterMilliseconds(
        CommandResult::SharedConst result,
        uint32_t responseWaitTime) const;

    virtual const string logHeader() const = 0;
    LoggerStream info() const;
    LoggerStream warning() const;
    LoggerStream error() const;
    LoggerStream debug() const;

public:
    mutable SendMessageSignal outgoingMessageIsReadySignal;
    mutable SendMessageToAddressSignal outgoingMessageToAddressReadySignal;
    mutable SendMessageWithCachingSignal sendMessageWithCachingSignal;
    mutable LaunchSubsidiaryTransactionSignal runSubsidiaryTransactionSignal;
    mutable ProcessConfirmationMessageSignal processConfirmationMessageSignal;
    mutable ProcessPongMessageSignal processPongMessageSignal;
    mutable TrustLineActionSignal trustLineActionSignal;
    mutable PublicKeysSharingSignal publicKeysSharingSignal;
    mutable AuditSignal auditSignal;

protected:
    TransactionType mType;
    TransactionUUID mTransactionUUID;
    SerializedEquivalent mEquivalent;
    deque<Message::Shared> mContext;
    deque<BaseResource::Shared> mResources;
    SerializedStep mStep;
    DateTime mTimeStarted;

    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_BASETRANSACTION_H
