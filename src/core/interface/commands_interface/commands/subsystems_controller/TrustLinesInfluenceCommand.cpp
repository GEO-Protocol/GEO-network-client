#include "TrustLinesInfluenceCommand.h"

TrustLinesInfluenceCommand::TrustLinesInfluenceCommand(
    const CommandUUID &uuid,
    const string &commandBuffer) :

    BaseUserCommand(
        uuid,
        identifier())
{
    if (commandBuffer.empty()) {
        throw ValueError(
            "TrustLinesInfluenceCommand: can't parse command."
                    "Received command buffer is too short.");
    }

    auto tokenSeparatorPos = commandBuffer.find(
        kTokensSeparator);

    if (tokenSeparatorPos != std::string::npos) {
        auto flagsStr = commandBuffer.substr(
            0,
            tokenSeparatorPos);
        mFlags = std::stoul(flagsStr);

        auto firstParameterStartPos = tokenSeparatorPos + 1;
        tokenSeparatorPos = commandBuffer.find(
            kTokensSeparator,
            firstParameterStartPos);
        auto strFirstParameter = commandBuffer.substr(
            firstParameterStartPos,
            tokenSeparatorPos - firstParameterStartPos);
        mFirstParameter = (uint32_t)std::stoi(strFirstParameter);

        auto secondParameterStartPos = tokenSeparatorPos + 1;
        tokenSeparatorPos = commandBuffer.find(
            kTokensSeparator,
            secondParameterStartPos);
        auto strSecondParameter = commandBuffer.substr(
            secondParameterStartPos,
            tokenSeparatorPos - secondParameterStartPos);
        mSecondParameter = (uint32_t)std::stoi(strSecondParameter);

        auto thirdParameterStartPos = tokenSeparatorPos + 1;
        auto strThirdParameter = commandBuffer.substr(
            thirdParameterStartPos,
            commandBuffer.size() - thirdParameterStartPos - 1);
        mThirdParameter = (uint32_t) std::stoi(strThirdParameter);
    } else {
        auto flagsStr = commandBuffer.substr(
            0,
            commandBuffer.size() - 1);
        mFlags = std::stoul(flagsStr);
        mFirstParameter = 0;
        mSecondParameter = 0;
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