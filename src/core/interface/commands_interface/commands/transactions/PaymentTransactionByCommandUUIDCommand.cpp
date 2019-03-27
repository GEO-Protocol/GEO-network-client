#include "PaymentTransactionByCommandUUIDCommand.h"

PaymentTransactionByCommandUUIDCommand::PaymentTransactionByCommandUUIDCommand(
    const CommandUUID &commandUUID,
    const string &commandBuffer):

    BaseUserCommand(
        commandUUID,
        identifier())
{
    uint32_t flag4 = 0, flag8 =0 , flag12 = 0;
    std::string paymentRecordCommandUUID;
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator || _attr(ctx) == kTokensSeparator) {
            throw ValueError("PaymentTransactionByCommandUUIDCommand: input is empty.");
        }
    };
    auto addUUID8Digits = [&](auto &ctx) {
        flag8++;
        if(flag8 >9 || (_attr(ctx) == kUUIDSeparator && flag8 < 9)) {
            throw ValueError("PaymentTransactionByCommandUUIDCommand: UUID Expect 8 digits.");
        }
        paymentRecordCommandUUID += _attr(ctx);
    };
    auto addUUID4Digits = [&](auto &ctx) {
        flag4++;
        if(flag4 >5 || (_attr(ctx) == kUUIDSeparator && flag4 < 5)) {
            throw ValueError("PaymentTransactionByCommandUUIDCommand: UUID Expect 4 digits.");
        } else if(_attr(ctx) == kUUIDSeparator) {
            flag4=0;
        }
        paymentRecordCommandUUID += _attr(ctx);
    };
    auto addUUID12Digits = [&](auto &ctx) {
        flag12++;
        if(flag12 >13 || (_attr(ctx) == kCommandsSeparator && flag12 < 13)) {
            throw ValueError("PaymentTransactionByCommandUUIDCommand: UUID Expect 12 digits.");
        } else if(_attr(ctx) == kCommandsSeparator ) { return; } else {
            paymentRecordCommandUUID += _attr(ctx);
        }
    };

    try {
        parse(
            commandBuffer.begin(),
            commandBuffer.end(),
            char_[check]);
        parse(
            commandBuffer.begin(),
            commandBuffer.end(), (
                UUIDLexeme<
                    decltype(addUUID8Digits),
                    decltype(addUUID4Digits),
                    decltype(addUUID12Digits)>(
                        addUUID8Digits,
                        addUUID4Digits,
                        addUUID12Digits) > eoi));
        mPaymentTransactionCommandUUID = boost::lexical_cast<uuids::uuid>(paymentRecordCommandUUID);
    } catch(...) {
        throw ValueError("PaymentTransactionByCommandUUIDCommand: cannot parse command.");
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
