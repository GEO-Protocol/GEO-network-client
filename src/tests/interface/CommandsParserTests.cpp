//#include "../../core/interface/commands/CommandsInterface.h"
//#include "../../core/transactions/TransactionsManager.h"
//
//
//class CommandsParserTests {
//public:
//    void run() {
//        //successCaseOfParsingOpenTrustLineCommand();
////        failureCaseOfParsingOpenTrustLineCommandWithEmptyCommandUUID();
//        //failureCaseOfParsingOpenTrustLineCommandWithInvalidCommandUUID();
//        //failureCaseOfParsingOpenTrustLineCommandWithEmptyCommandIdentifier();
//        //failureCaseOfParsingOpenTrustLineCommandWithInvalidCommandIdentifier();
//        //failureCaseOfParsingOpenTrustLineCommandWithEmptyContractorUUID();
//        //failureCaseOfParsingOpenTrustLineCommandWithInvalidContractorUUID();
//        //failureCaseOfParsingOpenTrustLineCommandWithEmptyTrustValue();
//        //failureCaseOfParsingOpenTrustLineCommandWithFloatTrustValue();
//        //failureCaseOfParsingOpenTrustLineCommandWithNonNumericTrustValue();
//        //failureCaseOfParsingOpenTrustLineCommandWithZeroTrustValue();
//    };
//
//
//    void successCaseOfParsingOpenTrustLineCommand(){
//        CommandsParser parser;
//
//        const char *command = "550e8400-e29b-41d4-a716-446655440000\rCREATE:contractors/trust-lines\r550e8400-e29b-41d4-a716-446655440000\r150\n";
//        auto response = parser.processReceivedCommands(command, strlen(command));
//        OpenTrustLineCommand *openTrustLineCommand = dynamic_cast<OpenTrustLineCommand *>(response.second.get());
//
//        assert(response.first);
//        assert(openTrustLineCommand->commandUUID() == boost::lexical_cast<uuids::uuid>("550e8400-e29b-41d4-a716-446655440000"));
//        assert(openTrustLineCommand->id() == string("CREATE:contractors/trust-lines"));
//        assert(openTrustLineCommand->contractorUUID().stringUUID() == string("550e8400-e29b-41d4-a716-446655440000"));
//        assert(openTrustLineCommand->amount() == 150);
//    }
//
//    void failureCaseOfParsingOpenTrustLineCommandWithEmptyCommandUUID(){
//        CommandsParser parser;
//
//        const char *command = "CREATE:contractors/trust-lines\r550e8400-e29b-41d4-a716-446655440000\r150\n";
//        auto response = parser.processReceivedCommands(command, strlen(command));
//
//        assert(!response.first);
//        assert(response.second.get() == nullptr);
//    }
//
//    void failureCaseOfParsingOpenTrustLineCommandWithInvalidCommandUUID(){
//        CommandsParser parser;
//
//        const char *command = "550e8400-446655440000\rCREATE:contractors/trust-lines\r550e8400-e29b-41d4-a716-446655440000\r150\n";
//        auto response = parser.processReceivedCommands(command, strlen(command));
//
//        assert(!response.first);
//        assert(response.second.get() == nullptr);
//    }
//
//    void failureCaseOfParsingOpenTrustLineCommandWithEmptyCommandIdentifier(){
//        CommandsParser parser;
//
//        const char *command = "550e8400-e29b-41d4-a716-446655440000\r550e8400-e29b-41d4-a716-446655440000\r150\n";
//        auto response = parser.processReceivedCommands(command, strlen(command));
//
//        assert(!response.first);
//        assert(response.second.get() == nullptr);
//    }
//
//    void failureCaseOfParsingOpenTrustLineCommandWithInvalidCommandIdentifier(){
//        CommandsParser parser;
//
//        const char *command = "550e8400-e29b-41d4-a716-446655440000\rtrustlines/open\r550e8400-e29b-41d4-a716-446655440000\r150\n";
//        auto response = parser.processReceivedCommands(command, strlen(command));
//
//        assert(!response.first);
//        assert(response.second.get() == nullptr);
//    }
//
//    void failureCaseOfParsingOpenTrustLineCommandWithEmptyContractorUUID(){
//        CommandsParser *parser = new CommandsParser();
//
//        const char *command = "550e8400-e29b-41d4-a716-446655440000\rCREATE:contractors/trust-lines\r150\n";
//        auto response = parser->processReceivedCommands(command, strlen(command));
//
//        assert(!response.first);
//        assert(response.second.get() == nullptr);
//
//        delete parser;
//    }
//
//    void failureCaseOfParsingOpenTrustLineCommandWithInvalidContractorUUID(){
//        CommandsParser parser;
//
//        const char *command = "550e8400-e29b-41d4-a716-446655440000\rCREATE:contractors/trust-lines\r550e8400-e29b\r150\n";
//        auto response = parser.processReceivedCommands(command, strlen(command));
//
//        assert(!response.first);
//        assert(response.second.get() == nullptr);
//    }
//
//    void failureCaseOfParsingOpenTrustLineCommandWithEmptyTrustValue(){
//        CommandsParser parser;
//
//        const char *command = "550e8400-e29b-41d4-a716-446655440000\rCREATE:contractors/trust-lines\r550e8400-e29b-41d4-a716-446655440000\n";
//        auto response = parser.processReceivedCommands(command, strlen(command));
//
//        assert(!response.first);
//        assert(response.second.get() == nullptr);
//    }
//
//    void failureCaseOfParsingOpenTrustLineCommandWithFloatTrustValue(){
//        CommandsParser parser;
//
//        const char *command = "550e8400-e29b-41d4-a716-446655440000\rCREATE:contractors/trust-lines\r550e8400-e29b-41d4-a716-446655440000\r150.12\n";
//        auto response = parser.processReceivedCommands(command, strlen(command));
//
//        assert(!response.first);
//        assert(response.second.get() == nullptr);
//    }
//
//    void failureCaseOfParsingOpenTrustLineCommandWithNonNumericTrustValue(){
//        CommandsParser parser;
//
//        const char *command = "550e8400-e29b-41d4-a716-446655440000\rCREATE:contractors/trust-lines\r550e8400-e29b-41d4-a716-446655440000\r12pzdc\n";
//        auto response = parser.processReceivedCommands(command, strlen(command));
//
//        assert(!response.first);
//        assert(response.second.get() == nullptr);
//    }
//
//    void failureCaseOfParsingOpenTrustLineCommandWithZeroTrustValue(){
//        CommandsParser parser;
//
//        const char *command = "550e8400-e29b-41d4-a716-446655440000\rCREATE:contractors/trust-lines\r550e8400-e29b-41d4-a716-446655440000\r0\n";
//        auto response = parser.processReceivedCommands(command, strlen(command));
//
//        assert(!response.first);
//        assert(response.second.get() == nullptr);
//    }
//
//    void checkParsingCloseTrustLineCommand(){
//        CommandsParser parser;
//
//        const char *command = "550e8400-e29b-41d4-a716-446655440000\rREMOVE:contractors/trust-lines\r550e8400-e29b-41d4-a716-446655440000\ntrustlines/open";
//        auto response = parser.processReceivedCommands(command, strlen(command));
//        BaseUserCommand *c = response.second.get();
//        CloseTrustLineCommand *closeTrustLineCommand = dynamic_cast<CloseTrustLineCommand *>(c);
//
//        assert(response.first);
//        assert(closeTrustLineCommand->commandUUID() == boost::lexical_cast<uuids::uuid>("550e8400-e29b-41d4-a716-446655440000"));
//        assert(closeTrustLineCommand->id() == string("REMOVE:contractors/trust-lines"));
//        assert(closeTrustLineCommand->contractorUUID().stringUUID() == string("550e8400-e29b-41d4-a716-446655440000"));
//    }
//
//    void checkParsingUpdateOutgoingTrustLineCommand(){
//        CommandsParser parser;
//
//        const char *command = "550e8400-e29b-41d4-a716-446655440000\rSET:contractors/trust-lines\r550e8400-e29b-41d4-a716-446655440000\r11456\ntrustlines/open";
//        auto response = parser.processReceivedCommands(command, strlen(command));
//        BaseUserCommand *c = response.second.get();
//        UpdateOutgoingTrustAmountCommand *updateTrustLineCommand = dynamic_cast<UpdateOutgoingTrustAmountCommand *>(c);
//
//        assert(response.first);
//        assert(updateTrustLineCommand->commandUUID() == boost::lexical_cast<uuids::uuid>("550e8400-e29b-41d4-a716-446655440000"));
//        assert(updateTrustLineCommand->id() == string("SET:contractors/trust-lines"));
//        assert(updateTrustLineCommand->contractorUUID().stringUUID() == string("550e8400-e29b-41d4-a716-446655440000"));
//        assert(updateTrustLineCommand->amount() == 11456);
//    }
//
//    void checkParsingUseCreditCommand(){
//        CommandsParser parser;
//
//        const char *command = "550e8400-e29b-41d4-a716-446655440000\rCREATE:contractors/transations\r550e8400-e29b-41d4-a716-446655440000\r11456\rprosto tak\ntrustlines/open";
//        auto response = parser.processReceivedCommands(command, strlen(command));
//        BaseUserCommand *c = response.second.get();
//        UseCreditCommand *useCeditCommand = dynamic_cast<UseCreditCommand *>(c);
//
//        assert(response.first);
//        assert(useCeditCommand->commandUUID() == boost::lexical_cast<uuids::uuid>("550e8400-e29b-41d4-a716-446655440000"));
//        assert(useCeditCommand->id() == string("CREATE:contractors/transations"));
//        assert(useCeditCommand->contractorUUID().stringUUID() == string("550e8400-e29b-41d4-a716-446655440000"));
//        assert(useCeditCommand->amount() == 11456);
//        assert(useCeditCommand->purpose() == string("prosto tak"));
//    }
//
//    pair<bool, shared_ptr<Result>> checkAcceptCommand(shared_ptr<BaseUserCommand> command){
//    }
//
//};