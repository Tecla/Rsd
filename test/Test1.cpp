
#include <iostream>

#include <Rsd/Parser.h>
#include <Rsd/File.h>


int main(int argc, char **argv)
{
    try
    {
        std::string testInput =
            "include \"whatever.rsd\";\n"
            "myValue = @\"Some\".Type\n"
            "{\n"
            "    someArray = @V3f [\n"
            "                       1.0,\n"
            "                       2.0,\n"
            "                       3.0\n"
            "                     ];\n"
            "    someArray2 = @whatever [\n"
            "                       someFunction(obj: null, on : true, name:\"x\"),\n"
            "                       @V3f [1, 2, 3],\n"
            "                       3.0\n"
            "                     ];\n"
            "};\n";

//        RenderSpud::Rsd::Parser::Parser parser;
//        RenderSpud::Rsd::Value value;
//        parser.parse(testInput, value);

//        std::cout << "Parsing successful!" << std::endl;

        RenderSpud::Rsd::File::FilePtr pFile = new RenderSpud::Rsd::File(testInput, std::string("input"));

        std::cout << "// Parsed:" << std::endl << pFile->str() << std::endl;
    }
    catch (RenderSpud::Rsd::Parser::ParseException& pe)
    {
        std::cerr << "input:" << pe.line() << ":" << pe.pos() << ": " << pe.what() << std::endl;
    }
    return 0;
}
