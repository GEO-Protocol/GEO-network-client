#include "ResetTrustLineSourceTransaction.h"

ResetTrustLineSourceTransaction::ResetTrustLineSourceTransaction(
    ResetTrustLineCommand::Shared command,
    ContractorsManager *contractorsManager,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::ResetTrustLineSourceTransactionType,
        command->equivalent(),
        logger),
    mCommand(command),
    mCountSendingAttempts(0),
    mContractorsManager(contractorsManager),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mKeysStore(keystore)
{}

TransactionResult::SharedConst ResetTrustLineSourceTransaction::run()
{
    info() << "step " << mStep;
    switch (mStep) {
        case Stages::Initialization: {
            return runInitializationStage();
        }
        case Stages::ResponseProcessing: {
            return runResponseProcessingStage();
        }
        default:
            throw ValueError(logHeader() + "::run: wrong value of mStep");
    }
}

TransactionResult::SharedConst ResetTrustLineSourceTransaction::runInitializationStage()
{
    info() << "Try reset TL to Contractor: " << mCommand->contractorID();
    if (!mContractorsManager->contractorPresent(mCommand->contractorID())) {
        warning() << "There is no contractor with requested ID";
        return resultProtocolError();
    }
    mContractorID = mCommand->contractorID();

    if (!mTrustLines->trustLineIsPresent(mContractorID)) {
        warning() << "Attempt to change not existing TL";
        return resultProtocolError();
    }

    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorID));
    auto ioTransaction = mStorageHandler->beginTransaction();

    try {
        auto actualAudit = keyChain.actualFullAudit(
            ioTransaction);
        if (actualAudit->auditNumber() >= mCommand->auditNumber()) {
            warning() << "Attempt to set invalid audit number " << mCommand->auditNumber()
                      << ". Actual audit number is " << actualAudit->auditNumber();
            return resultProtocolError();
        }

    } catch (IOError &e) {
        error() << "Can't check audit number data. Details " << e.what();
        throw e;
    }

    mTrustLines->setTrustLineAuditNumber(
        mContractorID,
        mCommand->auditNumber());

    mTrustLines->resetTrustLine(
        mContractorID,
        mCommand->incomingTrustAmount(),
        mCommand->outgoingTrustAmount(),
        mCommand->balance());

    info() << "Trust line was reset into values: audit: " << mCommand->auditNumber()
           << " ITA: " << mCommand->incomingTrustAmount()
           << " OTA: " << mCommand->outgoingTrustAmount()
           << " BA: " << mCommand->balance();

    sendMessage<TrustLineResetMessage>(
        mContractorID,
        mEquivalent,
        mContractorsManager->contractor(mContractorID),
        mTransactionUUID,
        mTrustLines->auditNumber(mContractorID),
        mCommand->incomingTrustAmount(),
        mCommand->outgoingTrustAmount(),
        mCommand->balance());
    mCountSendingAttempts++;
    info() << "Message with TL reset request was sent";

    mStep = ResponseProcessing;
    return resultOK();
}

TransactionResult::SharedConst ResetTrustLineSourceTransaction::runResponseProcessingStage()
{
    if (mContext.empty()) {
        warning() << "Contractor don't send response.";
        if (mCountSendingAttempts < kMaxCountSendingAttempts) {
            sendMessage<TrustLineResetMessage>(
                mContractorID,
                mEquivalent,
                mContractorsManager->contractor(mContractorID),
                mTransactionUUID,
                mTrustLines->auditNumber(mContractorID),
                mCommand->incomingTrustAmount(),
                mCommand->outgoingTrustAmount(),
                mCommand->balance());
            mCountSendingAttempts++;
            info() << "Send message " << mCountSendingAttempts << " times";
            return resultWaitForMessageTypes(
                {Message::TrustLines_Confirmation},
                kWaitMillisecondsForResponse);
        }
        info() << "Transaction will be closed";
        return resultDone();
    }
    auto message = popNextMessage<TrustLineConfirmationMessage>();
    info() << "contractor " << message->idOnReceiverSide << " send response on reset TL";
    if (message->idOnReceiverSide != mContractorID) {
        warning() << "Sender is not contractor of this transaction";
        return resultContinuePreviousState();
    }
    if (!mTrustLines->trustLineIsPresent(mContractorID)) {
        error() << "Something wrong, because TL must be created";
        // todo : need correct reaction
        return resultDone();
    }

    if (message->state() != ConfirmationMessage::OK) {
        warning() << "Contractor didn't accept reset TL. Response code: " << message->state();
        return resultDone();
    }

    info() << "Contractor accepted resetting TL";
    mTrustLines->setTrustLineState(
        mContractorID,
        TrustLine::Active);

    auditSignal(mContractorID, mEquivalent);
    return resultDone();
}

TransactionResult::SharedConst ResetTrustLineSourceTransaction::resultOK()
{
    return transactionResultFromCommandAndWaitForMessageTypes(
        mCommand->responseOK(),
        {Message::TrustLines_Confirmation},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst ResetTrustLineSourceTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

TransactionResult::SharedConst ResetTrustLineSourceTransaction::resultUnexpectedError()
{
    return transactionResultFromCommand(
        mCommand->responseUnexpectedError());
}

const string ResetTrustLineSourceTransaction::logHeader() const
{
    stringstream s;
    s << "[ResetTrustLineSourceTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

//void ResetTrustLineSourceTransaction::populateHistory(
//    IOTransaction::Shared ioTransaction,
//    TrustLineRecord::TrustLineOperationType operationType)
//{
//#ifndef TESTS
//    auto record = make_shared<TrustLineRecord>(
//            mTransactionUUID,
//            operationType,
//            mContractorsManager->contractor(mContractorID),
//            0);
//
//    ioTransaction->historyStorage()->saveTrustLineRecord(
//            record,
//            mEquivalent);
//#endif
//}