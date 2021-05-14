#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>

namespace conn {
class Connection
{
public:
    Connection();
    virtual ~Connection();
    bool isConnected() {return _connected;}

private:
    std::string _server_addr;
    bool _connected;
};
}

#endif // CONNECTION_H
