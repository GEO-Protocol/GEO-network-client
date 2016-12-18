#ifndef GEO_NETWORK_CLIENT_RESULTSINTERFACE_H
#define GEO_NETWORK_CLIENT_RESULTSINTERFACE_H

#include "../BaseFIFOInterface.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/MemoryError.h"

#include <boost/filesystem.hpp>

#include <string>



using namespace std;


class ResultsInterface:
    public BaseFIFOInterface {

public:
    static const constexpr char *kFIFOName = "results.fifo";
    static const constexpr unsigned int kPermissionsMask = 0755;

public:
    explicit ResultsInterface(
        as::io_service &ioService);

    ~ResultsInterface();

    void writeResult(
        const char *bytes,
        const size_t bytesCount);

private:
    as::io_service &mIOService;
    as::posix::stream_descriptor *mFIFOStreamDescriptor;
    vector<char> mCommandBuffer;

private:
    virtual const char* name() const;
};

#endif //GEO_NETWORK_CLIENT_RESULTSINTERFACE_H
