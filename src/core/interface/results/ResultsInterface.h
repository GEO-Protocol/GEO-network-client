#ifndef GEO_NETWORK_CLIENT_RESULTSINTERFACE_H
#define GEO_NETWORK_CLIENT_RESULTSINTERFACE_H

#include <vector>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include <sys/stat.h>
#include <sys/types.h>
#include "../BaseFIFOInterface.h"
#include "../../common/exceptions/IOError.h"

namespace as = boost::asio;
namespace fs = boost::filesystem;

class ResultsInterface : public BaseFIFOInterface {
private:
    as::io_service &mIOService;
    as::posix::stream_descriptor *mFIFOStreamDescriptor;
    vector<char> mResultBuffer;

private:
    void createFifo();

    void writeNextCommand();

    void writerHandler(const boost::system::error_code &error, const size_t bytesTransferred);

public:
    explicit ResultsInterface(as::io_service &ioService);

    ~ResultsInterface();
};

#endif //GEO_NETWORK_CLIENT_RESULTSINTERFACE_H
