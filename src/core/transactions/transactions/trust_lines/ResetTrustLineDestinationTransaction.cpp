#include "ResetTrustLineDestinationTransaction.h"

ResetTrustLineDestinationTransaction::ResetTrustLineDestinationTransaction(
    TrustLineResetMessage::Shared message,
    ContractorsManager *contractorsManager,
    TrustLinesManager *manager,
    Logger &logger):

    BaseTransaction(
        BaseTransaction::ResetTrustLineDestinationTransactionType,
        message->transactionUUID(),
        message->equivalent(),
        logger),
    mMessage(message),
    mContractorID(message->idOnReceiverSide),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(manager)
{}

TransactionResult::SharedConst ResetTrustLineDestinationTransaction::run()
{
    info() << "sender: " << mContractorID;
    if (!mContractorsManager->contractorPresent(mContractorID)) {
        warning() << "There is no contractor with requested ID";
        return resultDone();
    }

    if (!mTrustLinesManager->trustLineIsPresent(mContractorID)) {
        warning() << "Attempt to change not existing TL";
        return sendTrustLineErrorConfirmation(
            ConfirmationMessage::TrustLineIsAbsent);
    }

    if (mTrustLinesManager->trustLineState(mContractorID) != TrustLine::ResetPending) {
        warning() << "Invalid TL state: " << mTrustLinesManager->trustLineState(mContractorID) ;
        return sendTrustLineErrorConfirmation(
            ConfirmationMessage::TrustLineInvalidState);
    }

    info() << "Contractor reset values: audit: " << mMessage->auditNumber()
           << " ITA: " << mMessage->incomingAmount()
           << " OTA: " << mMessage->outgoingAmount()
           << " BA: " << mMessage->balance();

    if (mTrustLinesManager->auditNumber(mContractorID) != mMessage->auditNumber() or
        mTrustLinesManager->incomingTrustAmount(mContractorID) != mMessage->outgoingAmount() or
        mTrustLinesManager->outgoingTrustAmount(mContractorID) != mMessage->incomingAmount() or
        mTrustLinesManager->balance(mContractorID) != -1 * mMessage->balance()) {
        warning() << "Contractor reset data different from current";
        return sendTrustLineErrorConfirmation(
            ConfirmationMessage::Audit_Invalid);
    }

    mTrustLinesManager->setTrustLineState(
        mContractorID,
        TrustLine::Reset);

    // Sending confirmation back.
    sendMessageWithTemporaryCaching<TrustLineConfirmationMessage>(
        mContractorID,
        Message::TrustLines_Reset,
        kWaitMillisecondsForResponse / 1000 * kMaxCountSendingAttempts,
        mEquivalent,
        mContractorsManager->contractor(mContractorID),
        mTransactionUUID,
        false,
        ConfirmationMessage::OK);
    info() << "Confirmation was sent";
    return resultDone();
}

TransactionResult::SharedConst ResetTrustLineDestinationTransaction::sendTrustLineErrorConfirmation(
    ConfirmationMessage::OperationState errorState)
{
    sendMessage<TrustLineConfirmationMessage>(
        mContractorID,
        mEquivalent,
        mContractorsManager->contractor(mContractorID),
        mTransactionUUID,
        false,
        errorState);
    return resultDone();
}

const string ResetTrustLineDestinationTransaction::logHeader() const
{
    stringstream s;
    s << "[ResetTrustLineDestinationTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}