#include <stdio.h>
#include <stdint.h>
#include "hex_txt.h"
#include <string.h>

#define BIN_SIZE_ONE_READ 1
// one bin byte will be represent by 2 bytes text
#define TEXT_SIZE_ONE_READ 2 * BIN_SIZE_ONE_READ

#define CHUNK_SIZE_ONE_READ 1000

// SDClass SD;

uint16_t pos;
uint8_t idx0;
uint8_t idx1;

// Method that does the actual conversion
uint8_t HexTxt::convert_text2bin(const char* str, uint8_t* bytes, size_t blen){
	memset(bytes, 0, blen);
	for(pos = 0; ((pos < (blen * 2)) && (pos < strlen(str))); pos += 2){
		// Serial.printf("Pos: %u.\n", pos);
		// delay(50);
		idx0 = (uint8_t)str[pos + 0];
		idx1 = (uint8_t)str[pos + 1];

		bytes[pos / 2] = (uint8_t)(hashmap[idx0] << 4) | hashmap[idx1];
	}

	return (0);
}
// this function converts firmware file to text file and put it at http server
// this specific version is to be used outside as a stand alone program
bool HexTxt::convert_bin2text(void){
	FILE* bin_file;
	FILE* text_file;
	uint8_t buffer[BIN_SIZE_ONE_READ];

	bin_file = fopen("firmware.bin", "rb");
	text_file = fopen("firmware.txt", "w");

	if(!bin_file){
        // printf("Unable to open te file");
		return false;
	}

	int count = 0;
	while(1){
		size_t len = fread(&buffer, sizeof(char), BIN_SIZE_ONE_READ, bin_file);
		if(len == 0) break;

		char bin2text[3] = {0, 0, 0};
		sprintf(bin2text, "%02X", buffer[0]);
		// printf("%d - %d - %s\n", buffer[0], strlen(bin2text), bin2text);
		fprintf(text_file, "%s", bin2text);
		count++;
	}
	// printf("count = %d\n", count);
	fclose(bin_file);
	fclose(text_file);
	return true;
}


bool HexTxt::convert_text2binSPIFFS(fs::File txtFile, fs::File binFile){
	char BigBuffer[TEXT_SIZE_ONE_READ * CHUNK_SIZE_ONE_READ];

	if(!binFile || !txtFile){
#ifdef DEBUG
		if(!binFile){
			Serial.println("Bin file not opened");
		}
		if(!txtFile){
			Serial.println("Unable to open txt!");
		}
#endif
		return 1;
	}

	int count = 0;
	int index = 1;
#ifdef DEBUG
	Serial.printf("Txt File size: %u\n", txtFile.size());
	Serial.printf("Bin File size:%u\n", binFile.size());
#endif
	uint8_t* text2bin = new uint8_t[CHUNK_SIZE_ONE_READ];
	int binTotal = 0;
	int bufToRead = 0;

	while (1){
		if((txtFile.size() - binTotal) < CHUNK_SIZE_ONE_READ * 2){
			bufToRead = txtFile.size() - binTotal;
		}
		else{
			bufToRead = TEXT_SIZE_ONE_READ * CHUNK_SIZE_ONE_READ;
		}
		size_t length = txtFile.read((uint8_t *)BigBuffer, bufToRead);
		binTotal += length;
		if(length == 0){
			break;
		}
		else{
			convert_text2bin(BigBuffer, text2bin, length / 2);
			binFile.write(text2bin, length / 2);
			count += (length / 2);
			if(index >= 10){
				index = 0;
				Serial.printf("\nWritten %u / %u.", count, txtFile.size() / 2);
			}
		}
		index++;
	}
	Serial.println();

#ifdef DEBUG
	Serial.printf("Txt File size: %u\n", txtFile.size());
	Serial.printf("Bin File size: %u\n", binFile.size());
	Serial.printf("Size Written: %d\n", count);
	Serial.printf("Size of binFile: %u\n", binFile.size());
#endif

	return 0;
}




// ///////////////////////////////////////////////////////////
//
//  SD specific mehtods. Not up to date
//  Trying to find a way to not have to write two different
//  that do the same thing but for SPIFFS or SD
//
// ///////////////////////////////////////////////////////////

bool HexTxt::convert_text2binSD(SDLib::File txtFile, SDLib::File binFile){
	char BigBuffer[TEXT_SIZE_ONE_READ * 1024];
	if(!binFile || !txtFile){
#ifdef DEBUG
		if(!binFile){
			Serial.println("Unable to open Bin!");
		}
		if(!txtFile){
			Serial.println("Unable to open txt!");
		}
#endif
		return false;
	}

	int count = 0;
	uint8_t* text2bin = new uint8_t[1024];
	int binTotal = 0;
	int bufToRead = 0;
	while(1){
		if ((txtFile.size() - binTotal) < 2048){
			bufToRead = txtFile.size() - binTotal;
		}
		else{
			bufToRead = TEXT_SIZE_ONE_READ * 1024;
		}

		size_t length = txtFile.read(BigBuffer, bufToRead);
		binTotal += length;
		if(length == 0){
			break;
		}
		{
			convert_text2bin(BigBuffer, text2bin, length / 2);
			binFile.write(text2bin, length / 2);
			count += (length / 2);

#ifdef DEBUG
			Serial.printf("\nWritten %u / %u.", count, txtFile.size() / 2);
#endif
		}
	}
#ifdef DEBUG
	Serial.println("");
	Serial.printf("Size Written: %d\n", count);
	Serial.printf("Size of binFile: %u\n", binFile.size());
#endif
	return true;
}