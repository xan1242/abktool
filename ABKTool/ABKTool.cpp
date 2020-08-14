// EA Black Box ABK Tool
// by Xanvier 08/2020.
// TODO: lotsa stuff
// TODO: S10A bank format support
// TODO: repacking of banks (SX will mostly be used)

#include "stdafx.h"
#include <stdlib.h>
#include <string.h>
#include <direct.h>

#define SX_SCRIPT_FORMAT "sx.exe -wave \"%s\" -=\"%s\" -onetomany"

char GeneratedBnkOutName[255];
char FilenameNoExt[255];
char OutFilenameSX[255];
char BatchScript[255];

short BnkNumElements;

bool bBigEndian = false;

struct stat st = { 0 };

enum AemsPlatform
{
	PC = 0,
	Unk1,
	PS2,
	Xbox,
	GC,
	Xenon,
	PS3 = 10
};

char PlatformStrings[11][8] = {
	"PC",
	"Unknown",
	"PS2",
	"Xbox",
	"GC",
	"Xenon",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"PS3"
};

struct AemsDef_Snd10SampleBankHeader
{
	unsigned char id[4];
	unsigned char version;
	char pad;
	unsigned short serialNumber;
	int numSamples;
};

class CListDNode
{
	class CListDNode* pnext;
	class CListDNode* pprev;
};

struct AemsDef_TWEAKHEADER
{
	char id[4];
	unsigned char ver;
	unsigned char platform;
	char pad[2];
	int crc;
	int numcomponents;
};

struct AemsDef_ModuleBank
{
	char id[4];
	unsigned char ver;
	unsigned char veraimexmajor;
	unsigned char veraimexminor;
	unsigned char veraimexpatch;
	unsigned char platform;
	unsigned char targetType;
	unsigned short nummodules;
	int debugcrc;
	int uniqueid;
	int totalsize;
	int residentsize;
	int moduleoffset;
	int sfxbankoffset;
	int sfxbanksizepadded;
	int midibankoffset;
	int midibanksizepadded;
	int funcfixupoffset;
	int staticdatafixupoffset;
	int interfaceOffset;
	int modulebankhandle;
	struct AemsDef_Snd10SampleBankHeader* pSnd10SampleBankHeader;
	int midibhandle;
	char* streamfilepath;
	int streamfileoffset;
	class CListDNode ln;
	struct AemsDef_TWEAKHEADER* ptweakheader;
}InBank;

int ParseAemsBank(const char* InFilename)
{
	FILE* f = fopen(InFilename, "rb");
	if (!f)
	{
		printf("ERROR: Can't open file %s for reading!\n", InFilename);
		perror("ERROR");
		return -1;
	}

	fread(&InBank, sizeof(AemsDef_ModuleBank), 1, f);

	if (InBank.platform == GC || InBank.platform == Xenon || InBank.platform == PS3) // big endian platforms use 2 bytes for targetType 
	{
		bBigEndian = true;
		InBank.nummodules = _byteswap_ushort(InBank.nummodules);
		InBank.debugcrc = _byteswap_ulong(InBank.debugcrc);
		InBank.uniqueid = _byteswap_ulong(InBank.uniqueid);
		InBank.totalsize = _byteswap_ulong(InBank.totalsize);
		InBank.residentsize = _byteswap_ulong(InBank.residentsize);
		InBank.moduleoffset = _byteswap_ulong(InBank.moduleoffset);
		InBank.sfxbankoffset = _byteswap_ulong(InBank.sfxbankoffset);
		InBank.sfxbanksizepadded = _byteswap_ulong(InBank.sfxbanksizepadded);
		InBank.midibankoffset = _byteswap_ulong(InBank.midibankoffset);
		InBank.midibanksizepadded = _byteswap_ulong(InBank.midibanksizepadded);
		InBank.funcfixupoffset = _byteswap_ulong(InBank.funcfixupoffset);
		InBank.staticdatafixupoffset = _byteswap_ulong(InBank.staticdatafixupoffset);
		InBank.interfaceOffset = _byteswap_ulong(InBank.interfaceOffset);
		InBank.modulebankhandle = _byteswap_ulong(InBank.modulebankhandle);
		InBank.pSnd10SampleBankHeader = (AemsDef_Snd10SampleBankHeader*)_byteswap_ulong((long)InBank.pSnd10SampleBankHeader);
		InBank.midibhandle = _byteswap_ulong(InBank.midibhandle);
		InBank.streamfilepath = (char*)_byteswap_ulong((long)InBank.streamfilepath);
		InBank.streamfileoffset = _byteswap_ulong(InBank.streamfileoffset);
		//InBank.ln.ppnext = //_byteswap_ulong((long)InBank.ln); // not really necessary, we read until interfaceoffset anyway...
		InBank.ptweakheader = (AemsDef_TWEAKHEADER*)_byteswap_ulong((long)InBank.ptweakheader);
	}

	printf("ID: %c%c%c%c\n", InBank.id[0], InBank.id[1], InBank.id[2], InBank.id[3]);
	printf("Aimex Version: %hhd %hhd.%hhd patch: %hhd\n", InBank.ver, InBank.veraimexmajor, InBank.veraimexminor, InBank.veraimexpatch);
	printf("Platform: %s\n", PlatformStrings[InBank.platform]);
	printf("Target type: %hhd\n", InBank.targetType);
	printf("Num. modules: %hd\n", InBank.nummodules);
	printf("Debug CRC: %X\n", InBank.debugcrc);
	printf("Unique ID: %X\n", InBank.uniqueid);
	printf("Total size: %X\n", InBank.totalsize);
	printf("Resident size: %X\n", InBank.residentsize);
	printf("Module offset: %X\n", InBank.moduleoffset);
	printf("SFX bank offset: %X\n", InBank.sfxbankoffset);
	printf("SFX bank size padded: %X\n", InBank.sfxbanksizepadded);
	printf("MIDI bank offset: %X\n", InBank.midibankoffset);
	printf("MIDI bank size padded: %X\n", InBank.midibanksizepadded);
	printf("Func Fixup offset: %X\n", InBank.funcfixupoffset);
	printf("Static data Fixup offset: %X\n", InBank.staticdatafixupoffset);
	printf("Interface offset: %X\n", InBank.interfaceOffset);
	printf("\n");
	//getchar();
	//printf("pSnd10SampleBankHeader: %X\n", InBank.pSnd10SampleBankHeader);
	//printf("midibhandle: %X\n", InBank.midibhandle);


	fclose(f);
	return 0;
}

int ExtractSFXBank(const char* InFilename, const char* OutFilename)
{
	FILE* f = fopen(InFilename, "rb");
	FILE* fout = fopen(OutFilename, "wb");
	void* FileBuffer;
	if (!f)
	{
		printf("ERROR: Can't open file %s for reading!\n", InFilename);
		perror("ERROR");
		return -1;
	}

	if (!fout)
	{
		printf("ERROR: Can't open file %s for writing!\n", OutFilename);
		perror("ERROR");
		return -1;
	}

	fseek(f, InBank.sfxbankoffset, SEEK_SET);
	FileBuffer = malloc(InBank.sfxbanksizepadded);

	fread(FileBuffer, InBank.sfxbanksizepadded, 1, f);
	fwrite(FileBuffer, InBank.sfxbanksizepadded, 1, fout);

	free(FileBuffer);
	fclose(fout);
	fclose(f);
	return 0;
}

short GetBNKNumElements(const char* InFilename)
{
	FILE* f = fopen(InFilename, "rb");
	short ReturnVal = 0;
	if (!f)
	{
		printf("ERROR: Can't open file %s for reading!\n", InFilename);
		perror("ERROR");
		return -1;
	}
	fseek(f, 0x6, SEEK_SET);
	fread(&ReturnVal, sizeof(short), 1, f);
	if (bBigEndian)
		ReturnVal = _byteswap_ushort(ReturnVal);
	fclose(f);
	return ReturnVal - 1;
}

int ExtractBNK(const char* InFilename, const char* OutPath)
{
	// check bank's file extension first before starting SX
	FILE* f = fopen(InFilename, "rb");
	char* BasisPoint;
	char ReadBytes[4];
	if (!f)
	{
		printf("ERROR: Can't open file %s for reading!\n", InFilename);
		perror("ERROR");
		return -1;
	}

	fread(&ReadBytes, sizeof(char), 4, f);
	if (!(ReadBytes[0] == 'B') && !(ReadBytes[1] == 'N') && !(ReadBytes[2] == 'K'))
	{
		printf("ERROR: File %s cannot be extracted by SX due to an incompatible format!\n", InFilename); // TODO: Figure out S10A format! (used in Xenon MW and Carbon NextGen and newer, CG uses standard BNK)
		printf("ERROR: File should start with BNKx but this one starts with %c%c%c%c !\n", ReadBytes[0], ReadBytes[1], ReadBytes[2], ReadBytes[3]);
		fclose(f);
		return -1;
	}

	fclose(f);

	if (stat(OutPath, &st) == -1)
	{
		printf("Creating directory %s\n", OutPath);
		sprintf(BatchScript, "mkdir \"%s\"", OutPath);
		system(BatchScript);
	}

	char* FilenameBase = (char*)calloc(strlen(InFilename), sizeof(char));
	strcpy(FilenameBase, InFilename);
	BasisPoint = strrchr(FilenameBase, '\\');

	if (BasisPoint != NULL)
	{
		FilenameBase = BasisPoint;
		FilenameBase += 1;
	}

	FilenameBase[strlen(FilenameBase) - 4] = 0;
		

	sprintf(OutFilenameSX, "%s\\%s", OutPath, FilenameBase);
	sprintf(BatchScript, SX_SCRIPT_FORMAT, InFilename, OutFilenameSX);
	printf("STARTING SX\n");
	system(BatchScript);

	char* OldName = (char*)calloc(strlen(OutFilenameSX) + 6, sizeof(char));
	char* NewName = (char*)calloc(strlen(OutFilenameSX) + 6, sizeof(char));

	// fixup filenames generated by SX...
	for (short i = 0; i < BnkNumElements; i++)
	{
		sprintf(OldName, "%s.%hd", OutFilenameSX, i+1);
		sprintf(NewName, "%s\\%s_%hd.wav", OutPath, FilenameBase, i+1);
		printf("Renaming %s to %s\n", OldName, NewName);
		rename(OldName, NewName);
	}

	//free(NewName);
	//free(OldName);

	return 0;
}

int main(int argc, char *argv[])
{
	printf("EA Black Box AEMS Bank tool\n\n");
	if (argc < 2)
	{
		printf("ERROR: Too few arguments.\nUSAGE: %s FILE.ABK\n", argv[0]);
		return -1;
	}
	ParseAemsBank(argv[1]);
	strcpy(GeneratedBnkOutName, argv[1]);
	strcpy(FilenameNoExt, argv[1]);

	// generate an output filename based on the input ABK name, just swap the file ext.
	GeneratedBnkOutName[strlen(GeneratedBnkOutName) - 3] = 'b';
	GeneratedBnkOutName[strlen(GeneratedBnkOutName) - 2] = 'n';
	GeneratedBnkOutName[strlen(GeneratedBnkOutName) - 1] = 'k';
	// also string without extension
	FilenameNoExt[strlen(FilenameNoExt) - 4] = 0;

	ExtractSFXBank(argv[1], GeneratedBnkOutName);
	BnkNumElements = GetBNKNumElements(GeneratedBnkOutName);
	ExtractBNK(GeneratedBnkOutName, FilenameNoExt);

    return 0;
}
