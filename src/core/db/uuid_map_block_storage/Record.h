#ifndef GEO_NETWORK_CLIENT_BLOCK_H
#define GEO_NETWORK_CLIENT_BLOCK_H

#include "../../common/Types.h"

#include <string>
#include <malloc.h>
#include <memory>

namespace db {
    namespace uuid_map_block_storage {

        using namespace std;

        class Record {
        public:
            typedef shared_ptr<Record> Shared;

            explicit Record(
                byte *data,
                const size_t bytesCount);

            ~Record();

            const byte *data() const;

            const size_t bytesCount() const;

        private:
            byte *mData;
            size_t mBytesCount;
        };

    }
}

#endif //GEO_NETWORK_CLIENT_BLOCK_H
