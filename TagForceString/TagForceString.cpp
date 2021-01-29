// Yu-Gi-Oh! Tag Force Language Tool
// by Xan
// TODO: add string reusage

#include "stdafx.h"
#include <stdlib.h>
#include <string.h>
#include "TagForceString.h"

unsigned int StringCount = 0;
unsigned int* OffsetList;
wchar_t* StringBuffer;
wchar_t** StringList;

char* MBStringBuffer;
char** MBStringList;
struct stat st = { 0 };

bool MBMode = false;

int ParseStrings(const char* InLangFile, const char* InOffsetFile)
{
	FILE* foffsets = fopen(InOffsetFile, "rb");
	if (!foffsets)
	{
		printf("ERROR: Can't open offset file %s for reading!\n", InOffsetFile);
		perror("ERROR");
		return -1;
	}

	FILE* flang = fopen(InLangFile, "r");
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
	if (MBMode)
	{
		MBStringBuffer = (char*)calloc(st.st_size, sizeof(char));
		MBStringList = (char**)calloc(StringCount, sizeof(char*));
		fread(MBStringBuffer, st.st_size, 1, flang);
	}
	else
	{
		StringBuffer = (wchar_t*)calloc(st.st_size, sizeof(char));
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

int SpitStringsToFile(const char* InLangFile, const char* OutFilename)
{
	FILE* fout = fopen(OutFilename, "wb");

	if (!fout)
	{
		printf("ERROR: Can't open file %s for writing!\n", OutFilename);
		perror("ERROR");
		return -1;
	}

	if (MBMode)
		fprintf(fout, "%d\n", StringCount);
	else
		fwprintf(fout, L"%ld\n", StringCount);

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
		if (argv[1][2] == '8') // UTF-8
		{
			ParseUTF8Text(argv[2]);
			ExportUTF8LangFiles(argv[3], argv[4]);
			return 0;
		}

		ParseUTF16Text(argv[2]);
		ExportUTF16LangFiles(argv[3], argv[4]);
		return 0;
	}

	if (argv[1][0] == '-' && argv[1][1] == '8') // Read mode UTF-8
	{
		MBMode = true;
		ParseStrings(argv[2], argv[3]);
		SpitStringsToFile(argv[2], argv[4]);
		return 0;
	}

	ParseStrings(argv[1], argv[2]);
	SpitStringsToFile(argv[1], argv[3]);

    return 0;
}
