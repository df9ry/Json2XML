
#include "json2xml.hpp"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <filesystem>

#undef NDEBUG
#include <cassert>

using namespace std;
using namespace jsonx;
using namespace filesystem;

int main(int, const char *[])
{
    try {
        cout << "Json2XML Test START" << endl;

        Json2XML xmlio;

        static const string test_dir{"./_test"};

        if (exists(test_dir))
            remove_all(test_dir);
        create_directory(test_dir);
        if ((!exists(test_dir)) || (!is_directory(test_dir)))
            throw runtime_error("Unable to create directory \"./test\"");

        json test1{json::undefined};
        xmlio.json2xml(test1, test_dir + "/test1.xml");

        json test2{json::null};
        xmlio.json2xml(test2, test_dir + "/test2.xml");

        json test3{json(true)};
        xmlio.json2xml(test3, test_dir + "/test3.xml");

        json test4{json(-15)};
        xmlio.json2xml(test4, test_dir + "/test4.xml");

        json test5{json(static_cast<uint16_t>(45))};
        xmlio.json2xml(test5, test_dir + "/test5.xml");

        json test6{json(3.14)};
        xmlio.json2xml(test6, test_dir + "/test6.xml");

        json test7{json("\"The quick brown fox jumps over the lazy dog\": \\ <>")};
        xmlio.json2xml(test7, test_dir + "/test7.xml");

        json test8;
        test8.add("A");
        test8.add("C");
        test8.add("B");
        xmlio.json2xml(test8, test_dir + "/test8.xml");

        json test9 = jobject({
             jitem("First",99),
             jitem("Fourth",
                 jobject({
                     jitem("T2", "2"),
                     jitem("T1", 1),
                     jitem("T3", 3.14)
                 })
             ),
             jitem("Second","Blub"),
             jitem("Third", true)
        });
        xmlio.json2xml(test9, test_dir + "/test9.xml");



        json y1{xmlio.xml2json(test_dir + "/test1.xml")};
        assert(!y1.isDefined());

        json y2{xmlio.xml2json(test_dir + "/test2.xml")};
        assert(y2 == test2);

        json y3{xmlio.xml2json(test_dir + "/test3.xml")};
        assert(y3 == test3);

        json y4{xmlio.xml2json(test_dir + "/test4.xml")};
        assert(y4 == test4);

        json y5{xmlio.xml2json(test_dir + "/test5.xml")};
        assert(y5 == test5);

        json y6{xmlio.xml2json(test_dir + "/test6.xml")};
        assert(y6 == test6);

        json y7{xmlio.xml2json(test_dir + "/test7.xml")};
        assert(y7 == test7);

        json y8{xmlio.xml2json(test_dir + "/test8.xml")};
        assert(y8 == test8);

        json y9{xmlio.xml2json(test_dir + "/test9.xml")};
        assert(y9 == test9);

        cout << "Json2XML Test FINISHED" << endl;
        return EXIT_SUCCESS;
    }
    catch (const exception &ex) {
        cerr << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        cerr << "..." << endl;
        return EXIT_FAILURE;
    }
}
