#include "GetTrustLinesCommand.h"

GetTrustLinesCommand::GetTrustLinesCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):


    BaseUserCommand(
        uuid,
        identifier())
{
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator) {
            throw ValueError("GetTrustLinesCommand: there is no input ");
        }
    };
    auto mfrom_add = [&](auto &ctx){
        mFrom = _attr(ctx);
    };
    auto mcount_add = [&](auto &ctx) {
        mCount = _attr(ctx);
    };
    auto mequivalent_add = [&](auto &ctx) {
        mEquivalent = _attr(ctx);
    };

    try {
        parse(
            commandBuffer.begin(),
            commandBuffer.end(),
            char_[check]);
        parse(
            commandBuffer.begin(),
            commandBuffer.end(), (
                *(int_[mfrom_add])
                > char_(kTokensSeparator)
                > *(int_[mcount_add])
                > char_(kTokensSeparator)
                > *(int_[mequivalent_add])
                > eol));
    } catch(...) {
        throw ValueError("GetTrustLinesCommand: can't parse command");
    }
}

const string &GetTrustLinesCommand::identifier()
{
    static const string kIdentifier = "GET:contractors/trust-lines";
    return kIdentifier;
}

const size_t GetTrustLinesCommand::from() const
{
    return mFrom;
}

const size_t GetTrustLinesCommand::count() const
{
    return mCount;
}

const SerializedEquivalent GetTrustLinesCommand::equivalent() const
{
    return mEquivalent;
}

CommandResult::SharedConst GetTrustLinesCommand::resultOk(
    string &neighbors) const
{
    return make_shared<const CommandResult>(
        identifier(),
        UUID(),
        200,
        neighbors);
}
