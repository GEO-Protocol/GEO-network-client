#include "CreditUsageCommand.h"

CreditUsageCommand::CreditUsageCommand(
    const CommandUUID &uuid,
    const string &commandBuffer) :

    BaseUserCommand(
        uuid,
        identifier()) {

    parse(commandBuffer);
}

CreditUsageCommand::CreditUsageCommand(
    BytesShared buffer) :

    BaseUserCommand(identifier()) {

    deserializeFromBytes(buffer);
}

const string &CreditUsageCommand::identifier() {

    static const string identifier = "CREATE:contractors/transactions";
    return identifier;
}

const NodeUUID &CreditUsageCommand::contractorUUID() const {

    return mContractorUUID;
}

const TrustLineAmount &CreditUsageCommand::amount() const {

    return mAmount;
}

const string &CreditUsageCommand::reason() const {

    return mReason;
}

/**
 * @Throw std::bad_alloc;
 */
pair<BytesShared, size_t> CreditUsageCommand::serializeToBytes() {

    auto parentBytesAndCount = BaseUserCommand::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second +
        NodeUUID::kBytesSize +
        kTrustLineAmountBytesCount +
        mReason.size();
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        mContractorUUID.data,
        NodeUUID::kBytesSize);
    dataBytesOffset += NodeUUID::kBytesSize;
    //----------------------------------------------------
    vector<byte> buffer = trustLineAmountToBytes(mAmount);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        buffer.data(),
        buffer.size()
    );
    dataBytesOffset += kTrustLineAmountBytesCount;
    //----------------------------------------------------
    if (!mReason.empty()) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            mReason.c_str(),
            mReason.size()
        );
    }
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void CreditUsageCommand::deserializeFromBytes(
    BytesShared buffer) {

    BaseUserCommand::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = inheritED();
    //----------------------------------------------------
    memcpy(
        mContractorUUID.data,
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize
    );
    bytesBufferOffset += NodeUUID::kBytesSize;
    //----------------------------------------------------
    vector<byte> amountBytes(
        buffer.get() + bytesBufferOffset,
        buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);

    mAmount = bytesToTrustLineAmount(amountBytes);
    bytesBufferOffset += kTrustLineAmountBytesCount;
    //----------------------------------------------------
    mReason = string(reinterpret_cast<char *>(buffer.get() + bytesBufferOffset));
}

void CreditUsageCommand::parse(
    const string &command) {

    const auto amountTokenOffset = NodeUUID::kHexSize + 1;
    const auto minCommandLength = amountTokenOffset + 1;

    if (command.size() < minCommandLength) {
        throw ValueError("CreditUsageCommand::parse: "
                             "Can't parse command. Received command is to short.");
    }

    try {
        string hexUUID = command.substr(
            0,
            NodeUUID::kHexSize
        );
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError("CreditUsageCommand::parse: "
                             "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }

    size_t purposeTokenOffset = 0;
    size_t amountTokenLength = 0;
    try {
        for (size_t separatorPosition = amountTokenOffset; separatorPosition < command.length(); ++separatorPosition) {
            if (command.at(separatorPosition) == kCommandsSeparator || command.at(separatorPosition) == kTokensSeparator) {

                string amountToken = command.substr(
                    amountTokenOffset,
                    separatorPosition - amountTokenOffset
                );

                mAmount = TrustLineAmount(
                    amountToken
                );

                amountTokenLength = amountToken.length();
            }
        }

    } catch (...) {
        throw ValueError("CreditUsageCommand::parse: "
                             "Can't parse command. Error occurred while parsing 'Amount' token.");
    }

    if (mAmount == TrustLineAmount(0)){
        throw ValueError("CreditUsageCommand::parse: "
                             "Can't parse command. Received 'Amount' can't be 0.");
    }

    purposeTokenOffset = amountTokenOffset + amountTokenLength + 1;
    if (command.at(purposeTokenOffset) == kTokensSeparator) {
        for (size_t commandSeparatorPosition = purposeTokenOffset; commandSeparatorPosition < command.length(); ++ commandSeparatorPosition) {
            if (command.at(commandSeparatorPosition) == kCommandsSeparator) {
                mReason = command.substr(
                    purposeTokenOffset,
                    commandSeparatorPosition - purposeTokenOffset
                );
            }
        }
    }


}

const size_t CreditUsageCommand::kMinRequestedBufferSize() {

    static const size_t size = inheritED() + NodeUUID::kBytesSize + kTrustLineAmountBytesCount;
    return size;
}

CommandResult::SharedConst CreditUsageCommand::resultOk() const {

    return CommandResult::SharedConst(
        new CommandResult(
            UUID(),
            201
        )
    );
}
