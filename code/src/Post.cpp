#include "../includes/Post.hpp"

Post::Post() {}

Post::~Post() {}
// 수건, 안경, 렌즈통, 기침약, 잠옷, 약,
bool Post::isCgi(const std::string &fullUri, Request &request) {
  if (request.getHeaderByKey("Cgi") == "")
    return false;
  else if (fullUri.find(request.getHeaderByKey("Cgi")) == fullUri.npos)
    return false;
  else
    return true;
}

std::string Post::generateRandomString() {
  static std::string charset =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  std::string result;
  std::srand(std::time(0));

  for (int i = 0; i < 32; ++i)
    result.push_back(charset[std::rand() % charset.size()]);
  return result;
}

void Post::createResource(Response &response, std::string &fileName,
                          std::string &fullUri) {
  fileName += generateRandomString();
  std::ifstream tempif(fileName);
  while (tempif.is_open() == true) {
    tempif.close();
    fileName = fullUri;
    fileName += generateRandomString();
    std::ifstream tempif(fileName);
    //    tempif = std::ifstream(fileName); // TODO 클러스터 맥에서 대입연산자
    //    에러
  }
  response.setHeaders("Location", fileName);
  response.setStatusLine(201);
}

void Post::appendResource(const std::string &fileName, const Request &request) {
  std::ofstream tempof(fileName, std::ios::app);
  _path = fileName;
  tempof << request.getBody();
  tempof.close();
}

void Post::process(Request &request, Response &response) {
  try {
    std::string fullUri = request.getHeaderByKey("RootDir");
    fullUri += request.getHeaderByKey("CuttedURI");
    std::string fileName = fullUri;

    if (isCgi(fullUri, request) == true) {
      Cgi cgi;
      cgi.reqToEnvp(request.getHeaderMap());
      cgi.excute(request.getBody());
      response.convertCGI(cgi.getRes());
    } else {
      if (fileName.back() == '/') {
        if (request.getMime() != "directory") {
          throw ErrorException(400);
        }
        createResource(response, fileName, fullUri);
      } else {
        if (request.getMime() == "directory") {
          std::string tmp = request.getHeaderByKey("RawURI");
          tmp += "/";
          response.setHeaders("Location", tmp);
          throw ErrorException(301);
        }
        std::ifstream tempif(fileName);
        if (tempif.is_open() == false) throw ErrorException(404);
        tempif.close();
        response.setStatusLine(200);
      }
      appendResource(fileName, request);
      response.setHeaders("Content-Length", "0");
      response.setHeaders("Content-Type", "application/octet-stream");
      response.setResult();
    }
  } catch (ErrorException &e) {
    if (e.getErrorCode() >= 400) {
      response.setErrorRes(e.getErrorCode());
    } else {  // TODO 300번대 에러에 대해서 더 알아보기
      response.setRedirectRes(e.getErrorCode());
    }
  }
}