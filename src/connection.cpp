
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <QDebug>

#include <connection.h>

using namespace conn;

Connection::Connection()
    : _server_addr(""),
      _connected(false)
{

}

Connection::~Connection()
{
        _connected = false;
}
