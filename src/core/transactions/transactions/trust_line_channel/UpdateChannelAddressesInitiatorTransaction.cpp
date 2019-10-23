#include "UpdateChannelAddressesInitiatorTransaction.h"

UpdateChannelAddressesInitiatorTransaction::UpdateChannelAddressesInitiatorTransaction(
    ContractorsManager *contractorsManager,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::UpdateChannelAddressesInitiator,
        0,
        logger),
    mContractorsManager(contractorsManager)
{}

TransactionResult::SharedConst UpdateChannelAddressesInitiatorTransaction::run()
{
    info() << "run";
    for (const auto &contractor : mContractorsManager->allContractors()) {
        info() << "inform contractor " << contractor->getID() << " " << contractor->mainAddress()->fullAddress();
        // todo : apply logic for guaranteed delivery of this message
        sendMessage<UpdateChannelAddressesMessage>(
            contractor->getID(),
            contractor,
            mTransactionUUID,
            mContractorsManager->ownAddresses());
    }
    info() << "All contractors were informed";
    return resultDone();
}

const string UpdateChannelAddressesInitiatorTransaction::logHeader() const
{
    stringstream s;
    s << "[UpdateChannelAddressesTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}