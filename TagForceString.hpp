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

class StringBuffer 
{
public:
	explicit StringBuffer() : offset(0) {}

	uint32_t addString(const std::u16string& str) 
	{
		auto iter = stringOffsetMap.find(str);
		if (iter != stringOffsetMap.end()) 
		{
			// String is a duplicate, return the offset of the original
			return iter->second;
		}
		else {
			// String is unique, add it to the buffer and update the offset
			uint32_t currentOffset = offset;
			stringOffsetMap[str] = currentOffset;
			offset += static_cast<uint32_t>(str.length() * sizeof(char16_t)) + sizeof(char16_t); // Include null terminator

			// Append the string to the buffer
			buffer.insert(buffer.end(), str.begin(), str.end());
			buffer.push_back(u'\0'); // Add null terminator

			return currentOffset;
		}
	}

	uint32_t addString(const std::u8string& str) 
	{
		auto iter = stringOffsetMapU8.find(str);
		if (iter != stringOffsetMapU8.end()) {
			// String is a duplicate, return the offset of the original
			return iter->second;
		}
		else {
			// String is unique, add it to the buffer and update the offset
			uint32_t currentOffset = offset;
			stringOffsetMapU8[str] = currentOffset;
			offset += static_cast<uint32_t>(str.length()) + sizeof(char8_t); // Include null terminator

			// Append the string to the buffer
			u8buffer.insert(u8buffer.end(), str.begin(), str.end());
			u8buffer.push_back('\0'); // Add null terminator

			return currentOffset;
		}
	}

	// Get the buffer data
	const char16_t* getData() const 
	{
		return buffer.data();
	}

	const char8_t* u8GetData() const
	{
		return u8buffer.data();
	}

	uint32_t dataSize()
	{
		return offset;
	}

private:
	std::vector<char16_t> buffer;  // Buffer to store the strings
	std::vector<char8_t> u8buffer;  // Buffer to store the strings
	std::unordered_map<std::u16string, uint32_t> stringOffsetMap;  // Map to store string offsets
	std::unordered_map<std::u8string, uint32_t> stringOffsetMapU8;  // Map to store string offsets
	uint32_t offset;  // Current offset in the buffer
};

class YgStringResource
{
private:
#pragma pack(push,1)
	struct StrHdr
	{
		uint32_t count;
		uint32_t tblstart;
		uint32_t datastart;
	};
#pragma pack(pop)

	StrHdr* hdr;
	uint8_t* filebuffer;
	uint32_t* ptrTable;
	uintptr_t ptrData;
	uintmax_t dataSize;
	uintmax_t tblSize;
	uintmax_t fileSize;

	uint32_t nulldata;

	uintptr_t GetStrPtr(int index)
	{
		if (index >= hdr->count)
			return 0;

		uintptr_t result = ptrData + ptrTable[index];
		uintptr_t endLoc = reinterpret_cast<uintptr_t>(filebuffer) + fileSize;
		
		if (result >= endLoc)
			return reinterpret_cast<uintptr_t>(&nulldata);

		return result;
	}

public:

	char16_t* c_wstr(int index)
	{
		if (index >= hdr->count)
			return nullptr;
		return reinterpret_cast<char16_t*>(GetStrPtr(index));
	}

	char* c_str(int index)
	{
		if (index >= hdr->count)
			return nullptr;
		return reinterpret_cast<char*>(GetStrPtr(index));
	}

	std::u16string u16string(int index)
	{
		if (index >= hdr->count)
			return std::u16string();
		return std::u16string(c_wstr(index));
	}

	std::wstring wstring(int index)
	{
		if (index >= hdr->count)
			return std::wstring();
		std::u16string data = u16string(index);
		return std::wstring(data.begin(), data.end());
	}

	std::u8string u8string(int index)
	{
		if (index >= hdr->count)
			return std::u8string();
		return std::u8string(reinterpret_cast<char8_t*>(c_str(index)));
	}

	std::string string(int index)
	{
		if (index >= hdr->count)
			return std::string();
		return std::string(c_str(index));
	}

	int count()
	{
		return hdr->count;
	}

	uintmax_t tblsize()
	{
		return tblSize;
	}

	uintmax_t datasize()
	{
		return dataSize;
	}

	uintmax_t filesize()
	{
		return fileSize;
	}

	uint8_t* fileptr()
	{
		return filebuffer;
	}

	uint8_t* dataptr()
	{
		return reinterpret_cast<uint8_t*>(ptrData);
	}

	uint8_t* tblptr()
	{
		return reinterpret_cast<uint8_t*>(ptrTable);
	}

	//
	// Load a string resource from a file
	//
	void openFile(std::filesystem::path filename)
	{
		std::ifstream ifile;
		try
		{
			ifile.open(filename, std::ios::binary);
			if (!ifile.is_open())
			{
				filebuffer = nullptr;
				throw std::runtime_error(strerror(errno));
			}
		}
		catch (const std::exception& e)
		{
			throw e;
		}


		uintmax_t filesize = 0;
		try
		{
			filesize = std::filesystem::file_size(filename);
		}
		catch (const std::exception& e)
		{
			ifile.close();
			throw e;
		}

		if (filebuffer)
			free(filebuffer);

		filebuffer = (uint8_t*)malloc(filesize);
		ifile.read((char*)filebuffer, filesize);
		ifile.close();

		hdr = (StrHdr*)filebuffer;
		ptrTable = reinterpret_cast<uint32_t*>(&filebuffer[hdr->tblstart]);
		ptrData = reinterpret_cast<uintptr_t>(&filebuffer[hdr->datastart]);
		dataSize = filesize - hdr->datastart;
		tblSize = hdr->datastart - hdr->tblstart;
		fileSize = filesize;
	}

	//
	// Export the current string resource in memory to a file
	//
	void exportFile(std::filesystem::path filename)
	{
		if (filebuffer == nullptr)
		{
			throw std::runtime_error("YgStringResource filebuffer is null!");
		}

		std::ofstream ofile;
		try
		{
			ofile.open(filename, std::ios::binary);
			if (!ofile.is_open())
			{
				throw std::runtime_error(strerror(errno));
			}
		}
		catch (const std::exception& e)
		{
			throw e;
		}

		ofile.write((char*)filebuffer, fileSize);

		ofile.flush();
		ofile.close();
	}

	//
	// Builds a string resource out of a UTF-16 string vector
	//
	void build(std::vector<std::u16string>* strings)
	{
		if (filebuffer)
			free(filebuffer);

		// generate the header
		StrHdr strhdr;
		strhdr.count = strings->size();
		strhdr.tblstart = sizeof(StrHdr);
		strhdr.datastart = sizeof(StrHdr) + (strings->size() * sizeof(uint32_t));

		StringBuffer stringBuffer;
		std::vector<uint32_t> offsets;

		for (const auto& str : *strings) 
		{
			uint32_t currentOffset = stringBuffer.addString(str);
			offsets.push_back(currentOffset);
		}

		// new buffer
		uintmax_t newsize = strhdr.datastart + stringBuffer.dataSize();
		filebuffer = (uint8_t*)malloc(newsize);

		// copy data
		uintmax_t cursor = 0;
		memcpy(filebuffer, &strhdr, sizeof(StrHdr));
		cursor += sizeof(StrHdr);
		memcpy(&filebuffer[cursor], offsets.data(), offsets.size() * sizeof(uint32_t));
		cursor += offsets.size() * sizeof(uint32_t);
		memcpy(&filebuffer[cursor], stringBuffer.getData(), stringBuffer.dataSize());

		// update ptrs
		hdr = (StrHdr*)filebuffer;
		ptrTable = reinterpret_cast<uint32_t*>(&filebuffer[hdr->tblstart]);
		ptrData = reinterpret_cast<uintptr_t>(&filebuffer[hdr->datastart]);
		dataSize = newsize - hdr->datastart;
		tblSize = hdr->datastart - hdr->tblstart;
		fileSize = newsize;
	}

	//
	// Builds a string resource out of a UTF-8 string vector
	//
	void build(std::vector<std::u8string>* strings)
	{
		if (filebuffer)
			free(filebuffer);

		// generate the header
		StrHdr strhdr;
		strhdr.count = strings->size();
		strhdr.tblstart = sizeof(StrHdr);
		strhdr.datastart = sizeof(StrHdr) + (strings->size() * sizeof(uint32_t));

		StringBuffer stringBuffer;
		std::vector<uint32_t> offsets;

		for (const auto& str : *strings)
		{
			uint32_t currentOffset = stringBuffer.addString(str);
			offsets.push_back(currentOffset);
		}

		// new buffer
		uintmax_t newsize = strhdr.datastart + stringBuffer.dataSize();
		filebuffer = (uint8_t*)malloc(newsize);

		// copy data
		uintmax_t cursor = 0;
		memcpy(filebuffer, &strhdr, sizeof(StrHdr));
		cursor += sizeof(StrHdr);
		memcpy(&filebuffer[cursor], offsets.data(), offsets.size() * sizeof(uint32_t));
		cursor += offsets.size() * sizeof(uint32_t);
		memcpy(&filebuffer[cursor], stringBuffer.u8GetData(), stringBuffer.dataSize());

		// update ptrs
		hdr = (StrHdr*)filebuffer;
		ptrTable = reinterpret_cast<uint32_t*>(&filebuffer[hdr->tblstart]);
		ptrData = reinterpret_cast<uintptr_t>(&filebuffer[hdr->datastart]);
		dataSize = newsize - hdr->datastart;
		tblSize = hdr->datastart - hdr->tblstart;
		fileSize = newsize;
	}

	YgStringResource()
	{
		filebuffer = nullptr;
		hdr = nullptr;
		ptrTable = nullptr;
		ptrData = 0;
		nulldata = 0;
		dataSize = 0;
		fileSize = 0;
		tblSize = 0;
	}

	~YgStringResource()
	{
		if (filebuffer)
			free(filebuffer);
	}
};

class TFStoryScript
{
private:
	uint32_t* strIdx;
	size_t strCount;

	uint8_t* langBuffer;
	uintmax_t fileSizeLang;

	uint32_t nulldata;

	uintptr_t GetStrPtr(int index)
	{
		if (index >= strCount)
			return 0;

		uintptr_t result = reinterpret_cast<uintptr_t>(langBuffer) + (strIdx[index] * sizeof(char16_t));
		uintptr_t endLoc = reinterpret_cast<uintptr_t>(langBuffer) + fileSizeLang;

		if (result >= endLoc)
			return reinterpret_cast<uintptr_t>(&nulldata);

		return result;
	}

	uintptr_t GetStrPtrU8(int index)
	{
		if (index >= strCount)
			return 0;

		uintptr_t result = reinterpret_cast<uintptr_t>(langBuffer) + (strIdx[index] * sizeof(char8_t));
		uintptr_t endLoc = reinterpret_cast<uintptr_t>(langBuffer) + fileSizeLang;

		if (result >= endLoc)
			return reinterpret_cast<uintptr_t>(&nulldata);

		return result;
	}

public:
	char16_t* c_wstr(int index)
	{
		if (index >= strCount)
			return nullptr;
		return reinterpret_cast<char16_t*>(GetStrPtr(index));
	}

	char* c_str(int index)
	{
		if (index >= strCount)
			return nullptr;
		return reinterpret_cast<char*>(GetStrPtrU8(index));
	}

	std::u16string u16string(int index)
	{
		if (index >= strCount)
			return std::u16string();
		return std::u16string(c_wstr(index));
	}

	std::wstring wstring(int index)
	{
		if (index >= strCount)
			return std::wstring();
		std::u16string data = u16string(index);
		return std::wstring(data.begin(), data.end());
	}

	std::u8string u8string(int index)
	{
		if (index >= strCount)
			return std::u8string();
		return std::u8string(reinterpret_cast<char8_t*>(c_str(index)));
	}

	std::string string(int index)
	{
		if (index >= strCount)
			return std::string();
		return std::string(c_str(index));
	}

	int count()
	{
		return strCount;
	}

	uintmax_t datasize()
	{
		return fileSizeLang;
	}

	uint8_t* fileptr()
	{
		return langBuffer;
	}

	//
	// Load a story script index + lang pair from their files
	//
	void openFile(std::filesystem::path idxFilename, std::filesystem::path langFilename)
	{
		std::ifstream idxfile;
		try
		{
			idxfile.open(idxFilename, std::ios::binary);
			if (!idxfile.is_open())
			{
				langBuffer = nullptr;
				strIdx = nullptr;
				std::string excstr = "idx file failure: ";
				excstr += strerror(errno);
				throw std::runtime_error(excstr);
			}
		}
		catch (const std::exception& e)
		{
			throw e;
		}

		std::ifstream langfile;
		try
		{
			langfile.open(langFilename, std::ios::binary);
			if (!langfile.is_open())
			{
				langBuffer = nullptr;
				strIdx = nullptr;
				std::string excstr = "lang file failure: ";
				excstr += strerror(errno);
				throw std::runtime_error(strerror(errno));
			}
		}
		catch (const std::exception& e)
		{
			throw e;
		}

		if (langBuffer)
			free(langBuffer);

		if (strIdx)
			free(strIdx);

		uintmax_t idxfilesize = 0;
		try
		{
			idxfilesize = std::filesystem::file_size(idxFilename);
		}
		catch (const std::exception& e)
		{
			idxfile.close();
			langfile.close();
			throw e;
		}

		strIdx = (uint32_t*)malloc(idxfilesize);
		idxfile.read((char*)strIdx, idxfilesize);
		idxfile.close();
		strCount = idxfilesize / sizeof(uint32_t);

		uintmax_t langfilesize = 0;
		try
		{
			langfilesize = std::filesystem::file_size(langFilename);
		}
		catch (const std::exception& e)
		{
			langfile.close();
			idxfile.close();
			throw e;
		}

		langBuffer = (uint8_t*)malloc(langfilesize);
		langfile.read((char*)langBuffer, langfilesize);
		langfile.close();

		fileSizeLang = langfilesize;
	}

	//
	// Export the current story script in memory to a index + lang file pair
	//
	void exportFile(std::filesystem::path idxFilename, std::filesystem::path langFilename)
	{
		if (langBuffer == nullptr)
		{
			throw std::runtime_error("TFStoryScript langBuffer is null!");
		}

		if (strIdx == nullptr)
		{
			throw std::runtime_error("TFStoryScript strIdx is null!");
		}

		std::ofstream idxfile;
		try
		{
			idxfile.open(idxFilename, std::ios::binary);
			if (!idxfile.is_open())
			{
				std::string excstr = "idx file failure: ";
				excstr += strerror(errno);
				throw std::runtime_error(excstr);
			}
		}
		catch (const std::exception& e)
		{
			throw e;
		}

		idxfile.write((char*)strIdx, strCount * sizeof(uint32_t));

		idxfile.flush();
		idxfile.close();

		std::ofstream langfile;
		try
		{
			langfile.open(langFilename, std::ios::binary);
			if (!langfile.is_open())
			{
				std::string excstr = "lang file failure: ";
				excstr += strerror(errno);
				throw std::runtime_error(excstr);
			}
		}
		catch (const std::exception& e)
		{
			throw e;
		}

		langfile.write((char*)langBuffer, fileSizeLang);

		langfile.flush();
		langfile.close();
	}

	//
	// Builds story script data out of a UTF-16 string vector
	//
	void build(std::vector<std::u16string>* strings)
	{
		if (langBuffer)
			free(langBuffer);

		if (strIdx)
			free(strIdx);

		// index buffer
		strCount = strings->size();
		strIdx = (uint32_t*)malloc(strCount * sizeof(uint32_t));

		StringBuffer stringBuffer;
		std::vector<uint32_t> offsets;

		int sc = 0;
		for (const auto& str : *strings)
		{
			uint32_t currentOffset = stringBuffer.addString(str);
			if (currentOffset == 0)
				strIdx[sc] = 0;
			else
				strIdx[sc] = currentOffset / sizeof(char16_t);
			sc++;
		}

		// lang buffer
		uintmax_t newsize = stringBuffer.dataSize();
		langBuffer = (uint8_t*)malloc(newsize);

		// copy data
		memcpy(langBuffer, stringBuffer.getData(), newsize);

		// update ptrs
		fileSizeLang = newsize;
	}

	//
	// Builds story script data out of a UTF-8 string vector
	//
	void build(std::vector<std::u8string>* strings)
	{
		if (langBuffer)
			free(langBuffer);

		if (strIdx)
			free(strIdx);

		// index buffer
		strCount = strings->size();
		strIdx = (uint32_t*)malloc(strCount * sizeof(uint32_t));

		StringBuffer stringBuffer;
		std::vector<uint32_t> offsets;

		int sc = 0;
		for (const auto& str : *strings)
		{
			uint32_t currentOffset = stringBuffer.addString(str);
			strIdx[sc] = currentOffset;
			sc++;
		}

		// lang buffer
		uintmax_t newsize = stringBuffer.dataSize();
		langBuffer = (uint8_t*)malloc(newsize);

		// copy data
		memcpy(langBuffer, stringBuffer.u8GetData(), newsize);

		// update ptrs
		fileSizeLang = newsize;
	}

	TFStoryScript()
	{
		strIdx = nullptr;
		strCount = 0;
		langBuffer = nullptr;
		fileSizeLang = 0;
		nulldata = 0;
	}

	~TFStoryScript()
	{
		if (langBuffer)
			free(langBuffer);
		if (strIdx)
			free(strIdx);
	}
};

namespace TagForceString
{
	enum OperatingMode
	{
		BIN2TXT,
		TXT2BIN,
		LANG2TXT,
		TXT2LANG,
		FOLD2TXT,
		TXT2FOLD
	};

	struct Options
	{
		OperatingMode mode;
		std::filesystem::path inputFilePath1;
		std::filesystem::path inputFilePath2;
		std::filesystem::path outputFilePath1;
		std::filesystem::path outputFilePath2;
		bool useUTF8 = false;       // Default is UTF-16
		bool autodetectBOM = true;  // Default is autodetection
	};

	void printUsage(const char* programName)
	{
		std::cerr << "Usage: " << programName << " [OPTIONS] MODE INPUT OUTPUT\n"
			<< "\nOPTIONS:\n"
			<< "  -u, --utf8          Use UTF-8 encoding (default is UTF-16)\n"
			<< "  -d, --no-autodetect Disable BOM autodetection for input text files\n"
			<< "\nMODES:\n"
			<< "  1. bin2txt           Convert a string resource (strtbl) file to a text file\n"
			<< "  2. txt2bin           Convert a text file to a string resource (strtbl) file\n"
			<< "  3. lang2txt          Convert a pair of lang files (index and strings) to a text file\n"
			<< "  4. txt2lang          Convert a text file to a pair of lang files (index and strings)\n"
			<< "  5. fold2txt          Batch convert a folder with lang file pairs to a folder with text files\n"
			<< "  6. txt2fold          Batch convert a folder with text files to a folder with lang file pairs\n"
			<< "\nEXAMPLES:\n"
			<< "  " << programName << " bin2txt input_e.bin output.txt\n"
			<< "  " << programName << " txt2bin input.txt output_e.bin\n"
			<< "  " << programName << " lang2txt langIe.bin langLe.bin output.txt\n"
			<< "  " << programName << " --utf8 txt2lang input.txt outIe.bin outLe.bin\n"
			<< '\n' << "The encoding must match on both input and output files! The tool does not perform any conversion!\n";
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
			else if (arg == "-d" || arg == "--no-autodetect")
			{
				options.autodetectBOM = false;
			}
			else if (arg == "bin2txt")
			{
				options.mode = BIN2TXT;
			}
			else if (arg == "txt2bin")
			{
				options.mode = TXT2BIN;
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

#endif