#ifndef GEO_NETWORK_CLIENT_COMMON_H
#define GEO_NETWORK_CLIENT_COMMON_H


#include "../../../common/exceptions/IOError.h"

#include <boost/filesystem.hpp>

#include <string>
#include <cstdio>
#include <sys/stat.h>


namespace db {


using namespace std;
namespace fs = boost::filesystem;


class AbstractFileDescriptorHandler {
public:
    static const constexpr char *kReadAccessMode  = "r+";
    static const constexpr char *kWriteAccessMode = "w+";

public:
    AbstractFileDescriptorHandler(
        const char *filename,
        const char *path);
    ~AbstractFileDescriptorHandler();


    const string& filename() const;
    const string& path() const;

    virtual void open(
        const char *accessMode);
    void close();
    const bool exists() const;
    __off_t fileSize() const;

protected:
    FILE* mFileDescriptor;

    const string mFilename;
    const string mPath;
};


} // namespace db


#endif //GEO_NETWORK_CLIENT_COMMON_H
