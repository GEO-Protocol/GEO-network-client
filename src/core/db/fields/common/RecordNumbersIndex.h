#ifndef GEO_NETWORK_CLIENT_RECORDNUMBERSINDEX_H
#define GEO_NETWORK_CLIENT_RECORDNUMBERSINDEX_H


#include "AbstractFileDescriptorHandler.h"
#include "AbstractRecordsHandler.h"
#include "../../../common/Types.h"
#include "../../../common/exceptions/ValueError.h"
#include "../../../common/exceptions/NotFoundError.h"
#include "../../../common/exceptions/RuntimeError.h"


namespace db {
namespace fields {


/*
 * Simple RecordNumber -> DataOffset map,
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
 * 8,0K -rw-r--r-- 1 hsc hsc 977K гру 24 16:32 data.index
 *
 * Total file usage - 8K, when reported size is 977K.
 *
 *
 * todo: implement vacuum()
 */
class RecordNumbersIndex:
    protected AbstractFileDescriptorHandler,
    protected AbstractRecordsHandler {

public:
    typedef uint32_t IndexRecordOffset; // Offset of the record into the index.
    typedef uint32_t DataOffset;        // Offset of the data block in column storage.

public:
    RecordNumbersIndex(
        const fs::path &path);

    void set(
        const RecordNumber recN,
        const DataOffset storageOffset,
        const bool sync=false);

    void remove(
        const RecordNumber recN,
        const bool sync=false);

    const DataOffset dataOffset(
        const RecordNumber recN) const;

protected:
    struct FileHeader {
        explicit FileHeader();

        // Version is useful in case when data migration is needed.
        uint16_t version;
    };

    struct IndexRecord {
        IndexRecord(const DataOffset offset);

        DataOffset offset;

        // Records are physically removing from the file
        // only when vacuum() is called.
        // Instead of removing, records are only marking as removed.
        //
        // To mark record as removed - special record value is used:
        // max. possible offset.
        static const constexpr DataOffset kRemovedRecordValue =
            std::numeric_limits<DataOffset>::max();
    };

protected:
    void open();

    FileHeader loadFileHeader() const;
    void updateFileHeader(
        const FileHeader *header) const;

    inline const IndexRecordOffset recordOffset(
        const RecordNumber recN) const;
};

} // namespace fields
} // namespace db

#endif //GEO_NETWORK_CLIENT_RECORDNUMBERSINDEX_H
