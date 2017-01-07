#include "UseCreditCommand.h"

UseCreditCommand::UseCreditCommand(
        const CommandUUID &uuid,
        const string &commandBuffer) :

        BaseUserCommand(uuid, identifier()) {

    deserialize(commandBuffer);
}

const string &UseCreditCommand::identifier() {
    static const string kIdentifier = "CREATE:contractors/transactions";
    return kIdentifier;
}

const NodeUUID &UseCreditCommand::contractorUUID() const {
    return mContractorUUID;
}

const TrustLineAmount &UseCreditCommand::amount() const {
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

    uint64_t received = (uint64_t) receivedTransaction.total_microseconds();
    uint64_t proceed = (uint64_t) proceedTransaction.total_microseconds();

    string additionalInformation = transactionUUID.stringUUID() + "\t"
                                   + boost::lexical_cast<string>(resultCode) + "\t"
                                   + "123456789" + "\t"
                                   + "123456789";

    return new CommandResult(uuid(), 200, additionalInformation);
}

const CommandResult *UseCreditCommand::notEnoughtCreditAmountResult() const {
    return new CommandResult(uuid(), 412);
}

void UseCreditCommand::deserialize(
        const string &command) {

    const auto amountTokenOffset = CommandUUID::kUUIDLength + 1;
    const auto minCommandLength = amountTokenOffset + 1;

    if (command.size() < minCommandLength) {
        throw ValueError(
                "UseCreditCommand::deserialize: "
                        "Can't parse command. Received command is to short.");
    }


    try {
        string hexUUID = command.substr(0, NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
                "UseCreditCommand::deserialize: "
                        "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }


    size_t purposeStartPosition = 0;
    try {
        for (size_t nextTokenSeparatorPosition = amountTokenOffset; nextTokenSeparatorPosition < command.length(); ++nextTokenSeparatorPosition) {
            if (command.at(nextTokenSeparatorPosition) == kTokensSeparator || command.at(nextTokenSeparatorPosition) == kCommandsSeparator) {
                mAmount = TrustLineAmount(command.substr(amountTokenOffset, nextTokenSeparatorPosition - amountTokenOffset));
                purposeStartPosition = nextTokenSeparatorPosition + 1;
            }
        }
    } catch (...) {
        throw ValueError(
                "UseCreditCommand::deserialize: "
                        "Can't parse command. Error occurred while parsing 'Amount' token.");
    }

    if (mAmount == TrustLineAmount(0)) {
        throw ValueError(
                "UseCreditCommand::deserialize: "
                        "Can't parse command. Received 'Amount' can't be 0.");
    }


    try {
        for (size_t nextTokenSeparatorPosition = purposeStartPosition; nextTokenSeparatorPosition < command.length(); ++nextTokenSeparatorPosition) {
            if (command.at(nextTokenSeparatorPosition) == kTokensSeparator || command.at(nextTokenSeparatorPosition) == kCommandsSeparator) {
                mPurpose = command.substr(purposeStartPosition, nextTokenSeparatorPosition - purposeStartPosition);
            }
        }
    } catch (...) {
        throw ValueError(
            "UseCreditCommand::deserialize: "
                "Can't parse command. Error occurred while parsing 'Amount' token.");
    }

}