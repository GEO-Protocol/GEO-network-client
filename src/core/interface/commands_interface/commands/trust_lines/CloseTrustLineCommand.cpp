#include "CloseTrustLineCommand.h"


CloseTrustLineCommand::CloseTrustLineCommand(
    const CommandUUID &commandUUID,
    const string &commandBuffer):

    BaseUserCommand(
        commandUUID,
        identifier())
{
    try {
        auto contractorHexUUID = commandBuffer.substr(0, CommandUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(contractorHexUUID);

    } catch (...) {
        throw ValueError(
            "CloseTrustLineCommand::CloseTrustLineCommand: "
            "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }
}

const string &CloseTrustLineCommand::identifier()
    noexcept
{
    static const string identifier = "CLOSE:contractors/trust-lines";
    return identifier;
}

const NodeUUID &CloseTrustLineCommand::contractorUUID() const
    noexcept
{
    return mContractorUUID;
}

size_t CloseTrustLineCommand::kRequestedBufferSize()
{
    static const size_t size = kOffsetToInheritedBytes() + NodeUUID::kBytesSize;
    return size;
}
