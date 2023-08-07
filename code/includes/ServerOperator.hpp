#ifndef SERVEROPERATOR_HPP
#define SERVEROPERATOR_HPP
#include <list>
#include <arpa/inet.h>

#include "Delete.hpp"
#include "Get.hpp"
#include "IMethod.hpp"
#include "Kqueue.hpp"
#include "Post.hpp"
#include "Request.hpp"
#include "Server.hpp"

class ServerOperator {
 private:
  ServerMap &_serverMap;
  LocationMap &_locationMap;
  std::map<int, Request> _clients;
  // key: client socket, value: server socket
  std::map<int, int> _clientToServer;

  void disconnectClient(int clientSock);
  bool isExistClient(int clientSock);
  ServerBlock *getLocationBlock(Request &req, ServerBlock *sb);

  void handleEventError(struct kevent *event);
  void handleReadEvent(struct kevent *event, Kqueue kq);
  void handleWriteEvent(struct kevent *event, Kqueue kq);

 public:
  ServerOperator(ServerMap &serverMap, LocationMap &locationMap);
  ~ServerOperator();

  void run();
};

#endif
