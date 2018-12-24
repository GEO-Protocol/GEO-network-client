#ifndef GEO_NETWORK_CLIENT_PUBLICKEYSSHARINGSOURCETRANSACTION_H
#define GEO_NETWORK_CLIENT_PUBLICKEYSSHARINGSOURCETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/trust_lines/ShareKeysCommand.h"
#include "../../../contractors/ContractorsManager.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../crypto/keychain.h"

#include "../../../subsystems_controller/TrustLinesInfluenceController.h"

#include "../../../network/messages/trust_lines/PublicKeysSharingInitMessage.h"
#include "../../../network/messages/trust_lines/PublicKeyMessage.h"
#include "../../../network/messages/trust_lines/PublicKeyHashConfirmation.h"

class PublicKeysSharingSourceTransaction : public BaseTransaction {

public:
    typedef shared_ptr<PublicKeysSharingSourceTransaction> Shared;

public:
    PublicKeysSharingSourceTransaction(
        ContractorID contractorID,
        const SerializedEquivalent equivalent,
        ContractorsManager *contractorsManager,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Keystore *keystore,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &logger);

    PublicKeysSharingSourceTransaction(
        ShareKeysCommand::Shared command,
        ContractorsManager *contractorsManager,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Keystore *keystore,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    enum Stages {
        Initialization = 1,
        CommandInitialization = 2,
        ResponseProcessing = 3,
    };

protected: // log
    const string logHeader() const;

protected:
    TransactionResult::SharedConst resultOK();

    TransactionResult::SharedConst resultProtocolError();

    TransactionResult::SharedConst resultUnexpectedError();

private:
    TransactionResult::SharedConst runPublicKeysSharingInitializationStage();

    TransactionResult::SharedConst runCommandPublicKeysSharingInitializationStage();

    TransactionResult::SharedConst runPublicKeysSendNextKeyStage();

protected:
    // these constants should be the same as in PublicKeysSharingTargetTransaction
    static const uint32_t kWaitMillisecondsForResponse = 20000;
    static const uint16_t kMaxCountSendingAttempts = 3;

private:
    ShareKeysCommand::Shared mCommand;
    ContractorsManager *mContractorsManager;
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;
    Keystore *mKeysStore;

    ContractorID mContractorID;
    KeyNumber mCurrentKeyNumber;
    KeysCount mKeysCount;
    lamport::PublicKey::Shared mCurrentPublicKey;

    uint16_t mCountSendingAttempts;

    TrustLinesInfluenceController *mTrustLinesInfluenceController;
};


#endif //GEO_NETWORK_CLIENT_PUBLICKEYSSHARINGSOURCETRANSACTION_H
