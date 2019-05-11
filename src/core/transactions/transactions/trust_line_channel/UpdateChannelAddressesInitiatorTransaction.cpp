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
        sendMessage<UpdateChannelAddressesMessage>(
            contractor->getID(),
            contractor->ownIdOnContractorSide(),
            mTransactionUUID,
            mContractorsManager->ownAddresses(),
            contractor);
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