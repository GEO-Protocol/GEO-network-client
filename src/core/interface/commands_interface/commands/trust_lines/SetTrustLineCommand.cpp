#include "SetTrustLineCommand.h"

SetTrustLineCommand::SetTrustLineCommand(
    const CommandUUID &commandUUID,
    const string &commandBuffer) :

    BaseUserCommand(
    commandUUID,
    identifier()
    ) {

    deserialize(commandBuffer);
}

SetTrustLineCommand::SetTrustLineCommand(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const string &SetTrustLineCommand::identifier() {

    static const string identifier = "SET:contractors/trust-lines";
    return identifier;
}

const NodeUUID &SetTrustLineCommand::contractorUUID() const {

    return mContractorUUID;
}

const TrustLineAmount &SetTrustLineCommand::newAmount() const {

    return mNewAmount;
}

void SetTrustLineCommand::deserialize(
    const string &command) {

    const auto amountTokenOffset = NodeUUID::kHexSize + 1;
    const auto minCommandLength = amountTokenOffset + 1;

    if (command.size() < minCommandLength) {
        throw ValueError("SetTrustLineCommand::deserialize: "
                             "Can't parse command. Received command is to short.");
    }

    try {
        string hexUUID = command.substr(0, NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError("SetTrustLineCommand::deserialize: "
                             "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }

    try {
        for (size_t commandSeparatorPosition = amountTokenOffset; commandSeparatorPosition < command.length(); ++commandSeparatorPosition) {
            if (command.at(commandSeparatorPosition) == kCommandsSeparator) {
                mNewAmount = TrustLineAmount(command.substr(amountTokenOffset, commandSeparatorPosition - amountTokenOffset));
            }
        }

    } catch (...) {
        throw ValueError("SetTrustLineCommand::deserialize: "
                             "Can't parse command. Error occurred while parsing 'New amount' token.");
    }

    if (mNewAmount == TrustLineAmount(0)){
        throw ValueError("SetTrustLineCommand::deserialize: "
                             "Can't parse command. Received 'New amount' can't be 0.");
    }
}

pair<BytesShared, size_t> SetTrustLineCommand::serializeToBytes() {

    auto parentBytesAndCount = serializeParentToBytes();

    size_t bytesCount = parentBytesAndCount.second + NodeUUID::kBytesSize + kTrustLineAmountSize;
    byte *data = (byte *) calloc(bytesCount, sizeof(byte));
    //----------------------------------------------------
    memcpy(
        data,
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    //----------------------------------------------------
    memcpy(
        data + parentBytesAndCount.second,
        mContractorUUID.data,
        NodeUUID::kBytesSize
    );
    //----------------------------------------------------
    vector<byte> buffer;
    buffer.reserve(kTrustLineAmountSize);
    export_bits(
        mNewAmount,
        back_inserter(buffer),
        8
    );
    size_t unusedBufferPlace = kTrustLineAmountSize - buffer.size();
    for (size_t i = 0; i < unusedBufferPlace; ++i) {
        buffer.push_back(0);
    }
    memcpy(
        data + parentBytesAndCount.second + NodeUUID::kBytesSize,
        buffer.data(),
        buffer.size()
    );
    //----------------------------------------------------
    return make_pair(
        BytesShared(data, free),
        bytesCount
    );
}

void SetTrustLineCommand::deserializeFromBytes(
    BytesShared buffer) {

    deserializeParentFromBytes(buffer);
    //----------------------------------------------------
    memcpy(
        mContractorUUID.data,
        buffer.get() + kOffsetToInheritBytes(),
        NodeUUID::kBytesSize
    );
    //----------------------------------------------------
    vector<byte> amountBytes(
        buffer.get() + kOffsetToInheritBytes() + NodeUUID::kBytesSize,
        buffer.get() + kOffsetToInheritBytes() + NodeUUID::kBytesSize + kTrustLineAmountSize);

    vector<byte> amountNotZeroBytes;
    amountNotZeroBytes.reserve(kTrustLineAmountSize);

    for (auto &item : amountBytes) {
        if (item != 0) {
            amountNotZeroBytes.push_back(item);
        }
    }

    if (amountNotZeroBytes.size() > 0) {
        import_bits(
            mNewAmount,
            amountNotZeroBytes.begin(),
            amountNotZeroBytes.end()
        );

    } else {
        import_bits(
            mNewAmount,
            amountBytes.begin(),
            amountBytes.end()
        );
    }
}

const CommandResult *SetTrustLineCommand::resultOk() const {

    return new CommandResult(
        commandUUID(),
        200
    );
}

const CommandResult *SetTrustLineCommand::trustLineAbsentResult() const {

    return new CommandResult(
        commandUUID(),
        404
    );
}

const CommandResult *SetTrustLineCommand::resultConflict() const {

    return new CommandResult(
        commandUUID(),
        429
    );
}

const CommandResult *SetTrustLineCommand::resultNoResponse() const {

    return new CommandResult(
        commandUUID(),
        444
    );
}

const CommandResult *SetTrustLineCommand::resultTransactionConflict() const {

    return new CommandResult(
        commandUUID(),
        500
    );
}