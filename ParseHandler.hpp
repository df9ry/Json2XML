#ifndef SAX2PARSEHANDLER_HPP
#define SAX2PARSEHANDLER_HPP

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>

#include <jsonx.hpp>

#include <stack>
#include <sstream>
#include <memory>

XERCES_CPP_NAMESPACE_USE

enum class NodeType { DOC, NIL, BOOL, SIGNED, UNSIGNED, REAL, STRING, ARRAY, OBJECT, UNDEF };

enum class LevelId { ROOT, DOC, LEAF, ARRAY, OBJECT, OBJECT_ENTRY };

class Level;
typedef std::shared_ptr<Level> level_ptr_t;

class Level {
public:
    Level(LevelId _id): id{_id} {}

    const        LevelId id;
    jsonx::json  value{};
};

class ParseHandler : public DefaultHandler
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    ParseHandler();
    ~ParseHandler();

    // -----------------------------------------------------------------------
    //  Handlers for the SAX ContentHandler interface
    // -----------------------------------------------------------------------
    void startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const Attributes& attrs);
    void endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname);
    void characters(const XMLCh* const chars, const XMLSize_t length);
    void ignorableWhitespace(const XMLCh* const chars, const XMLSize_t length);
    void startDocument();
    jsonx::json getResult() { return rootLevel->value; }

    // -----------------------------------------------------------------------
    //  Handlers for the SAX ErrorHandler interface
    // -----------------------------------------------------------------------
    void warning(const SAXParseException& exc);
    void error(const SAXParseException& exc);
    void fatalError(const SAXParseException& exc);
    void resetErrors();

private:
    static NodeType getNodeType(const std::string &name);

    bool                    fSawErrors{false};
    std::stack<level_ptr_t> level_stack{};
    std::stringstream       ss{};

    level_ptr_t             rootLevel{new Level(LevelId::ROOT)};
};

#endif // SAX2PARSEHANDLER_HPP
