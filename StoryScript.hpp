#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include "TagForceString.hpp"
#include "TFStringClasses.hpp"

#ifndef STORYSCRIPT_HDR
#define STORYSCRIPT_HDR

namespace StoryScript
{
    //
    // Exports a story script index + lang pair to an ini-like formatted txt file (UTF-16)
    //
    int ExportU16(std::filesystem::path idxFilename, std::filesystem::path langFilename, std::filesystem::path txtFilename)
    {
        TFStoryScript tfs;
        try
        {
            tfs.openFile(idxFilename, langFilename);
        }
        catch (const std::exception& e)
        {
            std::cerr << "ERROR: Failed to open files: " << idxFilename.string() << " and " << langFilename.string() << " for reading.\n";
            std::cerr << "Reason: " << e.what() << '\n';
            return -1;
        }

        std::ofstream txtfile;
        try
        {
            txtfile.open(txtFilename, std::ios::out | std::ios::binary);
        }
        catch (const std::exception& e)
        {
            std::cerr << "ERROR: Failed to open file: " << txtFilename.string() << " for writing.\n";
            std::cerr << "Reason: " << e.what() << '\n';
            return -2;
        }

        // write BOM
        txtfile.put(0xFF);
        txtfile.put(0xFE);

        for (int i = 0; i < tfs.count(); i++)
        {
            // write section
            std::string sectionStr = '[' + std::to_string(i) + ']' + '\n';
            std::u16string u16section(sectionStr.begin(), sectionStr.end());
            txtfile.write((char*)u16section.data(), u16section.size() * sizeof(char16_t));

            // write data
            std::u16string u16data = tfs.u16string(i);
            txtfile.write((char*)u16data.data(), u16data.size() * sizeof(char16_t));

            // newline for next section
            char16_t nl = u'\n';
            txtfile.write((char*)&nl, sizeof(char16_t));

            txtfile.flush();
        }

        txtfile.flush();
        txtfile.close();

        return 0;
    }

    //
    // Exports a story script index + lang pair to an ini-like formatted txt file (UTF-8)
    //
    int ExportU8(std::filesystem::path idxFilename, std::filesystem::path langFilename, std::filesystem::path txtFilename)
    {
        TFStoryScript tfs;
        try
        {
            tfs.openFile(idxFilename, langFilename);
        }
        catch (const std::exception& e)
        {
            std::cerr << "ERROR: Failed to open files: " << idxFilename.string() << " and " << langFilename.string() << " for reading.\n";
            std::cerr << "Reason: " << e.what() << '\n';
            return -1;
        }

        std::ofstream txtfile;
        try
        {
            txtfile.open(txtFilename, std::ios::out | std::ios::binary);
        }
        catch (const std::exception& e)
        {
            std::cerr << "ERROR: Failed to open file: " << txtFilename.string() << " for writing.\n";
            std::cerr << "Reason: " << e.what() << '\n';
            return -2;
        }

        // write BOM
        txtfile.put(0xEF);
        txtfile.put(0xBB);
        txtfile.put(0xBF);

        for (int i = 0; i < tfs.count(); i++)
        {
            // write section
            std::string sectionStr = '[' + std::to_string(i) + ']' + '\n';
            std::u8string u8section(sectionStr.begin(), sectionStr.end());
            txtfile.write((char*)u8section.data(), u8section.size() * sizeof(char8_t));

            // write data
            std::u8string u8data = tfs.u8string(i);
            txtfile.write((char*)u8data.data(), u8data.size() * sizeof(char8_t));

            // newline for next section
            char8_t nl = '\n';
            txtfile.write((char*)&nl, sizeof(char8_t));

            txtfile.flush();
        }

        txtfile.flush();
        txtfile.close();

        return 0;
    }

    //
    // Imports an ini-like formatted txt file (UTF-16) and exports to a story script index + lang pair
    //
    int ImportU16(std::filesystem::path txtFilename, std::filesystem::path idxFilename, std::filesystem::path langFilename)
    {
        std::vector<std::u16string> strings;
        int errcode = TagForceString::ParseTxtU16(txtFilename, &strings);
        if (errcode < 0)
        {
            std::cerr << "ERROR: Text parser failed with code " << errcode << '\n';
            return errcode;
        }

        TFStoryScript tfs;
        tfs.build(&strings);

        try
        {
            tfs.exportFile(idxFilename, langFilename);
        }
        catch (const std::exception& e)
        {
            std::cerr << "ERROR: Failed to open files: " << idxFilename.string() << " and " << langFilename.string() << " for writing.\n";
            std::cerr << "Reason: " << e.what() << '\n';
            return -2;
        }

        return 0;
    }

    //
    // Imports an ini-like formatted txt file (UTF-8) and exports to a story script index + lang pair
    //
    int ImportU8(std::filesystem::path txtFilename, std::filesystem::path idxFilename, std::filesystem::path langFilename)
    {
        std::vector<std::u8string> strings;
        int errcode = TagForceString::ParseTxtU8(txtFilename, &strings);
        if (errcode < 0)
        {
            std::cerr << "ERROR: Text parser failed with code " << errcode << '\n';
            return errcode;
        }

        TFStoryScript tfs;
        tfs.build(&strings);

        try
        {
            tfs.exportFile(idxFilename, langFilename);
        }
        catch (const std::exception& e)
        {
            std::cerr << "ERROR: Failed to open files: " << idxFilename.string() << " and " << langFilename.string() << " for writing.\n";
            std::cerr << "Reason: " << e.what() << '\n';
            return -2;
        }

        return 0;
    }
}

#endif