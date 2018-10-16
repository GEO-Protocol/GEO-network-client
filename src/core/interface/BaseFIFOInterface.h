#ifndef GEO_NETWORK_CLIENT_BASEINTERFACE_H
#define GEO_NETWORK_CLIENT_BASEINTERFACE_H


#include "../common/exceptions/IOError.h"
#include "../common/exceptions/ConflictError.h"

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include <string>
#include <iostream>

/*
 * For mkfifo() support
 */
#include <sys/types.h>
#include <sys/stat.h>


using namespace std;
namespace as = boost::asio;
namespace fs = boost::filesystem;

/**
 * Commands and results are transferred through the FIFO files.
 * http://man7.org/linux/man-pages/man7/fifo.7.html
 *
 * BaseFIFOInterface implements base class for commands and results interfaces.
 * It provides logic for creating and opening of the FIFO.
 *
 * todo: FIFOs are not cross-platform (Linux Only).
 *       Substitutes must be developed for the rest of supported platforms.
 *
 * todo: tests needed.
 */
class BaseFIFOInterface {
public:
    static const constexpr char *kFIFODir = "fifo/";

protected:
    BaseFIFOInterface():
        mFIFODescriptor(0){};

    /**
     * Must be reimplemented by the descendant classes.
     * @returns the name of the FIFO file.
     */
    virtual const char* FIFOName() const = 0;

    /**
     * @returns relative path to the FIFO file including the name of the FIFO file.
     */
    const string FIFOFilePath() const{
        return string(kFIFODir) + string(FIFOName());
    }

    /**
     * @returns "true" if FIFO file with specified name and path is present.
     * Otherwise - returns "false".
     */
    const bool isFIFOExists() const {
        return fs::exists(fs::path(FIFOFilePath()));
    }

    /**
     * Attempts to create new FIFO file with configured name and path.
     * @param permissionsMask - specifies the permissions for the FIFO in the UNIX format (default is 0755).
     *
     * @throws IOError in case if FIFO can't be created.
     * @throws ConflictError in case if FIFO is present already.
     */
    void createFIFO(unsigned int permissionsMask=0755) {
        if (! isFIFOExists()) {
            fs::create_directories(kFIFODir);

            if (mkfifo(FIFOFilePath().c_str(), permissionsMask) != 0) {
                throw IOError(
                    "BaseFIFOInterface::createFIFO: "
                        "Can't create FIFO file.");
            }

        } else {
            throw ConflictError(
                "BaseFIFOInterface::createFIFO: "
                "FIFO is present already.");
        }
    }

protected:
    int mFIFODescriptor;
};

#endif //GEO_NETWORK_CLIENT_BASEINTERFACE_H
