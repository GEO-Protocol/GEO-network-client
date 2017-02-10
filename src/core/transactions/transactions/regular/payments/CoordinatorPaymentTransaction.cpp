#include "CoordinatorPaymentTransaction.h"



CoordinatorPaymentTransaction::CoordinatorPaymentTransaction(
    NodeUUID &currentNodeUUID,
    CreditUsageCommand::Shared command,
    TrustLinesManager *trustLines) :

    BaseTransaction(BaseTransaction::CoordinatorPaymentTransaction, currentNodeUUID),
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

    BaseTransaction(BaseTransaction::CoordinatorPaymentTransaction),
    mTrustLines(trustLines) {

    throw ValueError("Not implemented");
}

pair<BytesShared, size_t> CoordinatorPaymentTransaction::serializeToBytes() const  {

    throw ValueError("Not implemented");
}

void CoordinatorPaymentTransaction::deserializeFromBytes(
    BytesShared buffer) {

    throw ValueError("Not implemented");
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::run() {



    if (mStep == 1) {

    }

    return resultOK();
}

TransactionResult::SharedConst CoordinatorPaymentTransaction::resultOK() const {

    throw ValueError("Not implemented");
}
