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
        if(_attr(ctx) == kCommandsSeparator) {
            throw ValueError("PaymentTransactionByCommandUUIDCommand: there is no input ");
        }
    };
    auto parseUUID8Chars = [&](auto &ctx) {
        flag8++;
        if(flag8 >9) {
            throw 1;
        } else if(_attr(ctx) == '-' && flag8 < 9) {
            throw ValueError("Expect 8 digits");
        }
        paymentRecordCommandUUID += _attr(ctx);
    };
    auto parseUUID4Chars = [&](auto &ctx) {
        flag4++;
        if(flag4 >5 || (_attr(ctx) == '-' && flag4 < 5)) {
            throw ValueError("Expect 4 digits");
        } else if(_attr(ctx) == '-') {
            flag4=0;
        }
        paymentRecordCommandUUID += _attr(ctx);
    };
    auto parseUUID12Chars = [&](auto &ctx) {
        flag12++;
        if(flag12 >13 || (_attr(ctx) == '\n' && flag12 < 13)) {
            throw ValueError("Expect 12 digits");
        } else if(_attr(ctx) == '\n' ) {
        } else {
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
                *(char_[parseUUID8Chars] - char_('-')) > char_('-') [parseUUID8Chars]
                >*(char_[parseUUID4Chars] - char_('-')) > char_('-') [parseUUID4Chars]
                >*(char_[parseUUID4Chars] - char_('-')) > char_('-') [parseUUID4Chars]
                >*(char_[parseUUID4Chars] - char_('-')) > char_('-') [parseUUID4Chars]
                >*(char_[parseUUID12Chars] - char_(kCommandsSeparator) ) > char_(kCommandsSeparator) [parseUUID12Chars]));
        mPaymentTransactionCommandUUID = boost::lexical_cast<uuids::uuid>(paymentRecordCommandUUID);
    } catch(...) {
        throw ValueError("PaymentTransactionByCommandUUIDCommand: can't parse command");
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
