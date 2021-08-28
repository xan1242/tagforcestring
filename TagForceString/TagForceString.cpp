// Yu-Gi-Oh! Tag Force Language Tool
// by Xan
// TODO: add string reusage
// TODO: TF1 folder mode - fix memory leaks
// TODO: TF1 folder mode - integrate zlib directly to this
// 08.2021. - BOM & single-line mode support - SOURCE FILE IS NOW UNICODE

#include "stdafx.h"
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#include <strsafe.h>
#endif

#include "TagForceString.h"

unsigned int StringCount = 0;
unsigned int* OffsetList;
wchar_t* StringBuffer;
wchar_t** StringList;

char* MBStringBuffer;
char** MBStringList;
struct stat st = { 0 };

bool MBMode = false;

// TF1 folder stuff
bool bTF1FolderMode = false;
unsigned int FileCount;
char** FileDirectoryListing;
bool* bCompressedFileMarkers;
char BaseName[1024];
char IDFilePath[1024];
char StrFilePath[1024];
char OutFilePath[1024];
wchar_t MkDirPath[1024];
char LanguageLetter = 'e'; // possible Tag Force languages: j, e, g, f, i, s
char SystemCmdBuffer[1024];

// Cutin Character Words Table Stuff
struct WordsTbl
{
	unsigned int StrCount;
	unsigned int HeaderSize;
	unsigned int FullHeaderSize;
};

unsigned int CountLinesInFile(const char* InFilename)
{
	FILE* finput = fopen(InFilename, "rb");
	unsigned int LineCount = 0;
	wchar_t ReadCh;

	while (!feof(finput)) // WRONG WRONG WRONG
	{
		fread(&ReadCh, sizeof(wchar_t), 1, finput);
		if (ReadCh == L'\n')
			LineCount++;
	}
	fclose(finput);
	return LineCount - 1;
}

int ParseStrings(const char* InLangFile, const char* InOffsetFile)
{
	FILE* foffsets = fopen(InOffsetFile, "rb");
	if (!foffsets)
	{
		printf("ERROR: Can't open offset file %s for reading!\n", InOffsetFile);
		perror("ERROR");
		return -1;
	}

	FILE* flang = fopen(InLangFile, "rb");
	if (!flang)
	{
		printf("ERROR: Can't open language file %s for reading!\n", InLangFile);
		perror("ERROR");
		return -1;
	}

	if (stat(InOffsetFile, &st))
	{
		printf("ERROR: Can't find %s during size parsing for offset file!\n", InOffsetFile);
		return -1;
	}

	// get string count and offsets, allocate memory
	StringCount = st.st_size / sizeof(unsigned int);
	OffsetList = (unsigned int*)calloc(StringCount, sizeof(unsigned int));
	fread(OffsetList, sizeof(unsigned int), StringCount, foffsets);
	fclose(foffsets);

	// read strings to memory, allocate memory
	if (stat(InLangFile, &st))
	{
		printf("ERROR: Can't find %s during size parsing for language file!\n", InLangFile);
		return -1;
	}

	// boundary check for the offset list - early versions of files used last offset as a size descriptor
	if ((OffsetList[StringCount - 1] * 2) == (st.st_size))
	{
		if (!bTF1FolderMode)
			printf("WARNING: Last offset is out of bounds! You may be using an old variant of the file.\n");
		StringCount--;
	}
		

	if (MBMode)
	{
		MBStringBuffer = (char*)calloc(st.st_size, sizeof(char));
		MBStringList = (char**)calloc(StringCount, sizeof(char*));
		fread(MBStringBuffer, st.st_size, 1, flang);
	}
	else
	{
		StringBuffer = (wchar_t*)calloc(st.st_size, sizeof(char));
		//StringBuffer = (wchar_t*)malloc(st.st_size);
		StringList = (wchar_t**)calloc(StringCount, sizeof(wchar_t*));
		fread(StringBuffer, st.st_size, 1, flang);
	}
	
	fclose(flang);

	for (unsigned int i = 0; i < StringCount; i++)
	{
		if (MBMode)
			MBStringList[i] = (char*)((unsigned int)MBStringBuffer + OffsetList[i]);
		else
			StringList[i] = (wchar_t*)((unsigned int)StringBuffer + OffsetList[i] + OffsetList[i]);
	}
	
	return 0;
}

int SpitStringsToFile(const char* OutFilename)
{
	FILE* fout = fopen(OutFilename, "wb");

	if (!fout)
	{
		printf("ERROR: Can't open file %s for writing!\n", OutFilename);
		perror("ERROR");
		return -1;
	}

	if (MBMode)
	{
		// write BOM
		fputc(0xEF, fout);
		fputc(0xBB, fout);
		fputc(0xBF, fout);

		fprintf(fout, "%d\n", StringCount);
	}
	else
	{
		// write BOM
		fputc(0xFF, fout);
		fputc(0xFE, fout);

		fwprintf(fout, L"%ld\n", StringCount);
	}

	for (unsigned int i = 0; i < StringCount; i++)
	{
		if (MBMode)
			fprintf(fout, "[%d]\n%s\n", i, MBStringList[i]);
		else
			fwprintf(fout, L"[%ld]\n%ls\n", i, StringList[i]);
	}

	fclose(fout);

	return 0;
}

int ParseUTF16Text(const char* InFilename)
{
	FILE* fin = fopen(InFilename, "rb");
	wchar_t* cursor = NULL;
	wchar_t* startpoint = NULL;
	wchar_t* endpoint = NULL;
	unsigned int BOMdetector = 0;

	if (!fin)
	{
		printf("ERROR: Can't open text file %s for reading!\n", InFilename);
		perror("ERROR");
		return -1;
	}

	// read the entire text file to memory and generate string pointers

	if (stat(InFilename, &st))
	{
		printf("ERROR: Can't find %s during UTF-16 parsing!\n", InFilename);
		return -1;
	}
	// if BOM, skip BOM
	fread(&BOMdetector, sizeof(int), 1, fin);
	if ((BOMdetector & 0xFFFF) == 0xFEFF)
		fseek(fin, -2, SEEK_CUR);
	else
		fseek(fin, -4, SEEK_CUR);

	fwscanf(fin, L"%ld\n", &StringCount);
	StringBuffer = (wchar_t*)calloc(st.st_size, sizeof(char));
	StringList = (wchar_t**)calloc(StringCount, sizeof(wchar_t*));
	OffsetList = (unsigned int*)calloc(StringCount + 1, sizeof(unsigned int));
	fread(StringBuffer, st.st_size, 1, fin);
	fclose(fin);

	cursor = StringBuffer;

	for (unsigned int i = 0; i < StringCount; i++)
	{
		startpoint = wcschr(cursor, L']');

		startpoint += 2;
		endpoint = wcschr(startpoint, L'[');
		if (endpoint)
		{
			cursor = endpoint;
			endpoint -= 1;
			*endpoint = 0;
		}
		else
		{
			endpoint = startpoint + wcslen(startpoint) - 1;
			*endpoint = 0;
		}
		OffsetList[i + 1] = (wcslen(startpoint) + 1) + OffsetList[i];
		StringList[i] = startpoint;
	}
	

	return 0;
}

int ParseUTF8Text(const char* InFilename)
{
	FILE* fin = fopen(InFilename, "rb");
	char* cursor = NULL;
	char* startpoint = NULL;
	char* endpoint = NULL;
	unsigned int BOMdetector = 0;

	if (!fin)
	{
		printf("ERROR: Can't open text file %s for reading!\n", InFilename);
		perror("ERROR");
		return -1;
	}

	// read the entire text file to memory and generate string pointers

	if (stat(InFilename, &st))
	{
		printf("ERROR: Can't find %s during UTF-8 parsing!\n", InFilename);
		return -1;
	}
	// if BOM, skip BOM
	fread(&BOMdetector, sizeof(int), 1, fin);
	if ((BOMdetector & 0xFFFFFF) == 0xBFBBEF)
		fseek(fin, -1, SEEK_CUR);
	else
		fseek(fin, -4, SEEK_CUR);

	fscanf(fin, "%d\n", &StringCount);
	MBStringBuffer = (char*)calloc(st.st_size, sizeof(char));
	MBStringList = (char**)calloc(StringCount, sizeof(char*));
	OffsetList = (unsigned int*)calloc(StringCount + 1, sizeof(unsigned int));
	fread(MBStringBuffer, st.st_size, 1, fin);
	fclose(fin);

	cursor = MBStringBuffer;

	for (unsigned int i = 0; i < StringCount; i++)
	{
		startpoint = (char*)strchr(cursor, ']');

		startpoint += 2;
		endpoint = (char*)strchr(startpoint, '[');
		if (endpoint)
		{
			cursor = endpoint;
			endpoint -= 1;
			*endpoint = 0;
		}
		else
		{
			endpoint = startpoint + strlen(startpoint) - 1;
			*endpoint = 0;
		}
		OffsetList[i + 1] = (strlen(startpoint) + 1) + OffsetList[i];
		MBStringList[i] = startpoint;
	}


	return 0;

}

int ParseUTF16Text_SingleLine(const char* InFilename) // format used by Clickclaxer01 in his projects, adding the parsing support here...
{
	wchar_t* cursor = NULL;
	wchar_t* startpoint = NULL;
	wchar_t* endpoint = NULL;
	unsigned int BOMdetector = 0;

	int strlength = 0;
	wchar_t* cursor_repl = NULL;
	wchar_t* startpoint_repl = NULL;

	StringCount = CountLinesInFile(InFilename);
	StringList = (wchar_t**)calloc(StringCount, sizeof(wchar_t*));

	if (stat(InFilename, &st))
	{
		printf("ERROR: Can't find %s!\n", InFilename);
		return -1;
	}

	FILE* fin = fopen(InFilename, "rb");
	// if BOM, skip BOM
	fread(&BOMdetector, sizeof(int), 1, fin);
	if ((BOMdetector & 0xFFFF) == 0xFEFF)
		fseek(fin, -2, SEEK_CUR);
	else
		fseek(fin, -4, SEEK_CUR);

	StringBuffer = (wchar_t*)calloc(st.st_size, sizeof(char));
	fread(StringBuffer, st.st_size, 1, fin);
	fclose(fin);

	OffsetList = (unsigned int*)calloc(StringCount + 1, sizeof(unsigned int));


	cursor = StringBuffer;

	for (unsigned int i = 0; i < StringCount; i++)
	{
		startpoint = wcschr(cursor, L'＾') + 1;
		endpoint = wcschr(startpoint, L'\r');
		if (endpoint)
		{
			cursor = endpoint + 1;
			//endpoint -= 1;
			*endpoint = L'\0';
		}
		else
		{
			endpoint = startpoint + wcslen(startpoint) - 1;
			*endpoint = 0;
		}

		// replace <N> with \n and move the rest of the text back by 2 chars (copy it back)
		strlength = wcslen(startpoint);
		cursor_repl = startpoint;
		while (cursor_repl < strlength + startpoint)
		{
			startpoint_repl = wcsstr(cursor_repl, L"<N>");
			if (startpoint_repl)
			{
				*startpoint_repl = L'\n';
				cursor_repl = startpoint_repl + 1; // maybe do * sizeof(wchar_t) if it bugs out on movement!
				startpoint_repl += 3;
				wcscpy(cursor_repl, startpoint_repl);
			}
			else
				break;
		}

		OffsetList[i + 1] = (wcslen(startpoint) + 1) + OffsetList[i];
		StringList[i] = startpoint;
		strlength = 0;
	}

	return 0;
}

int ParseUTF8Text_SingleLine(const char* InFilename) // format used by Clickclaxer01 in his projects, adding the parsing support here...
{
	char* cursor = NULL;
	char* startpoint = NULL;
	char* endpoint = NULL;
	unsigned int BOMdetector = 0;

	int strlength = 0;
	char* cursor_repl = NULL;
	char* startpoint_repl = NULL;

	StringCount = CountLinesInFile(InFilename);
	MBStringList = (char**)calloc(StringCount, sizeof(char*));

	if (stat(InFilename, &st))
	{
		printf("ERROR: Can't find %s!\n", InFilename);
		return -1;
	}

	FILE* fin = fopen(InFilename, "rb");
	// if BOM, skip BOM
	fread(&BOMdetector, sizeof(int), 1, fin);
	if ((BOMdetector & 0xFFFFFF) == 0xBFBBEF)
		fseek(fin, -1, SEEK_CUR);
	else
		fseek(fin, -4, SEEK_CUR);

	MBStringBuffer = (char*)calloc(st.st_size, sizeof(char));
	fread(MBStringBuffer, st.st_size, 1, fin);
	fclose(fin);

	OffsetList = (unsigned int*)calloc(StringCount + 1, sizeof(unsigned int));


	cursor = MBStringBuffer;

	for (unsigned int i = 0; i < StringCount; i++)
	{
		//startpoint = strchr(cursor, '＾') + 1;
		startpoint = strstr(cursor, "\xEF\xBC\xBE");
		endpoint = strchr(startpoint, '\r');
		if (endpoint)
		{
			cursor = endpoint + 1;
			//endpoint -= 1;
			*endpoint = '\0';
		}
		else
		{
			endpoint = startpoint + strlen(startpoint) - 1;
			*endpoint = 0;
		}

		// replace <N> with \n and move the rest of the text back by 2 chars (copy it back)
		strlength = strlen(startpoint);
		cursor_repl = startpoint;
		while (cursor_repl < strlength + startpoint)
		{
			startpoint_repl = strstr(cursor_repl, "<N>");
			if (startpoint_repl)
			{
				*startpoint_repl = '\n';
				cursor_repl = startpoint_repl + 1;
				startpoint_repl += 3;
				strcpy(cursor_repl, startpoint_repl);
			}
			else
				break;
		}

		OffsetList[i + 1] = (strlen(startpoint) + 1) + OffsetList[i];
		MBStringList[i] = startpoint;
		strlength = 0;
	}

	return 0;
}

int ExportUTF16LangFiles(const char* OutLangFile, const char* OutOffsetFile)
{
	FILE* flangout = fopen(OutLangFile, "wb");
	if (!flangout)
	{
		printf("ERROR: Can't open file %s for writing!\n", OutLangFile);
		perror("ERROR");
		return -1;
	}

	FILE* foffsetsout = fopen(OutOffsetFile, "wb");
	if (!foffsetsout)
	{
		printf("ERROR: Can't open file %s for writing!\n", OutOffsetFile);
		perror("ERROR");
		return -1;
	}

	// write offsets file
	for (unsigned int i = 0; i < StringCount; i++)
	{
		fwrite(&OffsetList[i], sizeof(unsigned int), 1, foffsetsout);
	}

	fclose(foffsetsout);

	// write strings file
	for (unsigned int i = 0; i < StringCount; i++)
	{
		fwrite(StringList[i], sizeof(wchar_t), wcslen(StringList[i]) + 1, flangout);
	}

	fclose(flangout);

	return 0;
}

int ExportUTF8LangFiles(const char* OutLangFile, const char* OutOffsetFile)
{
	FILE* flangout = fopen(OutLangFile, "wb");
	if (!flangout)
	{
		printf("ERROR: Can't open file %s for writing!\n", OutLangFile);
		perror("ERROR");
		return -1;
	}

	FILE* foffsetsout = fopen(OutOffsetFile, "wb");
	if (!foffsetsout)
	{
		printf("ERROR: Can't open file %s for writing!\n", OutOffsetFile);
		perror("ERROR");
		return -1;
	}

	// write offsets file
	for (unsigned int i = 0; i < StringCount; i++)
	{
		fwrite(&OffsetList[i], sizeof(unsigned int), 1, foffsetsout);
	}

	fclose(foffsetsout);

	// write strings file
	for (unsigned int i = 0; i < StringCount; i++)
	{
		fwrite(MBStringList[i], sizeof(char), strlen(MBStringList[i]) + 1, flangout);
	}

	fclose(flangout);

	return 0;
}

#ifdef WIN32
DWORD GetDirectoryListing(const char* FolderPath) // platform specific code, using Win32 here, GNU requires use of dirent which MSVC doesn't have
{
	WIN32_FIND_DATA ffd = { 0 };
	TCHAR  szDir[MAX_PATH];
	char MBFilename[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;
	unsigned int NameCounter = 0;

	mbstowcs(szDir, FolderPath, MAX_PATH);
	StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

	if (strlen(FolderPath) > (MAX_PATH - 3))
	{
		_tprintf(TEXT("Directory path is too long.\n"));
		return -1;
	}

	hFind = FindFirstFile(szDir, &ffd);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		printf("FindFirstFile error\n");
		return dwError;
	}

	// count the files up first
	do
	{
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			FileCount++;
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES)
	{
		printf("FindFirstFile error\n");
	}
	FindClose(hFind);

	// then create a file list in an array, redo the code
	FileDirectoryListing = (char**)calloc(FileCount, sizeof(char*));
	bCompressedFileMarkers = (bool*)calloc(FileCount, sizeof(bool));
	//FileSizes = (unsigned int*)calloc(FileCount, sizeof(unsigned int*));

	ffd = { 0 };
	hFind = FindFirstFile(szDir, &ffd);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		printf("FindFirstFile error\n");
		return dwError;
	}

	do
	{
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			wcstombs(MBFilename, ffd.cFileName, MAX_PATH);
			FileDirectoryListing[NameCounter] = (char*)calloc(strlen(MBFilename) + 1, sizeof(char));
			strcpy(FileDirectoryListing[NameCounter], MBFilename);
			NameCounter++;
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES)
	{
		printf("FindFirstFile error\n");
	}

	FindClose(hFind);
	return dwError;
}
#else
void GetDirectoryListing(const char* FolderPath)
{
	printf("Directory listing unimplemented for non-Win32 platforms.\n");
}
#endif

int DetectEncodingType(const char* Filename)
{
	FILE* fin = fopen(Filename, "rb");

	if (!fin)
	{
		printf("ERROR: Can't open text file %s for reading!\n", Filename);
		perror("ERROR");
		return -1;
	}

	unsigned int ReadBytes = 0;

	fread(&ReadBytes, sizeof(int), 1, fin);
	
	// detect UTF-8 BOM
	if ((ReadBytes & 0xFFFFFF) == 0xBFBBEF)
	{
		fseek(fin, 3, SEEK_SET);
		fread(&ReadBytes, sizeof(int), 1, fin);
		// we care for the data at the topmost 3 bytes to detect singleline
		if ((ReadBytes & 0xFFFF0000) >> 8 == 0xBEBCEF)
			return ENCODETYPE_UTF8_SL;
		return ENCODETYPE_UTF8;
	}

	// detect UTF-16 BOM
	if ((ReadBytes & 0xFFFF) == 0xFEFF)
	{
		fseek(fin, 2, SEEK_SET);
		fread(&ReadBytes, sizeof(int), 1, fin);
		// we care for the data at the topmost 2 bytes to detect singleline
		if ((ReadBytes & 0xFFFF0000) >> 16 == 0xFF3E)
			return ENCODETYPE_UTF16_SL;
		return ENCODETYPE_UTF16;
	}

	// test for singleline with no BOM
	if ((ReadBytes & 0xFFFF0000) >> 8 == 0xBEBCEF)
		return ENCODETYPE_UTF8_SL;
	if ((ReadBytes & 0xFFFF0000) >> 16 == 0xFF3E)
		return ENCODETYPE_UTF16_SL;

	fclose(fin);
	return ENCODETYPE_UNK;
}

int ProcessTagForce1Folder(const char* InFolder, const char* OutFolder, const char InLanguageLetter) // TF1 has split files, this processes them recursively, gzip used where necessary
{
	char* FilenameEndPoint = 0;

	// get dir list and detect which are compressed behind gzip
	GetDirectoryListing(InFolder);
	for (unsigned int i = 0; i < FileCount; i++)
	{
		if (strcmp(strrchr(FileDirectoryListing[i], '.'), ".gz") == 0)
			bCompressedFileMarkers[i] = true;
	}

	// prep the output directory
	// check for folder existence, if it doesn't exist, make it
	if (stat(OutFolder, &st))
	{
		printf("Creating folder: %s\n", OutFolder);
		mbstowcs(MkDirPath, OutFolder, 1024);
		_wmkdir(MkDirPath);
	}

	// since there are always pairs of I*.bin and L*.bin files, we must always expect them to be together in the folder
	for (unsigned int i = 0; i < FileCount; i+=2)
	{
		strcpy(BaseName, FileDirectoryListing[i]);
		FilenameEndPoint = strchr(BaseName, '.') - 2;
		*FilenameEndPoint = 0;

		printf("Extracting: %s\n", BaseName);
		if (bCompressedFileMarkers[i])
		{
			sprintf(IDFilePath, "%s\\%sI%c.bin.gz", InFolder, BaseName, InLanguageLetter);
			sprintf(StrFilePath, "%s\\%sL%c.bin.gz", InFolder, BaseName, InLanguageLetter);
			sprintf(OutFilePath, "%s\\%s.gz.txt", OutFolder, BaseName);

			// decompression time
			// to avoid modifying the input directory, we must copy the input files to a temporary location (next to the binary)
			// this can be fixed by implementing zlib directly to this program
			// right now we're just using the gzip binary
			sprintf(SystemCmdBuffer, "@copy /Y \"%s\" tempI.bin.gz > NUL", IDFilePath);
			system(SystemCmdBuffer);
			sprintf(SystemCmdBuffer, "@copy /Y \"%s\" tempL.bin.gz > NUL", StrFilePath);
			system(SystemCmdBuffer);
			sprintf(SystemCmdBuffer, "@gzip -d -f -q tempI.bin.gz");
			system(SystemCmdBuffer);
			sprintf(SystemCmdBuffer, "@gzip -d -f -q tempL.bin.gz");
			system(SystemCmdBuffer);

			ParseStrings("tempL.bin", "tempI.bin");
			SpitStringsToFile(OutFilePath);
		}
		else
		{
			sprintf(IDFilePath, "%s\\%sI%c.bin", InFolder, BaseName, InLanguageLetter);
			sprintf(StrFilePath, "%s\\%sL%c.bin", InFolder, BaseName, InLanguageLetter);
			sprintf(OutFilePath, "%s\\%s.txt", OutFolder, BaseName);

			ParseStrings(StrFilePath, IDFilePath);
			SpitStringsToFile(OutFilePath);
		}
	}

	return 0;
}

int RepackTagForce1Folder(const char* InFolder, const char* OutFolder, const char InLanguageLetter)
{
	// get dir list and detect which are compressed behind gzip
	GetDirectoryListing(InFolder);
	for (unsigned int i = 0; i < FileCount; i++)
	{
		if (strcmp(strchr(FileDirectoryListing[i], '.'), ".gz.txt") == 0)
			bCompressedFileMarkers[i] = true;
	}

	// prep the output directory
	// check for folder existence, if it doesn't exist, make it
	if (stat(OutFolder, &st))
	{
		printf("Creating folder: %s\n", OutFolder);
		mbstowcs(MkDirPath, OutFolder, 1024);
		_wmkdir(MkDirPath);
	}

	for (unsigned int i = 0; i < FileCount; i++)
	{
		strcpy(BaseName, FileDirectoryListing[i]);
		*strchr(BaseName, '.') = 0;

		printf("Repacking: %s\n", BaseName);
		if (bCompressedFileMarkers[i])
		{
			sprintf(OutFilePath, "%s\\%s.gz.txt", InFolder, BaseName);
			sprintf(IDFilePath, "%s\\%sI%c.bin", OutFolder, BaseName, InLanguageLetter);
			sprintf(StrFilePath, "%s\\%sL%c.bin", OutFolder, BaseName, InLanguageLetter);

			switch (DetectEncodingType(OutFilePath))
			{
			case ENCODETYPE_UTF8_SL:
				ParseUTF8Text_SingleLine(OutFilePath);
				ExportUTF8LangFiles(StrFilePath, IDFilePath);
				break;
			case ENCODETYPE_UTF8:
				ParseUTF8Text(OutFilePath);
				ExportUTF8LangFiles(StrFilePath, IDFilePath);
				break;
			case ENCODETYPE_UTF16_SL:
				ParseUTF16Text_SingleLine(OutFilePath);
				ExportUTF16LangFiles(StrFilePath, IDFilePath);
				break;
			case ENCODETYPE_UTF16:
			default:
				ParseUTF16Text(OutFilePath); // UTF-16 default
				ExportUTF16LangFiles(StrFilePath, IDFilePath);
				break;
			}

			// compression time
			sprintf(SystemCmdBuffer, "@gzip -f -q \"%s\"", StrFilePath);
			system(SystemCmdBuffer);
			sprintf(SystemCmdBuffer, "@gzip -f -q \"%s\"", IDFilePath);
			system(SystemCmdBuffer);
		}
		else
		{
			sprintf(OutFilePath, "%s\\%s.txt", InFolder, BaseName);
			sprintf(IDFilePath, "%s\\%sI%c.bin", OutFolder, BaseName, InLanguageLetter);
			sprintf(StrFilePath, "%s\\%sL%c.bin", OutFolder, BaseName, InLanguageLetter);

			switch (DetectEncodingType(OutFilePath))
			{
			case ENCODETYPE_UTF8_SL:
				ParseUTF8Text_SingleLine(OutFilePath);
				ExportUTF8LangFiles(StrFilePath, IDFilePath);
				break;
			case ENCODETYPE_UTF8:
				ParseUTF8Text(OutFilePath);
				ExportUTF8LangFiles(StrFilePath, IDFilePath);
				break;
			case ENCODETYPE_UTF16_SL:
				ParseUTF16Text_SingleLine(OutFilePath);
				ExportUTF16LangFiles(StrFilePath, IDFilePath);
				break;
			case ENCODETYPE_UTF16:
			default:
				ParseUTF16Text(OutFilePath); // UTF-16 default
				ExportUTF16LangFiles(StrFilePath, IDFilePath);
				break;
			}
		}

	}

	return 0;

}

int ParseChrWordsTbl(const char* InFilename, const char* OutFilename)
{
	FILE* fin = fopen(InFilename, "rb");
	void* FileBuffer = NULL;
	WordsTbl* InWordsTbl = NULL;
	unsigned int* StrPosCalc = NULL;

	if (!fin)
	{
		printf("ERROR: Can't open file %s for reading!\n", InFilename);
		perror("ERROR");
		return -1;
	}

	// read the entire file to memory
	if (stat(InFilename, &st))
	{
		printf("ERROR: Can't find %s during size calculation!\n", InFilename);
		return -1;
	}

	FileBuffer = malloc(st.st_size);
	if (!FileBuffer)
	{
		printf("ERROR: Failed to allocate %d bytes for file %s!\n", st.st_size, InFilename);
		fclose(fin);
		return 0;
	}

	fread(FileBuffer, st.st_size, 1, fin);
	fclose(fin);

	InWordsTbl = (WordsTbl*)FileBuffer;
	StringList = (wchar_t**)((int)FileBuffer + InWordsTbl->HeaderSize);
	StrPosCalc = (unsigned int*)((int)FileBuffer + InWordsTbl->HeaderSize);
	StringCount = InWordsTbl->StrCount;

	// shift the list by header size + buffer location
	for (unsigned int i = 0; i < InWordsTbl->StrCount; i++)
	{
		StrPosCalc[i] += (int)FileBuffer + InWordsTbl->FullHeaderSize;;
	}

	// skip the very first one, because that's how the game IDs them
	//StringCount--;
	//StringList = (wchar_t**)((int)FileBuffer + InWordsTbl->HeaderSize + 4);

	SpitStringsToFile(OutFilename);
	free(FileBuffer);

	return 0;
}

int main(int argc, char *argv[])
{
	printf("Yu-Gi-Oh! Tag Force Language Tool\n");
	if (argc < 4)
	{
		printf(TFS_HELPTEXTSTRING);
		return -1;
	}

	if (argv[1][0] == '-' && argv[1][1] == 'w') // Write mode
	{
		if (argv[1][2] == '1') // TF1
		{
			bTF1FolderMode = true;
			if (argc < 5)
				printf("Missing LanguageLetter parameter! Defaulting to English (e)!\n");
			else
				LanguageLetter = argv[4][0];

			RepackTagForce1Folder(argv[2], argv[3], LanguageLetter);
			return 0;
		}

		if (argv[1][2] == 'c') // Single
		{
			printf("Unimplemented...\n");
			return 0;
		}

		if (argv[1][2] == '8') // UTF-8 explicit
		{
			ParseUTF8Text(argv[2]);
			ExportUTF8LangFiles(argv[3], argv[4]);
			return 0;
		}

		switch (DetectEncodingType(argv[2]))
		{
		case ENCODETYPE_UTF8_SL:
			ParseUTF8Text_SingleLine(argv[2]);
			ExportUTF8LangFiles(argv[3], argv[4]);
			return 0;
			break;
		case ENCODETYPE_UTF8:
			ParseUTF8Text(argv[2]);
			ExportUTF8LangFiles(argv[3], argv[4]);
			return 0;
			break;
		case ENCODETYPE_UTF16_SL:
			ParseUTF16Text_SingleLine(argv[2]);
			ExportUTF16LangFiles(argv[3], argv[4]);
			return 0;
			break;
		case ENCODETYPE_UTF16:
			ParseUTF16Text(argv[2]);
			ExportUTF16LangFiles(argv[3], argv[4]);
			return 0;
			break;
		default:
			ParseUTF16Text(argv[2]); // UTF-16 default
			ExportUTF16LangFiles(argv[3], argv[4]);
			break;
		}

		return 0;		
		
	}

	if (argv[1][0] == '-' && argv[1][1] == '8') // Read mode UTF-8
	{
		MBMode = true;
		ParseStrings(argv[2], argv[3]);
		SpitStringsToFile(argv[4]);
		return 0;
	}

	if (argv[1][0] == '-' && argv[1][1] == '1') // Read mode TF1
	{
		bTF1FolderMode = true;
		if (argc < 5)
			printf("Missing LanguageLetter parameter! Defaulting to English (e)!\n");
		else
			LanguageLetter = argv[4][0];

		ProcessTagForce1Folder(argv[2], argv[3], LanguageLetter);
		return 0;
	}

	if (argv[1][0] == '-' && argv[1][1] == 'c') // Read mode Single file
	{
		printf("Single mode\n");
		ParseChrWordsTbl(argv[2], argv[3]);
		return 0;
	}


	ParseStrings(argv[1], argv[2]);
	SpitStringsToFile(argv[3]);

    return 0;
}
