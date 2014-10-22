
#include <iostream>

#include <Rsd/File.h>
#include <Rsd/SchemaManager.h>
#include <Rsd/Parser.h>


using namespace RenderSpud::Rsd;


int main(int argc, char **argv)
{
    try
    {
        SchemaManager sm;
        File::FilePtr pFile = new File("testSchema1.rsd");
        
        sm.addSchemas(pFile);
        
        std::vector<std::string> validationResults;
        for (size_t i = 0; i < pFile->size(); ++i)
        {
            if (!sm.validate(pFile->value(i), &validationResults))
            {
                for (size_t i = 0; i < validationResults.size(); ++i)
                {
                    std::cout << "Schema validation failed: " << validationResults[i] << std::endl;
                }
            }
        }
    }
    catch (RenderSpud::Rsd::Parser::ParseException& pe)
    {
        std::cerr << "testSchema1.rsd:" << pe.line() << ":" << pe.pos() << ": " << pe.what() << std::endl;
    }
    return 0;
}
