#ifndef GEO_NETWORK_CLIENT_TRUSTLINEDIRECTIONCOLUMN_H
#define GEO_NETWORK_CLIENT_TRUSTLINEDIRECTIONCOLUMN_H


#include "../common/AbstractFileDescriptorHandler.h"
#include "../common/AbstractRecordsHandler.h"
#include "../../../common/Types.h"
#include "../../../common/exceptions/ValueError.h"
#include "../../../common/exceptions/NotFoundError.h"

#include <boost/container/flat_set.hpp>


namespace db {
namespace fields {


using namespace boost::container;


/*
 * Simple RecordNumber -> TrustLineDirection map,
 * built on top of sparse files.
 *
 * Implementation is very simple - seek to position and write.
 * This approach gives very efficient disk usage,
 * despite reported file size.
 *
 * > ls -alsh
 *
 * total 8,0K
 *    0 drwxr-xr-x 1 hsc hsc   16 гру 24 16:32 .
 *    0 drwxr-xr-x 1 hsc hsc   84 гру 24 16:32 ..
 * 8,0K -rw-r--r-- 1 hsc hsc 977K гру 24 16:32 data.bin
 *
 * Total file usage - 8K, when reported size is 977K.
 *
 *
 * todo: implement vacuum()
 */
class TrustLineDirectionColumn:
    protected AbstractFileDescriptorHandler,
    protected AbstractRecordsHandler {

public:
    typedef uint32_t IndexRecordOffset; // Offset of the record into the index.

public:
    TrustLineDirectionColumn(
        const fs::path &path);

    void set(
        const RecordNumber recN,
        const TrustLineDirection direction,
        const bool sync=false);

    void remove(
        const RecordNumber recN,
        const bool sync=false);

    const TrustLineDirection direction(
        const RecordNumber recN) const;

    const flat_set<RecordNumber>& incomingDirectedRecordsNumbers() const;
    const flat_set<RecordNumber>& outgoingDirectedRecordsNumbers() const;
    const flat_set<RecordNumber>& bothDirectedRecordsNumbers() const;

    void commit();

protected:
    struct FileHeader {
        explicit FileHeader();

        // Version is useful in case when data migration is needed.
        uint16_t version;
    };

    flat_set<RecordNumber> mIncomingRecordNumbers;
    flat_set<RecordNumber> mOutoingRecordNumbers;
    flat_set<RecordNumber> mBothDirectedRecordNumbers;

protected:
    void open();
    FileHeader loadFileHeader() const;
    void updateFileHeader(
        const FileHeader *header) const;

    void loadIndexIntoMemory();

    inline const IndexRecordOffset recordOffset(
        const RecordNumber recN) const;
};


} // namespace fields
} // db

#endif //GEO_NETWORK_CLIENT_TRUSTLINEDIRECTIONCOLUMN_H
