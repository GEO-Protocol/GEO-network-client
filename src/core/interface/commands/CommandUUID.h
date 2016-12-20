#ifndef GEO_NETWORK_CLIENT_COMMANDUUID_H
#define GEO_NETWORK_CLIENT_COMMANDUUID_H

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include <string>


using boost::uuids::uuid;
using namespace std;


class CommandUUID:
    public uuid {

public:
    static const size_t kLength = 16;

public:
    CommandUUID():
        uuid(boost::uuids::random_generator()()){};

    explicit CommandUUID(uuid const &u):
        boost::uuids::uuid(u){}

    explicit CommandUUID(CommandUUID const &u){
        boost::uuids::uuid(
            boost::lexical_cast<boost::uuids::uuid>(u.stringUUID()));
    }

    operator boost::uuids::uuid(){
        return static_cast<boost::uuids::uuid&>(*this);
    }

    operator boost::uuids::uuid() const {
        return static_cast<boost::uuids::uuid const&>(*this);
    }

    CommandUUID& operator=(const boost::uuids::uuid &u){
        memcpy(data, u.data, kLength);
        return *this;
    }

    const string stringUUID() const {
        uuid u;
        memcpy(&u.data, data, 16);
        return boost::lexical_cast<string>(u);
    }
};

#endif //GEO_NETWORK_CLIENT_COMMANDUUID_H
