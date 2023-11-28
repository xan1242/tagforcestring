# Yu-Gi-Oh! Tag Force Language & String Tool

A general purpose string data utility for Yu-Gi-Oh! Tag Force games.

This tool was designed to aid in translation of the games.

## FEATURES

- Convert between string resource (strtbl/wordstbl) bins and editable text files

- Convert between language bins and editable text files

- Batch convert a folder with language bins to a folder with editable text files (and back)

- UTF-8 writing option

- String reusage optimization

## USAGE

```
Usage: TagForceString [OPTIONS] MODE INPUT OUTPUT

OPTIONS:
  -u, --utf8          Use UTF-8 encoding (default is UTF-16)
  -d, --no-autodetect Disable BOM autodetection for input text files

MODES:
  1. bin2txt           Convert a string resource (strtbl) file to a text file
  2. txt2bin           Convert a text file to a string resource (strtbl) file
  3. lang2txt          Convert a pair of lang files (index and strings) to a text file
  4. txt2lang          Convert a text file to a pair of lang files (index and strings)
  5. fold2txt          Batch convert a folder with lang file pairs to a folder with text files
  6. txt2fold          Batch convert a folder with text files to a folder with lang file pairs
```

### Examples

1. Convert a string resource bin to a txt file
   
   `TagForceString bin2txt input_e.bin output.txt`

2. Convert a txt file back to a string resource bin
   
   `TagForceString txt2bin input.txt output_e.bin`

3. Convert a lang file pair to a txt file
   
   `TagForceString lang2txt langIe.bin langLe.bin output.txt`

4. Convert a txt file to a lang file pair
   
   `TagForceString txt2lang input.txt outIe.bin outLe.bin`

5. Convert a folder with lang file pairs to a folder with txt files
   
   `TagForceString fold2txt in_folder out_folder`

6. Convert a folder with txt files to a folder with lang file pairs
   
   `TagForceString txt2fold in_folder out_folder`

### Notes / Caveats

1. This tool does NOT perform any conversion on the fly! If you need to write a UTF-8 file, you must use a UTF-8 file as input. Same goes for UTF-16 - if you need to write a UTF-16 file, you must use a UTF-16 Little Endian file as input!

2. The tool attempts to autodetect BOM only for modes `txt2bin` & `txt2lang`. If not detected, it will default to UTF-16 Little Endian.

3. To be able to use UTF-8 in the games, the game must first be patched in order to support it!

4. The folder conversion modes MUST use the format of the original filenames in all cases! 

It must follow the format of `<name><typechar><langchar>.bin` (or `<name><typechar><langchar>.bin.gz` if it's compressed)

- Examples would be: `ImpChrMsgIe.bin` & `ImpChrMsgLe.bin`



Same goes for the txt files. They must follow the format it was exported as:

`<name>_<langchar>.txt` (or `<name>_<langchar>.gz.txt` if it's compressed)

- Example would be: `ImpChrMsg_e.txt`



Any deviations from this will result either in an error or the file being skipped!

## TXT FORMATTING

The txt file formatting is an **ini-like** (not ini) format.

Example from Tag Force 1 (English script of Crowler/Chronos):

```
[0]
Have a chat.
Ask to be partners.

[1]
You may be in the same
dorm as him...
[2]
But try not to become like
that dropout boy!
[3]
Haha!
Be partners... With you?!
```

### Notes

- Strings are read between the first line after the `]`  and the last line behind `[` (or EOF). 
  Everything between sections should be represented in the exact format as it is in the game's own dialog boxes.

- In the example provided above, the string 0 will have an empty line at the end

- There will be some empty strings - this is perfectly normal

- Strings that have exact same content will be condensed into one and each subsequent one will have a repeated pointer of the first one - this was done as an optimization to reduce file size and works perfectly fine with the games

- Strings' index references are hardcoded in the game code, so you cannot change them from here

## TODO

- Create a CMake (or some other type) project for this to make a more portable version - zlib has to be linked against this application!
