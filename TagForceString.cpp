//
// Yu-Gi-Oh! Tag Force Language & String Tool
// by Xan / Tenjoin
//

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <functional>
#include <charconv>
#include "TagForceString.hpp"

namespace TagForceString
{
    enum UnicodeBOMType
    {
        BOM_UNKNOWN,
        BOM_UTF8,
        BOM_UTF16LE,
        BOM_UTF16BE,
        BOM_COUNT
    };

    UnicodeBOMType GetBOM(std::ifstream& file)
    {
        std::streampos oldpos = file.tellg();

        uint8_t bomchk1 = file.get();
        uint8_t bomchk2 = file.get();
        uint8_t bomchk3 = file.get();

        file.seekg(oldpos, std::ios::beg);

        if ((bomchk1 == 0xEF) && (bomchk2 == 0xBB) && (bomchk3 == 0xBF))
            return UnicodeBOMType::BOM_UTF8;

        uint16_t bomchk = (uint16_t)(bomchk2 << 8) | bomchk1;
        if (bomchk == 0xFFFE)
            return UnicodeBOMType::BOM_UTF16BE;

        if (bomchk == 0xFEFF)
            return UnicodeBOMType::BOM_UTF16LE;

        return UnicodeBOMType::BOM_UNKNOWN;
    }

    UnicodeBOMType GetBOM(std::filesystem::path filename)
    {
        std::ifstream file;
        file.open(filename, std::ios::binary);
        try
        {
            file.open(filename, std::ios::binary);
            if (!file.is_open())
            {
                throw std::runtime_error(strerror(errno));
            }
        }
        catch (const std::exception& e)
        {
            throw e;
        }

        UnicodeBOMType result = GetBOM(file);

        file.close();
    }

    std::u16string readlineu16(std::ifstream& file)
    {
        std::u16string line;
        char16_t ch;

        // Read until newline character or end of file
        file.read((char*)&ch, sizeof(char16_t));
        while (ch && ch != L'\n')
        {
            // Check for surrogate pair
            if (ch >= 0xD800 && ch <= 0xDBFF) 
            {
                // This is the first code unit of a surrogate pair
                char16_t secondUnit;
                file.read((char*)&secondUnit, sizeof(char16_t));
                if (secondUnit && secondUnit >= 0xDC00 && secondUnit <= 0xDFFF)
                {
                    // Valid surrogate pair
                    line.push_back(ch);
                    line.push_back(secondUnit);
                }
                else 
                {
                    // Invalid surrogate pair, handle error or break the loop
                    break;
                }
            }
            else 
            {
                // Single code unit character
                line.push_back(ch);
            }

            file.read((char*)&ch, sizeof(char16_t));
        }

        return line;
    }

    std::u8string readlineu8(std::ifstream& file) 
    {
        std::u8string utf8Line;
        char buffer[4];  // UTF-8 characters can be up to 4 bytes
        char c;
        while (file.get(c) && c != '\n') 
        {
            if ((c & 0xC0) != 0x80) 
            {  
                // Check if it is the start of a UTF-8 character
                int bytesRead = 0;
                while ((c & 0xC0) == 0x80) 
                {
                    buffer[bytesRead++] = c;
                    file.get(c);
                }

                // Add the start byte
                buffer[bytesRead++] = c;
                buffer[bytesRead] = '\0';

                // Convert the buffer to a UTF-8 character and append to the u8string
                utf8Line += reinterpret_cast<const char8_t*>(buffer);
            }
            else 
            {
                // Handle single-byte character
                utf8Line += static_cast<char8_t>(c);
            }
        }

        return utf8Line;
    }

    void removeCRLF(std::u16string& str) 
    {
        if (!str.empty()) 
        {
            if (str.back() == u'\n') 
            {
                str.pop_back();
            }

            if (!str.empty() && str.back() == u'\r') 
            {
                str.pop_back();
            }
        }
    }

    void removeCRLF(std::u8string& str)
    {
        if (!str.empty())
        {
            if (str.back() == '\n')
            {
                str.pop_back();
            }

            if (!str.empty() && str.back() == '\r')
            {
                str.pop_back();
            }
        }
    }

    //
    // Parses an ini-like (UTF-16 LE BOM) formatted txt file and returns a vector to the given pointer.
    //
    int ParseTxtU16(std::filesystem::path txtFilename, std::vector<std::u16string>* outStrings)
    {
        std::ifstream txtfile;
        try
        {
            txtfile.open(txtFilename, std::ios::binary);
        }
        catch (const std::exception& e)
        {
            std::cerr << "ERROR: Failed to open file: " << txtFilename.string() << " for reading.\n";
            std::cerr << "Reason: " << e.what() << '\n';
            return -1;
        }

        // check BOM and skip if valid...
        UnicodeBOMType bt = GetBOM(txtfile);
        if (bt == UnicodeBOMType::BOM_UTF16BE)
        {
            txtfile.close();
            std::cerr << "Big endian BOM detected! Please only use little endian files!\n";
            return -2;
        }

        if (bt == UnicodeBOMType::BOM_UTF16LE)
            txtfile.seekg(2, std::ios::beg);
        else
            std::cout << "WARNING: Unknown or no BOM detected!\n";

        // create a map of strings per index to keep them in order
        std::map<int, std::u16string> strMap;

        while (!txtfile.eof())
        {
            std::u16string line = readlineu16(txtfile);
            // trim any newline chars
            line.erase(std::find_if(line.rbegin(), line.rend(), std::not_fn(std::function<int(int)>(::isspace))).base(), line.end());
            if (!(!line.empty() && line.front() == u'[' && line.back() == u']'))
                continue;

            std::u16string idStrU16 = line.substr(1, line.length() - 2);
            std::string idStr(idStrU16.begin(), idStrU16.end());
            int id = std::stoi(idStr);
            std::u16string data;
            while (!txtfile.eof())
            {
                char16_t ch;
                txtfile.read((char*)&ch, sizeof(char16_t));
                if (ch == u'[')
                {
                    txtfile.seekg(-static_cast<std::streamoff>(sizeof(char16_t)), std::ios::cur);
                    break;
                }
                if (txtfile.eof())
                    break;
                data.push_back(ch);
            }

            removeCRLF(data);
            strMap[id] = data;
        }

        // copy strings in index order
        for (const auto& pair : strMap)
        {
            outStrings->push_back(pair.second);
        }


        return 0;
    }

    //
    // Parses an ini-like (UTF-8) formatted txt file and returns a vector to the given pointer.
    //
    int ParseTxtU8(std::filesystem::path txtFilename, std::vector<std::u8string>* outStrings)
    {
        std::ifstream txtfile;
        try
        {
            txtfile.open(txtFilename, std::ios::binary);
        }
        catch (const std::exception& e)
        {
            std::cerr << "ERROR: Failed to open file: " << txtFilename.string() << " for reading.\n";
            std::cerr << "Reason: " << e.what() << '\n';
            return -1;
        }

        // check BOM and skip if valid...
        UnicodeBOMType bt = GetBOM(txtfile);
        if ((bt == UnicodeBOMType::BOM_UTF16LE) || (bt == UnicodeBOMType::BOM_UTF16BE))
        {
            txtfile.close();
            std::cerr << "UTF-16 BOM detected! Please check that you're using a UTF-8 file!\n";
            return -2;
        }
        if (bt == UnicodeBOMType::BOM_UTF8)
            txtfile.seekg(3, std::ios::beg);
        else
            std::cout << "WARNING: Unknown or no BOM detected!\n";

        // create a map of strings per index to keep them in order
        std::map<int, std::u8string> strMap;

        while (!txtfile.eof())
        {
            std::u8string line = readlineu8(txtfile);
            // trim any newline chars
            line.erase(std::find_if(line.rbegin(), line.rend(), std::not_fn(std::function<int(int)>(::isspace))).base(), line.end());
            if (!(!line.empty() && line.front() == '[' && line.back() == ']'))
                continue;

            std::u8string idStrU8 = line.substr(1, line.length() - 2);
            std::string idStr(idStrU8.begin(), idStrU8.end());
            int id = std::stoi(idStr);
            std::u8string data;
            while (!txtfile.eof())
            {
                char8_t ch;
                txtfile.read((char*)&ch, sizeof(char8_t));
                if (ch == '[')
                {
                    txtfile.seekg(-static_cast<std::streamoff>(sizeof(char8_t)), std::ios::cur);
                    break;
                }
                if (txtfile.eof())
                    break;
                data.push_back(ch);
            }

            removeCRLF(data);
            strMap[id] = data;
        }

        // copy strings in index order
        for (const auto& pair : strMap)
        {
            outStrings->push_back(pair.second);
        }


        return 0;
    }
}

namespace StrResource
{
    //
    // Exports a string resource file (strtbl) to an ini-like formatted txt file (UTF-16)
    //
    int ExportU16(std::filesystem::path binFilename, std::filesystem::path txtFilename)
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

        // write BOM
        txtfile.put(0xFF);
        txtfile.put(0xFE);

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
    int ExportU8(std::filesystem::path binFilename, std::filesystem::path txtFilename)
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

        // write BOM
        txtfile.put(0xEF);
        txtfile.put(0xBB);
        txtfile.put(0xBF);

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
int main(int argc, char* argv[])
{
    std::cout << "Yu-Gi-Oh! Tag Force Language & String Tool\n";

    //int errcode = StrResource::ExportU16("teststrings.bin", "test2.txt");
    //int errcode = TagForceString::ParseTxtU16(argv[2], nullptr);
    //int errcode = StrResource::ImportU16(argv[2], "teststrings.bin");
    //int errcode = StrResource::ImportU8("u8test.txt", "u8teststrings.bin");
    int errcode = StrResource::ExportU8("u8teststrings.bin", "u8test2.txt");

    return errcode;
}
