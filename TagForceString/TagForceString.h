#pragma once
#define TFS_HELPTEXTSTRING "USAGE: TagForceString InLanguageFile InOffsetFile OutFile\n\
USAGE (UTF-8): TagForceString -8 InLanguageFile InOffsetFile OutFile\n\
USAGE (write): TagForceString -w InTextFile OutLangFile OutOffsetFile\n\
USAGE (write UTF-8): TagForceString -w8 InTextFile OutLangFile OutOffsetFile\n\
USAGE (Tag Force 1 folder): TagForceString -1 InFolder OutFolder [LanguageLetter] (example: e = English)\n\
USAGE (Tag Force 1 folder repack): TagForceString -w1 InFolder OutFolder [LanguageLetter] (example: j = Japanese)\n\
USAGE (Single): TagForceString -c InWordsTbl OutFile\n\
USAGE (write Single): TagForceString -wc InWordsTxt OutFile\n\
NOTE: Tag Force 1 folder operations require the gzip binary (either in PATH or next to this binary!)\n"