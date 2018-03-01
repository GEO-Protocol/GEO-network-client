#include "PaymentTransactionByCommandUUIDCommand.h"

PaymentTransactionByCommandUUIDCommand::PaymentTransactionByCommandUUIDCommand(
    const CommandUUID &commandUUID,
    const string &commandBuffer):

    BaseUserCommand(
        commandUUID,
        identifier())
{
    static const auto minCommandLength = NodeUUID::kHexSize + 1;

    if (commandBuffer.size() < minCommandLength) {
        throw ValueError(
            "PaymentTransactionByCommandUUIDCommand: can't parse command. "
                    "Received command is to short.");
    }

    try {
        string hexUUID = commandBuffer.substr(0, NodeUUID::kHexSize);
        mPaymentTransactionCommandUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
            "PaymentTransactionByCommandUUIDCommand: can't parse command. "
                    "Error occurred while parsing 'Payment transaction command UUID' token.");
    }
}

const string &PaymentTransactionByCommandUUIDCommand::identifier()
{
    static const string identifier = "GET:transaction/command-uuid";
    return identifier;
}

const CommandUUID& PaymentTransactionByCommandUUIDCommand::paymentTransactionCommandUUID() const
{
    return mPaymentTransactionCommandUUID;
}

CommandResult::SharedConst PaymentTransactionByCommandUUIDCommand::resultOk(
    string &transactionUUIDStr) const
{
    return CommandResult::SharedConst(
        new CommandResult(
            identifier(),
            UUID(),
            200,
            transactionUUIDStr));
}
