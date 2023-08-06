#ifndef POST_HPP
#define POST_HPP

#include "Method.hpp"

class Post : public Method {
 private:
  std::string _result;
  std::string _path;

 public:
  Post();
  ~Post();

  void process(Request &request, Response &response);

  std::string makeStatusLine(Request &request, Response &response);
  std::string makeHeader(Request &request, Response &response);
  std::string makeBody(Request &request, Response &response);
};

#endif
