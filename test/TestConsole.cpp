
#include <iostream>

#include <Rsd/Parser.h>
#include <Rsd/File.h>


using namespace RenderSpud::Rsd;


int main(int argc, char **argv)
{
    std::string evaluate;
    bool evaluated = false;
    if (argc == 3)
    {
        evaluate = argv[2];
    }
    else if (argc != 2)
    {
        std::cerr << "ERROR: wrong number of commandline arguments." << std::endl;
        std::cerr << "Usage: " << argv[0] << " <infile.rsd> [eval-statement]" << std::endl;
        return 1;
    }

    std::string filename = argv[1];

    try
    {
        File::FilePtr pFile = new File(filename, true);
        
        std::cout << "Parsed RSD file \"" << filename << "\"";
        if (evaluate != "")
            std::cout << std::endl;
        else
            std::cout << "; press ctrl-D or type 'exit' and press enter to end session." << std::endl;

        while (true)
        {
            try
            {
                std::cout << ">> " << std::flush;
                std::string line;
                if (evaluate != "")
                {
                    if (evaluated)
                        break;

                    std::cout << evaluate << std::endl;
                    line = evaluate;
                    evaluated = true;
                }
                else
                {
                    std::getline(std::cin, line);
                    if (std::cin.eof())
                    {
                        std::cout << std::endl;
                        break;
                    }
                }
                Reference::Ptr pRef;
                Value::Ptr pResult;
                if (line == "exit")
                {
                    break;
                }
                else if (line != "")
                {
                    pRef = Reference::fromString(line);
                    pResult = pFile->find(*pRef);
                }
                else
                {
                    pResult = pFile;
                }
                if (pResult == NULL)
                {
                    Value::Ptr pValue = new Value(pRef);
                    std::cerr << "ERROR: Could not resolve value: "
                              << line << std::endl;
                    std::cerr << "ERROR: The closest we could get was: "
                              << pValue->str() << std::endl;
                    continue;
                }

                std::string resultStr;
                if (pResult->canConvertTo(Value::kTypeString))
                {
                    resultStr = pResult->asString();
                }
                else
                {
                    resultStr = pResult->str();
                }

                if (!pResult->allValuesResolvable(NULL, false))
                {
                    std::cerr << "ERROR: Could not resolve value: "
                              << line << std::endl;
                    std::cerr << "ERROR: The closest we could get was: "
                              << resultStr << std::endl;
                    continue;
                }
                else
                {
                    std::cout << resultStr << std::endl;
                }
            }
            catch (ValueException& ve)
            {
                std::cerr << "ERROR: ValueException: "
                          << ve.what() << std::endl;
            }
            catch (ValueConversionException& vce)
            {
                std::cerr << "ERROR: ValueConversionException: "
                          << vce.what() << std::endl;
            }
            catch (Parser::ParseException& pe)
            {
                std::cerr << "input:" << pe.line() << ":" << pe.pos() << ": "
                          << pe.what() << std::endl;
            }
        }
    }
    catch (Parser::ParseException& pe)
    {
        std::cerr << "ERROR: " << pe.source() << ':' << pe.line() << ":"
                  << pe.pos() << ": " << pe.description() << std::endl;
        return 2;
    }
    return 0;
}
