#include <iostream>
#include <fstream>
#include <filesystem>

#ifdef _MSC_VER
#ifdef WIN32
#define ZLIB_WINAPI
#endif
#endif

#include <zlib.h>

#ifndef ZLIBWRAPPER_HDR
#define ZLIBWRAPPER_HDR

namespace ZLibWrapper
{
    bool extractGzFile(std::filesystem::path gzFilePath, std::filesystem::path outputPath)
    {
        // Open the input file (gzipped file)
#ifdef _MSC_VER
        gzFile gzFile = gzopen_w(gzFilePath.wstring().c_str(), "rb");
#else
        gzFile gzFile = gzopen(gzFilePath.string().c_str(), "rb");
#endif
        if (gzFile == nullptr)
        {
            std::string errmsg = "Can't open the gzip file for reading: " + gzFilePath.string();
            throw std::runtime_error(errmsg);
            return false;
        }

        // Open the output file
        std::ofstream outputFile(outputPath, std::ios::out | std::ios::binary);
        if (!outputFile.is_open())
        {
            gzclose(gzFile);

            std::string errmsg = "Can't open gzip output file: " + outputPath.string();
            throw std::runtime_error(errmsg);

            return false;
        }

        const int bufferSize = 1024;
        char buffer[bufferSize];

        // Read and write data until the end of the gzipped file
        int bytesRead;
        while ((bytesRead = gzread(gzFile, buffer, bufferSize)) > 0)
        {
            outputFile.write(buffer, bytesRead);
        }

        // Check for errors or premature end of file
        if (gzeof(gzFile) == 0)
        {
            gzclose(gzFile);
            outputFile.close();

            std::string errmsg = "Can't read gzipped file: " + gzFilePath.string();
            throw std::runtime_error(errmsg);
            
            return false;
        }

        // Close the input and output files
        gzclose(gzFile);
        outputFile.close();

        return true;
    }

    bool packGzFile(const uint8_t* buffer, uintmax_t size, std::filesystem::path gzFilePath)
    {
#ifdef _MSC_VER
        gzFile file = gzopen_w(gzFilePath.wstring().c_str(), "wb");
#else
        gzFile file = gzopen(gzFilePath.string().c_str(), "wb");
#endif
        if (file == nullptr)
        {
            std::string errmsg = "Can't open a gzip file for writing: " + gzFilePath.string();
            throw std::runtime_error(errmsg);

            return false;
        }

        int result = gzwrite(file, buffer, size);

        if (result <= 0)
        {
            gzclose(file);

            std::string errmsg = "Failed to write data to: " + gzFilePath.string();
            throw std::runtime_error(errmsg);

            return false;
        }

        gzclose(file);

        return true;
    }
}

#endif