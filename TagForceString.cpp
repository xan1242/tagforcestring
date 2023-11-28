//
// Yu-Gi-Oh! Tag Force Language & String Tool
// by Xan / Tenjoin
//

#include <iostream>
#include "TagForceString.hpp"
#include "StrResource.hpp"
#include "StoryScript.hpp"
#include "TF1Folder.hpp"

int main(int argc, char* argv[])
{
    std::cout << "Yu-Gi-Oh! Tag Force Language & String Tool\n\n";
    if (argc < 4) 
    {
        //std::cerr << "Insufficient arguments.\n";
        TagForceString::printUsage(argv[0]);
        return 1;
    }

    TagForceString::Options options = TagForceString::parseCommandLine(argc, argv);
    TagForceString::UnicodeBOMType curBOM = TagForceString::UnicodeBOMType::BOM_UNKNOWN;

    if (options.useUTF8)
        std::cout << "UTF-8 mode enabled!\n";

    if (((options.mode == TagForceString::OperatingMode::TXT2BIN) || (options.mode == TagForceString::OperatingMode::TXT2LANG))
        && (options.autodetectBOM && !options.useUTF8))
    {
        try
        {
            curBOM = TagForceString::GetBOM(options.inputFilePath1);
        }
        catch (const std::exception& e)
        {
            std::cerr << "ERROR: Failed to open file: " << options.inputFilePath1.string() << " for reading.\n";
            std::cerr << "Reason: " << e.what() << '\n';
            return -1;
        }

        std::cout << "BOM: ";
        switch (curBOM)
        {
            case TagForceString::UnicodeBOMType::BOM_UTF8:
            {
                std::cout << "UTF-8";
                options.useUTF8 = true;
                break;
            }
        
            case TagForceString::UnicodeBOMType::BOM_UTF16LE:
            {
                std::cout << "UTF-16 Little Endian";
                break;
            }
        
            case TagForceString::UnicodeBOMType::BOM_UTF16BE:
            {
                std::cout << "UTF-16 Big Endian";
                break;
            }
        
            default:
            {
                std::cout << "Unknown";
                break;
            }
        }
        
        std::cout << '\n';

        if (curBOM == TagForceString::UnicodeBOMType::BOM_UTF16BE)
        {
            std::cerr << "Big endian BOM detected! Please only use little endian files!\n";
            return 2;
        }
    }

    switch (options.mode)
    {
        case TagForceString::OperatingMode::BIN2TXT:
        {
            std::cout << "Converting: " << '\n' 
                << " <- " << options.inputFilePath1.string() << '\n'
                << " -> " << options.outputFilePath1.string() << '\n';

            if (options.useUTF8)
                return StrResource::ExportU8(options.inputFilePath1, options.outputFilePath1);
            else
                return StrResource::ExportU16(options.inputFilePath1, options.outputFilePath1);

            break;
        }

        case TagForceString::OperatingMode::TXT2BIN:
        {
            std::cout << "Converting: " << '\n'
                << " <- " << options.inputFilePath1.string() << '\n'
                << " -> " << options.outputFilePath1.string() << '\n';

            if (options.useUTF8)
                return StrResource::ImportU8(options.inputFilePath1, options.outputFilePath1);
            else
                return StrResource::ImportU16(options.inputFilePath1, options.outputFilePath1);

            break;
        }

        case TagForceString::OperatingMode::LANG2TXT:
        {
            std::cout << "Converting: " << '\n'
                << " <- " << options.inputFilePath1.string() << '\n'
                << " <- " << options.inputFilePath2.string() << '\n'
                << " -> " << options.outputFilePath1.string() << '\n';

            if (options.useUTF8)
                return StoryScript::ExportU8(options.inputFilePath1, options.inputFilePath2, options.outputFilePath1);
            else
                return StoryScript::ExportU16(options.inputFilePath1, options.inputFilePath2, options.outputFilePath1);

            break;
        }

        case TagForceString::OperatingMode::TXT2LANG:
        {
            std::cout << "Converting: " << '\n'
                << " <- " << options.inputFilePath1.string() << '\n'
                << " -> " << options.outputFilePath1.string() << '\n'
                << " -> " << options.outputFilePath2.string() << '\n';

            if (options.useUTF8)
                return StoryScript::ImportU8(options.inputFilePath1, options.outputFilePath1, options.outputFilePath2);
            else
                return StoryScript::ImportU16(options.inputFilePath1, options.outputFilePath1, options.outputFilePath2);

            break;
        }

        case TagForceString::OperatingMode::FOLD2TXT:
        {
            std::cout << "Converting: " << '\n'
                << " <- " << options.inputFilePath1.string() << '\n'
                << " -> " << options.outputFilePath1.string() << '\n';

            if (options.useUTF8)
                return TF1Folder::ExportFolderU8(options.inputFilePath1, options.outputFilePath1);
            else
                return TF1Folder::ExportFolderU16(options.inputFilePath1, options.outputFilePath1);

            break;
        }

        case TagForceString::OperatingMode::TXT2FOLD:
        {
            std::cout << "Converting: " << '\n'
                << " <- " << options.inputFilePath1.string() << '\n'
                << " -> " << options.outputFilePath1.string() << '\n';

            if (options.useUTF8)
                return TF1Folder::ImportFolderU8(options.inputFilePath1, options.outputFilePath1);
            else
                return TF1Folder::ImportFolderU16(options.inputFilePath1, options.outputFilePath1);

            break;
        }
    }

    return 0;
}
