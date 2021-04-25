#include "ParseHandler.hpp"
#include "SAXPrint.hpp"

#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>

#include <utils/string_utils.hpp>

#include <unordered_map>

using namespace std;
using namespace jsonx;

typedef unordered_map<string, NodeType> id_map_t;
static const id_map_t id_map {
    { "json",     NodeType::DOC      },
    { "null",     NodeType::NIL      },
    { "bool",     NodeType::BOOL     },
    { "signed",   NodeType::SIGNED   },
    { "unsigned", NodeType::UNSIGNED },
    { "real",     NodeType::REAL     },
    { "string",   NodeType::STRING   },
    { "array",    NodeType::ARRAY    },
    { "object",   NodeType::OBJECT   }
};

ParseHandler::ParseHandler()
{
}

ParseHandler::~ParseHandler()
{
}

NodeType ParseHandler::getNodeType(const string &name)
{
    id_map_t::const_iterator iter{id_map.find(name)};
    return (iter != id_map.end()) ? iter->second : NodeType::UNDEF;
}

// ---------------------------------------------------------------------------
//  SAX2CountHandlers: Implementation of the SAX DocumentHandler interface
// ---------------------------------------------------------------------------
void ParseHandler::startElement(const XMLCh* const /* uri */
                                   , const XMLCh* const localname
                                   , const XMLCh* const /* qname */
                                   , const Attributes& /*attrs*/)
{
    // Clear character buffer:
    ss.clear();
    ss.str("");

    level_ptr_t parentLevel{level_stack.empty() ? rootLevel : level_stack.top()};
    level_ptr_t thisLevel{};

    // Special case object:
    if (parentLevel->id == LevelId::OBJECT) {
        thisLevel.reset(new Level(LevelId::OBJECT_ENTRY));
        level_stack.push(thisLevel);
        return;
    }

    // In all other cases check the wanted type:
    NodeType type{getNodeType(StrX(localname).localForm())};
    switch (type) {
    case NodeType::ARRAY:
        thisLevel.reset(new Level(LevelId::ARRAY));
        thisLevel->value = jarray({});
        break;
    case NodeType::OBJECT:
        thisLevel.reset(new Level(LevelId::OBJECT));
        thisLevel->value = jobject({});
        break;
    case NodeType::DOC:
        thisLevel.reset(new Level(LevelId::DOC));
        rootLevel->value = json();
        break;
    default:
        thisLevel.reset(new Level(LevelId::LEAF));
        thisLevel->value = json();
        break;
    } // end switch //
    level_stack.push(thisLevel);
}

void ParseHandler::endElement(const XMLCh* const /* uri */
                                   , const XMLCh* const localname
                                   , const XMLCh* const /* qname */)
{
    // Load current level descriptor:
    level_ptr_t thisLevel{level_stack.top()};
    level_stack.pop();

    switch (thisLevel->id) {
    case LevelId::ROOT:
        throw runtime_error("Level underrun error");
    case LevelId::DOC:
        rootLevel->value = thisLevel->value;
        return;
    case LevelId::OBJECT_ENTRY:
        {   // Special case: Key is in localname, value in value:
            string key{StrX(localname).localForm()};
            level_ptr_t obj_ptr{level_stack.top()};
            obj_ptr->value[key] = thisLevel->value;
        }
        return;
    case LevelId::ARRAY:
    case LevelId::OBJECT:
        {
            level_ptr_t parent_ptr{level_stack.top()};
            switch (parent_ptr->id) {
            case LevelId::ROOT:
                throw runtime_error("Level underrun error");
            case LevelId::DOC:
                parent_ptr->value = thisLevel->value;
                return;
            case LevelId::OBJECT_ENTRY:
                parent_ptr->value = thisLevel->value;
                return;
            case LevelId::ARRAY:
                parent_ptr->value.add(thisLevel->value);
                return;
            default:
                throw runtime_error("Level corruption error");
            } // end switch //
        }
    default:
        break;
    } // end switch //

    // Leafs must be constructed by the texts:
    // The type is encoded in localname:
    NodeType type{getNodeType(StrX(localname).localForm())};

    // And the value could now be parsed:
    switch (type) {
    case NodeType::NIL:
        thisLevel->value.set(json::null);
        break;
    case NodeType::BOOL:
        {
            string s;
            ss >> s;
            thisLevel->value.set((s == "true") || (s == "1"));
        }
        break;
    case NodeType::SIGNED:
        {
            int64_t x;
            ss >> x;
            thisLevel->value.set(x);
        }
        break;
    case NodeType::UNSIGNED:
        {
            uint64_t x;
            ss >> x;
            thisLevel->value.set(x);
        }
        break;
    case NodeType::REAL:
        {
            double x;
            ss >> x;
            thisLevel->value.set(x);
        }
        break;
    case NodeType::STRING:
        thisLevel->value.set(Utils::trim(ss.str()));
        break;
    default:
        throw runtime_error("Invalid simple node <" + string(StrX(localname).localForm()) + ">");
    } // end switch //

    // Lookup and insert into container:
    level_ptr_t parent_ptr{level_stack.top()};
    switch (parent_ptr->id) {
    case LevelId::ROOT:
        throw runtime_error("Level underrun error");
    case LevelId::DOC:
        parent_ptr->value = thisLevel->value;
        return;
    case LevelId::OBJECT_ENTRY:
        parent_ptr->value = thisLevel->value;
        return;
    case LevelId::ARRAY:
        parent_ptr->value.add(thisLevel->value);
        return;
    default:
        throw runtime_error("Level corruption error");
    } // end switch //
}

void ParseHandler::characters(  const   XMLCh* const   chars
                                    , const XMLSize_t length)
{
    ss << string(StrX(chars).localForm(), length);
}

void ParseHandler::ignorableWhitespace( const   XMLCh* const /* chars */
                                            , const XMLSize_t /* length */)
{
}

void ParseHandler::startDocument()
{
}

// ---------------------------------------------------------------------------
//  SAX2CountHandlers: Overrides of the SAX ErrorHandler interface
// ---------------------------------------------------------------------------
void ParseHandler::error(const SAXParseException& e)
{
    fSawErrors = true;
    XERCES_STD_QUALIFIER cerr << "\nError at file " << StrX(e.getSystemId())
         << ", line " << e.getLineNumber()
         << ", char " << e.getColumnNumber()
         << "\n  Message: " << StrX(e.getMessage()) << XERCES_STD_QUALIFIER endl;
}

void ParseHandler::fatalError(const SAXParseException& e)
{
    fSawErrors = true;
    XERCES_STD_QUALIFIER cerr << "\nFatal Error at file " << StrX(e.getSystemId())
         << ", line " << e.getLineNumber()
         << ", char " << e.getColumnNumber()
         << "\n  Message: " << StrX(e.getMessage()) << XERCES_STD_QUALIFIER endl;
}

void ParseHandler::warning(const SAXParseException& e)
{
    XERCES_STD_QUALIFIER cerr << "\nWarning at file " << StrX(e.getSystemId())
         << ", line " << e.getLineNumber()
         << ", char " << e.getColumnNumber()
         << "\n  Message: " << StrX(e.getMessage()) << XERCES_STD_QUALIFIER endl;
}

void ParseHandler::resetErrors()
{
    fSawErrors = false;
}
