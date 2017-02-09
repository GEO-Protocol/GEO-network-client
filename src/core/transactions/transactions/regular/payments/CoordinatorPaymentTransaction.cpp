#include "CoordinatorPaymentTransaction.h"

CoordinatorPaymentTransaction::CoordinatorPaymentTransaction(
    NodeUUID &currentNodeUUID,
    CreditUsageCommand::Shared command,
    TrustLinesManager *trustLines,
    Logger *log) :

    BaseTransaction(
        BaseTransaction::CoordinatorPaymentTransaction, currentNodeUUID),
    mCommand(command),
    mTrustLines(trustLines),
    mLog(log){
}

/*!
 * Deserialization constructor.
 * Sets "mCommand", "mNodeUUID" and other transaction specific fields from the bytes "buffer".
 *
 * "trustLines" is pointer and can't be deserialized.
 */
CoordinatorPaymentTransaction::CoordinatorPaymentTransaction(
    BytesShared buffer,
    TrustLinesManager *trustLines) :

    BaseTransaction(
        BaseTransaction::CoordinatorPaymentTransaction),
    mTrustLines(trustLines) {

    throw ValueError("Not implemented");
}

TransactionResult::Shared CoordinatorPaymentTransaction::run() {

    // todo: optimisation
    // Check if total outgoing possibilities of this node
    // are not smaller, than total operation amount.
    // In case if so - there is no reason to begin the operation:
    // current node would not be able to pay such an amount.

    switch (mStep) {
        case 1: {

#ifdef TRANSACTIONS_LOG
            {
                auto info = mLog->info("CoordinatorPaymentTransaction");
                info << "(UUID: " << UUID() << ") "
                     << "Initialising payment operation to node " << mCommand->contractorUUID();
            }
#endif
            return initPaymentOperation();
        }

        case 2: {
#ifdef TRANSACTIONS_LOG
            {
                auto info = mLog->info("CoordinatorPaymentTransaction");
                info << "awakened";
            }
#endif
        }
    }

    return resultOK();
}


TransactionResult::Shared CoordinatorPaymentTransaction::initPaymentOperation() {
    auto message =
        make_shared<ReceiverInitPaymentMessage>(
            mCommand->amount());

    addMessage(
        message,
        mCommand->contractorUUID());

    mStep += 1;
    return make_shared<TransactionResult>(
        TransactionState::awakeAfterMilliseconds(100));
}

TransactionResult::Shared CoordinatorPaymentTransaction::resultOK() const {
    return transactionResultFromCommand(
        mCommand->resultOk());
}

pair<BytesShared, size_t> CoordinatorPaymentTransaction::serializeToBytes() {
    throw ValueError("Not implemented");
}

void CoordinatorPaymentTransaction::deserializeFromBytes(BytesShared buffer) {
    throw ValueError("Not implemented");
}
