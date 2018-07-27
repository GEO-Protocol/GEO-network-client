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

        auto forbiddenMessageTypeStartPos = tokenSeparatorPos + 1;
        tokenSeparatorPos = commandBuffer.find(
            kTokensSeparator,
            forbiddenMessageTypeStartPos);
        auto strForbiddenMessageType = commandBuffer.substr(
            forbiddenMessageTypeStartPos,
            tokenSeparatorPos - forbiddenMessageTypeStartPos);
        mForbiddenReceiveMessageType = (Message::MessageType) std::stoi(strForbiddenMessageType);

        auto countForbiddenMessagesReceivedStartPos = tokenSeparatorPos + 1;
        auto strCountForbiddenMessagesReceived = commandBuffer.substr(
            countForbiddenMessagesReceivedStartPos,
            commandBuffer.size() - countForbiddenMessagesReceivedStartPos - 1);
        mCountForbiddenReceivedMessages = (uint32_t) std::stoi(strCountForbiddenMessagesReceived);
    } else {
        auto flagsStr = commandBuffer.substr(
            0,
            commandBuffer.size() - 1);
        mFlags = std::stoul(flagsStr);
        mForbiddenReceiveMessageType = Message::Debug;
        mCountForbiddenReceivedMessages = 0;
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

const Message::MessageType TrustLinesInfluenceCommand::forbiddenReceiveMessageType() const
{
    return mForbiddenReceiveMessageType;
}

const uint32_t TrustLinesInfluenceCommand::countForbiddenReceivedMessages() const
{
    return mCountForbiddenReceivedMessages;
}