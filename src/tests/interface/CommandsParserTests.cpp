#include "../../core/interface/commands/CommandsInterface.h"


class CommandsParserTests {
public:
    void run() {
        /*checkDataConcatenation();
        checkParsingWithoutTerminationSymbol();
        checkParsingWithInvalidUUID();
        checkParsingWithEmptyUUID();
        checkParsingWithEmptyIdentifier();
        checkParsingWithValidIdentifier();
        checkParsingOneAndHalfCommand();*/

        checkParsingOpenTrustLineCommand();
        checkParsingCloseTrustLineCommand();
        checkParsingUpdateOutgoingTrustLineCommand();
        checkParsingUseCreditCommand();
    };

    /*void checkDataConcatenation() {
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
            auto resp = parser.processReceivedCommandPart(message, sizeof(message));

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
        auto resp = parser.processReceivedCommandPart(message, strlen(message));
        assert(resp.first == false);
        assert(resp.second == nullptr);

        // Parsing should not be triggered.
        // There is no termination symbol;
        assert(parser.mBuffer.size() == 38);
    }

    void checkParsingWithInvalidUUID() {
        CommandsParser parser;

        const char *message = "invalid uuid invalid uuid invalid uuid \n";
        auto resp = parser.processReceivedCommandPart(message, strlen(message));
        assert(resp.first == false);
        assert(resp.second == nullptr);

        // Command should be rejected and internal buffer should be cleared.
        assert(parser.mBuffer.size() == 0);
    };

    void checkParsingWithEmptyUUID() {
        CommandsParser parser;

        const char *message = "\n";
        auto resp = parser.processReceivedCommandPart(message, strlen(message));
        assert(resp.first == false);
        assert(resp.second == nullptr);

        // Command should be rejected and internal buffer should be cleared.
        assert(parser.mBuffer.size() == 0);
    }

    void checkParsingWithEmptyIdentifier() {
        CommandsParser parser;

        // Valid UUID but empty identifier
        const char *message = "550e8400-e29b-41d4-a716-446655440000 \n";
        auto resp = parser.processReceivedCommandPart(message, strlen(message));
        assert(resp.first == false);
        assert(resp.second == nullptr);

        // Command should be rejected and internal buffer should be cleared.
        assert(parser.mBuffer.size() == 0);
    }

    void checkParsingWithValidIdentifier() {
        CommandsParser parser;

        // Valid UUID but empty identifier
        const char *message = "550e8400-e29b-41d4-a716-446655440000 Identifier \n";
        auto resp = parser.processReceivedCommandPart(message, strlen(message));
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
        auto resp = parser.processReceivedCommandPart(message, strlen(message));
        assert(resp.first == true);
        assert(resp.second.get()->identifier() == string("Identifier"));

        // Command should be rejected and internal buffer should be cleared.
        assert(parser.mBuffer == string("550e8400-e29b-41d4-a716-446655440000"));
    }*/

    void checkParsingOpenTrustLineCommand(){
        CommandsParser parser;

        const char *command = "550e8400-e29b-41d4-a716-446655440000 trustlines/open 550e8400-e29b-41d4-a716-446655440000 150\ntrustlines/open";
        auto response = parser.processReceivedCommandPart(command, strlen(command));
        Command *c = response.second.get();
        OpenTrustLineCommand *openTrustLineCommand = dynamic_cast<OpenTrustLineCommand *>(c);

        assert(response.first);
        assert(openTrustLineCommand->commandUUID() == boost::lexical_cast<uuids::uuid>("550e8400-e29b-41d4-a716-446655440000"));
        assert(openTrustLineCommand->id() == string("trustlines/open"));
        assert(openTrustLineCommand->contractorUUID().stringUUID() == string("550e8400-e29b-41d4-a716-446655440000"));
        assert(openTrustLineCommand->amount() == 150);
    }

    void checkParsingCloseTrustLineCommand(){
        CommandsParser parser;

        const char *command = "550e8400-e29b-41d4-a716-446655440000 trustlines/close 550e8400-e29b-41d4-a716-446655440000\ntrustlines/open";
        auto response = parser.processReceivedCommandPart(command, strlen(command));
        Command *c = response.second.get();
        CloseTrustLineCommand *closeTrustLineCommand = dynamic_cast<CloseTrustLineCommand *>(c);

        assert(response.first);
        assert(closeTrustLineCommand->commandUUID() == boost::lexical_cast<uuids::uuid>("550e8400-e29b-41d4-a716-446655440000"));
        assert(closeTrustLineCommand->id() == string("trustlines/close"));
        assert(closeTrustLineCommand->contractorUUID().stringUUID() == string("550e8400-e29b-41d4-a716-446655440000"));
    }

    void checkParsingUpdateOutgoingTrustLineCommand(){
        CommandsParser parser;

        const char *command = "550e8400-e29b-41d4-a716-446655440000 trustlines/update 550e8400-e29b-41d4-a716-446655440000 11456\ntrustlines/open";
        auto response = parser.processReceivedCommandPart(command, strlen(command));
        Command *c = response.second.get();
        UpdateOutgoingTrustAmountCommand *updateTrustLineCommand = dynamic_cast<UpdateOutgoingTrustAmountCommand *>(c);

        assert(response.first);
        assert(updateTrustLineCommand->commandUUID() == boost::lexical_cast<uuids::uuid>("550e8400-e29b-41d4-a716-446655440000"));
        assert(updateTrustLineCommand->id() == string("trustlines/update"));
        assert(updateTrustLineCommand->contractorUUID().stringUUID() == string("550e8400-e29b-41d4-a716-446655440000"));
        assert(updateTrustLineCommand->amount() == 11456);
    }

    void checkParsingUseCreditCommand(){
        CommandsParser parser;

        const char *command = "550e8400-e29b-41d4-a716-446655440000 transactions/usecredit 550e8400-e29b-41d4-a716-446655440000 11456 [prosto tak]\ntrustlines/open";
        auto response = parser.processReceivedCommandPart(command, strlen(command));
        Command *c = response.second.get();
        UseCreditCommand *useCeditCommand = dynamic_cast<UseCreditCommand *>(c);

        assert(response.first);
        assert(useCeditCommand->commandUUID() == boost::lexical_cast<uuids::uuid>("550e8400-e29b-41d4-a716-446655440000"));
        assert(useCeditCommand->id() == string("transactions/usecredit"));
        assert(useCeditCommand->contractorUUID().stringUUID() == string("550e8400-e29b-41d4-a716-446655440000"));
        assert(useCeditCommand->amount() == 11456);
        assert(useCeditCommand->purpose() == string("prosto tak"));
    }

};