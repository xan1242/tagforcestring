#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>

#ifndef TF1FOLDER_HDR
#define TF1FOLDER_HDR

namespace TF1Folder
{
    int ExportFolderU16(std::filesystem::path inFolder, std::filesystem::path outFolder)
    {
        if (!std::filesystem::exists(inFolder))
        {
            std::cerr << "ERROR: Folder " << inFolder.string() << " does not exist!\n";
            return -1;
        }

        if (!std::filesystem::exists(outFolder))
        {
            try
            {
                std::filesystem::create_directory(outFolder);
            }
            catch (const std::exception& e)
            {
                std::cerr << "ERROR: Folder " << outFolder.string() << " does not exist & could not be created!\n";
                std::cerr << "Reason: " << e.what() << '\n';
                return -2;
            }
        }

        std::vector<std::u8string> processedEntries;

        //
        // expected filenames are in format:
        // <name><type><lang>.bin
        // <name><type><lang>.bin.gz
        //
        // <name> - arbitrary length
        // <type> - 1 char - can either be I or L
        // <lang> - 1 char - first letter of a language in English, can be: j, e, g, f, i or s
        //

        for (const auto& entry : std::filesystem::directory_iterator(inFolder))
        {
            if ((entry.path().extension() != ".bin") && (entry.path().extension() != ".gz"))
                continue;

            bool bCompressed = false;
            bool bOtherCompressed = false;
            bool bOtherIsIdx = false;
            int posType = 6;
            if ((entry.path().extension() == ".gz"))
            {
                bCompressed = true;
                bOtherCompressed = true;
                posType += 3;
            }

            std::u8string strEntry = entry.path().filename().u8string();
            std::u8string strName = strEntry.substr(0, strEntry.size() - posType);

            if (std::find(processedEntries.begin(), processedEntries.end(), strName) != processedEntries.end())
                continue;

            std::u8string strTail = strEntry.substr(strEntry.size() - posType);
            std::u8string strType = strTail.substr(0, 1);
            std::u8string strLang = strTail.substr(1, 1);
            std::u8string strFullExt = strTail.substr(2);
            std::u8string strOtherType = u8"L";
            

            if (strType == u8"L")
            {
                bOtherIsIdx = true;
                strOtherType = u8"I";
            }

            std::u8string strOtherName = strName + strOtherType + strLang + strFullExt;

            std::filesystem::path otherEntry = entry.path().parent_path() / strOtherName;

            std::cout << "Processing: " << (char*)strName.c_str() << '\n'
                << " <- " << entry.path().string() << '\n';

            if (!std::filesystem::exists(otherEntry))
            {
                // try to find the opposite just in case
                if (bCompressed)
                {
                    strOtherName = strName + strOtherType + strLang + u8".bin";
                    otherEntry = entry.path().parent_path() / strOtherName;
                }
                else
                {
                    strOtherName = strName + strOtherType + strLang + u8".bin.gz";
                    otherEntry = entry.path().parent_path() / strOtherName;
                }

                if (!std::filesystem::exists(otherEntry))
                {
                    std::cerr << "ERROR: Can't find " << otherEntry.string() << " !\n";
                    processedEntries.push_back(strName);
                    continue;
                }

                bOtherCompressed = !bOtherCompressed;
            }

            std::cout << " <- " << otherEntry.string() << '\n';

            std::u8string outName = strName;
            if (bCompressed)
                outName += u8".gz";
            outName += u8".txt";

            std::filesystem::path outPath = outFolder / outName;

            std::cout << " -> " << outPath.string() << '\n';

            std::filesystem::path idxPath;
            std::filesystem::path langPath;
            if (bOtherIsIdx)
            {
                idxPath = otherEntry;
                langPath = entry.path();
            }
            else
            {
                idxPath = entry.path();
                langPath = otherEntry;
            }

            if (bCompressed)
            {
                // decompress the first file

                // TODO: put decompress code here & update name with temp
                // if (bOtherIsIdx)
                // {
                // 
                // }
                // else
                // {
                // 
                // }

                processedEntries.push_back(strName);
                continue;
            }

            if (bOtherCompressed)
            {
                // decompress the second file

                // TODO: put decompress code here & update name with temp
                // if (bOtherIsIdx)
                // {
                // 
                // }
                // else
                // {
                // 
                // }

                processedEntries.push_back(strName);
                continue;
            }

            StoryScript::ExportU16(idxPath, langPath, outPath);

            std::cout << '\n';

            processedEntries.push_back(strName);
        }

        return 0;
    }

    int ExportFolderU8(std::filesystem::path inFolder, std::filesystem::path outFolder)
    {
        std::cout << "Unimplemented\n";
        return 1;
    }

    int ImportFolderU16(std::filesystem::path inFolder, std::filesystem::path outFolder)
    {
        std::cout << "Unimplemented\n";
        return 1;
    }

    int ImportFolderU8(std::filesystem::path inFolder, std::filesystem::path outFolder)
    {
        std::cout << "Unimplemented\n";
        return 1;
    }
}

#endif