#pragma once
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

#ifndef TFSTRINGCLASSES_HDR
#define TFSTRINGCLASSES_HDR

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

	uint32_t addStringAligned(const std::u16string& str)
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
			uint32_t strsize = static_cast<uint32_t>(str.length() * sizeof(char16_t)) + sizeof(char16_t); // Include null terminator
			offset += strsize;

			uint32_t alignoffset = calculate_aligned_address(offset, 4);


			// Append the string to the buffer
			appendRawData(rawbuffer, (uint8_t*)str.data(), (str.length() * sizeof(char16_t)) + sizeof(char16_t));

			uintmax_t alignbytes = alignoffset - offset;
			offset += alignbytes;
			strsize += alignbytes;
			while (alignbytes)
			{
				rawbuffer.push_back(0);
				alignbytes--;
			}
			
			stringAlignSizesMap[str] = strsize;

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

	uint32_t addStringAligned(const std::u8string& str)
	{
		auto iter = stringOffsetMapU8.find(str);
		if (iter != stringOffsetMapU8.end())
		{
			// String is a duplicate, return the offset of the original
			return iter->second;
		}
		else {
			// String is unique, add it to the buffer and update the offset
			uint32_t currentOffset = offset;
			stringOffsetMapU8[str] = currentOffset;
			uint32_t strsize = static_cast<uint32_t>(str.length() * sizeof(char8_t)) + sizeof(char8_t); // Include null terminator
			offset += strsize;

			uint32_t alignoffset = calculate_aligned_address(offset, 4);

			// Append the string to the buffer
			appendRawData(rawbuffer, (uint8_t*)str.data(), (str.size() * sizeof(char8_t)) + sizeof(char8_t));

			uintmax_t alignbytes = alignoffset - offset;
			offset += alignbytes;
			strsize += alignbytes;
			while (alignbytes)
			{
				rawbuffer.push_back(0);
				alignbytes--;
			}

			stringAlignSizesMapU8[str] = strsize;

			return currentOffset;
		}
	}

	uint32_t addStringRawAligned(const std::string& str)
	{
		auto iter = stringOffsetMapRaw.find(str);
		if (iter != stringOffsetMapRaw.end())
		{
			// String is a duplicate, return the offset of the original
			return iter->second;
		}
		else {
			// String is unique, add it to the buffer and update the offset
			uint32_t currentOffset = offset;
			stringOffsetMapRaw[str] = currentOffset;
			uint32_t strsize = static_cast<uint32_t>(str.length() * sizeof(char)) + sizeof(char); // Include null terminator
			offset += strsize;

			uint32_t alignoffset = calculate_aligned_address(offset, 4);

			// Append the string to the buffer
			appendRawData(rawbuffer, (uint8_t*)str.data(), (str.size() * sizeof(char)) + sizeof(char));

			uintmax_t alignbytes = alignoffset - offset;
			offset += alignbytes;
			strsize += alignbytes;
			while (alignbytes)
			{
				rawbuffer.push_back(0);
				alignbytes--;
			}

			stringAlignSizesMapRaw[str] = strsize;

			return currentOffset;
		}
	}

	uint32_t addStringRaw(const std::string& str)
	{
		auto iter = stringOffsetMapRaw.find(str);
		if (iter != stringOffsetMapRaw.end()) {
			// String is a duplicate, return the offset of the original
			return iter->second;
		}
		else {
			// String is unique, add it to the buffer and update the offset
			uint32_t currentOffset = offset;
			stringOffsetMapRaw[str] = currentOffset;
			offset += static_cast<uint32_t>(str.length()) + sizeof(char); // Include null terminator

			// Append the string to the buffer
			appendRawData(rawbuffer, (uint8_t*)str.data(), (str.size() * sizeof(char)) + sizeof(char));

			return currentOffset;
		}
	}

	// Get the buffer data
	const char16_t* getData()
	{
		return buffer.data();
	}

	const char8_t* u8GetData()
	{
		return u8buffer.data();
	}

	const uint8_t* rawGetData()
	{
		return rawbuffer.data();
	}

	uint32_t dataSize()
	{
		return offset;
	}

	uint32_t getAlignStrSize(std::u16string str)
	{
		return stringAlignSizesMap[str];
	}

	uint32_t getAlignStrSize(std::u8string str)
	{
		return stringAlignSizesMapU8[str];
	}

	uint32_t getAlignStrSize(std::string str)
	{
		return stringAlignSizesMapRaw[str];
	}

private:
	std::vector<char16_t> buffer;  // Buffer to store the strings
	std::vector<char8_t> u8buffer;  // Buffer to store the strings
	std::vector<uint8_t> rawbuffer;  // Buffer to store the strings
	std::unordered_map<std::u16string, uint32_t> stringOffsetMap;  // Map to store string offsets
	std::unordered_map<std::u8string, uint32_t> stringOffsetMapU8;  // Map to store string offsets
	std::unordered_map<std::string, uint32_t> stringOffsetMapRaw;  // Map to store string offsets
	std::unordered_map<std::u16string, uint32_t> stringAlignSizesMap;
	std::unordered_map<std::u8string, uint32_t> stringAlignSizesMapU8;
	std::unordered_map<std::string, uint32_t> stringAlignSizesMapRaw;

	uint32_t offset;  // Current offset in the buffer

	uintptr_t calculate_aligned_address(uintptr_t address, size_t alignment) 
	{
		if ((alignment & (alignment - 1)) != 0) 
		{
			return address;
		}

		size_t remainder = address % alignment;

		size_t adjustment = (alignment - remainder) % alignment;

		uintptr_t aligned_address = address + adjustment;

		return aligned_address;
	}

	void appendRawData(std::vector<uint8_t>& destination, const uint8_t* source, size_t size) 
	{
		size_t currentSize = destination.size();

		destination.resize(currentSize + size);

		std::copy(source, source + size, destination.begin() + currentSize);
	}
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

	wchar_t* c_wstr(int index)
	{
		if (index >= hdr->count)
			return nullptr;
		return reinterpret_cast<wchar_t*>(GetStrPtr(index));
	}

	char16_t* c_u16str(int index)
	{
		if (index >= hdr->count)
			return nullptr;
		return reinterpret_cast<char16_t*>(GetStrPtr(index));
	}

	char8_t* c_u8str(int index)
	{
		if (index >= hdr->count)
			return nullptr;
		return reinterpret_cast<char8_t*>(GetStrPtr(index));
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
		return std::u16string(c_u16str(index));
	}

	std::wstring wstring(int index)
	{
		if (index >= hdr->count)
			return std::wstring();
		return std::wstring(c_wstr(index));
	}

	std::u8string u8string(int index)
	{
		if (index >= hdr->count)
			return std::u8string();
		return std::u8string(c_u8str(index));
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

	//
	// Builds a string resource out of a raw string vector
	//
	void build(std::vector<std::string>* strings)
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
			uint32_t currentOffset = stringBuffer.addStringRaw(str);
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
		memcpy(&filebuffer[cursor], stringBuffer.rawGetData(), stringBuffer.dataSize());

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

	uintptr_t GetStrPtrRaw(int index)
	{
		if (index >= strCount)
			return 0;

		uintptr_t result = reinterpret_cast<uintptr_t>(langBuffer) + (strIdx[index] * sizeof(char));
		uintptr_t endLoc = reinterpret_cast<uintptr_t>(langBuffer) + fileSizeLang;

		if (result >= endLoc)
			return reinterpret_cast<uintptr_t>(&nulldata);

		return result;
	}

public:
	wchar_t* c_wstr(int index)
	{
		if (index >= strCount)
			return nullptr;
		return reinterpret_cast<wchar_t*>(GetStrPtr(index));
	}

	char16_t* c_u16str(int index)
	{
		if (index >= strCount)
			return nullptr;
		return reinterpret_cast<char16_t*>(GetStrPtr(index));
	}

	char8_t* c_u8str(int index)
	{
		if (index >= strCount)
			return nullptr;
		return reinterpret_cast<char8_t*>(GetStrPtrU8(index));
	}

	char* c_str(int index)
	{
		if (index >= strCount)
			return nullptr;
		return reinterpret_cast<char*>(GetStrPtrRaw(index));
	}

	std::u16string u16string(int index)
	{
		if (index >= strCount)
			return std::u16string();
		return std::u16string(c_u16str(index));
	}

	std::wstring wstring(int index)
	{
		if (index >= strCount)
			return std::wstring();
		return std::wstring(c_wstr(index));
	}

	std::u8string u8string(int index)
	{
		if (index >= strCount)
			return std::u8string();
		return std::u8string(c_u8str(index));
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

	uint8_t* idxptr()
	{
		return reinterpret_cast<uint8_t*>(strIdx);
	}

	uintmax_t idxsize()
	{
		return strCount * sizeof(uint32_t);
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

	//
	// Builds story script data out of a raw string vector
	//
	void build(std::vector<std::string>* strings)
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
			uint32_t currentOffset = stringBuffer.addStringRaw(str);
			strIdx[sc] = currentOffset;
			sc++;
		}

		// lang buffer
		uintmax_t newsize = stringBuffer.dataSize();
		langBuffer = (uint8_t*)malloc(newsize);

		// copy data
		memcpy(langBuffer, stringBuffer.rawGetData(), newsize);

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

//
// Text Resources - used in Tag Force Special. These are different than YgStringResource
// Has a simpler array at the top of the file
// Each text item has an offset and a size
// These items always have to be written in order
// Offsets are memory aligned!
//
class YgTextResource
{
private:
#pragma pack(push,1)
	struct TxtItem
	{
		uint32_t offset;
		uint32_t size;
	};
#pragma pack(pop)

	TxtItem* items;
	uintmax_t itemcount;
	uint8_t* filebuffer;
	uintptr_t ptrData;
	uintmax_t dataSize;
	uintmax_t tblSize;
	uintmax_t fileSize;

	uint32_t nulldata;

	uintptr_t GetStrPtr(int index)
	{
		if (index >= itemcount)
			return 0;

		uintptr_t result = reinterpret_cast<uintptr_t>(filebuffer) + items[index].offset;
		uintptr_t endLoc = reinterpret_cast<uintptr_t>(filebuffer) + fileSize;

		if (result >= endLoc)
			return reinterpret_cast<uintptr_t>(&nulldata);

		return result;
	}

public:
	wchar_t* c_wstr(int index)
	{
		if (index >= itemcount)
			return nullptr;
		return reinterpret_cast<wchar_t*>(GetStrPtr(index));
	}

	char16_t* c_u16str(int index)
	{
		if (index >= itemcount)
			return nullptr;
		return reinterpret_cast<char16_t*>(GetStrPtr(index));
	}

	char8_t* c_u8str(int index)
	{
		if (index >= itemcount)
			return nullptr;
		return reinterpret_cast<char8_t*>(GetStrPtr(index));
	}

	char* c_str(int index)
	{
		if (index >= itemcount)
			return nullptr;
		return reinterpret_cast<char*>(GetStrPtr(index));
	}

	uintmax_t itemsize(int index)
	{
		if (index >= itemcount)
			return 0;
		return items[index].size;
	}

	std::u16string u16string(int index)
	{
		if (index >= itemcount)
			return std::u16string();
		return std::u16string(c_u16str(index));
	}

	std::wstring wstring(int index)
	{
		if (index >= itemcount)
			return std::wstring();
		return std::wstring(c_wstr(index));
	}

	std::u8string u8string(int index)
	{
		if (index >= itemcount)
			return std::u8string();
		return std::u8string(c_u8str(index));
	}

	std::string string(int index)
	{
		if (index >= itemcount)
			return std::string();
		return std::string(c_str(index));
	}

	int count()
	{
		return itemcount;
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
		return reinterpret_cast<uint8_t*>(items);
	}

	//
	// Load a text resource from a file
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

		items = (TxtItem*)filebuffer;
		itemcount = items[0].offset / sizeof(TxtItem);

		ptrData = reinterpret_cast<uintptr_t>(&filebuffer[items[0].offset]);
		dataSize = filesize - items[0].offset;
		tblSize = items[0].offset;
		fileSize = filesize;
	}

	//
	// Export the current text resource in memory to a file
	//
	void exportFile(std::filesystem::path filename)
	{
		if (filebuffer == nullptr)
		{
			throw std::runtime_error("YgTextResource filebuffer is null!");
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
	// Builds a text resource out of a UTF-16 string vector
	//
	void build(std::vector<std::u16string>* strings)
	{
		if (filebuffer)
			free(filebuffer);

		// generate the header
		tblSize = strings->size() * sizeof(TxtItem);

		StringBuffer stringBuffer;
		std::vector<TxtItem> newitems;

		for (const auto& str : *strings)
		{
			uint32_t currentOffset = stringBuffer.addStringAligned(str);
			uint32_t size = stringBuffer.getAlignStrSize(str);
			TxtItem ni = { currentOffset + tblSize , size };
			newitems.push_back(ni);
		}

		// new buffer
		uintmax_t newsize = tblSize + stringBuffer.dataSize();
		filebuffer = (uint8_t*)malloc(newsize);

		// copy data
		uintmax_t cursor = 0;
		memcpy(filebuffer, newitems.data(), tblSize);
		cursor += tblSize;
		memcpy(&filebuffer[cursor], stringBuffer.rawGetData(), stringBuffer.dataSize());

		// update ptrs
		items = (TxtItem*)filebuffer;
		ptrData = reinterpret_cast<uintptr_t>(&filebuffer[items[0].offset]);
		dataSize = newsize - items[0].offset;
		fileSize = newsize;
	}

	//
	// Builds a text resource out of a UTF-8 string vector
	//
	void build(std::vector<std::u8string>* strings)
	{
		if (filebuffer)
			free(filebuffer);

		// generate the header
		tblSize = strings->size() * sizeof(TxtItem);

		StringBuffer stringBuffer;
		std::vector<TxtItem> newitems;

		for (const auto& str : *strings)
		{
			uint32_t currentOffset = stringBuffer.addStringAligned(str);
			uint32_t size = stringBuffer.getAlignStrSize(str);
			TxtItem ni = { currentOffset + tblSize , size };
			newitems.push_back(ni);
		}

		// new buffer
		uintmax_t newsize = tblSize + stringBuffer.dataSize();
		filebuffer = (uint8_t*)malloc(newsize);

		// copy data
		uintmax_t cursor = 0;
		memcpy(filebuffer, newitems.data(), tblSize);
		cursor += tblSize;
		memcpy(&filebuffer[cursor], stringBuffer.rawGetData(), stringBuffer.dataSize());

		// update ptrs
		items = (TxtItem*)filebuffer;
		ptrData = reinterpret_cast<uintptr_t>(&filebuffer[items[0].offset]);
		dataSize = newsize - items[0].offset;
		fileSize = newsize;
	}

	//
	// Builds a text resource out of a raw string vector
	//
	void build(std::vector<std::string>* strings)
	{
		if (filebuffer)
			free(filebuffer);

		// generate the header
		tblSize = strings->size() * sizeof(TxtItem);

		StringBuffer stringBuffer;
		std::vector<TxtItem> newitems;

		for (const auto& str : *strings)
		{
			uint32_t currentOffset = stringBuffer.addStringRawAligned(str);
			uint32_t size = stringBuffer.getAlignStrSize(str);
			TxtItem ni = { currentOffset + tblSize , size };
			newitems.push_back(ni);
		}

		// new buffer
		uintmax_t newsize = tblSize + stringBuffer.dataSize();
		filebuffer = (uint8_t*)malloc(newsize);

		// copy data
		uintmax_t cursor = 0;
		memcpy(filebuffer, newitems.data(), tblSize);
		cursor += tblSize;
		memcpy(&filebuffer[cursor], stringBuffer.rawGetData(), stringBuffer.dataSize());

		// update ptrs
		items = (TxtItem*)filebuffer;
		ptrData = reinterpret_cast<uintptr_t>(&filebuffer[items[0].offset]);
		dataSize = newsize - items[0].offset;
		fileSize = newsize;
	}

	YgTextResource()
	{
		filebuffer = nullptr;
		items = nullptr;
		itemcount = 0;
		ptrData = 0;
		nulldata = 0;
		dataSize = 0;
		fileSize = 0;
		tblSize = 0;
	}

	~YgTextResource()
	{
		if (filebuffer)
			free(filebuffer);
	}
};

#endif