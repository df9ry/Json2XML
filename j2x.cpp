
#include "json2xml.hpp"

#include <jsonx.hpp>

#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/XMLException.hpp>
#include <xercesc/util/XMLString.hpp>

#include <iostream>
#include <string>
#include <algorithm>
#include <stdexcept>

using namespace std;
using namespace jsonx;
XERCES_CPP_NAMESPACE_USE

// ---------------------------------------------------------------------------
//  This is a simple class that lets us do easy (though not terribly efficient)
//  trancoding of XMLCh data to local code page for display.
// ---------------------------------------------------------------------------
class StrX
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    StrX(const XMLCh* const toTranscode)
    {
        // Call the private transcoding method
        fLocalForm = XMLString::transcode(toTranscode);
    }

    ~StrX()
    {
        XMLString::release(&fLocalForm);
    }

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    const char* localForm() const
    {
        return fLocalForm;
    }

private :
    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fLocalForm
    //      This is the local code page form of the string.
    // -----------------------------------------------------------------------
    char*   fLocalForm;
};

inline XERCES_STD_QUALIFIER ostream& operator<<(XERCES_STD_QUALIFIER ostream& target, const StrX& toDump)
{
    target << toDump.localForm();
    return target;
}

std::string str_tolower(string s) {
    transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return tolower(c); } );
    return s;
}

int main(int /*argc*/, char *argv[])
{
    string prog{str_tolower(argv[0])};
    bool inverse{prog.ends_with("x2j") || prog.ends_with("x2j.exe")};

    try {
        Json2XML j2x;
        json j;
        if (inverse) {
            j = j2x.xml2json(cin);
            cout << j;
        } else {
            cin >> j;
            j2x.json2xml(j, cout);
        }
    }
    catch (const OutOfMemoryException&)
    {
        cerr << "OutOfMemoryException" << endl;
        return EXIT_FAILURE;
    }
    catch (const XMLException& toCatch)
    {
        cerr << "\nAn error occurred\n  Error: "
             << StrX(toCatch.getMessage()) << endl;
        return EXIT_FAILURE;
    }
    catch (const exception& toCatch)
    {
        cerr << "\nAn error occurred\n  Error: "
             << toCatch.what() << endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        cerr << "\nAn error occurred\n" << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
