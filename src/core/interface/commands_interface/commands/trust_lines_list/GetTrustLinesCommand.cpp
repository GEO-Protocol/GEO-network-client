#include "GetTrustLinesCommand.h"

GetTrustLinesCommand::GetTrustLinesCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):


    BaseUserCommand(
        uuid,
        identifier())
{
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator || _attr(ctx) == kTokensSeparator) {
            throw ValueError("GetTrustLinesCommand: there is no input ");
        }
    };
    auto trustLinesFromParse = [&](auto &ctx){
        mFrom = _attr(ctx);
    };
    auto trustLinesCountParse = [&](auto &ctx) {
        mCount = _attr(ctx);
    };
    auto equivalentParse = [&](auto &ctx) {
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
                *(int_[trustLinesFromParse])
                > char_(kTokensSeparator)
                > *(int_[trustLinesCountParse])
                > char_(kTokensSeparator)
                > *(int_[equivalentParse])
                > eol > eoi));
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
