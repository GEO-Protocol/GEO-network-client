#include "OpenTrustLineCommand.h"


OpenTrustLineCommand::OpenTrustLineCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier()
    ) {

    deserialize(commandBuffer);
}

OpenTrustLineCommand::OpenTrustLineCommand(
    BytesShared buffer) {

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
        mAmount,
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

void OpenTrustLineCommand::deserializeFromBytes(
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
            mAmount,
            amountNotZeroBytes.begin(),
            amountNotZeroBytes.end()
        );

    } else {
        import_bits(
            mAmount,
            amountBytes.begin(),
            amountBytes.end()
        );
    }
}

const size_t OpenTrustLineCommand::kRequestedBufferSize() {

    const size_t trustAmountBytesSize = 32;
    static const size_t size = kOffsetToInheritBytes() + NodeUUID::kBytesSize + trustAmountBytesSize;
    return size;
}

/**
 * Throws ValueError if deserialization was unsuccessful.
 */
void OpenTrustLineCommand::deserialize(
    const string &command) {

    const auto amountTokenOffset = NodeUUID::kHexSize + 1;
    const auto minCommandLength = amountTokenOffset + 1;

    if (command.size() < minCommandLength) {
        throw ValueError("OpenTrustLineCommand::deserialize: "
                             "Can't parse command. Received command is to short.");
    }

    try {
        string hexUUID = command.substr(0, NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError("OpenTrustLineCommand::deserialize: "
                             "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }

    try {
        for (size_t commandSeparatorPosition = amountTokenOffset; commandSeparatorPosition < command.length(); ++commandSeparatorPosition) {
            if (command.at(commandSeparatorPosition) == kCommandsSeparator) {
                mAmount = TrustLineAmount(command.substr(amountTokenOffset, commandSeparatorPosition - amountTokenOffset));
            }
        }

    } catch (...) {
        throw ValueError("OpenTrustLineCommand::deserialize: "
                             "Can't parse command. Error occurred while parsing 'Amount' token.");
    }

    if (mAmount == TrustLineAmount(0)){
        throw ValueError("OpenTrustLineCommand::deserialize: "
                             "Can't parse command. Received 'Amount' can't be 0.");
    }
}

const CommandResult *OpenTrustLineCommand::resultOk() const{

    return new CommandResult(
        commandUUID(),
        201
    );
}

const CommandResult *OpenTrustLineCommand::trustLineAlreadyPresentResult() const{

    return new CommandResult(
        commandUUID(),
        409
    );
}

const CommandResult *OpenTrustLineCommand::resultConflict() const {

    return new CommandResult(
        commandUUID(),
        429
    );
}

const CommandResult *OpenTrustLineCommand::resultNoResponse() const {

    return new CommandResult(
        commandUUID(),
        444
    );
}

const CommandResult *OpenTrustLineCommand::resultTransactionConflict() const {

    return new CommandResult(
        commandUUID(),
        500
    );
}