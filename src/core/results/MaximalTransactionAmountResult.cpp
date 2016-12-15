#include "MaximalTransactionAmountResult.h"

MaximalTransactionAmountResult::MaximalTransactionAmountResult(Command *command,
                                                               const uint16_t &resultCode,
                                                               const string &timestampCompleted,
                                                               const NodeUUID &contractorUUID,
                                                               const trust_amount &amount) :
        Result(command, resultCode, timestampCompleted), mContractorUUID(contractorUUID), mAmount(amount) {}

const uuids::uuid &MaximalTransactionAmountResult::commandUUID() const {
    return commandsUUID();
}

const string &MaximalTransactionAmountResult::id() const {
    return identifier();
}

const uint16_t &MaximalTransactionAmountResult::resultCode() const {
    return resCode();
}

const string &MaximalTransactionAmountResult::timestampExcepted() const {
    return exceptedTimestamp();
}

const string &MaximalTransactionAmountResult::timestampCompleted() const {
    return completedTimestamp();
}

const NodeUUID &MaximalTransactionAmountResult::contractorUUID() const {
    return mContractorUUID;
}

const trust_amount &MaximalTransactionAmountResult::amount() const {
    return mAmount;
}

string MaximalTransactionAmountResult::serialize() {
    if (resultCode() == 200){
        return boost::lexical_cast<string>(commandUUID()) + " " +
               boost::lexical_cast<string>(resultCode()) + " " +
               boost:: lexical_cast<string>(mAmount) +
               "\n";
    }
    return boost::lexical_cast<string>(commandUUID()) + " " +
           boost::lexical_cast<string>(resultCode()) +
           "\n";
}