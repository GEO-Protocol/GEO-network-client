#include "Core.h"

Core::Core() {
    zeroPointers();
}

Core::~Core() {
    cleanupMemory();
}

int Core::run() {
    auto initCode = initCoreComponents();
    if (initCode != 0) {
        mLog.logFatal("Core", "Core components can't be initialised. Process will now be closed.");
        return initCode;
    }

    try {
        // todo: register in uuid2address service;
        mCommunicator->beginAcceptMessages();
        mCommandsInterface->beginAcceptCommands();

        mLog.logSuccess("Core", "Processing started.");
        mIOService.run();
        return 0;

    } catch (const std::exception &e) {
        mLog.logException("Core", e);
        return -1;
    }
}

int Core::initCoreComponents() {
    int initCode;

    initCode = initSettings();
    if (initCode != 0)
        return initCode;

    json conf;
    try {
        // Optimised conf read.
        // (several params at once)
        // For the details - see settings realisation.
        conf = mSettings->loadParsedJSON();

    } catch (std::exception &e) {
        mLog.logException("Settings", e);
        return -1;
    }

    try {
        mNodeUUID = mSettings->nodeUUID(&conf);
    } catch (RuntimeError &) {
        mLog.logFatal("Core", "Can't read uuid of the node from the settings.");
        return -1;
    }

    initCode = initCommunicator(conf);
    if (initCode != 0)
        return initCode;

    initCode = initResultsInterface();
    if (initCode != 0)
        return initCode;

    initCode = initTransactionsManager();
    if (initCode != 0)
        return initCode;

    initCode = initCommandsInterface();
    if (initCode != 0)
        return initCode;

    return 0;
}

int Core::initSettings() {
    try {
        mSettings = new Settings();
        mLog.logSuccess("Core", "Settings are successfully initialised");
        return 0;

    } catch (const std::exception &e) {
        mLog.logException("Core", e);
        return -1;
    }
}

int Core::initCommunicator(const json &conf) {
    try {
        mCommunicator = new Communicator(
                mIOService, mNodeUUID,
                mSettings->interface(&conf), mSettings->port(&conf),
                mSettings->uuid2addressHost(&conf), mSettings->uuid2addressPort(&conf));
        mLog.logSuccess("Core", "Network communicator is successfully initialised");

    } catch (const std::exception &e) {
        mLog.logError("Core", "Can't initialize network communicator.");
        mLog.logException("Core", e);
        return -1;
    }

    return 0;
}

int Core::initResultsInterface() {
    try {
        mResultsInterface = new ResultsInterface(&mLog);
        mLog.logSuccess("Core", "Results interface is successfully initialised");
        return 0;

    } catch (const std::exception &e) {
        mLog.logException("Core", e);
        return -1;
    }
}

int Core::initTransactionsManager() {
    try {
        mTransactionsManager = new TransactionsManager(mIOService, mResultsInterface, &mLog);
        mLog.logSuccess("Core", "Transactions handler is successfully initialised");
        return 0;

    } catch (const std::exception &e) {
        mLog.logException("Core", e);
        return -1;
    }
}

int Core::initCommandsInterface() {
    try {
        mCommandsInterface = new CommandsInterface(mIOService, mTransactionsManager, &mLog);
        mLog.logSuccess("Core", "Commands interface is successfully initialised");
        return 0;

    } catch (const std::exception &e) {
        mLog.logException("Core", e);
        return -1;
    }
}

void Core::cleanupMemory() {
    if (mSettings != nullptr) {
        delete mSettings;
    }

    if (mCommunicator != nullptr) {
        delete mCommunicator;
    }

    if (mCommandsInterface != nullptr) {
        delete mCommandsInterface;
    }

    if (mResultsInterface != nullptr) {
        delete mResultsInterface;
    }

    if (mTransactionsManager != nullptr) {
        delete mTransactionsManager;
    }
}

void Core::zeroPointers() {
    mSettings = nullptr;
    mCommunicator = nullptr;
    mCommandsInterface = nullptr;
}
