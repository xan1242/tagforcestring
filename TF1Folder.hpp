#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include "TFStringClasses.hpp"
#include "ZlibWrapper.hpp"

#ifndef TF1FOLDER_HDR
#define TF1FOLDER_HDR

namespace TF1Folder
{
    //
    // Batch exports story script index + lang pairs to ini-like formatted txt files (UTF-16)
    //
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
                std::cerr << "ERROR: Folder " << outFolder.string() << " could not be created!\n";
                std::cerr << "Reason: " << e.what() << '\n';
                return -2;
            }
        }

        std::vector<std::u8string> processedEntries;
        bool bHaveTempFiles = false;

        std::filesystem::path tempPath;
        try
        {
            tempPath = std::filesystem::temp_directory_path();
        }
        catch (const std::exception& e)
        {
            std::cout << "WARNING: Can't retrieve the temp directory path! Using the current directory...\n";
            std::cout << "Reason: " << e.what() << '\n';

            tempPath = ".";
        }

        //
        // expected filenames are in format:
        // <name><type><lang>.bin
        // <name><type><lang>.bin.gz
        //
        // <name> - arbitrary length
        // <type> - 1 char - can either be I or L
        // <lang> - 1 char - first letter of a western language in English, can be: j, e, g, f, i or s (Japanese, English, German, French, Italian or Spanish)
        //

        for (const auto& entry : std::filesystem::directory_iterator(inFolder))
        {
            if ((entry.path().extension() != ".bin") && (entry.path().extension() != ".gz"))
            {
                // std::cout << "Skipping file: " << entry.path() << '\n';
                continue;
            }

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
            
            if ((strType != u8"L") && (strType != u8"I"))
            {
                std::cerr << "ERROR: File " << entry.path() << " does not follow the correct filename format!\n";
                std::cerr << "Reason: Missing type character ('I' or 'L') in filename!\n";
               
                continue;
            }

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

            std::u8string outName = strName + u8'_' + strLang;
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
                // decompress the file to a temp directory
                bHaveTempFiles = true;
                std::filesystem::path tempFile = tempPath / "tfstemp1.bin";

                bool resultDecomp = ZLibWrapper::extractGzFile(entry.path(), tempFile);
                if (!resultDecomp)
                {
                    std::cout << '\n';
                    processedEntries.push_back(strName);
                    continue;
                }

                if (bOtherIsIdx)
                    langPath = tempFile;
                else
                    idxPath = tempFile;
            }

            if (bOtherCompressed)
            {
                // decompress the file to a temp directory
                bHaveTempFiles = true;
                std::filesystem::path tempFile = tempPath / "tfstemp2.bin";

                bool resultDecomp = ZLibWrapper::extractGzFile(otherEntry, tempFile);
                if (!resultDecomp)
                {
                    std::cout << '\n';
                    processedEntries.push_back(strName);
                    continue;
                }

                if (bOtherIsIdx)
                    idxPath = tempFile;
                else
                    langPath = tempFile;
            }

            StoryScript::ExportU16(idxPath, langPath, outPath);

            processedEntries.push_back(strName);
        }

        // cleanup temp files
        if (bHaveTempFiles)
        {
            std::filesystem::path tempFile1 = std::filesystem::temp_directory_path() / "tfstemp1.bin";
            std::filesystem::path tempFile2 = std::filesystem::temp_directory_path() / "tfstemp2.bin";

            if (std::filesystem::exists(tempFile1))
                std::filesystem::remove(tempFile1);

            if (std::filesystem::exists(tempFile2))
                std::filesystem::remove(tempFile2);
        }


        return 0;
    }

    //
    // Batch exports story script index + lang pairs to ini-like formatted txt files (UTF-8)
    //
    int ExportFolderU8(std::filesystem::path inFolder, std::filesystem::path outFolder)
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
                std::cerr << "ERROR: Folder " << outFolder.string() << " could not be created!\n";
                std::cerr << "Reason: " << e.what() << '\n';
                return -2;
            }
        }

        std::vector<std::u8string> processedEntries;
        bool bHaveTempFiles = false;

        std::filesystem::path tempPath;
        try
        {
            tempPath = std::filesystem::temp_directory_path();
        }
        catch (const std::exception& e)
        {
            std::cout << "WARNING: Can't retrieve the temp directory path! Using the current directory...\n";
            std::cout << "Reason: " << e.what() << '\n';

            tempPath = ".";
        }

        //
        // expected filenames are in format:
        // <name><type><lang>.bin
        // <name><type><lang>.bin.gz
        //
        // <name> - arbitrary length
        // <type> - 1 char - can either be I or L
        // <lang> - 1 char - first letter of a western language in English, can be: j, e, g, f, i or s (Japanese, English, German, French, Italian or Spanish)
        //

        for (const auto& entry : std::filesystem::directory_iterator(inFolder))
        {
            if ((entry.path().extension() != ".bin") && (entry.path().extension() != ".gz"))
            {
                // std::cout << "Skipping file: " << entry.path() << '\n';
                continue;
            }

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

            if ((strType != u8"L") && (strType != u8"I"))
            {
                std::cerr << "ERROR: File " << entry.path() << " does not follow the correct filename format!\n";
                std::cerr << "Reason: Missing type character ('I' or 'L') in filename!\n";

                continue;
            }

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

            std::u8string outName = strName + u8'_' + strLang;
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
                // decompress the file to a temp directory
                bHaveTempFiles = true;
                std::filesystem::path tempFile = tempPath / "tfstemp1.bin";

                bool resultDecomp = ZLibWrapper::extractGzFile(entry.path(), tempFile);
                if (!resultDecomp)
                {
                    std::cout << '\n';
                    processedEntries.push_back(strName);
                    continue;
                }

                if (bOtherIsIdx)
                    langPath = tempFile;
                else
                    idxPath = tempFile;
            }

            if (bOtherCompressed)
            {
                // decompress the file to a temp directory
                bHaveTempFiles = true;
                std::filesystem::path tempFile = tempPath / "tfstemp2.bin";

                bool resultDecomp = ZLibWrapper::extractGzFile(otherEntry, tempFile);
                if (!resultDecomp)
                {
                    std::cout << '\n';
                    processedEntries.push_back(strName);
                    continue;
                }

                if (bOtherIsIdx)
                    idxPath = tempFile;
                else
                    langPath = tempFile;
            }

            StoryScript::ExportU8(idxPath, langPath, outPath);

            processedEntries.push_back(strName);
        }

        // cleanup temp files
        if (bHaveTempFiles)
        {
            std::filesystem::path tempFile1 = std::filesystem::temp_directory_path() / "tfstemp1.bin";
            std::filesystem::path tempFile2 = std::filesystem::temp_directory_path() / "tfstemp2.bin";

            if (std::filesystem::exists(tempFile1))
                std::filesystem::remove(tempFile1);

            if (std::filesystem::exists(tempFile2))
                std::filesystem::remove(tempFile2);
        }


        return 0;
    }

    //
    // Batch exports story script index + lang pairs to ini-like formatted txt files (raw)
    //
    int ExportFolderRaw(std::filesystem::path inFolder, std::filesystem::path outFolder)
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
                std::cerr << "ERROR: Folder " << outFolder.string() << " could not be created!\n";
                std::cerr << "Reason: " << e.what() << '\n';
                return -2;
            }
        }

        std::vector<std::u8string> processedEntries;
        bool bHaveTempFiles = false;

        std::filesystem::path tempPath;
        try
        {
            tempPath = std::filesystem::temp_directory_path();
        }
        catch (const std::exception& e)
        {
            std::cout << "WARNING: Can't retrieve the temp directory path! Using the current directory...\n";
            std::cout << "Reason: " << e.what() << '\n';

            tempPath = ".";
        }

        //
        // expected filenames are in format:
        // <name><type><lang>.bin
        // <name><type><lang>.bin.gz
        //
        // <name> - arbitrary length
        // <type> - 1 char - can either be I or L
        // <lang> - 1 char - first letter of a western language in English, can be: j, e, g, f, i or s (Japanese, English, German, French, Italian or Spanish)
        //

        for (const auto& entry : std::filesystem::directory_iterator(inFolder))
        {
            if ((entry.path().extension() != ".bin") && (entry.path().extension() != ".gz"))
            {
                // std::cout << "Skipping file: " << entry.path() << '\n';
                continue;
            }

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

            if ((strType != u8"L") && (strType != u8"I"))
            {
                std::cerr << "ERROR: File " << entry.path() << " does not follow the correct filename format!\n";
                std::cerr << "Reason: Missing type character ('I' or 'L') in filename!\n";

                continue;
            }

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

            std::u8string outName = strName + u8'_' + strLang;
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
                // decompress the file to a temp directory
                bHaveTempFiles = true;
                std::filesystem::path tempFile = tempPath / "tfstemp1.bin";

                bool resultDecomp = ZLibWrapper::extractGzFile(entry.path(), tempFile);
                if (!resultDecomp)
                {
                    std::cout << '\n';
                    processedEntries.push_back(strName);
                    continue;
                }

                if (bOtherIsIdx)
                    langPath = tempFile;
                else
                    idxPath = tempFile;
            }

            if (bOtherCompressed)
            {
                // decompress the file to a temp directory
                bHaveTempFiles = true;
                std::filesystem::path tempFile = tempPath / "tfstemp2.bin";

                bool resultDecomp = ZLibWrapper::extractGzFile(otherEntry, tempFile);
                if (!resultDecomp)
                {
                    std::cout << '\n';
                    processedEntries.push_back(strName);
                    continue;
                }

                if (bOtherIsIdx)
                    idxPath = tempFile;
                else
                    langPath = tempFile;
            }

            StoryScript::ExportRaw(idxPath, langPath, outPath);

            processedEntries.push_back(strName);
        }

        // cleanup temp files
        if (bHaveTempFiles)
        {
            std::filesystem::path tempFile1 = std::filesystem::temp_directory_path() / "tfstemp1.bin";
            std::filesystem::path tempFile2 = std::filesystem::temp_directory_path() / "tfstemp2.bin";

            if (std::filesystem::exists(tempFile1))
                std::filesystem::remove(tempFile1);

            if (std::filesystem::exists(tempFile2))
                std::filesystem::remove(tempFile2);
        }


        return 0;
    }

    //
    // Batch imports ini-like formatted txt files (UTF-16) and exports to story script index + lang pairs
    //
    int ImportFolderU16(std::filesystem::path inFolder, std::filesystem::path outFolder)
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
                std::cerr << "ERROR: Folder " << outFolder.string() << " could not be created!\n";
                std::cerr << "Reason: " << e.what() << '\n';
                return -2;
            }
        }

        std::vector<std::u8string> processedEntries;

        //
        // expected filenames are in format:
        // <name>_<lang>.txt
        // <name>_<lang>.gz.txt
        //
        // <name> - arbitrary length
        // <lang> - 1 char - first letter of a western language in English, can be: j, e, g, f, i or s (Japanese, English, German, French, Italian or Spanish)
        //

        for (const auto& entry : std::filesystem::directory_iterator(inFolder))
        {
            if (entry.path().extension() != ".txt")
            {
                // std::cout << "Skipping file: " << entry.path() << '\n';
                continue;
            }

            bool bCompressed = false;
            int posType = 6;
            std::u8string strEntry = entry.path().filename().u8string();
            if (strEntry.find(u8".gz") != strEntry.npos)
            {
                bCompressed = true;
                posType += 3;
            }

            std::u8string strName = strEntry.substr(0, strEntry.size() - posType);
            std::u8string strTail = strEntry.substr(strEntry.size() - posType);
            std::u8string strUnderline = strTail.substr(0, 1);
            std::u8string strLang = strTail.substr(1, 1);

            std::cout << "Processing: " << (char*)strName.c_str() << '\n'
                << " <- " << entry.path().string() << '\n';

            if (strUnderline != u8"_")
            {
                std::cerr << "ERROR: File " << entry.path() << " does not follow the correct filename format!\n";
                std::cerr << "Reason: Missing underline character in filename!\n";

                continue;
            }

            // parse
            std::vector<std::u16string> strings;
            int errparse = TagForceString::ParseTxtU16(entry.path(), &strings);

            if (errparse < 0)
            {
                std::cerr << "ERROR: Can't parse: " << entry.path() << '\n';

                processedEntries.push_back(strName);
                continue;
            }

            // build the data
            TFStoryScript tfs;
            tfs.build(&strings);

            std::filesystem::path idxPath;
            std::filesystem::path langPath;

            if (bCompressed)
            {
                std::u8string idxName = strName + u8'I' + strLang + u8".bin.gz";
                std::u8string langName = strName + u8'L' + strLang + u8".bin.gz";

                idxPath = outFolder / idxName;
                langPath = outFolder / langName;

                std::cout << " -> " << idxPath.string() << '\n';

                bool bGZStatus = ZLibWrapper::packGzFile(tfs.idxptr(), tfs.idxsize(), idxPath);
                if (!bGZStatus)
                {
                    std::cerr << "ERROR: Can't compress data to: " << idxPath.string() << '\n';

                    processedEntries.push_back(strName);
                    continue;
                }

                std::cout << " -> " << langPath.string() << '\n';

                bGZStatus = ZLibWrapper::packGzFile(tfs.fileptr(), tfs.datasize(), langPath);
                if (!bGZStatus)
                {
                    std::cerr << "ERROR: Can't compress data to: " << langPath.string() << '\n';

                    processedEntries.push_back(strName);
                    continue;
                }
            }
            else
            {
                std::u8string idxName = strName + u8'I' + strLang + u8".bin";
                std::u8string langName = strName + u8'L' + strLang + u8".bin";

                idxPath = outFolder / idxName;
                langPath = outFolder / langName;

                std::cout << " -> " << idxPath.string() << '\n';
                std::cout << " -> " << langPath.string() << '\n';

                try
                {
                    tfs.exportFile(idxPath, langPath);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "ERROR: Failed to open files: " << idxPath.string() << " and " << langPath.string() << " for writing.\n";
                    std::cerr << "Reason: " << e.what() << '\n';

                    processedEntries.push_back(strName);
                    continue;
                }
            }

            processedEntries.push_back(strName);
            continue;
        }

        return 0;
    }

    //
    // Batch imports ini-like formatted txt files (UTF-8) and exports to story script index + lang pairs
    //
    int ImportFolderU8(std::filesystem::path inFolder, std::filesystem::path outFolder)
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
                std::cerr << "ERROR: Folder " << outFolder.string() << " could not be created!\n";
                std::cerr << "Reason: " << e.what() << '\n';
                return -2;
            }
        }

        std::vector<std::u8string> processedEntries;

        //
        // expected filenames are in format:
        // <name>_<lang>.txt
        // <name>_<lang>.gz.txt
        //
        // <name> - arbitrary length
        // <lang> - 1 char - first letter of a western language in English, can be: j, e, g, f, i or s (Japanese, English, German, French, Italian or Spanish)
        //

        for (const auto& entry : std::filesystem::directory_iterator(inFolder))
        {
            if (entry.path().extension() != ".txt")
            {
                // std::cout << "Skipping file: " << entry.path() << '\n';
                continue;
            }

            bool bCompressed = false;
            int posType = 6;
            std::u8string strEntry = entry.path().filename().u8string();
            if (strEntry.find(u8".gz") != strEntry.npos)
            {
                bCompressed = true;
                posType += 3;
            }

            std::u8string strName = strEntry.substr(0, strEntry.size() - posType);
            std::u8string strTail = strEntry.substr(strEntry.size() - posType);
            std::u8string strUnderline = strTail.substr(0, 1);
            std::u8string strLang = strTail.substr(1, 1);

            std::cout << "Processing: " << (char*)strName.c_str() << '\n'
                << " <- " << entry.path().string() << '\n';

            if (strUnderline != u8"_")
            {
                std::cerr << "ERROR: File " << entry.path() << " does not follow the correct filename format!\n";
                std::cerr << "Reason: Missing underline character in filename!\n";

                continue;
            }

            // parse
            std::vector<std::u8string> strings;
            int errparse = TagForceString::ParseTxtU8(entry.path(), &strings);

            if (errparse < 0)
            {
                std::cerr << "ERROR: Can't parse: " << entry.path() << '\n';

                processedEntries.push_back(strName);
                continue;
            }

            // build the data
            TFStoryScript tfs;
            tfs.build(&strings);

            std::filesystem::path idxPath;
            std::filesystem::path langPath;

            if (bCompressed)
            {
                std::u8string idxName = strName + u8'I' + strLang + u8".bin.gz";
                std::u8string langName = strName + u8'L' + strLang + u8".bin.gz";

                idxPath = outFolder / idxName;
                langPath = outFolder / langName;

                std::cout << " -> " << idxPath.string() << '\n';

                try
                {
                    ZLibWrapper::packGzFile(tfs.idxptr(), tfs.idxsize(), idxPath);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "ERROR: Can't compress data to: " << idxPath.string() << '\n';
                    std::cerr << "Reason: " << e.what() << '\n';

                    processedEntries.push_back(strName);
                    continue;
                }

                std::cout << " -> " << langPath.string() << '\n';

                try
                {
                    ZLibWrapper::packGzFile(tfs.fileptr(), tfs.datasize(), langPath);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "ERROR: Can't compress data to: " << langPath.string() << '\n';
                    std::cerr << "Reason: " << e.what() << '\n';

                    processedEntries.push_back(strName);
                    continue;
                }
            }
            else
            {
                std::u8string idxName = strName + u8'I' + strLang + u8".bin";
                std::u8string langName = strName + u8'L' + strLang + u8".bin";

                idxPath = outFolder / idxName;
                langPath = outFolder / langName;

                std::cout << " -> " << idxPath.string() << '\n';
                std::cout << " -> " << langPath.string() << '\n';

                try
                {
                    tfs.exportFile(idxPath, langPath);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "ERROR: Failed to open files: " << idxPath.string() << " and " << langPath.string() << " for writing.\n";
                    std::cerr << "Reason: " << e.what() << '\n';

                    processedEntries.push_back(strName);
                    continue;
                }
            }

            processedEntries.push_back(strName);
            continue;
        }

        return 0;
    }

    //
    // Batch imports ini-like formatted txt files (raw) and exports to story script index + lang pairs
    //
    int ImportFolderRaw(std::filesystem::path inFolder, std::filesystem::path outFolder)
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
                std::cerr << "ERROR: Folder " << outFolder.string() << " could not be created!\n";
                std::cerr << "Reason: " << e.what() << '\n';
                return -2;
            }
        }

        std::vector<std::u8string> processedEntries;

        //
        // expected filenames are in format:
        // <name>_<lang>.txt
        // <name>_<lang>.gz.txt
        //
        // <name> - arbitrary length
        // <lang> - 1 char - first letter of a western language in English, can be: j, e, g, f, i or s (Japanese, English, German, French, Italian or Spanish)
        //

        for (const auto& entry : std::filesystem::directory_iterator(inFolder))
        {
            if (entry.path().extension() != ".txt")
            {
                // std::cout << "Skipping file: " << entry.path() << '\n';
                continue;
            }

            bool bCompressed = false;
            int posType = 6;
            std::u8string strEntry = entry.path().filename().u8string();
            if (strEntry.find(u8".gz") != strEntry.npos)
            {
                bCompressed = true;
                posType += 3;
            }

            std::u8string strName = strEntry.substr(0, strEntry.size() - posType);
            std::u8string strTail = strEntry.substr(strEntry.size() - posType);
            std::u8string strUnderline = strTail.substr(0, 1);
            std::u8string strLang = strTail.substr(1, 1);

            std::cout << "Processing: " << (char*)strName.c_str() << '\n'
                << " <- " << entry.path().string() << '\n';

            if (strUnderline != u8"_")
            {
                std::cerr << "ERROR: File " << entry.path() << " does not follow the correct filename format!\n";
                std::cerr << "Reason: Missing underline character in filename!\n";

                continue;
            }

            // parse
            std::vector<std::string> strings;
            int errparse = TagForceString::ParseTxtRaw(entry.path(), &strings);

            if (errparse < 0)
            {
                std::cerr << "ERROR: Can't parse: " << entry.path() << '\n';

                processedEntries.push_back(strName);
                continue;
            }

            // build the data
            TFStoryScript tfs;
            tfs.build(&strings);

            std::filesystem::path idxPath;
            std::filesystem::path langPath;

            if (bCompressed)
            {
                std::u8string idxName = strName + u8'I' + strLang + u8".bin.gz";
                std::u8string langName = strName + u8'L' + strLang + u8".bin.gz";

                idxPath = outFolder / idxName;
                langPath = outFolder / langName;

                std::cout << " -> " << idxPath.string() << '\n';

                try
                {
                    ZLibWrapper::packGzFile(tfs.idxptr(), tfs.idxsize(), idxPath);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "ERROR: Can't compress data to: " << idxPath.string() << '\n';
                    std::cerr << "Reason: " << e.what() << '\n';

                    processedEntries.push_back(strName);
                    continue;
                }

                std::cout << " -> " << langPath.string() << '\n';

                try
                {
                    ZLibWrapper::packGzFile(tfs.fileptr(), tfs.datasize(), langPath);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "ERROR: Can't compress data to: " << langPath.string() << '\n';
                    std::cerr << "Reason: " << e.what() << '\n';

                    processedEntries.push_back(strName);
                    continue;
                }
            }
            else
            {
                std::u8string idxName = strName + u8'I' + strLang + u8".bin";
                std::u8string langName = strName + u8'L' + strLang + u8".bin";

                idxPath = outFolder / idxName;
                langPath = outFolder / langName;

                std::cout << " -> " << idxPath.string() << '\n';
                std::cout << " -> " << langPath.string() << '\n';

                try
                {
                    tfs.exportFile(idxPath, langPath);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "ERROR: Failed to open files: " << idxPath.string() << " and " << langPath.string() << " for writing.\n";
                    std::cerr << "Reason: " << e.what() << '\n';

                    processedEntries.push_back(strName);
                    continue;
                }
            }

            processedEntries.push_back(strName);
            continue;
        }

        return 0;
    }
}

#endif