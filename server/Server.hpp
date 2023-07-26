#ifndef SERVER_HPP
#define SERVER_HPP

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <vector>

#include "Cgi.hpp"
#include "Kqueue.hpp"
#include "Request.hpp"
#include "Response.hpp"

class Server {
 private:
  int _socket;
  struct sockaddr_in _serverAddr;

 public:
  Server();
  ~Server();

  int init();
  int run();
};

#endif