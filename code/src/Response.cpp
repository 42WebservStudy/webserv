#include "../includes/Response.hpp"

#include "../includes/Utils.hpp"

Response::Response() {
  _statusCodes[200] = " OK";
  _statusCodes[201] = " Created";
  _statusCodes[202] = " Accepted";
  _statusCodes[204] = " No Content";
  _statusCodes[300] = " Multiple Choice";
  _statusCodes[301] = " Moved Permanently";
  _statusCodes[303] = " See Other";
  _statusCodes[304] = " Not Modified";
  _statusCodes[307] = " Temporary Redirect";
  _statusCodes[400] = " Bad Request";
  _statusCodes[401] = " Unauthorized";
  _statusCodes[403] = " Forbidden";
  _statusCodes[404] = " Not Found";
  _statusCodes[405] = " Method Not Allowed";
  _statusCodes[406] = " Not Acceptable";
  _statusCodes[409] = " Conflict";
  _statusCodes[410] = " Gone";
  _statusCodes[412] = " Precondition Failed";
  _statusCodes[414] = " URI Too Long";
  _statusCodes[415] = " Unsupported Media Type";
  _statusCodes[500] = " Server Error";
}

Response::~Response() {}

void Response::convertCGI(const std::string &cgiResult) {
  std::stringstream ss(cgiResult);
  std::string line;

  _headers.clear();
  _body.clear();
  _statusLine.clear();

  while (std::getline(ss, line, '\r') && line != "\n") {
    if (line.find("HTTP/1.1") != std::string::npos) {
      _statusLine += line;
    } else {
      size_t pos = line.find(":");
      if (pos == line.npos) break;
      _headers[line.substr(0, pos)] = line.substr(pos + 2);
    }
  }
  while (std::getline(ss, line)) {
    _body += line;
    if (!ss.eof()) _body += '\n';
  }
  if (_statusLine == "") {
    setStatusLine(200);
  }
  if (_headers.find("Content-Length") == _headers.end()) {
    setHeaders("Content-Length", ftItos(_body.length()));
  }
  setResult();
}

void Response::directoryListing(std::string path) {
  DIR *dir;
  struct dirent *ent;

  if ((dir = opendir(path.c_str())) != NULL) {
    /* print all the files and directories within directory */
    readdir(dir);
    readdir(dir);
    while ((ent = readdir(dir)) != NULL) {
      _body += "<a href=\"";
      _body += ent->d_name;
      if (ent->d_type == DT_DIR)
        _body += "/\">";
      else if (std::string(&ent->d_name[ent->d_namlen - 5]) != ".html")
        _body += "\" download>";
      else
        _body += "\">";
      _body += ent->d_name;
      _body += "</a><br>";
    }
    closedir(dir);
  } else {
    std::cout << "directory error : " << path.c_str() << std::endl;
    /* could not open directory */
    return;
  }
  _headers["Content-Type"] = "text/html";
  _headers["Content-Length"] = ftItos(_body.length());
  setStatusLine(200);
  setResult();
}

int Response::sendResponse(int clientSocket) {
  if (write(clientSocket, _result.c_str(), _result.length()) == -1) return (1);
  return (0);
}

const std::string &Response::getBody() const { return _body; }

void Response::setErrorRes(int statusCode) {
  _statusLine.clear();
  _headers.clear();
  _body.clear();
  _statusLine += "HTTP/1.1 ";
  _statusLine += ftItos(statusCode);
  _statusLine += _statusCodes[statusCode];
  _headers["Content-Type"] = "text/plain";
  _body += _statusCodes[statusCode];
  _body += ": Error";
  _headers["Content-Length"] = ftItos(_body.length());
  setResult();
}

bool Response::isInHeader(const std::string &key) {
  if (_headers.find(key) == _headers.end()) return false;
  return true;
}

void Response::setResult() {
  _result += _statusLine;
  _result += "\r\n";

  std::map<std::string, std::string>::iterator it;
  for (it = _headers.begin(); _headers.end() != it; it++) {
    _result += it->first;
    _result += ": ";
    _result += it->second;
    _result += "\r\n";
  }
  _result += "\r\n";
  _result += _body;
}

void Response::setStatusLine(int code) {
  _statusLine += "HTTP/1.1 ";
  _statusLine += ftItos(code);
  _statusLine += _statusCodes[code];
}
// std::string getFileExtension(const std::string &fileName) {
//   size_t dotPos = fileName.rfind('.');
//   if (dotPos == std::string::npos) return "";
//   return fileName.substr(dotPos + 1);
// }

void Response::setHeaders(const std::string &key, const std::string &value) {
  // std::string contentType = "text/html";  // default

  // if (ext == "css") {
  //   std::cout << "css file" << std::endl;
  //   contentType = "text/css";
  // }
  // if (ext == "js") {
  //   std::cout << "js file" << std::endl;
  //   contentType = "text/javascript";
  // }
  _headers[key] = value;
}

void Response::setBody(const std::string &body) { _body = body; }
