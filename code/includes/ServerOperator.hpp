#ifndef SERVEROPERATOR_HPP
#define SERVEROPERATOR_HPP
#include <list>

#include "Kqueue.hpp"
#include "Server.hpp"

typedef std::map<int, Server *> ServerMap; // socket : key, server class pointer : value

class ServerOperator
{
private:
  ServerMap &_serverMap;
  LocationMap &_locationMap;
  std::map<int, std::string> _clients; // key : client socket, value : read된 client Contents (buf)

  void addClientContents(int clientSock, std::string buffer);
  void setClientContentsClear(int clientSock);
  void disconnectClient(int clientSock);
  const std::string getClientContents(int clientSock);
  bool isExistClient(int clientSock);
  RootBlock *ServerOperator::getLocationBlock(Request &req, ServerBlock *sb);

  void handleEventError(struct kevent *event);
  void handleReadEvent(struct kevent *event, Kqueue kq);
  void handleWriteEvent(struct kevent *event);

public:
  ServerOperator(ServerMap &serverMap, LocationMap &locationMap);
  ~ServerOperator();

  void run();
};

#endif