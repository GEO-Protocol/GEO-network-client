#ifndef GEO_NETWORK_CLIENT_COMMANDSRECEIVER_H
#define GEO_NETWORK_CLIENT_COMMANDSRECEIVER_H

#include "../../BaseFIFOInterface.h"

#include "../../../transactions/manager/TransactionsManager.h"
#include "../../../logger/Logger.h"

#include "../commands/trust_lines/OpenTrustLineCommand.h"
#include "../commands/trust_lines/CloseTrustLineCommand.h"
#include "../commands/trust_lines/SetTrustLineCommand.h"
#include "../commands/payments/CreditUsageCommand.h"

#include "../../../common/exceptions/IOError.h"
#include "../../../common/exceptions/ValueError.h"
#include "../../../common/exceptions/MemoryError.h"
#include "../../../common/exceptions/RuntimeError.h"

#include <boost/bind.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include <string>

using namespace std;
using namespace boost::uuids;

/**
 * User commands are transmitted via text protocol.
 * CommandsParser is used for parsing received user input
 * and deserializing them into commands instances.
 *
 *
 * Note: shares almost the same logic as "MessagesParser"
 * (see IncomingMessagesHandler.h for details).
 *
 * todo: hsc: review this class
 */
class CommandsParser {
    friend class CommandsParserTests;

public:
    CommandsParser(
        Logger *log);

    void appendReadData(
        as::streambuf *buffer,
        const size_t receivedBytesCount);

    pair<bool, BaseUserCommand::Shared> processReceivedCommands();

private:
    inline pair<bool, BaseUserCommand::Shared> tryDeserializeCommand();

    inline pair<bool, BaseUserCommand::Shared> tryParseCommand(
        const CommandUUID &uuid,
        const string &identifier,
        const string &buffer);

    inline pair<bool, BaseUserCommand::Shared> commandIsInvalidOrIncomplete();

    void cutBufferUpToNextCommand();

public:
    static const size_t kUUIDHexRepresentationSize = 36;
    static const size_t kMinCommandSize = kUUIDHexRepresentationSize + 2;
    static const size_t kAverageCommandIdentifierLength = 15;
    static const char kCommandsSeparator = '\n';
    static const char kTokensSeparator = '\t';

private:
    string mBuffer;
    Logger *mLog;
};


// todo: refactor, use signals

/**
 * User commands are transmitted via named pipe (FIFO on Linux).
 * This class is used to asynchronously receive them, parse,
 * and transfer for the further execution.
 */
class CommandsInterface: public BaseFIFOInterface {
public:
    explicit CommandsInterface(
        as::io_service &ioService,
        TransactionsManager *transactionsManager,
        Logger *logger);

    ~CommandsInterface();

    void beginAcceptCommands();

public:
    static const constexpr char *kFIFOName = "commands.fifo";
    static const constexpr unsigned int kPermissionsMask = 0755;

protected:
    virtual const char* FIFOName() const;

    void asyncReceiveNextCommand();

    void handleReceivedInfo(
        const boost::system::error_code &error,
        const size_t bytesTransferred);

    void handleTimeout(
        const boost::system::error_code &error);

protected:
    static const constexpr size_t kCommandBufferSize = 1024;

    as::io_service &mIOService;
    TransactionsManager *mTransactionsManager;
    Logger *mLog;

    as::streambuf mCommandBuffer;

    // todo: use unique_ptr
    as::posix::stream_descriptor *mFIFOStreamDescriptor;
    as::deadline_timer *mReadTimeoutTimer;
    CommandsParser *mCommandsParser;
};

#endif //GEO_NETWORK_CLIENT_COMMANDSRECEIVER_H
