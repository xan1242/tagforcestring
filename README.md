# Yu-Gi-Oh Tag Force Language Tool

This is a utility designed to extract and repack the language transcript of Yu-Gi-Oh! Tag Force games.

Specifically the files located in script/story_scr_x.ehp (EveIx.bin and EveLx.bin)

## FEATURES

- Extract a script to an editable text file
- Repack from that text file back to a UTF-16 or UTF-8 script (both EveIx.bin and EveLx.bin)

## USAGE

```
USAGE: TagForceString InLanguageFile InOffsetFile OutFile
USAGE (UTF-8): TagForceString -8 InLanguageFile InOffsetFile OutFile
USAGE (write): TagForceString -w InTextFile OutLangFile OutOffsetFile
USAGE (write UTF-8): TagForceString -w8 InTextFile OutLangFile OutOffsetFile
USAGE (Tag Force 1 folder): TagForceString -1 InFolder OutFolder [LanguageLetter] (example: e = English)
USAGE (Tag Force 1 folder repack): TagForceString -w1 InFolder OutFolder [LanguageLetter] (example: j = Japanese)
USAGE (Single): TagForceString -c InWordsTbl OutFile
USAGE (write Single): TagForceString -wc InWordsTxt OutFile
```

Example (read): `TagForceString EveLe.bin EveIe.bin tagforce5.txt`

Example (write): `TagForceString.exe -w tagforce5.txt EveLe.bin EveIe.bin`

Example (write in UTF-8): `TagForceString.exe -w8 tagforce5.txt EveLe.bin EveIe.bin`

Example (Tag Force 1 Folder Extraction): `TagForceString.exe -1 st01_t01_e TF1Script e`

Example (Tag Force 1 Folder Repack): `TagForceString.exe -w1 TF1Script st01_t01_e e`

The utility extracts the result into a UTF-16 text file which can be opened by any text editor that supports it, such as [Kate](https://kate-editor.org). You may only need to manually set the format to be specifically Unicode UTF-16 and not UCS-2.

## Tag Force 1 Folder Notes

This mode requires usage of the gzip binary either next to the executable or in your PATH.

You can find gnuwin32 gzip here: http://gnuwin32.sourceforge.net/downlinks/gzip-bin-zip.php

Please note that this is only temporary and is planned to be removed.

## TXT FORMATTING

The format is as follows (example from Tag Force 6 translation by Clickclaxer01):

```
39385 // total count of strings
[0] // string number designator (number isn't read actually, only the brackets)
Oh man, check out this crowd! // string
It's huge!
Who would've thought there'd be so many
people here?
[1]
The guests of honor at today's reception are quite
popular it seems.
```

## TODO

- Add string reusing - this optimization will allow for some repeated strings (such as ".......") be reused allowing for even smaller resulting files
- Integrate zlib to the code to remove gzip dependency
- Fix memory leaks introduced by TF1 folder mode
- Remove Windows dependency, dir listing currently works on Win32 API only
