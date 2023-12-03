#pragma once
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <functional>

#ifndef TFSTRING_HDR
#define TFSTRING_HDR

namespace TagForceString
{
	enum OperatingMode
	{
		BIN2TXT,
		TXT2BIN,
		TBIN2TXT,
		TXT2TBIN,
		LANG2TXT,
		TXT2LANG,
		FOLD2TXT,
		TXT2FOLD
	};

	struct Options
	{
		OperatingMode mode = BIN2TXT;
		std::filesystem::path inputFilePath1;
		std::filesystem::path inputFilePath2;
		std::filesystem::path outputFilePath1;
		std::filesystem::path outputFilePath2;
		bool useUTF8 = false;       // Default is UTF-16
		bool useBOM = true;
		bool useRAW = false;
	};

	void printUsage(const char* programName)
	{
		std::cerr << "Usage: " << programName << " [OPTIONS] MODE INPUT OUTPUT\n"
			<< "\nOPTIONS:\n"
			<< "  -u, --utf8          Use UTF-8 / 8-bit encoding (default is UTF-16)\n"
			<< "  -d, --no-bom        Disable BOM autodetection for input text files and BOM writing for output\n"
			<< "  -r, --raw           Treat string data as raw data. Useful for Shift-JIS.\n"
			<< "\nSTRING RESOURCE MODES:\n"
			<< "  bin2txt           Convert a string resource (strtbl) file to a text file\n"
			<< "  txt2bin           Convert a text file to a string resource (strtbl) file\n"
			<< "\nTEXT RESOURCE MODES:\n"
			<< "  tbin2txt          Convert a text resource (e.g. tutorial) file to a text file\n"
			<< "  txt2tbin          Convert a text file to a text resource (e.g. tutorial) file\n"
			<< "\nLANG FILE MODES:\n"
			<< "  lang2txt          Convert a pair of lang files (index and strings) to a text file\n"
			<< "  txt2lang          Convert a text file to a pair of lang files (index and strings)\n"
			<< "\nFOLDER MODES:\n"
			<< "  fold2txt          Batch convert a folder with lang file pairs to a folder with text files\n"
			<< "  txt2fold          Batch convert a folder with text files to a folder with lang file pairs\n"
			<< "\nEXAMPLES:\n"
			<< "  " << programName << " bin2txt input_e.bin output.txt\n"
			<< "  " << programName << " txt2bin input.txt output_e.bin\n"
			<< "  " << programName << " lang2txt langIe.bin langLe.bin output.txt\n"
			<< "  " << programName << " --utf8 txt2lang input.txt outIe.bin outLe.bin\n"
			<< "  " << programName << " fold2txt in_folder out_folder\n"
			<< "  " << programName << " txt2fold in_folder out_folder\n"
			<< "\nNOTES:\n"
			<< " - Folder modes MUST follow the correct filename format! (e.g. langIe.bin & langLe.bin & lang_e.txt)\n"
			<< " - The encoding must match on both input and output files! The tool does not perform any conversion!\n"
			<< "For more information, please read the README."
			<< '\n';
	}

	Options parseCommandLine(int argc, char* argv[])
	{
		Options options;

		for (int i = 1; i < argc; ++i)
		{
			std::string arg = argv[i];

			if (arg == "-u" || arg == "--utf8")
			{
				options.useUTF8 = true;
			}
			else if (arg == "-d" || arg == "--no-bom")
			{
				options.useBOM = false;
			}
			else if (arg == "-r" || arg == "--raw")
			{
				options.useRAW = true;
			}
			else if (arg == "bin2txt")
			{
				options.mode = BIN2TXT;
			}
			else if (arg == "txt2bin")
			{
				options.mode = TXT2BIN;
			}
			else if (arg == "tbin2txt")
			{
				options.mode = TBIN2TXT;
			}
			else if (arg == "txt2tbin")
			{
				options.mode = TXT2TBIN;
			}
			else if (arg == "fold2txt")
			{
				options.mode = FOLD2TXT;
			}
			else if (arg == "txt2fold")
			{
				options.mode = TXT2FOLD;
			}
			else if (arg == "lang2txt")
			{
				options.mode = LANG2TXT;
				if (i + 3 < argc)
				{
					options.inputFilePath1 = argv[++i];
					options.inputFilePath2 = argv[++i];
					options.outputFilePath1 = argv[++i];
				}
				else
				{
					std::cerr << "Insufficient arguments for lang2txt. Use '" << argv[0] << "' for help.\n";
					//printUsage(argv[0]);
					exit(1);
				}
			}
			else if (arg == "txt2lang")
			{
				options.mode = TXT2LANG;
				if (i + 3 < argc)
				{
					options.inputFilePath1 = argv[++i];
					options.outputFilePath1 = argv[++i];
					options.outputFilePath2 = argv[++i];
				}
				else
				{
					std::cerr << "Insufficient arguments for txt2lang. Use '" << argv[0] << "' for help.\n";
					//printUsage(argv[0]);
					exit(1);
				}
			}
			else if (i + 2 <= argc)
			{
				options.inputFilePath1 = argv[i++];
				options.outputFilePath1 = argv[i++];
			}
			else
			{
				std::cerr << "Invalid arguments. Use '" << argv[0] << "' for help.\n";
				//printUsage(argv[0]);
				exit(1);
			}
		}

		return options;
	}

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

		return result;
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

	std::string readline(std::ifstream& file)
	{
		std::string line;
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
				line += reinterpret_cast<const char*>(buffer);
			}
			else
			{
				// Handle single-byte character
				line += static_cast<char>(c);
			}
		}

		return line;
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
			if (str.back() == u8'\n')
			{
				str.pop_back();
			}

			if (!str.empty() && str.back() == u8'\r')
			{
				str.pop_back();
			}
		}
	}

	void removeCRLF(std::string& str)
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

	bool isStrNumeric(const std::string& str) 
	{
		for (char ch : str)
		{
			if (ch >= -1 && ch <= 255)
			{
				if (!std::isdigit(ch))
				{
					return false;
				}
			}
			else
				return false;
		}
		return true;
	}

	std::string escapeCharacter(const std::string& input, char characterToEscape) 
	{
		std::string result;

		for (char c : input) {
			if (c == characterToEscape) {
				result += '\\';
			}
			result += c;
		}

		return result;
	}

	std::u16string escapeCharacter(const std::u16string& input, char16_t characterToEscape)
	{
		std::u16string result;

		for (char16_t c : input) {
			if (c == characterToEscape) {
				result += '\\';
			}
			result += c;
		}

		return result;
	}

	std::u8string escapeCharacter(const std::u8string& input, char8_t characterToEscape)
	{
		std::u8string result;

		for (char8_t c : input) {
			if (c == characterToEscape) {
				result += '\\';
			}
			result += c;
		}

		return result;
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
			if (!txtfile.is_open())
			{
				throw std::runtime_error(strerror(errno));
			}
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

		int linecounter = 0;

		while (!txtfile.eof())
		{
			std::u16string line = readlineu16(txtfile);
			
			// trim any newline chars
			line.erase(std::find_if(line.rbegin(), line.rend(), std::not_fn(std::function<int(int)>(::isspace))).base(), line.end());
			if (!(!line.empty() && line.front() == u'[' && line.back() == u']'))
				continue;

			std::u16string idStrU16 = line.substr(1, line.length() - 2);
			std::string idStr(idStrU16.begin(), idStrU16.end());
			if (!isStrNumeric(idStr))
			{
				std::cout << "WARNING: Section " << idStr << " ignored!\n";
				continue;
			}
			int id = std::stoi(idStr);
			std::u16string data;
			while (!txtfile.eof())
			{
				char16_t ch;
				txtfile.read((char*)&ch, sizeof(char16_t));
				if (ch == u'\\')
				{
					char16_t nxch = u'\0';
					if (!txtfile.eof())
						txtfile.read((char*)&nxch, sizeof(char16_t));
					if (nxch == u'[')
					{
						data.push_back(nxch);
						if (txtfile.eof())
							break;
					}
					else if (nxch == u'\\')
					{
						data.push_back(nxch);
						if (txtfile.eof())
							break;
					}
					else
					{
						txtfile.seekg(-static_cast<std::streamoff>(sizeof(char16_t)), std::ios::cur);
						data.push_back(ch);
						std::cout << "WARNING: Unknown escape character '" << (char)ch << (char)nxch << "' at string " << linecounter << '\n';
					}
				}
				else
				{
					if (ch == u'[')
					{
						txtfile.seekg(-static_cast<std::streamoff>(sizeof(char16_t)), std::ios::cur);
						break;
					}
					if (txtfile.eof())
						break;
					data.push_back(ch);
				}
			}

			removeCRLF(data);
			strMap[id] = data;
			linecounter++;
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
			if (!txtfile.is_open())
			{
				throw std::runtime_error(strerror(errno));
			}
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

		int linecounter = 0;

		while (!txtfile.eof())
		{
			std::u8string line = readlineu8(txtfile);
			
			// trim any newline chars
			line.erase(std::find_if(line.rbegin(), line.rend(), std::not_fn(std::function<int(int)>(::isspace))).base(), line.end());
			if (!(!line.empty() && line.front() == u8'[' && line.back() == u8']'))
				continue;

			std::u8string idStrU8 = line.substr(1, line.length() - 2);
			std::string idStr(idStrU8.begin(), idStrU8.end());
			if (!isStrNumeric(idStr))
			{
				std::cout << "WARNING: Section " << idStr << " ignored!\n";
				continue;
			}
			int id = std::stoi(idStr);
			std::u8string data;
			while (!txtfile.eof())
			{
				char8_t ch;
				txtfile.read((char*)&ch, sizeof(char8_t));
				if (ch == u8'\\')
				{
					char8_t nxch = u8'\0';
					if (!txtfile.eof())
						txtfile.read((char*)&nxch, sizeof(char8_t));
					if (nxch == u8'[')
					{
						data.push_back(nxch);
						if (txtfile.eof())
							break;
					}
					else if (nxch == u8'\\')
					{
						data.push_back(nxch);
						if (txtfile.eof())
							break;
					}
					else
					{
						txtfile.seekg(-static_cast<std::streamoff>(sizeof(char8_t)), std::ios::cur);
						data.push_back(ch);
						std::cout << "WARNING: Unknown escape character '" << (char)ch << (char)nxch << "' at string " << linecounter << '\n';
					}
				}
				else
				{
					if (ch == u8'[')
					{
						txtfile.seekg(-static_cast<std::streamoff>(sizeof(char8_t)), std::ios::cur);
						break;
					}
					if (txtfile.eof())
						break;
					data.push_back(ch);
				}
			}

			removeCRLF(data);
			strMap[id] = data;
			linecounter++;
		}

		// copy strings in index order
		for (const auto& pair : strMap)
		{
			outStrings->push_back(pair.second);
		}


		return 0;
	}

	//
	// Parses an ini-like formatted txt file with raw data and returns a vector to the given pointer.
	//
	int ParseTxtRaw(std::filesystem::path txtFilename, std::vector<std::string>* outStrings)
	{
		std::ifstream txtfile;
		try
		{
			txtfile.open(txtFilename, std::ios::binary);
			if (!txtfile.is_open())
			{
				throw std::runtime_error(strerror(errno));
			}
		}
		catch (const std::exception& e)
		{
			std::cerr << "ERROR: Failed to open file: " << txtFilename.string() << " for reading.\n";
			std::cerr << "Reason: " << e.what() << '\n';
			return -1;
		}

		if (txtfile.get() != '[')
		{
			std::cerr << "ERROR: Invalid file format.\n";
			txtfile.close();
			return -2;
		}

		txtfile.seekg(0, std::ios::beg);


		// create a map of strings per index to keep them in order
		std::map<int, std::string> strMap;

		int linecounter = 0;

		while (!txtfile.eof())
		{
			std::string line = readline(txtfile);

			// trim any newline chars
			line.erase(std::find_if(line.rbegin(), line.rend(), std::not_fn(std::function<int(int)>(::isspace))).base(), line.end());
			if (!(!line.empty() && line.front() == '[' && line.back() == ']'))
				continue;

			std::string idStr = line.substr(1, line.length() - 2);
			if (!isStrNumeric(idStr))
			{
				std::cout << "WARNING: Section " << idStr << " ignored!\n";
				continue;
			}
			int id = std::stoi(idStr);
			std::string data;
			while (!txtfile.eof())
			{
				char ch = txtfile.get();
				if (ch == '[')
				{
					// search for the ']' character within the next 6 chars
					int seekcount = 0;
					bool bFoundSectionEnd = false;
					std::string bdata;
					for (int i = 0; i < 6; i++)
					{
						if (txtfile.eof())
							break;
						char ch2 = txtfile.get();
						if (ch2 == ']')
						{
							seekcount++;

							if (isStrNumeric(bdata))
								bFoundSectionEnd = true;
							break;
						}
						else
							bdata.push_back(ch2);

						seekcount++;
					}

					txtfile.seekg(-static_cast<std::streamoff>((seekcount) * sizeof(char)), std::ios::cur);

					if (bFoundSectionEnd)
					{
						txtfile.seekg(-static_cast<std::streamoff>(sizeof(char)), std::ios::cur);
						break;
					}
				}
				if (txtfile.eof())
					break;
				data.push_back(ch);
			}

			removeCRLF(data);
			strMap[id] = data;
			linecounter++;
		}

		// copy strings in index order
		for (const auto& pair : strMap)
		{
			outStrings->push_back(pair.second);
		}


		return 0;
	}
}

#endif