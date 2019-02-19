#include "TrustLinesInfluenceCommand.h"

TrustLinesInfluenceCommand::TrustLinesInfluenceCommand(
    const CommandUUID &uuid,
    const string &commandBuffer) :

    BaseUserCommand(
        uuid,
        identifier())
{
    auto check = [&](auto &ctx) { if(_attr(ctx) == '\n'){throw ValueError("TrustLinesInfluenceCommand: there is no input ");}};
    auto flags_add = [&](auto &ctx)
    {
        mFlags = _attr(ctx);
        mFirstParameter = 0;
        mSecondParameter = 0;
         mThirdParameter = 0;
    };
    auto firstparam_add = [&](auto &ctx) {mFirstParameter = _attr(ctx);};
    auto secondparam_add = [&](auto &ctx) {mSecondParameter = _attr(ctx);};
    auto thirdparam_add = [&](auto &ctx) {mThirdParameter = _attr(ctx);};
    try
    {
        parse(commandBuffer.begin(), commandBuffer.end(),char_[check]);
        parse(commandBuffer.begin(), commandBuffer.end(),
              (
                      *(int_[flags_add])
                      >-(char_('\t')
                      >*(int_[firstparam_add])
                      >char_('\t')
                      >*(int_[secondparam_add])
                      >char_('\t')
                      >*(int_[thirdparam_add]))
                      >eol
                      ));

    }
    catch(...)
    {
        throw ValueError("TrustLinesInfluenceCommand: can't parse command");
    }
}

const string& TrustLinesInfluenceCommand::identifier()
{
    static const string identifier = "SET:subsystems_controller/trust_lines_influence/flags";
    return identifier;
}

size_t TrustLinesInfluenceCommand::flags() const
{
    return mFlags;
}

const uint32_t TrustLinesInfluenceCommand::firstParameter() const
{
    return mFirstParameter;
}

const uint32_t TrustLinesInfluenceCommand::secondParameter() const
{
    return mSecondParameter;
}

const uint32_t TrustLinesInfluenceCommand::thirdParameter() const
{
    return mThirdParameter;
}