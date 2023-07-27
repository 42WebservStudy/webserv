#include "../includes/LocationBlock.hpp"

LocationBlock::LocationBlock()
    : _path(), _index(), _root(), _autoindex(), _limitExcept(),
      _clientMaxBodySize(0) {}

LocationBlock::~LocationBlock() {}

void LocationBlock::setPath(std::string value) { _path = value; }

void LocationBlock::setIndex(std::string value) { _index = value; }

void LocationBlock::setRoot(std::string value) { _root = value; }

void LocationBlock::setAutoindex(std::string value) { _autoindex = value; }

void LocationBlock::setLimitExcept(std::string value) { _limitExcept = value; }

void LocationBlock::setClientMaxBodySize(std::string value) {
    _clientMaxBodySize = stoul(value); // TODO stoul() 동작 함수 만들기
}

const std::string LocationBlock::getPath() const { return _path; }

const std::string LocationBlock::getIndex() const { return _index; }

const std::string LocationBlock::getRoot() const { return _root; }

const std::string LocationBlock::getAutoindex() const { return _autoindex; }

const std::string LocationBlock::getLimitExcept() const { return _limitExcept; }

unsigned long LocationBlock::getClientMaxBodySize() const {
    return _clientMaxBodySize;
}

void LocationBlock::setKeyVal(std::string key, std::string value) {
    typedef void (LocationBlock::*funcptr)(std::string);
    typedef std::map<std::string, funcptr> funcMap;
    typedef funcMap::iterator funcIter;
    funcMap map;
    funcIter iter;

    map["path"] = &LocationBlock::setPath;
    map["index"] = &LocationBlock::setIndex;
    map["root"] = &LocationBlock::setRoot;
    map["autoindex"] = &LocationBlock::setAutoindex;
    map["limit_except"] = &LocationBlock::setLimitExcept;
    map["client_max_body_size"] = &LocationBlock::setClientMaxBodySize;
    //  std::cout << "key: <" << key << "> value: <" << value << ">" <<
    //  std::endl;
    iter = map.find(key);
    if (iter != map.end())
        (this->*(iter->second))(value);
}

// TODO test
void LocationBlock::test() {
    std::cout << "===========LOCATION===========" << std::endl;
    std::cout << "_path: " << _path << std::endl;
    std::cout << "_index: " << _index << std::endl;
    std::cout << "_root: " << _root << std::endl;
    std::cout << "_autoindex: " << _autoindex << std::endl;
    std::cout << "_limitExcept: " << _limitExcept << std::endl;
    std::cout << "_clientMaxBodySize: " << _clientMaxBodySize << std::endl;
    std::cout << std::endl;
}