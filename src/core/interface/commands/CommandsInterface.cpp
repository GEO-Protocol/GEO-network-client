#include "CommandsInterface.h"


/*!
 * Handles received data from FIFO.
 * Writes it to the internal buffer for further processing.
 * Calls tryDeserializeCommand() internally for buffer processing.
 * Return value is similar to the tryDeserializeCommand() return value.
 */
pair<bool, shared_ptr<Command>> CommandsParser::processReceivedCommandPart(const char *commandPart, const size_t receivedBytesCount) {

    // Memory reservation for whole command part
    // (to prevent huge amount of memory reallocations).
    mBuffer.reserve(mBuffer.size() + receivedBytesCount);

    // Concatenating with the previously received parts of the command.
    for (size_t i = 0; i < receivedBytesCount; ++i) {
        mBuffer.push_back(commandPart[i]);
    }

    return tryDeserializeCommand();
}

/*!
 * Tries to deserialise received command, using buffered data.
 * In case when buffer is too short to contains even one command -
 * will return immideately.
 *
 * First value of returned pair indicates if command was parsed succesfully,
 * and, if so, - the second one will contains shared pointer
 * to the command instance itself.
 * Otherwise - the second value of the pair will contain nullptr.
 */
pair<bool, shared_ptr<Command>> CommandsParser::tryDeserializeCommand() {
    if (mBuffer.size() < kMinCommandSize) {
        // If command contains trailing symbol,
        // but is shorter than min command size -
        // then this command is invalid and should be rejected.
        if (mBuffer.find(kCommandsSeparator) != string::npos) {
            cutNextCommandFromTheBuffer();
        }

        return commandIsInvalidOrIncomplete();
    }

    // Check if received string contains command separator.
    // If no - then command may be received partially.
    size_t nextCommandSeparatorIndex = mBuffer.find(kCommandsSeparator);
    if (nextCommandSeparatorIndex == string::npos) {
        return commandIsInvalidOrIncomplete();
    }

    // UUID parsing
    uuids::uuid uuid;
    string hexUUID = mBuffer.substr(0, kUUIDHexRepresentationSize);
    try {
        uuid = boost::lexical_cast<uuids::uuid>(hexUUID);
    } catch (...) {
        // UUID can't be parsed correctly.
        // Command seems to be broken and should be rejected.
        cutNextCommandFromTheBuffer();
        return commandIsInvalidOrIncomplete();
    }

    // Command identifier parsing
    const size_t averageCommandIdentifierLength = 15; // for optimisations purpose only.
    const size_t identifierOffset = kUUIDHexRepresentationSize + 1; // + separator

    string identifier;
    identifier.reserve(averageCommandIdentifierLength);
    size_t contractorOffset = identifierOffset;
    for (size_t i = identifierOffset; i < mBuffer.size(); ++i) {
        char symbol = mBuffer.at(i);
        if (symbol == kTokensSeparator || symbol == kCommandsSeparator) {
            break;
        }

        contractorOffset += 1;
        identifier.push_back(symbol);
    }

    // Validate received identifier
    if (identifier.size() == 0) {
        // Command identifier can't be parsed.
        // Command seems to be broken and should be rejected.
        cutNextCommandFromTheBuffer();
        return commandIsInvalidOrIncomplete();
    }

    contractorOffset += 2; // + separator after identifier

    return tryParseCommand(uuid, identifier, mBuffer.substr(contractorOffset, mBuffer.size()));
}

/*!
 * @commandUUID - uuid of the received command (parsed on previous step).
 * @commandIdentifier - identifier of the received command (parsed on the previous step).
 *
 * Accepts "commandsIdentifier" and tries to deserialise the rest of the command.
 * In case when "commandsIdentifier" is unexpected - raises ValueError exception.
 *
 * First value of returned pair indicates if command was parsed succesfully,
 * and, if so, - the second one will contains shared pointer
 * to the command instance itself.
 * Otherwise - the second value of the pair will contain nullptr.
 */
pair<bool, shared_ptr<Command>> CommandsParser::tryParseCommand(const uuids::uuid &commandUUID,
                                                                const string &commandIdentifier,
                                                                const string &buffer) {

    Command *command = nullptr;
    long timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if(strcmp(kTrustLinesOpenIdentifier, commandIdentifier.c_str()) == 0){
        command = new OpenTrustLineCommand(commandUUID, commandIdentifier,
                                           std::to_string(timestamp), buffer);
    }
    if(strcmp(kTrustLinesCloseIdentifier, commandIdentifier.c_str()) == 0){
        command = new CloseTrustLineCommand(commandUUID, commandIdentifier,
                                            std::to_string(timestamp), buffer);
    }
    if(strcmp(kTrustLinesUpdateIdentifier, commandIdentifier.c_str()) == 0){
        command = new UpdateOutgoingTrustAmountCommand(commandUUID, commandIdentifier,
                                                       std::to_string(timestamp), buffer);
    }
    if(strcmp(kTransactionsUseCreditIdentifier, commandIdentifier.c_str()) == 0){
        command = new UseCreditCommand(commandUUID, commandIdentifier,
                                       std::to_string(timestamp), buffer);
    }

    cutNextCommandFromTheBuffer();

    if(command == nullptr){
        return commandIsInvalidOrIncomplete();
    }

    return pair <bool, shared_ptr<Command>> (true, shared_ptr<Command>(command));
}

/*!
 * Removes last command (valid, or invalid) from the buffer.
 * Stops on commands separator symbol.
 * If no commands separator symbol is present - clears buffer at all.
 */
void CommandsParser::cutNextCommandFromTheBuffer() {
    size_t nextCommandSeparatorIndex = mBuffer.find(kCommandsSeparator);
    if (mBuffer.size() > (nextCommandSeparatorIndex + 1)) {
        // Buffer contains other commands (or their parts), and them should be keept;
        mBuffer = mBuffer.substr(nextCommandSeparatorIndex + 1, mBuffer.size() - 1);

    } else {
        // Buffer doesn't contains any other commands (even parts).
        mBuffer.clear();
    }

    // Resize the capacity to free unused memory.
    mBuffer.shrink_to_fit();
}

pair<bool, shared_ptr<Command>> CommandsParser::commandIsInvalidOrIncomplete() {
    return pair <bool, shared_ptr<Command>> (false, nullptr);
}


CommandsInterface::CommandsInterface(as::io_service &ioService) :
        mIOService(ioService) {

    if (!FIFOExists()) {
        createFIFO();
    }

    mFIFODescriptor = open(FIFOPath().c_str(), O_RDONLY);
    if (mFIFODescriptor == -1) {
        throw IOError("Can't open FIFO file");
    }

    try {
        mFIFOStreamDescriptor = new as::posix::stream_descriptor(mIOService, mFIFODescriptor);
    } catch (const std::exception &e) {
        std::cout << e.what();
        std::cout.flush();
        throw IOError("Can't assign FIFO descriptor to async reader");
    }

    mCommandsParser = new CommandsParser();
}

CommandsInterface::~CommandsInterface() {
    close(mFIFODescriptor);
    delete mFIFOStreamDescriptor;
    delete mCommandsParser;
}

void CommandsInterface::beginAcceptCommands() {
    asyncReceiveNextCommand();
}

void CommandsInterface::createFIFO() {
    if (!FIFOExists()) {
        fs::create_directories(dir());
        mkfifo(FIFOPath().c_str(), 0420); // Read; Write; No access; // todo: tests mask
    }
}

void CommandsInterface::asyncReceiveNextCommand() {
    mFIFOStreamDescriptor->async_read_some(
            as::buffer(mCommandBuffer), boost::bind(
                    &CommandsInterface::handleReceivedInfo, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

void CommandsInterface::handleReceivedInfo(
        const boost::system::error_code &error, const size_t bytesTransferred) {

    if (!error || error == as::error::message_size) {
        mCommandsParser->processReceivedCommandPart(
                mCommandBuffer.data(), bytesTransferred);
    }

    // In all cases - commands receiving should be continued
    asyncReceiveNextCommand();
}
