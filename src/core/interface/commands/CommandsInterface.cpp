#include "CommandsInterface.h"


/*!
 * Handles received data from FIFO.
 * Writes it to the internal buffer for further processing.
 * Calls tryDeserializeCommand() internally for buffer processing.
 * Return value is similar to the tryDeserializeCommand() return value.
 */
pair<bool, shared_ptr<Command>> CommandsParser::processReceivedCommandPart(
    const char *commandPart,
    const size_t receivedBytesCount) {

    if (receivedBytesCount == 0) {
        return commandIsInvalidOrIncomplete();
    }

    mBuffer.reserve(mBuffer.size() + receivedBytesCount);
    for (size_t i = 0; i < receivedBytesCount; ++i) {
        mBuffer.push_back(commandPart[i]);
    }

    return tryDeserializeCommand();
}

/*!
 * Tries to deserialize received command, using buffered data.
 * In case when buffer is too short to contains even one command -
 * will return immediately.
 *
 * First value of returned pair indicates if command was parsed succesfully,
 * and, if so, - the second one will contains shared pointer to the command instance itself.
 * Otherwise - the second value of the pair will contains nullptr.
 */
pair<bool, shared_ptr<Command>> CommandsParser::tryDeserializeCommand() {
    if (mBuffer.size() < kMinCommandSize) {
        if (mBuffer.find(kCommandsSeparator) != string::npos) {
            cutNextCommandFromTheBuffer();
        }
        return commandIsInvalidOrIncomplete();
    }

    size_t nextCommandSeparatorIndex = mBuffer.find(kCommandsSeparator);
    if (nextCommandSeparatorIndex == string::npos) {
        return commandIsInvalidOrIncomplete();
    }

    uuids::uuid commandUUID;
    try {
        string hexUUID = mBuffer.substr(0, kUUIDHexRepresentationSize);
        commandUUID = boost::lexical_cast<uuids::uuid>(hexUUID);
    } catch (...) {
        cutNextCommandFromTheBuffer();
        return commandIsInvalidOrIncomplete();
    }

    const size_t averageCommandIdentifierLength = 15;
    const size_t identifierOffset = kUUIDHexRepresentationSize + 1;

    string commandIdentifier;
    commandIdentifier.reserve(averageCommandIdentifierLength);
    size_t nextTokenOffset = identifierOffset;
    for (size_t i = identifierOffset; i < mBuffer.size(); ++i) {
        char symbol = mBuffer.at(i);
        if (symbol == kTokensSeparator || symbol == kCommandsSeparator) {
            break;
        }
        nextTokenOffset += 1;
        commandIdentifier.push_back(symbol);
    }
    nextTokenOffset += 1;

    if (commandIdentifier.size() == 0) {
        cutNextCommandFromTheBuffer();
        return commandIsInvalidOrIncomplete();
    }

    return tryParseCommand(
        commandUUID, commandIdentifier,
        mBuffer.substr(
            nextTokenOffset, mBuffer.size()));
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
pair<bool, shared_ptr<Command>> CommandsParser::tryParseCommand(
    const uuids::uuid &commandUUID,
    const string &commandIdentifier,
    const string &buffer) {

    long timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    Command *command = nullptr;
    string sinceEpoch = to_string(timestamp);
    try{
        if(strcmp(kTrustLinesOpenIdentifier, commandIdentifier.c_str()) == 0) {
            command = new OpenTrustLineCommand(commandUUID, commandIdentifier,
                                               sinceEpoch, buffer);
        } else if(strcmp(kTrustLinesCloseIdentifier, commandIdentifier.c_str()) == 0) {
            command = new CloseTrustLineCommand(commandUUID, commandIdentifier,
                                                sinceEpoch, buffer);
        } else if(strcmp(kTrustLinesUpdateIdentifier, commandIdentifier.c_str()) == 0) {
            command = new UpdateOutgoingTrustAmountCommand(commandUUID, commandIdentifier,
                                                           sinceEpoch, buffer);
        } else if(strcmp(kTransactionsUseCreditIdentifier, commandIdentifier.c_str()) == 0) {
            command = new UseCreditCommand(commandUUID, commandIdentifier,
                                           sinceEpoch, buffer);
        } else if (strcmp(kTransactionsMaximalAmountIdentifier, commandIdentifier.c_str()) == 0) {
            command = new MaximalTransactionAmountCommand(commandUUID, commandIdentifier,
                                                          sinceEpoch, buffer);
        } else if (strcmp(kBalanceGetTotalBalanceIdentifier, commandIdentifier.c_str()) == 0) {
            command = new TotalBalanceCommand(commandUUID, commandIdentifier,
                                              sinceEpoch);
        } else if (strcmp(kContractorsGetAllContractorsIdentifier, commandIdentifier.c_str()) == 0) {
            command = new ContractorsListCommand(commandUUID, commandIdentifier,
                                                 sinceEpoch);
        }
    } catch (std::exception &e){
        cutNextCommandFromTheBuffer();
        return commandIsInvalidOrIncomplete();
    }

    cutNextCommandFromTheBuffer();

    if(command == nullptr){
        return commandIsInvalidOrIncomplete();
    }

    return pair<bool, shared_ptr<Command>> (true, shared_ptr<Command>(command));
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


/*
 * Throws IOError.
 * Throws MemoryError.
 */
CommandsInterface::CommandsInterface(
    as::io_service &ioService,
    TransactionsManager *transactionsHandler,
    Logger *logger) :

    mIOService(ioService),
    mTransactionsManager(transactionsHandler),
    mLog(logger){

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(transactionsHandler != nullptr);
    assert(logger != nullptr);
#endif

    if (!isFIFOExists()) {
        createFIFO(kPermissionsMask);
    }

    // Try to open FIFO file in non-blocking manner.
    // In case if this file will be opened in standard blocking manner -
    // it will freeze whole the process,
    // until writer would connect to the FIFO from the other side.
    //
    // For the server realisation this makes the process unusable,
    // because it can't be demonized, until the commands writer will
    // open the commands file for writing.
    mFIFODescriptor = open(FIFOFilePath().c_str(), O_RDONLY | O_NONBLOCK);
    if (mFIFODescriptor == -1) {
        throw IOError(
            "CommandsInterface::CommandsInterface: "
                "Can't open FIFO file.");
    }

    try {
        mFIFOStreamDescriptor = new as::posix::stream_descriptor(mIOService, mFIFODescriptor);
        mFIFOStreamDescriptor->non_blocking(true);
    } catch (exception &) {
        throw MemoryError(
            "CommandsInterface::CommandsInterface: "
                "Can't allocate enough space for mFIFOStreamDescriptor. "
                "Can't open FIFO file.");
    }

    // Init ASIO buffer.
    // In case when buffer is zero-size - ASIO will fall into infinite loop
    // with zero-length messages.
    mCommandBuffer.resize(kCommandBufferSize);

    try {
        mReadTimeoutTimer = new as::deadline_timer(
            mIOService, boost::posix_time::seconds(2));

    } catch (exception &) {
        throw MemoryError(
            "CommandsInterface::CommandsInterface: "
                "Can't allocate enough space for mReadTimeoutTimer.");
    }

    try {
        mCommandsParser = new CommandsParser();
    } catch (exception &) {
        throw MemoryError(
            "CommandsInterface::CommandsInterface: "
                "Can't allocate enough space for mCommandsParser.");
    }
}

CommandsInterface::~CommandsInterface() {
    // Cancel last delayed read;
    mFIFOStreamDescriptor->cancel();
    delete mFIFOStreamDescriptor;

    // Cancel last timeout operation
    mReadTimeoutTimer->cancel();
    delete mReadTimeoutTimer;

    close(mFIFODescriptor);
    delete mCommandsParser;
}

void CommandsInterface::beginAcceptCommands() {
    asyncReceiveNextCommand();
}

void CommandsInterface::asyncReceiveNextCommand() {
    as::async_read(
        *mFIFOStreamDescriptor,
        as::buffer(mCommandBuffer),
        as::transfer_at_least(1),
        boost::bind(
            &CommandsInterface::handleReceivedInfo, this,
            as::placeholders::error,
            as::placeholders::bytes_transferred));
}

void CommandsInterface::handleReceivedInfo(
    const boost::system::error_code &error,
    const size_t bytesTransferred) {

    if (!error || error == as::error::message_size) {
        auto parsingResult =
            mCommandsParser->processReceivedCommandPart(
                mCommandBuffer.data(), bytesTransferred);

        if (parsingResult.first){
            mTransactionsManager->processCommand(parsingResult.second);
        }

        // In case of successive read - next command should be received
        // regardless of command processing.
        asyncReceiveNextCommand();

    } else {
        if (error == as::error::eof) {

            // It is possible, that FIFO file is not opened by the other side.
            // Next read attempt should be performed with a timeout,
            // to prevent heavy processor usage.
            mReadTimeoutTimer->expires_from_now(boost::posix_time::seconds(1));
            mReadTimeoutTimer->async_wait(
                boost::bind(
                    &CommandsInterface::handleTimeout, this, as::placeholders::error));

        } else {
            auto msg = string("Can't receive command. Error details are: ") + error.message();
            mLog->logError("CommandsInterface", msg);

            // Looks like FIFO file is corrupted or unreachable.
            // Next read attempt should be performed with a long timeout.
            mReadTimeoutTimer->expires_from_now(boost::posix_time::minutes(10));
            mReadTimeoutTimer->async_wait(
                boost::bind(
                    &CommandsInterface::handleTimeout, this, as::placeholders::error));
        }
    }
}

const char *CommandsInterface::name() const {
    return kFIFOName;
}

void CommandsInterface::handleTimeout(
    const boost::system::error_code &error) {

    if (error) {
        mLog->logError("CommandsInterface::handleTimeout", error.message());
    }

    asyncReceiveNextCommand();
}
