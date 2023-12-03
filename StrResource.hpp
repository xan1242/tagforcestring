#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include "TagForceString.hpp"
#include "TFStringClasses.hpp"

#ifndef STRRESOURCE_HDR
#define STRRESOURCE_HDR

namespace StrResource
{
    //
    // Exports a string resource file (strtbl) to an ini-like formatted txt file (UTF-16)
    //
    int ExportU16(std::filesystem::path binFilename, std::filesystem::path txtFilename, bool bWriteBOM = true)
    {
        YgStringResource ysr;
        try
        {
            ysr.openFile(binFilename);
        }
        catch (const std::exception& e)
        {
            std::cerr << "ERROR: Failed to open file: " << binFilename.string() << " for reading.\n";
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

        if (bWriteBOM)
        {
            // write BOM
            txtfile.put(0xFF);
            txtfile.put(0xFE);
        }

        for (int i = 0; i < ysr.count(); i++)
        {
            // write section
            std::string sectionStr = '[' + std::to_string(i) + ']' + '\n';
            std::u16string u16section(sectionStr.begin(), sectionStr.end());
            txtfile.write((char*)u16section.data(), u16section.size() * sizeof(char16_t));

            // write data
            std::u16string u16data = ysr.u16string(i);
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
    // Exports a string resource file (strtbl) to an ini-like formatted txt file (UTF-8)
    //
    int ExportU8(std::filesystem::path binFilename, std::filesystem::path txtFilename, bool bWriteBOM = true)
    {
        YgStringResource ysr;
        try
        {
            ysr.openFile(binFilename);
        }
        catch (const std::exception& e)
        {
            std::cerr << "ERROR: Failed to open file: " << binFilename.string() << " for reading.\n";
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

        if (bWriteBOM)
        {
            // write BOM
            txtfile.put(0xEF);
            txtfile.put(0xBB);
            txtfile.put(0xBF);
        }

        for (int i = 0; i < ysr.count(); i++)
        {
            // write section
            std::string sectionStr = '[' + std::to_string(i) + ']' + '\n';
            std::u8string u8section(sectionStr.begin(), sectionStr.end());
            txtfile.write((char*)u8section.data(), u8section.size() * sizeof(char8_t));

            // write data
            std::u8string u8data = ysr.u8string(i);
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
    // Imports an ini-like formatted txt file (UTF-16) and exports to a string resource file (strtbl)
    //
    int ImportU16(std::filesystem::path txtFilename, std::filesystem::path binFilename)
    {
        std::vector<std::u16string> strings;
        int errcode = TagForceString::ParseTxtU16(txtFilename, &strings);
        if (errcode < 0)
        {
            std::cerr << "ERROR: Text parser failed with code " << errcode << '\n';
            return errcode;
        }

        YgStringResource ysr;
        ysr.build(&strings);

        try
        {
            ysr.exportFile(binFilename);
        }
        catch (const std::exception& e)
        {
            std::cerr << "ERROR: Failed to open file: " << binFilename.string() << " for writing.\n";
            std::cerr << "Reason: " << e.what() << '\n';
            return -2;
        }

        return 0;
    }


    //
    // Imports an ini-like formatted txt file (UTF-8) and exports to a string resource file (strtbl)
    //
    int ImportU8(std::filesystem::path txtFilename, std::filesystem::path binFilename)
    {
        std::vector<std::u8string> strings;
        int errcode = TagForceString::ParseTxtU8(txtFilename, &strings);
        if (errcode < 0)
        {
            std::cerr << "ERROR: Text parser failed with code " << errcode << '\n';
            return errcode;
        }

        YgStringResource ysr;
        ysr.build(&strings);

        try
        {
            ysr.exportFile(binFilename);
        }
        catch (const std::exception& e)
        {
            std::cerr << "ERROR: Failed to open file: " << binFilename.string() << " for writing.\n";
            std::cerr << "Reason: " << e.what() << '\n';
            return -2;
        }

        return 0;
    }
}

#endif