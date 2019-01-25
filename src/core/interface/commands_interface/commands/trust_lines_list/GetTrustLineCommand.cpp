#include "GetTrustLineCommand.h"

GetTrustLineCommand::GetTrustLineCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    std::string address;
    uint32_t addressType, equivalentID;
    auto check = [&](auto &ctx) { if(_attr(ctx) == '\n'){throw ValueError("GetTrustLineCommand: there is no input ");}};
    auto parserType = [&](auto &ctx) { addressType = _attr(ctx); };
    auto address_add = [&](auto &ctx) { address += _attr(ctx); };
    auto address_number_add = [&](auto &ctx) { address += std::to_string(_attr(ctx)); };
    auto address_vector = [&](auto &ctx)
    {

        switch (addressType)
        {
            case BaseAddress::IPv4_IncludingPort:
            {
                mContractorAddress = make_shared<IPv4WithPortAddress>(address);
                break;

            }
        }

        address.erase();
    };

    auto equivalentID_add = [&](auto &ctx) { equivalentID = _attr(ctx); };

    try
    {
        parse(commandBuffer.begin(), commandBuffer.end(), char_[check]);
        parse(commandBuffer.begin(), commandBuffer.end(),
              (
                     *(int_[parserType] - char_('\t')) > char_('\t')
                     > repeat(3)[int_[address_number_add]> char_('.') [address_add]]
                     > int_[address_number_add] > char_(':') [address_add]
                     > int_[address_number_add] > char_('\t') [address_vector]
                      > +(int_[equivalentID_add]) > eol
              )
        );
    }
    catch(...)
    {
        throw ValueError("InitTrustLineCommand: can't parse command.");
    }
    mEquivalent = equivalentID;
}

const string &GetTrustLineCommand::identifier()
{
    static const string kIdentifier = "GET:contractors/trust-lines/one";
    return kIdentifier;
}

CommandResult::SharedConst GetTrustLineCommand::resultOk(
    string &neighbor) const
{
    return make_shared<const CommandResult>(
        identifier(),
        UUID(),
        200,
        neighbor);
}

BaseAddress::Shared GetTrustLineCommand::contractorAddress() const
{
    return mContractorAddress;
}

const SerializedEquivalent GetTrustLineCommand::equivalent() const
{
    return mEquivalent;
}
