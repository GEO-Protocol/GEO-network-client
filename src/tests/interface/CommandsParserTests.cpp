#include "../../core/interface/CommandsInterface.h"


class CommandsParserTests {
public:
    void run() {
        checkDataConcatenation();
        checkParsingWithoutTerminationSymbol();
        checkParsingWithInvalidUUID();
        checkParsingWithEmptyUUID();
        checkParsingWithEmptyIdentifier();
        checkParsingWithValidIdentifier();
        checkParsingOneAndHalfCommand();

        // todo: add test when command identifier is unexpected.

        // ..
        // Commands tests goes here
    };

    void checkDataConcatenation() {
        // This test tries to parse several short messages.
        // It ensures that input data, that would be received
        // will be collected in the buffer for further parsing.
        //
        // Data may arrive partially,
        // so the parser must collect all the data,
        // and start parsing only in case when whole the command was received.

        CommandsParser parser;

        const char *message = "short";

        for (int i=0; i<5; ++i) {
            auto resp = parser.processReceivedCommand(message, sizeof(message));

            // Command should not be parsed until '\n'
            // would not appear in the data stream.
            assert(resp.first == false);
            assert(resp.second == nullptr);
        }

        // Check if command was collected into the buffer.
        assert(parser.mBuffer.size() == 5*sizeof(message));
    };

    void checkParsingWithoutTerminationSymbol() {
        // This test tries to parse command without termination symbol;

        CommandsParser parser;

        const char *message = "invalid uuid invalid uuid invalid uuid";
        auto resp = parser.processReceivedCommand(message, strlen(message));
        assert(resp.first == false);
        assert(resp.second == nullptr);

        // Parsing should not be triggered.
        // There is no termination symbol;
        assert(parser.mBuffer.size() == 38);
    }

    void checkParsingWithInvalidUUID() {
        CommandsParser parser;

        const char *message = "invalid uuid invalid uuid invalid uuid \n";
        auto resp = parser.processReceivedCommand(message, strlen(message));
        assert(resp.first == false);
        assert(resp.second == nullptr);

        // Command should be rejected and internal buffer should be cleared.
        assert(parser.mBuffer.size() == 0);
    };

    void checkParsingWithEmptyUUID() {
        CommandsParser parser;

        const char *message = "\n";
        auto resp = parser.processReceivedCommand(message, strlen(message));
        assert(resp.first == false);
        assert(resp.second == nullptr);

        // Command should be rejected and internal buffer should be cleared.
        assert(parser.mBuffer.size() == 0);
    }

    void checkParsingWithEmptyIdentifier() {
        CommandsParser parser;

        // Valid UUID but empty identifier
        const char *message = "550e8400-e29b-41d4-a716-446655440000 \n";
        auto resp = parser.processReceivedCommand(message, strlen(message));
        assert(resp.first == false);
        assert(resp.second == nullptr);

        // Command should be rejected and internal buffer should be cleared.
        assert(parser.mBuffer.size() == 0);
    }

    void checkParsingWithValidIdentifier() {
        CommandsParser parser;

        // Valid UUID but empty identifier
        const char *message = "550e8400-e29b-41d4-a716-446655440000 Identifier \n";
        auto resp = parser.processReceivedCommand(message, strlen(message));
        assert(resp.first == true);
        assert(resp.second.get()->identifier() == string("Identifier"));

        // Command should be rejected and internal buffer should be cleared.
        assert(parser.mBuffer.size() == 0);
    }

    void checkParsingOneAndHalfCommand() {
        // This test emulates the case,
        // when second command arrives right in the moment with previous one,
        // and makes asio taking them both.
        //
        // The second command probably will arrive partially,
        // this test aims to check that the part of the second command will be keept,
        // after first command is parsed.

        CommandsParser parser;

        // Valid UUID but empty identifier
        const char *message = "550e8400-e29b-41d4-a716-446655440000 Identifier \n550e8400-e29b-41d4-a716-446655440000";
        auto resp = parser.processReceivedCommand(message, strlen(message));
        assert(resp.first == true);
        assert(resp.second.get()->identifier() == string("Identifier"));

        // Command should be rejected and internal buffer should be cleared.
        assert(parser.mBuffer == string("550e8400-e29b-41d4-a716-446655440000"));
    }

};