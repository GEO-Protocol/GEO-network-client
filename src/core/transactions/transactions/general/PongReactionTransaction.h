#ifndef GEO_NETWORK_CLIENT_PONGREACTIONTRANSACTION_H
#define GEO_NETWORK_CLIENT_PONGREACTIONTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../network/messages/general/PongMessage.h"

#include "../../../equivalents/EquivalentsSubsystemsRouter.h"
#include "../../../io/storage/StorageHandler.h"

namespace signals = boost::signals2;

class PongReactionTransaction : public BaseTransaction {
public:
    typedef shared_ptr<PongReactionTransaction> Shared;

public:
    typedef signals::signal<void(
            const NodeUUID&,
            ContractorID,
            const SerializedEquivalent,
            const TransactionType)> ResumeTransactionSignal;

public:
    PongReactionTransaction(
        const NodeUUID &nodeUUID,
        PongMessage::Shared message,
        EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
        StorageHandler *storageHandler,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

public:
    mutable ResumeTransactionSignal mResumeTransactionSignal;

private:
    NodeUUID mContractorUUID;
    ContractorID mContractorID;
    EquivalentsSubsystemsRouter *mEquivalentsSubsystemsRouter;
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_PONGREACTIONTRANSACTION_H
