#ifndef GEO_NETWORK_CLIENT_RESULTSINTERFACE_H
#define GEO_NETWORK_CLIENT_RESULTSINTERFACE_H


#include "../../BaseFIFOInterface.h"
#include "../../../common/exceptions/IOError.h"
#include "../../../common/exceptions/MemoryError.h"
#include "../../../logger/Logger.h"

#include <boost/bind.hpp>

#include <string>
#include <iostream>

#ifdef DEBUG
#define DEBUG_RESULTS_INTERFACE
#endif


using namespace std;


class ResultsInterface:
    public BaseFIFOInterface {

public:
    static const constexpr char *kFIFOName = "results.fifo";
    static const constexpr unsigned int kPermissionsMask = 0755;

public:
    explicit ResultsInterface(Logger *logger);
    ~ResultsInterface();

    void writeResult(
        const char *bytes,
        const size_t bytesCount);

private:
    // remote
    Logger *mLog;

private:
    virtual const char* FIFOName() const;
};

#endif //GEO_NETWORK_CLIENT_RESULTSINTERFACE_H
