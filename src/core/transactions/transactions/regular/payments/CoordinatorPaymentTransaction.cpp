#include "CoordinatorPaymentTransaction.h"



CoordinatorPaymentTransaction::CoordinatorPaymentTransaction(
    NodeUUID &currentNodeUUID,
    CreditUsageCommand::Shared command,
    TrustLinesManager *trustLines) :

    BaseTransaction(
        BaseTransaction::CoordinatorPaymentTransaction, currentNodeUUID),
    mTrustLines(trustLines),
    mCommand(command) {}

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



    if (mStep == 1) {
        // Context-exchange stage




        cout << "started!";
    }

    return resultOK();
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
