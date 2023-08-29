#include "../includes/ServerOperator.hpp"

ServerOperator::ServerOperator(ServerMap &serverMap, LocationMap &locationMap)
    : _serverMap(serverMap), _locationMap(locationMap) {}

ServerOperator::~ServerOperator() {}

void ServerOperator::run()
{
  /* init kqueue & add event for server socket*/
  Kqueue kq;
  if (kq.init(_serverMap) == EXIT_FAILURE)
    exit(EXIT_FAILURE);

  struct kevent *currEvent;
  int eventNb;
  while (1)
  {
    eventNb = kq.countEvents();

    kq.clearCheckList(); // clear change_list for new changes

    for (int i = 0; i < eventNb; ++i)
    {
      currEvent = &(kq.getEventList())[i];
      if (currEvent->flags & EV_ERROR)
      {
        handleEventError(currEvent);
      }
      else if (currEvent->filter == EVFILT_READ)
      {
        handleReadEvent(currEvent, kq);
      }
      else if (currEvent->filter == EVFILT_WRITE)
      {
        handleWriteEvent(currEvent, kq);
      }
      else if (currEvent->filter == EVFILT_TIMER)
      {
        handleRequestTimeOut(currEvent->ident, kq);
      }
    }
  }
}

void ServerOperator::handleEventError(struct kevent *event)
{
  if (_serverMap.find(event->ident) != _serverMap.end())
  {
    std::cerr << "server socket error" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::cerr << "client socket error" << std::endl;
  disconnectClient(event->ident);
}

void ServerOperator::handleRequestTimeOut(int clientSock, Kqueue &kq)
{
  kq.changeEvents(clientSock, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
  Response res;
  res.setErrorRes(408);
  res.sendResponse(clientSock);
  disconnectClient(clientSock);
}

void ServerOperator::handleReadEvent(struct kevent *event, Kqueue &kq)
{
  if (_serverMap.find(event->ident) != _serverMap.end())
  {
    int clientSocket;

    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    if ((clientSocket = accept(event->ident, (struct sockaddr *)&clientAddr,
                               &clientAddrLen)) == -1) {
      std::cerr << "accept() error\n";
      exit(EXIT_FAILURE);
    }
    std::cout << "accept new client: " << clientSocket << std::endl;
    char *clientIp = inet_ntoa(clientAddr.sin_addr);
    _clientToServer[clientSocket] = event->ident;
    fcntl(clientSocket, F_SETFL, O_NONBLOCK);

    kq.changeEvents(
        clientSocket, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0,
        _serverMap[event->ident]->getSPSBList()->front()->getKeepAliveTime() *
            1000,
        NULL);
    kq.changeEvents(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    _clients[clientSocket] = new Request();
    _clients[clientSocket]->addHeader("ClientIP", clientIp);
  }
  else if (isExistClient(event->ident))
  {
    Request *req = _clients[event->ident];
    static char buf[8092];
    int n;

    n = read(event->ident, buf, sizeof(buf) - 1);
    if (n == 0) {
      disconnectClient(event->ident);
      return;
    }
    else if (n == -1) {
      return;
    }
    else {

      req->addRawContents(buf, n);

      if (n < (int)sizeof(buf) - 1 ||
          recv(event->ident, buf, sizeof(int), MSG_PEEK) == -1)
      {
        req->parsing(_serverMap[_clientToServer[event->ident]]->getSPSBList(),
                    _locationMap);
      }

      if (req->isFullReq())
      {
        kq.changeEvents(event->ident, EVFILT_TIMER, EV_ENABLE, 0,
                        _serverMap[_clientToServer[event->ident]]
                                ->getSPSBList()
                                ->front()
                                ->getKeepAliveTime() *
                            1000,
                        NULL);
        kq.changeEvents(event->ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
        kq.changeEvents(event->ident, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0,
                        NULL);
        return;
      }
    }
  }
}

void ServerOperator::handleWriteEvent(struct kevent *event, Kqueue &kq)
{
  Response *res = new Response();
  Request *req = _clients[event->ident];
  ServerBlock *locBlock = req->getLocBlock();


  if (req->getStatus() != 200)
  {
    res->setErrorRes(req->getStatus());
  }
  else
  {
    Method *method;
    const std::string &limit = locBlock->getLimitExcept();

    if ((req->getMethod() == "GET" || req->getMethod() == "HEAD") && (limit == "GET" || limit == ""))
      method = new Get();
    else if ((req->getMethod() == "POST" || req->getMethod() == "PUT") &&
             (limit == "POST" || limit == ""))
    {
      method = new Post();
    }
    else if (req->getMethod() == "DELETE" &&
             (limit == "DELETE" ||
              limit == ""))
      method = new Delete();
    else
    {
      method = new Method();
    }
    method->process(*req, *res);
    delete method;
  }

  if (res->sendResponse(event->ident) == EXIT_FAILURE)
  {
    std::cerr << "client write error!" << std::endl;
    disconnectClient(event->ident);
  }
  else if (req->getStatus() == 413)
  {
    disconnectClient(event->ident);
  }
  else
  {
    kq.changeEvents(event->ident, EVFILT_TIMER, EV_ENABLE, 0,
                    req->getLocBlock()->getKeepAliveTime() * 1000, NULL);
    req->clear();
    kq.changeEvents(event->ident, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    kq.changeEvents(event->ident, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
  }
  delete res;
}

bool ServerOperator::isExistClient(int clientSock)
{
  if (_clients.find(clientSock) == _clients.end())
    return false;
  return true;
}

void ServerOperator::disconnectClient(int clientSock)
{
  std::cout << "client disconnected: " << clientSock << std::endl;
  close(clientSock);
  delete _clients[clientSock];
  _clients.erase(clientSock);
  _clientToServer.erase(clientSock);
}
