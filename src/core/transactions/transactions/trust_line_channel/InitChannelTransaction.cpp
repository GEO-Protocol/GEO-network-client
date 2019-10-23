#include "InitChannelTransaction.h"

InitChannelTransaction::InitChannelTransaction(
    InitChannelCommand::Shared command,
    ContractorsManager *contractorsManager,
    StorageHandler *storageHandler,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::OpenChannelTransaction,
        0,
        logger),
    mCommand(command),
    mCountSendingAttempts(0),
    mContractorsManager(contractorsManager),
    mStorageHandler(storageHandler)
{}

TransactionResult::SharedConst InitChannelTransaction::run()
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

TransactionResult::SharedConst InitChannelTransaction::runInitializationStage()
{
    info() << "Try init channel to Contractor on addresses:";
    for (const auto &address : mCommand->contractorAddresses()) {
        info() << address->fullAddress();
    }
    if (mCommand->contractorAddresses().empty()) {
        warning() << "Contractor addresses are empty";
        return resultProtocolError();
    }
    if (mContractorsManager->selfContractor()->containsAtLeastOneAddress(
        mCommand->contractorAddresses())) {
        warning() << "Contractor's addresses contain at least one address is equal to own address";
        return resultProtocolError();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        if (mCommand->cryptoKey().empty()) {
            info() << "Channel crypto key generation";
            mContractor = mContractorsManager->createContractor(
                ioTransaction,
                mCommand->contractorAddresses());
            info() << "Init channel to contractor with ID " << mContractor->getID()
                   << " and crypto key " << *mContractor->cryptoKey()->publicKey;
            return resultOK();
        } else {
            info() << "Channel crypto key: " << mCommand->cryptoKey();
            mContractor = mContractorsManager->createContractor(
                ioTransaction,
                mCommand->contractorAddresses(),
                mCommand->cryptoKey(),
                mCommand->contractorChannelID());
            info() << "Init channel to contractor with ID " << mContractor->getID();
            info() << "Channel ID on contractor side " << mContractor->ownIdOnContractorSide();
            sendMessage<InitChannelMessage>(
                mContractor->getID(),
                mContractorsManager->ownAddresses(),
                mTransactionUUID,
                mContractor);
            mStep = ResponseProcessing;
            return resultOKAndWaitMessage();
        }
    } catch (ValueError &e) {
        error() << "Can't create channel. Details: " << e.what();
        return resultChannelAlreadyExist();
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Error during creating channel. Details: " << e.what();
        return resultUnexpectedError();
    }
}

TransactionResult::SharedConst InitChannelTransaction::runResponseProcessingStage()
{
    if (mContext.empty()) {
        warning() << "Contractor don't send response.";
        if (mCountSendingAttempts < kMaxCountSendingAttempts) {
            sendMessage<InitChannelMessage>(
                mContractor->getID(),
                mContractorsManager->ownAddresses(),
                mTransactionUUID,
                mContractor);
            mCountSendingAttempts++;
            info() << "Send message " << mCountSendingAttempts << " times";
            return resultWaitForMessageTypes(
                {Message::Channel_Confirm},
                kWaitMillisecondsForResponse);
        }
        info() << "Channel was not confirm";
        return resultDone();
    }
    auto message = popNextMessage<ConfirmChannelMessage>();
    // todo : check response
    info() << "contractor " << message->idOnReceiverSide << " send channel confirmation";
    return resultDone();
}

TransactionResult::SharedConst InitChannelTransaction::resultOK()
{
    stringstream ss;
    ss << mContractor->getID() << kTokensSeparator << *mContractor->cryptoKey()->publicKey;
    auto channelInfo = ss.str();
    return transactionResultFromCommand(
        mCommand->responseOk(channelInfo));
}

TransactionResult::SharedConst InitChannelTransaction::resultOKAndWaitMessage()
{
    stringstream ss;
    ss << mContractor->getID() << kTokensSeparator << *mContractor->cryptoKey()->publicKey;
    auto channelInfo = ss.str();
    return transactionResultFromCommandAndWaitForMessageTypes(
        mCommand->responseOk(channelInfo),
        {Message::Channel_Confirm},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst InitChannelTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

TransactionResult::SharedConst InitChannelTransaction::resultChannelAlreadyExist()
{
    return transactionResultFromCommand(
        mCommand->responseAlreadyCreated());
}

TransactionResult::SharedConst InitChannelTransaction::resultUnexpectedError()
{
    return transactionResultFromCommand(
        mCommand->responseUnexpectedError());
}

const string InitChannelTransaction::logHeader() const
{
    stringstream s;
    s << "[InitChannelTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}