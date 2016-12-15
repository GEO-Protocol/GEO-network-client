#include "CommandsInterface.h"


/*!
 * Handles received data from FIFO.
 * Writes it to the internal buffer for further processing.
 * Calls tryDeserializeCommand() internally for buffer processing.
 * Return value is similar to the tryDeserializeCommand() return value.
 */
pair<bool, shared_ptr<Command>> CommandsParser::processReceivedCommandPart(const char *commandPart, const size_t receivedBytesCount) {
    mBuffer.reserve(mBuffer.size() + receivedBytesCount);
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

    return tryParseCommand(commandUUID, commandIdentifier, mBuffer.substr(nextTokenOffset, mBuffer.size()));
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
    mTransactionsManager = new TransactionsManager();
}

CommandsInterface::~CommandsInterface() {
    close(mFIFODescriptor);
    delete mFIFOStreamDescriptor;
    delete mCommandsParser;
    delete mTransactionsManager;
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
    mFIFOStreamDescriptor->async_read_some(as::buffer(mCommandBuffer),
                                           boost::bind(&CommandsInterface::handleReceivedInfo,
                                                    this,
                                                    boost::asio::placeholders::error,
                                                    boost::asio::placeholders::bytes_transferred)
    );
}

void CommandsInterface::handleReceivedInfo(const boost::system::error_code &error, const size_t bytesTransferred) {

    if (!error || error == as::error::message_size) {
        auto parsingResult = mCommandsParser->processReceivedCommandPart(mCommandBuffer.data(), bytesTransferred);
        if (parsingResult.first){
            mTransactionsManager->acceptCommand(parsingResult.second.get());
        }
    }
    // In all cases - commands receiving should be continued
    asyncReceiveNextCommand();
}
