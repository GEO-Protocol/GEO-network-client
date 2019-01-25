#include "PaymentTransactionByCommandUUIDCommand.h"

PaymentTransactionByCommandUUIDCommand::PaymentTransactionByCommandUUIDCommand(
    const CommandUUID &commandUUID,
    const string &commandBuffer):

    BaseUserCommand(
        commandUUID,
        identifier())
{
    uint32_t flag_4 = 0, flag_8 =0 , flag_12 = 0;
    std::string paymentRecordCommandUUID;
    auto check = [&](auto &ctx) { if(_attr(ctx) == '\n'){throw ValueError("PaymentTransactionByCommandUUIDCommand: there is no input ");}};
    auto add_8 = [&](auto &ctx)
    {
        flag_8++;
        if(flag_8 >9){throw 1;}
        else if(_attr(ctx) == '-' && flag_8 < 9) {throw ValueError("Expect 8 digits");}
        paymentRecordCommandUUID += _attr(ctx);


    };

    auto add_4 = [&](auto &ctx)
    {
        flag_4++;
        if(flag_4 >5 || (_attr(ctx) == '-' && flag_4 < 5)){throw ValueError("Expect 4 digits");}
        else if(_attr(ctx) == '-'){flag_4=0;}
        paymentRecordCommandUUID += _attr(ctx);


    };

    auto add_12 = [&](auto &ctx)
    {
        flag_12++;
        if(flag_12 >13 || (_attr(ctx) == '\n' && flag_12 < 13)){throw ValueError("Expect 12 digits");}
        else if(_attr(ctx) == '\n' ) {}
        else { paymentRecordCommandUUID += _attr(ctx);}

    };

    try
    {
        parse(commandBuffer.begin(), commandBuffer.end(), char_[check]);
        parse(commandBuffer.begin(), commandBuffer.end(),
              (
                      *(char_[add_8] - char_('-')) > char_('-') [add_8]
                      >*(char_[add_4] - char_('-')) > char_('-') [add_4]
                      >*(char_[add_4] - char_('-')) > char_('-') [add_4]
                      >*(char_[add_4] - char_('-')) > char_('-') [add_4]
                      >*(char_[add_12] - char_('\n') ) > char_('\n') [add_12]

              )
             );

        mPaymentTransactionCommandUUID = boost::lexical_cast<uuids::uuid>(paymentRecordCommandUUID);
    }
    catch(...)
    {
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
