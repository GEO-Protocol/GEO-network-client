#include "CommandsInterface.h"
#include "../common/exceptions/ValueError.h"

pair<bool, shared_ptr<Command>> CommandsParser::processReceivedCommand(
    const char *commandPart, const size_t receivedBytesCount) {

    // Memory reservation for whole the command
    // to prevent huge amount of memory reallocations.
    mBuffer.reserve(mBuffer.size() + receivedBytesCount);

    // Concatenating with the previously received command parts.
    for (size_t i=0; i<receivedBytesCount; ++i) {
        mBuffer.push_back(commandPart[i]);
    }

    return tryDeserializeCommand();
}

pair<bool, shared_ptr<Command>> CommandsParser::tryDeserializeCommand() {
    if (mBuffer.size() < kMinCommandSize) {
        // if command contains trailing symbol,
        // but is shorter than min command size -
        // then this command is invalid and should be rejected.
        if (mBuffer.find(kCommandsSeparator) != string::npos) {
            cutCommandFromTheBuffer();
        }

        return invalidMessageReturnValue();
    }

    // Check if received sting contains command separator.
    // If no - the command is received partially.
    size_t nextCommandSeparatorIndex = mBuffer.find(kCommandsSeparator);
    if (nextCommandSeparatorIndex == string::npos) {
        return invalidMessageReturnValue();
    }

    // UUID parsing
    uuids::uuid uuid;
    string hexUUID = mBuffer.substr(0, kUUIDHexRepresentationSize);
    try {
        uuid = boost::lexical_cast<uuids::uuid>(hexUUID);
    } catch (...) {
        // UUID can't be parsed correctly.
        // Command seems to be broken and should be rejected.
        cutCommandFromTheBuffer();
        return invalidMessageReturnValue();
    }

    // Command identifier parsing
    const size_t averageCommandIdentifierLength = 12; // for optimisations reasons only.
    const size_t identifierOffset = kUUIDHexRepresentationSize + 1; // + separator

    string identifier;
    identifier.reserve(averageCommandIdentifierLength);
    for (size_t i=identifierOffset; i<mBuffer.size(); ++i){
        char symbol = mBuffer.at(i);
        if (symbol == kTokensSeparator || symbol == kCommandsSeparator){
            break;
        }

        identifier.push_back(symbol);
    }

    // Validate received identifier
    if (identifier.size() == 0) {
        // Command identifier can't be parsed.
        // Command seems to be broken and should be rejected.
        cutCommandFromTheBuffer();
        return invalidMessageReturnValue();
    }

    return tryParseCommand(uuid, identifier);
}

pair<bool, shared_ptr<Command>> CommandsParser::tryParseCommand(
    const uuids::uuid &commandUUID, const string &commandIdentifier) {

    // todo: add realisation here
#ifdef DEBUG
    std::cout << "Command received: " << commandIdentifier << std::endl;
#endif

    cutCommandFromTheBuffer();

    auto command = new Command(commandIdentifier);
    return pair<bool, shared_ptr<Command>>(
        true, shared_ptr<Command>(command));
}

void CommandsParser::cutCommandFromTheBuffer() {
    size_t nextCommandSeparatorIndex = mBuffer.find(kCommandsSeparator);
    if (mBuffer.size() > (nextCommandSeparatorIndex + 1)) {
        // Buffer contains other commands (or their parts), and them should be keept;
        mBuffer = mBuffer.substr(nextCommandSeparatorIndex+1, mBuffer.size()-1);

    } else {
        // Buffer doesn't contains any other commands (even parts).
        mBuffer.clear();
    }

    // Resize the capacity to free unused memory.
    mBuffer.shrink_to_fit();
}

pair<bool, shared_ptr<Command>> CommandsParser::invalidMessageReturnValue() {
    return pair<bool, shared_ptr<Command>>(false, nullptr);
}

CommandsInterface::CommandsInterface(as::io_service &ioService, CommandsAPI *API):
    mIOService(ioService), mAPI(API) {

    if (API == nullptr) {
        throw ValueError("Commands API can't be nullptr.");
    }

    if (! FIFOExists()) {
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
    if (! FIFOExists()) {
        auto path = FIFOPath();
        fs::create_directories(dir());
        mkfifo(path.c_str(), 0420); // Read; Write; No access; // todo: tests mask
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
        mCommandsParser->processReceivedCommand(
            mCommandBuffer.data(), bytesTransferred);
    }

    // In all cases - commands receiving should be continued
    asyncReceiveNextCommand();
}
