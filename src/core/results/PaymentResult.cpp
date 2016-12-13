#include "PaymentResult.h"

PaymentResult::PaymentResult(Command *command,
                             const uint16_t &resultCode,
                             const string &timestampCompleted,
                             const uuids::uuid &transactionUUID,
                             const NodeUUID &contractorUUID,
                             const trust_amount &amount,
                             const string &purpose) :
        Result(command, resultCode, timestampCompleted) {
    mTransactionUUID = transactionUUID;
    mContractorUUID = contractorUUID;
    mAmount = amount;
    mPurpose = purpose;
}

const uuids::uuid &PaymentResult::commandUUID() const {
    return commandsUUID();
}

const string &PaymentResult::id() const {
    return identifier();
}

const uint16_t &PaymentResult::resultCode() const {
    return resCode();
}

const string &PaymentResult::timestampExcepted() const {
    return exceptedTimestamp();
}

const string &PaymentResult::timestampCompleted() const {
    return completedTimestamp();
}

const uuids::uuid & PaymentResult::transactionUUID() const {
    return mTransactionUUID;
}

const NodeUUID &PaymentResult::contractorUUID() const {
    return mContractorUUID;
}

const trust_amount &PaymentResult::amount() const {
    return mAmount;
}

const string & PaymentResult::purpose() const {
    return mPurpose;
}

string PaymentResult::serialize() {
    if (resultCode() == 200) {
        return boost::lexical_cast<string>(commandUUID()) + " " +
               boost::lexical_cast<string>(mTransactionUUID) + " " +
               boost::lexical_cast<string>(mAmount) + " " +
               boost::lexical_cast<string>(resultCode()) + " " +
               timestampExcepted() + " " +
               timestampCompleted() + " " +
               mPurpose +
               "\n";
    }
    return boost::lexical_cast<string>(commandUUID()) + " " +
           boost::lexical_cast<string>(resultCode()) +
           "\n";
}

