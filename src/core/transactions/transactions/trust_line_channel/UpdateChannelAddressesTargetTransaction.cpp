#include "UpdateChannelAddressesTargetTransaction.h"

UpdateChannelAddressesTargetTransaction::UpdateChannelAddressesTargetTransaction(
    UpdateChannelAddressesMessage::Shared message,
    ContractorsManager *contractorsManager,
    StorageHandler *storageHandler,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::UpdateChannelAddressesTarget,
        message->transactionUUID(),
        0,
        logger),
    mMessage(message),
    mContractorsManager(contractorsManager),
    mStorageHandler(storageHandler)
{}

TransactionResult::SharedConst UpdateChannelAddressesTargetTransaction::run()
{
    info() << "Contractor " << mMessage->idOnReceiverSide << " send new addresses:";
    for (const auto &address : mMessage->newSenderAddresses()) {
        info() << address->fullAddress();
    }
    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        mContractorsManager->updateContractorAddresses(
            ioTransaction,
            mMessage->idOnReceiverSide,
            mMessage->newSenderAddresses());
    } catch (NotFoundError &e) {
        error() << "There is no contractor with ID " << mMessage->idOnReceiverSide;
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't update contractor addresses. Details " << e.what();
    }

    return resultDone();
}

const string UpdateChannelAddressesTargetTransaction::logHeader() const
{
    stringstream s;
    s << "[UpdateChannelAddressesTargetTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}