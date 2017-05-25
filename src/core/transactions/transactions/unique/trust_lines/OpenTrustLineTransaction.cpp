#include "OpenTrustLineTransaction.h"


OpenTrustLineTransaction::OpenTrustLineTransaction(
    const NodeUUID &nodeUUID,
    OpenTrustLineCommand::Shared command,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger &logger)
    noexcept :

    TrustLineTransaction(
        BaseTransaction::TransactionType::OpenTrustLineTransactionType,
        nodeUUID,
        logger),
    mCommand(command),
    mTrustLines(manager),
    mStorageHandler(storageHandler)
{}

TransactionResult::SharedConst OpenTrustLineTransaction::initOperation()
{
    const auto kContractor = mCommand->contractorUUID();

    if (kContractor == mNodeUUID) {
        info() << "Attempt to launch transaction against itself was prevented.";
        return resultProtocolError();
    }


    if (mTrustLines->isNeighbor(kContractor) and
        mTrustLines->outgoingTrustAmount(kContractor) > 0) {

        info() << "Attempt to re-open trust line to the node " << kContractor << " prevented. "
               << "There is an outgoing trust line already present.";
        return resultTrustLineIsAlreadyPresent();
    }

    // Requesting remote node to open trust line.
    // It is OK, if this message would be lost: in this case trust line would not be opened,
    // and no trust lines desync will occure.
    //
    // In case if message would be received by the contractor, it would accept the trust line,
    // but the response would be lost - then trasnaction desync would appear,
    // but it would also handled on the next operations
    // (for example, in payment operations).
    sendMessage<OpenTrustLineMessage>(
        kContractor,
        mNodeUUID,
        mTransactionUUID,
        mCommand->amount());

    mStep = Stages::ResponseProcessing;
    return resultWaitForMessageTypes(
        {Message::ResponseMessageType}, // ToDo: replace Message::ResponseMessageType by the proper message
        3000);
}

TransactionResult::SharedConst OpenTrustLineTransaction::run()
{
    switch (mStep) {
    case Stages::Initial:
        return initOperation();

    case Stages::ResponseProcessing:
        return processResponse();

    default:
        throw RuntimeError(
            "OpenTrustLineTransaction::run: "
            "unexpected step occured.");
    }
}

TransactionResult::SharedConst OpenTrustLineTransaction::processResponse()
{
    const auto kContractor = mCommand->contractorUUID();

    // Processing response
    if (mContext.size() == 0) {
        info() << "Attempt to open trust line to the node " << kContractor << " failed. "
               << "Remote node is unexpected.";
        return resultRemoteNodeIsInaccessible();
    }

    const auto kMessage = popNextMessage<Response>();
    if (kMessage->code() != AcceptTrustLineMessage::kResultCodeAccepted) {
        info() << "Attempt to open trust line to the node " << kContractor << " failed. "
               << "Remote node rejected the request.";
        return resultRejected();
    }


    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        mTrustLines->open(
            ioTransaction,
            kContractor,
            mCommand->amount());

        updateHistory(ioTransaction);


        // Launching transaction for routing tables population
        if (mTrustLines->trustLineReadOnly(kContractor)->direction() != TrustLineDirection::Both) {
            const auto kTransaction = make_shared<TrustLineStatesHandlerTransaction>(
                currentNodeUUID(),
                currentNodeUUID(),
                currentNodeUUID(),
                mCommand->contractorUUID(),
                TrustLineStatesHandlerTransaction::Created,
                0,
                mTrustLines,
                mStorageHandler,
                mLog);

            launchSubsidiaryTransaction(kTransaction);
        }

        info() << "Trust line to the node " << kContractor << " was successfully opened.";
        return resultOK();


    } catch (ConflictError &) {
        ioTransaction->rollback();
        info() << "Attempt to open trust line to the node " << kContractor << " failed. "
               << "It seems that o  ther transaction already opened the trust line during response receiveing.";
        return resultTrustLineIsAlreadyPresent();

    } catch (IOError &e) {
        ioTransaction->rollback();
        info() << "Attempt to open trust line to the node " << kContractor << " failed. "
               << "IO transaction can't be completed. "
               << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish propely and no result may be returned.
        throw e;
    }
}

void OpenTrustLineTransaction::updateHistory(
    IOTransaction::Shared ioTransaction)
{
    auto record = make_shared<TrustLineRecord>(
        uuid(mTransactionUUID),
        TrustLineRecord::Opening,
        mCommand->contractorUUID(),
        mCommand->amount());

    ioTransaction->historyStorage()->saveTrustLineRecord(record);
}

TransactionResult::SharedConst OpenTrustLineTransaction::resultOK()
{
    return transactionResultFromCommand(
        mCommand->responseCreated());
}

TransactionResult::SharedConst OpenTrustLineTransaction::resultTrustLineIsAlreadyPresent()
{
    return transactionResultFromCommand(
        mCommand->responseTrustlineIsAlreadyPresent());
}

TransactionResult::SharedConst OpenTrustLineTransaction::resultRejected()
{
    return transactionResultFromCommand(
        mCommand->responseTrustlineRejected());
}

TransactionResult::SharedConst OpenTrustLineTransaction::resultRemoteNodeIsInaccessible()
{
    return transactionResultFromCommand(
        mCommand->responseRemoteNodeIsInaccessible());
}

TransactionResult::SharedConst OpenTrustLineTransaction::resultProtocolError() {
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

const string OpenTrustLineTransaction::logHeader() const
{
    stringstream s;
    s << "[OpenTrustLineTA: " << currentTransactionUUID() << "]";
    return s.str();
}
