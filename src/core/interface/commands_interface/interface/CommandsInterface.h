#ifndef GEO_NETWORK_CLIENT_COMMANDSRECEIVER_H
#define GEO_NETWORK_CLIENT_COMMANDSRECEIVER_H

#include "../../BaseFIFOInterface.h"

#include "../../../logger/Logger.h"

#include "../commands/trust_lines/SetOutgoingTrustLineCommand.h"
#include "../commands/payments/CreditUsageCommand.h"
#include "../commands/max_flow_calculation/InitiateMaxFlowCalculationCommand.h"
#include "../commands/total_balances/TotalBalancesCommand.h"
#include "../commands/total_balances/TotalBalancesRemouteNodeCommand.h"
#include "../commands/history/HistoryPaymentsCommand.h"
#include "../commands/history/HistoryAdditionalPaymentsCommand.h"
#include "../commands/history/HistoryTrustLinesCommand.h"
#include "../commands/history/HistoryWithContractorCommand.h"
#include "../commands/trust_lines_list/GetFirstLevelContractorsCommand.h"
#include "../commands/trust_lines_list/GetTrustLinesCommand.h"
#include "../commands/trust_lines_list/GetTrustLineCommand.h"
#include "../commands/subsystems_controller/SubsystemsInfluenceCommand.h"

#include "../../../common/exceptions/IOError.h"
#include "../../../common/exceptions/ValueError.h"
#include "../../../common/exceptions/MemoryError.h"
#include "../../../common/exceptions/RuntimeError.h"

#include <boost/signals2.hpp>
#include <boost/bind.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include <string>
#include <memory>

#ifndef TESTS__TRUSTLINES
#include "../../../common/Types.h"
#include "../../../common/time/TimeUtils.h"
#endif


using namespace std;
using namespace boost::uuids;
namespace signals = boost::signals2;

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
        Logger &log);

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

protected:

    template <typename CommandType, typename... Args>
    inline pair<bool, BaseUserCommand::Shared> newCommand(
        const CommandUUID &uuid,
        const string &buffer) const
    {
        return make_pair(
            true,
            static_pointer_cast<BaseUserCommand>(
                make_shared<CommandType>(
                    uuid,
                    buffer)));
    }

private:
    string mBuffer;
    Logger &mLog;
};


// todo: refactor, use signals

/**
 * User commands are transmitted via named pipe (FIFO on Linux).
 * This class is used to asynchronously receive them, parse,
 * and transfer for the further execution.
 */
class CommandsInterface: public BaseFIFOInterface {
public:
    signals::signal<void(BaseUserCommand::Shared)> commandReceivedSignal;

public:
    explicit CommandsInterface(
        as::io_service &ioService,
        Logger &logger);

    ~CommandsInterface();

    void beginAcceptCommands();

protected:
    void asyncReceiveNextCommand();

    void handleReceivedInfo(
        const boost::system::error_code &error,
        const size_t bytesTransferred);

    void handleTimeout(
        const boost::system::error_code &error);

    virtual const char* FIFOName() const;

public:
    static const constexpr char *kFIFOName = "commands.fifo";
    static const constexpr unsigned int kPermissionsMask = 0755;

protected:
    static const constexpr size_t kCommandBufferSize = 1024;

    as::io_service &mIOService;
    Logger &mLog;

    as::streambuf mCommandBuffer;

    unique_ptr<as::posix::stream_descriptor> mFIFOStreamDescriptor;
    unique_ptr<as::deadline_timer> mReadTimeoutTimer;
    unique_ptr<CommandsParser> mCommandsParser;
};

#endif //GEO_NETWORK_CLIENT_COMMANDSRECEIVER_H
