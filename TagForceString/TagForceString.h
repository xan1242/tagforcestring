#pragma once
#define TFS_HELPTEXTSTRING "USAGE: TagForceString InLanguageFile InOffsetFile OutFile\n\
USAGE (UTF-8): TagForceString -8 InLanguageFile InOffsetFile OutFile\n\
USAGE (write): TagForceString -w InTextFile OutLangFile OutOffsetFile\n\
USAGE (write UTF-8): TagForceString -w8 InTextFile OutLangFile OutOffsetFile\n\
USAGE (Tag Force 1 folder): TagForceString -1 InFolder OutFolder [LanguageLetter] (example: e = English)\n\
USAGE (Tag Force 1 folder repack): TagForceString -w1 InFolder OutFolder [LanguageLetter] (example: j = Japanese)\n\
USAGE (Single): TagForceString -c InWordsTbl OutFile\n\
USAGE (write Single): TagForceString -wc InWordsTxt OutFile [UNIMPLEMENTED]\n\
NOTE: Tag Force 1 folder operations require the gzip binary (either in PATH or next to this binary!)\n\
NOTE: If a supported BOM (Byte Order Mark) is detected, it'll use that format! Supported: UTF-16 and UTF-8.\n"

#define ENCODETYPE_UNK 0
#define ENCODETYPE_UTF16 1
#define ENCODETYPE_UTF8 2
#define ENCODETYPE_UTF16_SL 3
#define ENCODETYPE_UTF8_SL 4
