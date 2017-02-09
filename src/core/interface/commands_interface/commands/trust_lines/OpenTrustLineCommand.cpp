#include "OpenTrustLineCommand.h"

OpenTrustLineCommand::OpenTrustLineCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier()
    ) {

    parse(commandBuffer);
}

OpenTrustLineCommand::OpenTrustLineCommand(
    BytesShared buffer) :

    BaseUserCommand(identifier()) {

    deserializeFromBytes(buffer);
}

const string &OpenTrustLineCommand::identifier() {

    static const string identifier = "CREATE:contractors/trust-lines";
    return identifier;
}

const NodeUUID &OpenTrustLineCommand::contractorUUID() const {

    return mContractorUUID;
}

const TrustLineAmount &OpenTrustLineCommand::amount() const {

    return mAmount;
}

pair<BytesShared, size_t> OpenTrustLineCommand::serializeToBytes() {

    auto parentBytesAndCount = BaseUserCommand::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second +
        NodeUUID::kBytesSize +
        kTrustLineAmountBytesCount;
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
        NodeUUID::kBytesSize
    );
    dataBytesOffset += NodeUUID::kBytesSize;
    //----------------------------------------------------
    vector<byte> buffer = trustLineAmountToBytes(mAmount);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        buffer.data(),
        buffer.size()
    );
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void OpenTrustLineCommand::deserializeFromBytes(
    BytesShared buffer) {

    BaseUserCommand::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = BaseUserCommand::kOffsetToInheritedBytes();
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
}

/**
 * Throws ValueError if deserialization was unsuccessful.
 */
void OpenTrustLineCommand::parse(
    const string &command) {

    const auto amountTokenOffset = NodeUUID::kHexSize + 1;
    const auto minCommandLength = amountTokenOffset + 1;

    if (command.size() < minCommandLength) {
        throw ValueError("OpenTrustLineCommand::parse: "
                             "Can't parse command. Received command is to short.");
    }

    try {
        string hexUUID = command.substr(
            0,
            NodeUUID::kHexSize
        );
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError("OpenTrustLineCommand::parse: "
                             "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }

    try {
        for (size_t commandSeparatorPosition = amountTokenOffset; commandSeparatorPosition < command.length(); ++commandSeparatorPosition) {
            if (command.at(commandSeparatorPosition) == kCommandsSeparator) {
                mAmount = TrustLineAmount(
                    command.substr(
                        amountTokenOffset,
                        commandSeparatorPosition - amountTokenOffset
                    )
                );
            }
        }

    } catch (...) {
        throw ValueError("OpenTrustLineCommand::parse: "
                             "Can't parse command. Error occurred while parsing 'Amount' token.");
    }

    if (mAmount == TrustLineAmount(0)){
        throw ValueError("OpenTrustLineCommand::parse: "
                             "Can't parse command. Received 'Amount' can't be 0.");
    }
}

const size_t OpenTrustLineCommand::kRequestedBufferSize() {

    static const size_t size = kOffsetToInheritedBytes() + NodeUUID::kBytesSize + kTrustLineAmountBytesCount;
    return size;
}

CommandResult::SharedConst OpenTrustLineCommand::resultOk() const{

    return CommandResult::SharedConst(
        new CommandResult(
            UUID(),
            201
        )
    );
}

CommandResult::SharedConst OpenTrustLineCommand::trustLineAlreadyPresentResult() const{

    return CommandResult::SharedConst(
        new CommandResult(
            UUID(),
            409
        )
    );
}

CommandResult::SharedConst OpenTrustLineCommand::resultConflict() const {

    return CommandResult::SharedConst(
        new CommandResult(
            UUID(),
            429
        )
    );
}

CommandResult::SharedConst OpenTrustLineCommand::resultNoResponse() const {

    return CommandResult::SharedConst(
        new CommandResult(
            UUID(),
            444
        )
    );
}

CommandResult::SharedConst OpenTrustLineCommand::resultTransactionConflict() const {

    return CommandResult::SharedConst(
        new CommandResult(
            UUID(),
            500
        )
    );
}