This program is a (command-line only currently) ECL interpreter for the newer Team Shanghai Alice ("Touhou Project") games.
I'm mostly writing it as a learning exercise rather than to be something useful, and the code is pretty messy at the moment.
I'll put some things I've learned that aren't documented anywhere else outside of source code in here as well.

# ECL Overview
ECL stands for "Enemy-control Language," a translation of the Japanese 敵コントロール言語.
ECL files are compiled scripts responsible for controlling stages, including enemy and bullet spawns and boss patterns.
Everything below describes the ECL file format for MoF (Touhou 10) and later.
All multiple-byte integers use Intel/x86 byte order, i.e. little-endian.

## Header
ECL files begin with a fixed-length header with the following fields in order:
* `magic`, 4 bytes. Always the string "SCPT"
* `unknown1`, 2 bytes. Always the value 1 as far as I know.
* `include_length`, 2 bytes. The length of the entire ANIM/ECLI include section in bytes
* `include_offset`, 4 bytes. The offset in bytes to the ANIM/ECLI include section. The length of the header.
* `zero1`, 4 bytes. Always zero.
* `sub_count`, 4 bytes. The number of subroutines ("subs") in the ECL file.
* `zero2`, 16 bytes. Always zero.

## Includes
Immediately following the header are two lists of filenames to include, one for ANM files ("ANIM") and
the other for other ECL files ("ECLI"). Each one is structured as follows:
* `name`, 4 bytes. Either "ECLI" or "ANIM" depending on which list it is.
* `count`, 4 bytes. Number of files in this include list.
* `data`, variable size. The filenames as a sequence of null-terminated strings.

There is one final caveat: each include list has zero padding at the end to make the offset of what comes next
4-byte aligned with respect to the *start of the file*. You can make a pointer to the end of an include list
aligned by doing something like
```C
uint8_t* end = ...; /* end of the current list */
uint8_t* p = end + (((uintptr_t)end) & 0x03);
```

## Sub offsets and names
After the includes (plus any padding for alignment; see the previous section) are two lists.
The first list gives offsets in bytes to each sub in the file, and will consist of `sub_count` 4-byte unsigned integers.
The second list gives the respective names of each sub, and is a sequence of null-terminated strings.

## Subs
After the sub offsets and names are the subs themselves. They have a simple header:
* `magic`, 4 bytes. Always "ECLH".
* `data_offset`, 4 bytes. Relative offset within the sub structure to the first instruction.
* `zero`, 8 bytes. Always zero.
Immediately following the header is the first instruction in the sub.

## Instructions
Instructions probably have the most complex structure of objects in the ECL format.
* `time`, 4 bytes. The earliest time (in frames) the instruction will execute.
* `id`, 2 bytes. The identifier/opcode of the instruction. See [this list](https://priw8.github.io/#s=modding/ins).
* `size`, 2 bytes. Total size of the instruction including parameters.
* `param_mask`, 2 bytes. Mask for parameters indicating whether they are immediate or variable references.
* `rank_mask`, 1 byte. Bitmask of difficulties to execute the instruction on. `1111LHNE`
* `param_count`, 1 byte. Number of parameters of the instruction.
* `zero`, 4 bytes. From TH13 on, is used for something I don't understand yet.

The parameters of the instruction follow this object immediately.
Floats, ints, and variable references seem to just be 4-byte values right after the instruction.
If an instruction takes a string, it is given as a null-terminated string after the instruction as well.
Jump instructions have an offset (4-byte signed integer) and time (4-byte unsigned integer) as params.

# Interpreter State
ECL uses a stack based interpreter. Most instructions operate on the stack and local variables are
also stored on the stack. The call stack is separate from the main stack.
There are also global variables and "local" variables which exist outside of the stack.

# Sources
Where I got information I used for implementation.
* [thtk source](https://github.com/thpatch/thtk), mostly that of [thecl](https://github.com/thpatch/thtk/tree/master/thecl) and [thecl10.c](https://github.com/thpatch/thtk/blob/master/thecl/thecl10.c) in particular
* [Touhou 17 data types](https://gist.github.com/32th-System/797a91e427e3db24dc4b994189b7386f) - the bullet manager structure used by WBaWC
* [ECL Instruction Table](https://priw8.github.io/#s=modding/ins) and [ECL Variable Table](https://priw8.github.io/#s=modding/vars)
* [Documentation on the Chinese wiki](https://thwiki.cc/%E8%84%9A%E6%9C%AC%E5%AF%B9%E7%85%A7%E8%A1%A8/ECL)
* [Dass' documentation on the wiki](https://en.touhouwiki.net/wiki/User:Mddass/Touhou_File_Format_Specification/ECL)
* Looking at ECL files in a hex editor, as well as "decompiled" ECL source from the official games
* Research by Priw, 32th System, and Dai
