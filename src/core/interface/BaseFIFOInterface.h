#ifndef GEO_NETWORK_CLIENT_BASEINTERFACE_H
#define GEO_NETWORK_CLIENT_BASEINTERFACE_H

#include <boost/filesystem.hpp>

#include <string>


using namespace std;
namespace fs = boost::filesystem;

class BaseFIFOInterface {
protected:
    virtual const char* dir() const {
        return "fifo/";
    }

    virtual const char* name() const {
        return "commands.fifo";
    }

    virtual const char* resultsFIleName() const {
        return "results.fifo";
    }

    const string FIFOPath() const {
        return (string(dir()) + string(name())).c_str();
    }

    const string FIFOResultsPath() const {
        return (string(dir()) + string(resultsFIleName())).c_str();
    }

    const bool FIFOExists() const {
        return fs::exists(fs::path(FIFOPath()));
    }

    const bool FIFOResultsExists() const {
        return fs::exists(fs::path(FIFOResultsPath()));
    }

protected:
    int mFIFODescriptor;

    int mFIFOResultsDescriptor;
};

#endif //GEO_NETWORK_CLIENT_BASEINTERFACE_H
