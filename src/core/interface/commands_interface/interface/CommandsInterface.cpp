#include "CommandsInterface.h"


CommandsParser::CommandsParser(
    Logger &log):

    mLog(log)
{}

/**
 * Copies data, received from the commands FIFO, into the internal buffer.
 * There is a non-zero probability, that buffer will contains some part of previous command,
 * that was not parsed on previous steps. It may happen, when async read from FIFO returned
 * one and a half of the command. In such a case - newly received data will be appended to the
 * previously received.
 *
 * @param buffer - pointer to the buffer, that collects raw input from the user (ASIO buffer).
 * @param receivedBytesCount - specifies how many bytes was received by the last FIFO read.
 *      (how many bytes are in the "buffer" at the moment)
 */
void CommandsParser::appendReadData(
    as::streambuf *buffer,
    const size_t receivedBytesCount)
{
    if (receivedBytesCount == 0) {
        return;
    }

    // TODO: check if using buffer->data() doesn't leads to the buffer overflow.
    const char *data = as::buffer_cast<const char*>(buffer->data());
    for (size_t i = 0; i < receivedBytesCount; ++i) {
        mBuffer.push_back(*data++);
    }
}

/**
 * Parses internal buffer for user commands.
 * Returns <true, command> in case, when command was parsed well.
 * Otherwise returns <false, nullptr>.
 *
 * This method should be called several times to parse all received commands.
 *
 * See: tryDeserializeCommand() for the details.
 */
pair<bool, BaseUserCommand::Shared> CommandsParser::processReceivedCommands()
{
    return tryDeserializeCommand();
}

/*!
 * Tries to deserialize received command.
 * In case when buffer is too short to contains even one command -
 * will return immediately.
 *
 * First value of returned pair indicates if command was parsed succesfully,
 * and, if so, - the second one will contains shared pointer to the command instance itself.
 * Otherwise - the second value of the pair will contains "nullptr".
 */
pair<bool, BaseUserCommand::Shared> CommandsParser::tryDeserializeCommand()
{
    if (mBuffer.size() < kMinCommandSize) {
        return commandIsInvalidOrIncomplete();
    }

    size_t nextCommandSeparatorIndex = mBuffer.find(kCommandsSeparator);
    if (nextCommandSeparatorIndex == string::npos) {
        return commandIsInvalidOrIncomplete();
    }

    CommandUUID commandUUID;
    try {
        string hexUUID = mBuffer.substr(0, kUUIDHexRepresentationSize);
        commandUUID = boost::lexical_cast<uuid>(hexUUID);

    } catch (...) {
        cutBufferUpToNextCommand();
        return commandIsInvalidOrIncomplete();
    }


    string commandIdentifier;
    const size_t identifierOffset = kUUIDHexRepresentationSize + 1;

    commandIdentifier.reserve(kAverageCommandIdentifierLength);
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
        cutBufferUpToNextCommand();
        return commandIsInvalidOrIncomplete();
    }

    try {
        auto nextCommandBegin = mBuffer.find(kCommandsSeparator);
        if (nextCommandBegin == string::npos) {
            nextCommandBegin = mBuffer.size();
        }

        auto command = tryParseCommand(
            commandUUID,
            commandIdentifier,
            mBuffer.substr(
                nextTokenOffset,
                nextCommandBegin - nextTokenOffset + 1));

        cutBufferUpToNextCommand();
        return command;

    } catch (std::exception &e) {
        cutBufferUpToNextCommand();
        return commandIsInvalidOrIncomplete();
    }
}

/*!
 * Checks identifier and tries to build relevant command object.
 * @returns <true, command object> in case of success, otherwise returns <false, nullptr>.
 *
 * @param uuid - uuid of the received command (parsed on previous step).
 * @param identifier - identifier of the received command (parsed on the previous step).
 * @param buffer - content of the command without it's identifier.
 *
 * @throws ValueError in case if @param identifier would be unexpected.
 */
pair<bool, BaseUserCommand::Shared> CommandsParser::tryParseCommand(
    const CommandUUID &uuid,
    const string &identifier,
    const string &buffer)
{
    BaseUserCommand *command = nullptr;
    try {
        if (identifier == SetOutgoingTrustLineCommand::identifier()) {
            command = new SetOutgoingTrustLineCommand(
                uuid,
                buffer);

        } else if (identifier == CloseIncomingTrustLineCommand::identifier()) {
            command = new CloseIncomingTrustLineCommand(
                uuid,
                buffer);

        } else if (identifier == CreditUsageCommand::identifier()) {
            command = new CreditUsageCommand(
                uuid,
                buffer);

        } else if (identifier == InitiateMaxFlowCalculationCommand::identifier()) {
            command = new InitiateMaxFlowCalculationCommand(
                uuid,
                buffer);

        } else if (identifier == InitiateMaxFlowCalculationFullyCommand::identifier()) {
            command = new InitiateMaxFlowCalculationFullyCommand(
                uuid,
                buffer);

        } else if (identifier == TotalBalancesCommand::identifier()) {
            command = new TotalBalancesCommand(
                uuid,
                buffer);

        } else if (identifier == HistoryPaymentsCommand::identifier()) {
            command = new HistoryPaymentsCommand(
                uuid,
                buffer);

        } else if (identifier == HistoryAdditionalPaymentsCommand::identifier()) {
            command = new HistoryAdditionalPaymentsCommand(
                uuid,
                buffer);

        } else if (identifier == HistoryTrustLinesCommand::identifier()) {
            command = new HistoryTrustLinesCommand(
                uuid,
                buffer);

        } else if (identifier == HistoryWithContractorCommand::identifier()) {
            command = new HistoryWithContractorCommand(
                uuid,
                buffer);

        } else if (identifier == GetFirstLevelContractorsCommand::identifier()) {
            command = new GetFirstLevelContractorsCommand(
                uuid,
                buffer);

        } else if (identifier == GetTrustLinesCommand::identifier()) {
            command = new GetTrustLinesCommand(
                uuid,
                buffer);

        } else if (identifier == GetTrustLineCommand::identifier()) {
            command = new GetTrustLineCommand(
                uuid,
                buffer);

        } else if (identifier == EquivalentListCommand::identifier()) {
            command = new EquivalentListCommand(
                uuid,
                buffer);

        } else if (identifier == SubsystemsInfluenceCommand::identifier()) {
            return newCommand<SubsystemsInfluenceCommand>(
                uuid,
                buffer);

        } else if (identifier == TrustLinesInfluenceCommand::identifier()) {
            return newCommand<TrustLinesInfluenceCommand>(
                uuid,
                buffer);

            // Black list command
        } else if (identifier == AddNodeToBlackListCommand::identifier()) {
            return newCommand<AddNodeToBlackListCommand>(
                uuid,
                buffer);

        } else if (identifier == RemoveNodeFromBlackListCommand::identifier()) {
            return newCommand<RemoveNodeFromBlackListCommand>(
                uuid,
                buffer);

        } else if (identifier == CheckIfNodeInBlackListCommand::identifier()) {
            return newCommand<CheckIfNodeInBlackListCommand>(
                uuid,
                buffer);

        } else if (identifier == GetBlackListCommand::identifier()) {
            return newCommand<GetBlackListCommand>(
                uuid,
                buffer);

        } else if (identifier == PaymentTransactionByCommandUUIDCommand::identifier()) {
            return newCommand<PaymentTransactionByCommandUUIDCommand>(
                uuid,
                buffer);

        } else {
            throw RuntimeError(
                "CommandsParser::tryParseCommand: "
                    "unexpected command identifier received. " + identifier);
        }

    } catch (bad_alloc &) {
        error() << "tryParseCommand: Memory allocation error occurred on command instance creation. "
                << "Command was dropped. ";

        return commandIsInvalidOrIncomplete();

    } catch (exception &e){
        mLog.logException("CommandsParser", e);
        return commandIsInvalidOrIncomplete();
    }

    return make_pair(
        true,
        BaseUserCommand::Shared(command));
}

/*!
 * Removes last command (valid, or invalid) from the buffer.
 * Stops on commands separator symbol.
 * If no commands separator symbol is present - clears buffer at all.
 */
void CommandsParser::cutBufferUpToNextCommand()
{
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

pair<bool, BaseUserCommand::Shared> CommandsParser::commandIsInvalidOrIncomplete()
{
    return make_pair(
        false,
        nullptr);
}

CommandsInterface::CommandsInterface(
    as::io_service &ioService,
    Logger &logger) :

    mIOService(ioService),
    mLog(logger)
{
    // Try to open FIFO file in non-blocking manner.
    // In case if this file will be opened in standard blocking manner -
    // it will freeze whole the process,
    // until writer would connect to the FIFO from the other side.
    //
    // For the server realisation this makes the process unusable,
    // because it can't be demonized, until the commands writer will
    // open the commands file for writing.
    if (!isFIFOExists()) {
        createFIFO(kPermissionsMask);
    }

    mFIFODescriptor = open(
        FIFOFilePath().c_str(),
        O_RDONLY | O_NONBLOCK);

    if (mFIFODescriptor == -1) {
        throw IOError("CommandsInterface::CommandsInterface: "
                          "Can't open FIFO file.");
    }

    try {
        mFIFOStreamDescriptor = unique_ptr<as::posix::stream_descriptor> (new as::posix::stream_descriptor(
            mIOService,
            mFIFODescriptor));
        mFIFOStreamDescriptor->non_blocking(true);

    } catch (std::bad_alloc &) {
        throw MemoryError("CommandsInterface::CommandsInterface: "
                              "Can not allocate enough memory for fifo stream descriptor.");
    }

    try {
        mReadTimeoutTimer = unique_ptr<as::deadline_timer> (new as::deadline_timer(
            mIOService,
            boost::posix_time::seconds(2)));

    } catch (std::bad_alloc &) {
        throw MemoryError(
            "CommandsInterface::CommandsInterface: "
                "Can not allocate enough memory for fifo read timer.");
    }

    try {
        mCommandsParser = unique_ptr<CommandsParser> (new CommandsParser(mLog));

    } catch (std::bad_alloc &) {
        throw MemoryError(
            "CommandsInterface::CommandsInterface: "
                "Can not allocate enough memory for commands parser.");
    }

    mCommandBuffer.prepare(kCommandBufferSize);
}

CommandsInterface::~CommandsInterface()
{
    mFIFOStreamDescriptor->cancel();
    mReadTimeoutTimer->cancel();
    close(mFIFODescriptor);
}

void CommandsInterface::beginAcceptCommands()
{
    asyncReceiveNextCommand();
}

void CommandsInterface::asyncReceiveNextCommand()
{
    as::async_read_until(
        *mFIFOStreamDescriptor,
        mCommandBuffer,
        kCommandsSeparator,
        boost::bind(
            &CommandsInterface::handleReceivedInfo,
            this,
            as::placeholders::error,
            as::placeholders::bytes_transferred));
}

/** Parse received commands data until parsing is successfully.
 *  (several commands may be read at once, so the parsing should be repeated
 *  until all the received data would be processed)
 *
 *  It is possible, that FIFO file is not opened by the other side.
 *  Next read attempt should be performed with a Timeout,
 *  to prevent heavy processor usage.
 */
void CommandsInterface::handleReceivedInfo(
    const boost::system::error_code &error,
    const size_t bytesTransferred)
{
    if (!error || error == as::error::message_size) {
        mCommandsParser->appendReadData(
            &mCommandBuffer,
            bytesTransferred);
        mCommandBuffer.consume(bytesTransferred);

        while (true) {
            auto flagAndCommand = mCommandsParser->processReceivedCommands();
            if (flagAndCommand.first){
                commandReceivedSignal(flagAndCommand.second);
            } else {
                break;
            }
        }
        asyncReceiveNextCommand();

    } else {
        if (error == as::error::eof) {
            mReadTimeoutTimer->expires_from_now(boost::posix_time::seconds(1));
            mReadTimeoutTimer->async_wait(
                boost::bind(
                    &CommandsInterface::handleTimeout,
                    this,
                    as::placeholders::error));

        } else {
            // Looks like FIFO file is corrupted or unreachable.
            // Next read attempt should be performed with a long Timeout.
            mReadTimeoutTimer->expires_from_now(boost::posix_time::minutes(10));
            mReadTimeoutTimer->async_wait(
                boost::bind(
                &CommandsInterface::handleTimeout,
                this,
                as::placeholders::error));
        }
    }
}

void CommandsInterface::handleTimeout(
    const boost::system::error_code &errorMessage)
{
    if (errorMessage) {
        error() << "handleTimeout error: " << errorMessage.message();
    }

    asyncReceiveNextCommand();
}

const char *CommandsInterface::FIFOName() const {

    return kFIFOName;
}

string CommandsInterface::logHeader()
    noexcept
{
    return "[CommandsInterface]";
}

LoggerStream CommandsInterface::error() const
    noexcept
{
    return mLog.error(logHeader());
}

string CommandsParser::logHeader()
    noexcept
{
    return "[CommandsParser]";
}

LoggerStream CommandsParser::error() const
    noexcept
{
    return mLog.error(logHeader());
}
