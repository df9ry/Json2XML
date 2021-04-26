#ifndef JSON2XML_HPP
#define JSON2XML_HPP

#include <jsonx.hpp>

#include <iostream>

class Json2XML
{
public:
    Json2XML();
    ~Json2XML();

    void json2xml(const jsonx::json &j, std::ostream &os);
    void json2xml(const jsonx::json &j, const std::string &xmlFile);

    jsonx::json xml2json(const std::string &xmlFile);
    jsonx::json xml2json(std::istream &is);
};

#endif // JSON2XML_HPP
