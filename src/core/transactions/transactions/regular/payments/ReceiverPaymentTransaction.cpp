#include "ReceiverPaymentTransaction.h"

ReceiverPaymentTransaction::ReceiverPaymentTransaction(
    ReceiverInitPaymentMessage::Shared message,
    TrustLinesManager *trustLines,
    Logger *log) :

    BaseTransaction(
        BaseTransaction::ReceiverPaymentTransaction),
    mMessage(message),
    mTrustLines(trustLines),
    mLog(log) {
}

ReceiverPaymentTransaction::ReceiverPaymentTransaction(
    BytesShared buffer,
    TrustLinesManager *trustLines,
    Logger *log) :

    BaseTransaction(
        BaseTransaction::ReceiverPaymentTransaction),
    mTrustLines(trustLines),
    mLog(log){

    deserializeFromBytes(buffer);
}

TransactionResult::Shared ReceiverPaymentTransaction::run() {

    switch (mStep) {
        case 1: {

#ifdef TRANSACTIONS_LOG
            {
                auto info = mLog->info("ReceiverPaymentTransaction");
                info << "(UUID: " << UUID() << ") "
                     << "Initialising payment operation from node " << mMessage->senderUUID();
            }
#endif
        }
    }

    return make_shared<TransactionResult>(TransactionState::exit());
}

pair<BytesShared, size_t> ReceiverPaymentTransaction::serializeToBytes() {
    throw ValueError("Not implemented");
}

void ReceiverPaymentTransaction::deserializeFromBytes(BytesShared buffer) {
    throw ValueError("Not implemented");
}


