#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include "TagForceString.hpp"
#include "TFStringClasses.hpp"

#ifndef TXTRESOURCE_HDR
#define TXTRESOURCE_HDR

namespace TxtResource
{
    //
    // Exports a text resource file to an ini-like formatted txt file (UTF-16)
    //
    int ExportU16(std::filesystem::path binFilename, std::filesystem::path txtFilename, bool bWriteBOM = true)
    {
        YgTextResource ytr;
        try
        {
            ytr.openFile(binFilename);
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

        for (int i = 0; i < ytr.count(); i++)
        {
            // write section
            std::string sectionStr = '[' + std::to_string(i) + ']' + '\n';
            std::u16string u16section(sectionStr.begin(), sectionStr.end());
            txtfile.write((char*)u16section.data(), u16section.size() * sizeof(char16_t));

            // write data
            std::u16string u16data = ytr.u16string(i);
            u16data = TagForceString::escapeCharacter(u16data, u'\\');
            u16data = TagForceString::escapeCharacter(u16data, u'[');
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
    // Exports a text resource file to an ini-like formatted txt file (UTF-8)
    //
    int ExportU8(std::filesystem::path binFilename, std::filesystem::path txtFilename, bool bWriteBOM = true)
    {
        YgTextResource ytr;
        try
        {
            ytr.openFile(binFilename);
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

        for (int i = 0; i < ytr.count(); i++)
        {
            // write section
            std::string sectionStr = '[' + std::to_string(i) + ']' + '\n';
            std::u8string u8section(sectionStr.begin(), sectionStr.end());
            txtfile.write((char*)u8section.data(), u8section.size() * sizeof(char8_t));

            // write data
            std::u8string u8data = ytr.u8string(i);
            u8data = TagForceString::escapeCharacter(u8data, u8'\\');
            u8data = TagForceString::escapeCharacter(u8data, u8'[');
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
    // Exports a text resource file to an ini-like formatted txt file (raw data)
    //
    int ExportRaw(std::filesystem::path binFilename, std::filesystem::path txtFilename)
    {
        YgTextResource ytr;
        try
        {
            ytr.openFile(binFilename);
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

        for (int i = 0; i < ytr.count(); i++)
        {
            // write section
            std::string sectionStr = '[' + std::to_string(i) + ']' + '\n';
            txtfile.write(sectionStr.data(), sectionStr.size());

            // write data
            
            // skip zeros
            uintmax_t datasize = ytr.itemsize(i);
            char* data = ytr.c_str(i);
            while (data[datasize-1] == '\0')
                datasize--;

            txtfile.write(ytr.c_str(i), datasize);

            // newline for next section
            char nl = '\n';
            txtfile.write(&nl, sizeof(char));

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

        YgTextResource ytr;
        ytr.build(&strings);

        try
        {
            ytr.exportFile(binFilename);
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

        YgTextResource ytr;
        ytr.build(&strings);

        try
        {
            ytr.exportFile(binFilename);
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
    // Imports an ini-like formatted txt file (raw) and exports to a string resource file (strtbl)
    //
    int ImportRaw(std::filesystem::path txtFilename, std::filesystem::path binFilename)
    {
        std::vector<std::string> strings;
        int errcode = TagForceString::ParseTxtRaw(txtFilename, &strings);
        if (errcode < 0)
        {
            std::cerr << "ERROR: Text parser failed with code " << errcode << '\n';
            return errcode;
        }

        YgTextResource ytr;
        ytr.build(&strings);

        try
        {
            ytr.exportFile(binFilename);
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