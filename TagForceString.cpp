//
// Yu-Gi-Oh! Tag Force Language & String Tool
// by Xan / Tenjoin
//

#include <iostream>
// #include <fstream>
// #include <filesystem>
// #include <string>
// #include <sstream>
// #include <map>
// #include <vector>
// #include <functional>
// #include <charconv>
#include "TagForceString.hpp"
#include "StrResource.hpp"
#include "StoryScript.hpp"
#include "TF1Folder.hpp"

// namespace TagForceString
// {
//     enum OperatingMode 
//     {
//         BIN2TXT,
//         TXT2BIN,
//         LANG2TXT,
//         TXT2LANG
//     };
// 
//     struct Options 
//     {
//         OperatingMode mode;
//         std::filesystem::path inputFilePath1;
//         std::filesystem::path inputFilePath2;
//         std::filesystem::path outputFilePath1;
//         std::filesystem::path outputFilePath2;
//         bool useUTF8 = false;       // Default is UTF-16
//         bool autodetectBOM = true;  // Default is autodetection
//     };
// 
//     void printUsage(const char* programName) 
//     {
//         std::cerr << "Usage: " << programName << " [OPTIONS] MODE INPUT OUTPUT\n"
//             << "\nOPTIONS:\n"
//             << "  -u, --utf8          Use UTF-8 encoding (default is UTF-16)\n"
//             << "  -d, --no-autodetect Disable BOM autodetection for input text files\n"
//             << "\nMODES:\n"
//             << "  1. bin2txt           Convert a string resource (strtbl) file to a text file\n"
//             << "  2. txt2bin           Convert a text file to a string resource (strtbl) file\n"
//             << "  3. lang2txt          Convert a pair of lang files (index and strings) to a text file\n"
//             << "  4. txt2lang          Convert a text file to a pair of lang files (index and strings)\n"
//             << "\nEXAMPLES:\n"
//             << "  " << programName << " bin2txt input_e.bin output.txt\n"
//             << "  " << programName << " txt2bin input.txt output_e.bin\n"
//             << "  " << programName << " lang2txt langIe.bin langLe.bin output.txt\n"
//             << "  " << programName << " --utf8 txt2lang input.txt outIe.bin outLe.bin\n"
//             << '\n' << "The encoding must match on both input and output files! The tool does not perform any conversion!\n";
//     }
// 
//     Options parseCommandLine(int argc, char* argv[]) 
//     {
//         Options options;
// 
//         for (int i = 1; i < argc; ++i) 
//         {
//             std::string arg = argv[i];
// 
//             if (arg == "-u" || arg == "--utf8") 
//             {
//                 options.useUTF8 = true;
//             }
//             else if (arg == "-d" || arg == "--no-autodetect") 
//             {
//                 options.autodetectBOM = false;
//             }
//             else if (arg == "bin2txt") 
//             {
//                 options.mode = BIN2TXT;
//             }
//             else if (arg == "txt2bin") 
//             {
//                 options.mode = TXT2BIN;
//             }
//             else if (arg == "lang2txt") 
//             {
//                 options.mode = LANG2TXT;
//                 if (i + 3 < argc) 
//                 {
//                     options.inputFilePath1 = argv[++i];
//                     options.inputFilePath2 = argv[++i];
//                     options.outputFilePath1 = argv[++i];
//                 }
//                 else 
//                 {
//                     std::cerr << "Insufficient arguments for lang2txt. Use '" << argv[0] << "' for help.\n";
//                     //printUsage(argv[0]);
//                     exit(1);
//                 }
//             }
//             else if (arg == "txt2lang") 
//             {
//                 options.mode = TXT2LANG;
//                 if (i + 3 < argc) 
//                 {
//                     options.inputFilePath1 = argv[++i];
//                     options.outputFilePath1 = argv[++i];
//                     options.outputFilePath2 = argv[++i];
//                 }
//                 else 
//                 {
//                     std::cerr << "Insufficient arguments for txt2lang. Use '" << argv[0] << "' for help.\n";
//                     //printUsage(argv[0]);
//                     exit(1);
//                 }
//             }
//             else if (i + 2 <= argc) 
//             {
//                 options.inputFilePath1 = argv[i++];
//                 options.outputFilePath1 = argv[i++];
//             }
//             else 
//             {
//                 std::cerr << "Invalid arguments. Use '" << argv[0] << "' for help.\n";
//                 //printUsage(argv[0]);
//                 exit(1);
//             }
//         }
// 
//         return options;
//     }
// 
//     enum UnicodeBOMType
//     {
//         BOM_UNKNOWN,
//         BOM_UTF8,
//         BOM_UTF16LE,
//         BOM_UTF16BE,
//         BOM_COUNT
//     };
// 
//     UnicodeBOMType GetBOM(std::ifstream& file)
//     {
//         std::streampos oldpos = file.tellg();
// 
//         uint8_t bomchk1 = file.get();
//         uint8_t bomchk2 = file.get();
//         uint8_t bomchk3 = file.get();
// 
//         file.seekg(oldpos, std::ios::beg);
// 
//         if ((bomchk1 == 0xEF) && (bomchk2 == 0xBB) && (bomchk3 == 0xBF))
//             return UnicodeBOMType::BOM_UTF8;
// 
//         uint16_t bomchk = (uint16_t)(bomchk2 << 8) | bomchk1;
//         if (bomchk == 0xFFFE)
//             return UnicodeBOMType::BOM_UTF16BE;
// 
//         if (bomchk == 0xFEFF)
//             return UnicodeBOMType::BOM_UTF16LE;
// 
//         return UnicodeBOMType::BOM_UNKNOWN;
//     }
// 
//     UnicodeBOMType GetBOM(std::filesystem::path filename)
//     {
//         std::ifstream file;
//         try
//         {
//             file.open(filename, std::ios::binary);
//             if (!file.is_open())
//             {
//                 throw std::runtime_error(strerror(errno));
//             }
//         }
//         catch (const std::exception& e)
//         {
//             throw e;
//         }
// 
//         UnicodeBOMType result = GetBOM(file);
// 
//         file.close();
// 
//         return result;
//     }
// 
//     std::u16string readlineu16(std::ifstream& file)
//     {
//         std::u16string line;
//         char16_t ch;
// 
//         // Read until newline character or end of file
//         file.read((char*)&ch, sizeof(char16_t));
//         while (ch && ch != L'\n')
//         {
//             // Check for surrogate pair
//             if (ch >= 0xD800 && ch <= 0xDBFF) 
//             {
//                 // This is the first code unit of a surrogate pair
//                 char16_t secondUnit;
//                 file.read((char*)&secondUnit, sizeof(char16_t));
//                 if (secondUnit && secondUnit >= 0xDC00 && secondUnit <= 0xDFFF)
//                 {
//                     // Valid surrogate pair
//                     line.push_back(ch);
//                     line.push_back(secondUnit);
//                 }
//                 else 
//                 {
//                     // Invalid surrogate pair, handle error or break the loop
//                     break;
//                 }
//             }
//             else 
//             {
//                 // Single code unit character
//                 line.push_back(ch);
//             }
// 
//             file.read((char*)&ch, sizeof(char16_t));
//         }
// 
//         return line;
//     }
// 
//     std::u8string readlineu8(std::ifstream& file) 
//     {
//         std::u8string utf8Line;
//         char buffer[4];  // UTF-8 characters can be up to 4 bytes
//         char c;
//         while (file.get(c) && c != '\n') 
//         {
//             if ((c & 0xC0) != 0x80) 
//             {  
//                 // Check if it is the start of a UTF-8 character
//                 int bytesRead = 0;
//                 while ((c & 0xC0) == 0x80) 
//                 {
//                     buffer[bytesRead++] = c;
//                     file.get(c);
//                 }
// 
//                 // Add the start byte
//                 buffer[bytesRead++] = c;
//                 buffer[bytesRead] = '\0';
// 
//                 // Convert the buffer to a UTF-8 character and append to the u8string
//                 utf8Line += reinterpret_cast<const char8_t*>(buffer);
//             }
//             else 
//             {
//                 // Handle single-byte character
//                 utf8Line += static_cast<char8_t>(c);
//             }
//         }
// 
//         return utf8Line;
//     }
// 
//     void removeCRLF(std::u16string& str) 
//     {
//         if (!str.empty()) 
//         {
//             if (str.back() == u'\n') 
//             {
//                 str.pop_back();
//             }
// 
//             if (!str.empty() && str.back() == u'\r') 
//             {
//                 str.pop_back();
//             }
//         }
//     }
// 
//     void removeCRLF(std::u8string& str)
//     {
//         if (!str.empty())
//         {
//             if (str.back() == '\n')
//             {
//                 str.pop_back();
//             }
// 
//             if (!str.empty() && str.back() == '\r')
//             {
//                 str.pop_back();
//             }
//         }
//     }
// 
//     //
//     // Parses an ini-like (UTF-16 LE BOM) formatted txt file and returns a vector to the given pointer.
//     //
//     int ParseTxtU16(std::filesystem::path txtFilename, std::vector<std::u16string>* outStrings)
//     {
//         std::ifstream txtfile;
//         try
//         {
//             txtfile.open(txtFilename, std::ios::binary);
//         }
//         catch (const std::exception& e)
//         {
//             std::cerr << "ERROR: Failed to open file: " << txtFilename.string() << " for reading.\n";
//             std::cerr << "Reason: " << e.what() << '\n';
//             return -1;
//         }
// 
//         // check BOM and skip if valid...
//         UnicodeBOMType bt = GetBOM(txtfile);
//         if (bt == UnicodeBOMType::BOM_UTF16BE)
//         {
//             txtfile.close();
//             std::cerr << "Big endian BOM detected! Please only use little endian files!\n";
//             return -2;
//         }
// 
//         if (bt == UnicodeBOMType::BOM_UTF16LE)
//             txtfile.seekg(2, std::ios::beg);
//         else
//             std::cout << "WARNING: Unknown or no BOM detected!\n";
// 
//         // create a map of strings per index to keep them in order
//         std::map<int, std::u16string> strMap;
// 
//         while (!txtfile.eof())
//         {
//             std::u16string line = readlineu16(txtfile);
//             // trim any newline chars
//             line.erase(std::find_if(line.rbegin(), line.rend(), std::not_fn(std::function<int(int)>(::isspace))).base(), line.end());
//             if (!(!line.empty() && line.front() == u'[' && line.back() == u']'))
//                 continue;
// 
//             std::u16string idStrU16 = line.substr(1, line.length() - 2);
//             std::string idStr(idStrU16.begin(), idStrU16.end());
//             int id = std::stoi(idStr);
//             std::u16string data;
//             while (!txtfile.eof())
//             {
//                 char16_t ch;
//                 txtfile.read((char*)&ch, sizeof(char16_t));
//                 if (ch == u'[')
//                 {
//                     txtfile.seekg(-static_cast<std::streamoff>(sizeof(char16_t)), std::ios::cur);
//                     break;
//                 }
//                 if (txtfile.eof())
//                     break;
//                 data.push_back(ch);
//             }
// 
//             removeCRLF(data);
//             strMap[id] = data;
//         }
// 
//         // copy strings in index order
//         for (const auto& pair : strMap)
//         {
//             outStrings->push_back(pair.second);
//         }
// 
// 
//         return 0;
//     }
// 
//     //
//     // Parses an ini-like (UTF-8) formatted txt file and returns a vector to the given pointer.
//     //
//     int ParseTxtU8(std::filesystem::path txtFilename, std::vector<std::u8string>* outStrings)
//     {
//         std::ifstream txtfile;
//         try
//         {
//             txtfile.open(txtFilename, std::ios::binary);
//         }
//         catch (const std::exception& e)
//         {
//             std::cerr << "ERROR: Failed to open file: " << txtFilename.string() << " for reading.\n";
//             std::cerr << "Reason: " << e.what() << '\n';
//             return -1;
//         }
// 
//         // check BOM and skip if valid...
//         UnicodeBOMType bt = GetBOM(txtfile);
//         if ((bt == UnicodeBOMType::BOM_UTF16LE) || (bt == UnicodeBOMType::BOM_UTF16BE))
//         {
//             txtfile.close();
//             std::cerr << "UTF-16 BOM detected! Please check that you're using a UTF-8 file!\n";
//             return -2;
//         }
//         if (bt == UnicodeBOMType::BOM_UTF8)
//             txtfile.seekg(3, std::ios::beg);
//         else
//             std::cout << "WARNING: Unknown or no BOM detected!\n";
// 
//         // create a map of strings per index to keep them in order
//         std::map<int, std::u8string> strMap;
// 
//         while (!txtfile.eof())
//         {
//             std::u8string line = readlineu8(txtfile);
//             // trim any newline chars
//             line.erase(std::find_if(line.rbegin(), line.rend(), std::not_fn(std::function<int(int)>(::isspace))).base(), line.end());
//             if (!(!line.empty() && line.front() == '[' && line.back() == ']'))
//                 continue;
// 
//             std::u8string idStrU8 = line.substr(1, line.length() - 2);
//             std::string idStr(idStrU8.begin(), idStrU8.end());
//             int id = std::stoi(idStr);
//             std::u8string data;
//             while (!txtfile.eof())
//             {
//                 char8_t ch;
//                 txtfile.read((char*)&ch, sizeof(char8_t));
//                 if (ch == '[')
//                 {
//                     txtfile.seekg(-static_cast<std::streamoff>(sizeof(char8_t)), std::ios::cur);
//                     break;
//                 }
//                 if (txtfile.eof())
//                     break;
//                 data.push_back(ch);
//             }
// 
//             removeCRLF(data);
//             strMap[id] = data;
//         }
// 
//         // copy strings in index order
//         for (const auto& pair : strMap)
//         {
//             outStrings->push_back(pair.second);
//         }
// 
// 
//         return 0;
//     }
// }
// 
// namespace StrResource
// {
//     //
//     // Exports a string resource file (strtbl) to an ini-like formatted txt file (UTF-16)
//     //
//     int ExportU16(std::filesystem::path binFilename, std::filesystem::path txtFilename)
//     {
//         YgStringResource ysr;
//         try
//         {
//             ysr.openFile(binFilename);
//         }
//         catch (const std::exception& e)
//         {
//             std::cerr << "ERROR: Failed to open file: " << binFilename.string() << " for reading.\n";
//             std::cerr << "Reason: " << e.what() << '\n';
//             return -1;
//         }
// 
//         std::ofstream txtfile;
//         try
//         {
//             txtfile.open(txtFilename, std::ios::out | std::ios::binary);
//         }
//         catch (const std::exception& e)
//         {
//             std::cerr << "ERROR: Failed to open file: " << txtFilename.string() << " for writing.\n";
//             std::cerr << "Reason: " << e.what() << '\n';
//             return -2;
//         }
// 
//         // write BOM
//         txtfile.put(0xFF);
//         txtfile.put(0xFE);
// 
//         for (int i = 0; i < ysr.count(); i++)
//         {
//             // write section
//             std::string sectionStr = '[' + std::to_string(i) + ']' + '\n';
//             std::u16string u16section(sectionStr.begin(), sectionStr.end());
//             txtfile.write((char*)u16section.data(), u16section.size() * sizeof(char16_t));
// 
//             // write data
//             std::u16string u16data = ysr.u16string(i);
//             txtfile.write((char*)u16data.data(), u16data.size() * sizeof(char16_t));
// 
//             // newline for next section
//             char16_t nl = u'\n';
//             txtfile.write((char*)&nl, sizeof(char16_t));
//                 
//             txtfile.flush();
//         }
// 
//         txtfile.flush();
//         txtfile.close();
// 
//         return 0;
//     }
// 
//     //
//     // Exports a string resource file (strtbl) to an ini-like formatted txt file (UTF-8)
//     //
//     int ExportU8(std::filesystem::path binFilename, std::filesystem::path txtFilename)
//     {
//         YgStringResource ysr;
//         try
//         {
//             ysr.openFile(binFilename);
//         }
//         catch (const std::exception& e)
//         {
//             std::cerr << "ERROR: Failed to open file: " << binFilename.string() << " for reading.\n";
//             std::cerr << "Reason: " << e.what() << '\n';
//             return -1;
//         }
// 
//         std::ofstream txtfile;
//         try
//         {
//             txtfile.open(txtFilename, std::ios::out | std::ios::binary);
//         }
//         catch (const std::exception& e)
//         {
//             std::cerr << "ERROR: Failed to open file: " << txtFilename.string() << " for writing.\n";
//             std::cerr << "Reason: " << e.what() << '\n';
//             return -2;
//         }
// 
//         // write BOM
//         txtfile.put(0xEF);
//         txtfile.put(0xBB);
//         txtfile.put(0xBF);
// 
//         for (int i = 0; i < ysr.count(); i++)
//         {
//             // write section
//             std::string sectionStr = '[' + std::to_string(i) + ']' + '\n';
//             std::u8string u8section(sectionStr.begin(), sectionStr.end());
//             txtfile.write((char*)u8section.data(), u8section.size() * sizeof(char8_t));
// 
//             // write data
//             std::u8string u8data = ysr.u8string(i);
//             txtfile.write((char*)u8data.data(), u8data.size() * sizeof(char8_t));
// 
//             // newline for next section
//             char8_t nl = '\n';
//             txtfile.write((char*)&nl, sizeof(char8_t));
// 
//             txtfile.flush();
//         }
// 
//         txtfile.flush();
//         txtfile.close();
// 
//         return 0;
//     }
// 
//     //
//     // Imports an ini-like formatted txt file (UTF-16) and exports to a string resource file (strtbl)
//     //
//     int ImportU16(std::filesystem::path txtFilename, std::filesystem::path binFilename)
//     {
//         std::vector<std::u16string> strings;
//         int errcode = TagForceString::ParseTxtU16(txtFilename, &strings);
//         if (errcode < 0)
//         {
//             std::cerr << "ERROR: Text parser failed with code " << errcode << '\n';
//             return errcode;
//         }
// 
//         YgStringResource ysr;
//         ysr.build(&strings);
// 
//         try
//         {
//             ysr.exportFile(binFilename);
//         }
//         catch (const std::exception& e)
//         {
//             std::cerr << "ERROR: Failed to open file: " << binFilename.string() << " for writing.\n";
//             std::cerr << "Reason: " << e.what() << '\n';
//             return -2;
//         }
// 
//         return 0;
//     }
// 
// 
//     //
//     // Imports an ini-like formatted txt file (UTF-8) and exports to a string resource file (strtbl)
//     //
//     int ImportU8(std::filesystem::path txtFilename, std::filesystem::path binFilename)
//     {
//         std::vector<std::u8string> strings;
//         int errcode = TagForceString::ParseTxtU8(txtFilename, &strings);
//         if (errcode < 0)
//         {
//             std::cerr << "ERROR: Text parser failed with code " << errcode << '\n';
//             return errcode;
//         }
// 
//         YgStringResource ysr;
//         ysr.build(&strings);
// 
//         try
//         {
//             ysr.exportFile(binFilename);
//         }
//         catch (const std::exception& e)
//         {
//             std::cerr << "ERROR: Failed to open file: " << binFilename.string() << " for writing.\n";
//             std::cerr << "Reason: " << e.what() << '\n';
//             return -2;
//         }
// 
//         return 0;
//     }
// }
// 
// namespace StoryScript
// {
//     //
//     // Exports a story script index + lang pair to an ini-like formatted txt file (UTF-16)
//     //
//     int ExportU16(std::filesystem::path idxFilename, std::filesystem::path langFilename, std::filesystem::path txtFilename)
//    {
//        TFStoryScript tfs;
//        try
//        {
//            tfs.openFile(idxFilename, langFilename);
//        }
//        catch (const std::exception& e)
//        {
//            std::cerr << "ERROR: Failed to open files: " << idxFilename.string() << " and " << langFilename.string() << " for reading.\n";
//            std::cerr << "Reason: " << e.what() << '\n';
//            return -1;
//        }
//
//        std::ofstream txtfile;
//        try
//        {
//            txtfile.open(txtFilename, std::ios::out | std::ios::binary);
//        }
//        catch (const std::exception& e)
//        {
//            std::cerr << "ERROR: Failed to open file: " << txtFilename.string() << " for writing.\n";
//            std::cerr << "Reason: " << e.what() << '\n';
//            return -2;
//        }
//
//        // write BOM
//        txtfile.put(0xFF);
//        txtfile.put(0xFE);
//
//        for (int i = 0; i < tfs.count(); i++)
//        {
//            // write section
//            std::string sectionStr = '[' + std::to_string(i) + ']' + '\n';
//            std::u16string u16section(sectionStr.begin(), sectionStr.end());
//            txtfile.write((char*)u16section.data(), u16section.size() * sizeof(char16_t));
//
//            // write data
//            std::u16string u16data = tfs.u16string(i);
//            txtfile.write((char*)u16data.data(), u16data.size() * sizeof(char16_t));
//
//            // newline for next section
//            char16_t nl = u'\n';
//            txtfile.write((char*)&nl, sizeof(char16_t));
//
//            txtfile.flush();
//        }
//
//        txtfile.flush();
//        txtfile.close();
//
//        return 0;
//    }
// 
//     //
//     // Exports a story script index + lang pair to an ini-like formatted txt file (UTF-8)
//     //
//     int ExportU8(std::filesystem::path idxFilename, std::filesystem::path langFilename, std::filesystem::path txtFilename)
//     {
//         TFStoryScript tfs;
//         try
//         {
//             tfs.openFile(idxFilename, langFilename);
//         }
//         catch (const std::exception& e)
//         {
//             std::cerr << "ERROR: Failed to open files: " << idxFilename.string() << " and " << langFilename.string() << " for reading.\n";
//             std::cerr << "Reason: " << e.what() << '\n';
//             return -1;
//         }
// 
//         std::ofstream txtfile;
//         try
//         {
//             txtfile.open(txtFilename, std::ios::out | std::ios::binary);
//         }
//         catch (const std::exception& e)
//         {
//             std::cerr << "ERROR: Failed to open file: " << txtFilename.string() << " for writing.\n";
//             std::cerr << "Reason: " << e.what() << '\n';
//             return -2;
//         }
// 
//         // write BOM
//         txtfile.put(0xEF);
//         txtfile.put(0xBB);
//         txtfile.put(0xBF);
// 
//         for (int i = 0; i < tfs.count(); i++)
//         {
//             // write section
//             std::string sectionStr = '[' + std::to_string(i) + ']' + '\n';
//             std::u8string u8section(sectionStr.begin(), sectionStr.end());
//             txtfile.write((char*)u8section.data(), u8section.size() * sizeof(char8_t));
// 
//             // write data
//             std::u8string u8data = tfs.u8string(i);
//             txtfile.write((char*)u8data.data(), u8data.size() * sizeof(char8_t));
// 
//             // newline for next section
//             char8_t nl = '\n';
//             txtfile.write((char*)&nl, sizeof(char8_t));
// 
//             txtfile.flush();
//         }
// 
//         txtfile.flush();
//         txtfile.close();
// 
//         return 0;
//     }
// 
//     //
//     // Imports an ini-like formatted txt file (UTF-16) and exports to a story script index + lang pair
//     //
//     int ImportU16(std::filesystem::path txtFilename, std::filesystem::path idxFilename, std::filesystem::path langFilename)
//     {
//         std::vector<std::u16string> strings;
//         int errcode = TagForceString::ParseTxtU16(txtFilename, &strings);
//         if (errcode < 0)
//         {
//             std::cerr << "ERROR: Text parser failed with code " << errcode << '\n';
//             return errcode;
//         }
// 
//         TFStoryScript tfs;
//         tfs.build(&strings);
// 
//         try
//         {
//             tfs.exportFile(idxFilename, langFilename);
//         }
//         catch (const std::exception& e)
//         {
//             std::cerr << "ERROR: Failed to open files: " << idxFilename.string() << " and " << langFilename.string() << " for writing.\n";
//             std::cerr << "Reason: " << e.what() << '\n';
//             return -2;
//         }
// 
//         return 0;
//     }
// 
//     //
//     // Imports an ini-like formatted txt file (UTF-8) and exports to a story script index + lang pair
//     //
//     int ImportU8(std::filesystem::path txtFilename, std::filesystem::path idxFilename, std::filesystem::path langFilename)
//     {
//         std::vector<std::u8string> strings;
//         int errcode = TagForceString::ParseTxtU8(txtFilename, &strings);
//         if (errcode < 0)
//         {
//             std::cerr << "ERROR: Text parser failed with code " << errcode << '\n';
//             return errcode;
//         }
// 
//         TFStoryScript tfs;
//         tfs.build(&strings);
// 
//         try
//         {
//             tfs.exportFile(idxFilename, langFilename);
//         }
//         catch (const std::exception& e)
//         {
//             std::cerr << "ERROR: Failed to open files: " << idxFilename.string() << " and " << langFilename.string() << " for writing.\n";
//             std::cerr << "Reason: " << e.what() << '\n';
//             return -2;
//         }
// 
//         return 0;
//     }
// }
// 
// namespace TF1Folder
// {
//     int ExportFolderU16(std::filesystem::path inFolder, std::filesystem::path outFolder)
//     {
//         if (!std::filesystem::exists(inFolder))
//         {
//             std::cerr << "ERROR: Folder " << inFolder.string() << " does not exist!\n";
//             return -1;
//         }
// 
//         if (!std::filesystem::exists(outFolder))
//         {
//             try
//             {
//                 std::filesystem::create_directory(outFolder);
//             }
//             catch (const std::exception& e)
//             {
//                 std::cerr << "ERROR: Folder " << outFolder.string() << " does not exist & could not be created!\n";
//                 std::cerr << "Reason: " << e.what() << '\n';
//                 return -2;
//             }
//         }
// 
//         std::vector<std::u8string> processedEntries;
// 
//         // expected filenames are in format:
//         // <name><type><lang>.bin
//         // <name><type><lang>.bin.gz
//         for (const auto& entry : std::filesystem::directory_iterator(inFolder))
//         {
//             if ((entry.path().extension() != ".bin") || (entry.path().extension() != ".gz"))
//                 continue;
// 
//             bool bCompressed = false;
//             bool bOtherCompressed = false;
//             bool bOtherIsIdx = false;
//             int posType = 6;
//             if ((entry.path().extension() == ".gz"))
//             {
//                 bCompressed = true;
//                 bOtherCompressed = true;
//                 posType += 3;
//             }
//             std::u8string strEntry = entry.path().filename().u8string();
//             std::u8string strFullExt = strEntry.substr(strEntry.size() - posType);
//             std::u8string strType = strFullExt.substr(0, 1);
//             std::u8string strLang = strFullExt.substr(1, 1);
//             std::u8string strOtherType = u8"L";
//             std::u8string strName = strEntry.substr(0, strEntry.size() - posType);
// 
//             if (std::find(processedEntries.begin(), processedEntries.end(), strName) != processedEntries.end())
//                 continue;
// 
//             if (strType == u8"L")
//             {
//                 bOtherIsIdx = true;
//                 strOtherType = u8"I";
//             }
//             std::u8string strOtherName = strName + strOtherType + strLang + entry.path().extension().u8string();
// 
//             std::filesystem::path otherEntry = entry.path().parent_path() / strOtherName;
// 
//             std::cout << "Processing: " << (char*)strName.c_str() << '\n'
//                 << " <- " << entry.path().string() << '\n';
// 
//             if (!std::filesystem::exists(otherEntry))
//             {
//                 // try to find the opposite just in case
//                 if (bCompressed)
//                 {
//                     strOtherName = strName + strOtherType + strLang + u8".bin";
//                     otherEntry = entry.path().parent_path() / strOtherName;
//                 }
//                 else
//                 {
//                     strOtherName = strName + strOtherType + strLang + u8".bin.gz";
//                     otherEntry = entry.path().parent_path() / strOtherName;
//                 }
// 
//                 if (!std::filesystem::exists(otherEntry))
//                 {
//                     std::cerr << "ERROR: Can't find " << otherEntry.string() << " !\n";
//                     processedEntries.push_back(strName);
//                     continue;
//                 }
// 
//                 bOtherCompressed = !bOtherCompressed;
//             }
// 
//             std::cout << " <- " << otherEntry.string() << '\n';
//             
//             
// 
//             processedEntries.push_back(strName);
//         }
//     }
// }


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
    }

    return 0;
}
