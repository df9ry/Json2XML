#include "json2xml.hpp"

#include "SAXPrint.hpp"
#include "ParseHandler.hpp"
#include "attributelistimpl.hpp"

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/sax/AttributeList.hpp>

#include <memory>
#include <fstream>
#include <algorithm>
#include <filesystem>

using namespace std;
using namespace jsonx;
using namespace XERCES_CPP_NAMESPACE;

static AttributeListImpl attrs;

static void new_line(SAXPrintHandlers &ph, int indent)
{
    if (indent < 0)
        return;
    ph.stream() << endl;
    while (indent--)
        ph.stream() << ' ';
}

static void to_string(SAXPrintHandlers &ph, const string &str)
{
    ph.characters(X(str.c_str()), str.length());
}

static void j2x(const jsonx::json &j, SAXPrintHandlers &ph, int index, bool pp)
{
    if (!j.isDefined())
        return;
    if (pp)
        new_line(ph, index);
    if (j.isNull()) {
        ph.startElement(X("null"), attrs);
        ph.endElement(X("null"));
        return;
    }
    if (j.isBool()) {
        ph.startElement(X("bool"), attrs);
        ::to_string(ph, j.toBool() ? "true" : "false");
        ph.endElement(X("bool"));
        return;
    }
    if (j.isSigned()) {
        ph.startElement(X("signed"), attrs);
        ::to_string(ph, std::to_string(j.toSigned()));
        ph.endElement(X("signed"));
        return;
    }
    if (j.isUnsigned()) {
        ph.startElement(X("unsigned"), attrs);
        ::to_string(ph, std::to_string(j.toUnsigned()));
        ph.endElement(X("unsigned"));
        return;
    }
    if (j.isReal()) {
        ph.startElement(X("real"), attrs);
        ::to_string(ph, std::to_string(j.toReal()));
        ph.endElement(X("real"));
        return;
    }
    if (j.isString()) {
        ph.startElement(X("string"), attrs);
        ::to_string(ph, j.toString());
        ph.endElement(X("string"));
        return;
    }
    if (j.isArray()) {
        ph.startElement(X("array"), attrs);
        for (size_t i = 0; i < j.size(); ++i)
            j2x(j[i], ph, index+1, pp);
        if (pp)
            new_line(ph, index);
        ph.endElement(X("array"));
        return;
    }
    ph.startElement(X("object"), attrs);
    json_object_t obj{j.toObject()};
    for_each(obj.begin(), obj.end(), [&] (const pair<const string, json> &v) {
        if (pp)
            new_line(ph, index+1);
        ph.startElement(X(v.first.c_str()), attrs);
        j2x(v.second, ph, index+2, pp);
        if (pp)
            new_line(ph, index+1);
        ph.endElement(X(v.first.c_str()));
    });
    if (pp)
        new_line(ph, index);
    ph.endElement(X("object"));
}


Json2XML::Json2XML()
{
    XMLPlatformUtils::Initialize();
}

Json2XML::~Json2XML()
{
    XMLPlatformUtils::Terminate();
}

void Json2XML::json2xml(const jsonx::json &j, const std::string &xmlFile)
{
    ofstream ofs;
    ofs.open(xmlFile);
    if (!ofs.is_open())
        throw runtime_error("Unable to open \"" + xmlFile + "\"");
    json2xml(j, ofs);
}

void Json2XML::json2xml(const jsonx::json &j, std::ostream &os)
{
    XMLFormatter::UnRepFlags unrepFlags{XMLFormatter::UnRep_CharRef};
    SAXPrintHandlers printHandlers(os, "UTF-8", unrepFlags);
    printHandlers.startDocument();
    printHandlers.startElement(X("json"), attrs);
    j2x(j, printHandlers, 1, true);
    new_line(printHandlers, 0);
    printHandlers.endElement(X("json"));
    new_line(printHandlers, 0);
    printHandlers.endDocument();
}

json Json2XML::xml2json(const string &xmlFile)
{
    unique_ptr<SAX2XMLReader> parser{XMLReaderFactory::createXMLReader()};
    parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);
    parser->setFeature(XMLUni::fgXercesSchema, true);
    parser->setFeature(XMLUni::fgXercesHandleMultipleImports, true);
    parser->setFeature(XMLUni::fgXercesSchemaFullChecking, false);
    parser->setFeature(XMLUni::fgXercesIdentityConstraintChecking, true);
    parser->setFeature(XMLUni::fgSAX2CoreValidation, true);
    parser->setFeature(XMLUni::fgXercesDynamic, true);

    ParseHandler handler;
    parser->setContentHandler(&handler);
    parser->setErrorHandler(&handler);

    parser->parse(xmlFile.c_str());
    return handler.getResult();
}

static void remove_temp(const string &file)
{
    try {
        if (filesystem::exists(file))
            filesystem::remove(file);
    }
    catch (...)
    {}
}

json Json2XML::xml2json(istream &is)
{
    const size_t S_COPYBUF = 4096;
    string temp_name;
    json result;

    try {
#if _WIN32
        {
            errno_t err;
            char buf[L_tmpnam_s];
            err = tmpnam_s(buf, L_tmpnam_s);
            if (err)
                throw runtime_error("Unable to get tempfile name. ERC="
                                    + to_string(err));
            temp_name = buf;
        }
#else
        temp_name = tmpnam(nullptr);
#endif

        ofstream file_stream;
        file_stream.open(temp_name);
        if (!file_stream.is_open()) {
            throw runtime_error(string("Unable to open \"") + temp_name + "\"");
        }
        {
            char copybuf[S_COPYBUF];
            while (true) {
                is.read(copybuf, S_COPYBUF);
                streamsize n{is.gcount()};
                file_stream.write(copybuf, n);
                if ((is.gcount() < S_COPYBUF) || (!is.good()))
                    break;
            } // end while //
        }
        file_stream.close();
        result = xml2json(temp_name);
    }
    catch (...) {
        remove_temp(temp_name);
        throw;
    }
    remove_temp(temp_name);
    return result;
}
