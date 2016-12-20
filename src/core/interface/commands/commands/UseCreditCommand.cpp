#include "UseCreditCommand.h"

UseCreditCommand::UseCreditCommand(
        const CommandUUID &uuid,
        const string &commandBuffer) :

        BaseUserCommand(uuid, identifier()) {

    deserialize(commandBuffer);
}

const string UseCreditCommand::identifier() {
    static const string kIdentifier = "CREATE:contractors/transactions";
}

const NodeUUID &UseCreditCommand::contractorUUID() const {
    return mContractorUUID;
}

const trust_amount &UseCreditCommand::amount() const {
    return mAmount;
}

const string &UseCreditCommand::purpose() const {
    return mPurpose;
}

const CommandResult *UseCreditCommand::resultOk(TransactionUUID &transactionUUID,
                                                uint16_t resultCode,
                                                Timestamp &transactionReceived,
                                                Timestamp &transactionProceed) const {
    Timestamp epoch(boost::gregorian::date(1970, 1, 1));
    Duration receivedTransaction = transactionReceived - epoch;
    Duration proceedTransaction = transactionProceed - epoch;

    microseconds_timestamp received = (microseconds_timestamp) receivedTransaction.total_microseconds();
    microseconds_timestamp proceed = (microseconds_timestamp) proceedTransaction.total_microseconds();

    string additionalInformation = transactionUUID.stringUUID() + "\r" +
                                   boost::lexical_cast<string>(resultCode) + "\r" +
                                   to_string(received) + "\r" +
                                   to_string(proceed);

    return new CommandResult(uuid(), 200, additionalInformation);
}

const CommandResult *UseCreditCommand::notEnoughtCreditAmountResult() const {
    return new CommandResult(uuid(), 412);
}

void UseCreditCommand::deserialize(
        const string &command) {

    const auto amountTokenOffset = CommandUUID::kLength + 1;
    const auto minCommandLength = amountTokenOffset + 1;

    if (command.size() < minCommandLength) {
        throw ValueError(
                "UseCreditCommand::deserialize: "
                        "Can't parse command. Received command is to short.");
    }


    try {
        string hexUUID = command.substr(0, NodeUUID::kUUIDLength);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
                "UseCreditCommand::deserialize: "
                        "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }


    try {
        mAmount = trust_amount(command.substr(
                amountTokenOffset, command.find_last_of(kTokensSeparator)));
    } catch (...) {
        throw ValueError(
                "UseCreditCommand::deserialize: "
                        "Can't parse command. Error occurred while parsing 'Amount' token.");
    }

    if (mAmount == trust_amount(0)) {
        throw ValueError(
                "UseCreditCommand::deserialize: "
                        "Can't parse command. Received 'Amount' can't be 0.");
    }


//    try {
//        mPurpose = command.substr(
//            amountTokenOffset, command.find_last_of(kTokensSeparator)));
//    } catch (...) {
//        throw ValueError(
//            "UseCreditCommand::deserialize: "
//                "Can't parse command. Error occurred while parsing 'Amount' token.");
//    }
//
//    if (mAmount == trust_amount(0)){
//        throw ValueError(
//            "UseCreditCommand::deserialize: "
//                "Can't parse command. Received 'Amount' can't be 0.");
//    }

}