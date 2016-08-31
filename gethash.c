#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "hash_helper.h"

typedef struct {
    unsigned short value;
} xorStruct;

typedef struct {
    unsigned int value;
} crcStruct;

void hashXor(xorStruct *xorh,int input){
    xorh->value = xorh->value ^ input;
}

void crc32b(int byte, crcStruct *crc) {
   int i, j;
   unsigned int mask;

   i = 0;

   crc->value = crc->value ^ byte;
   for (j = 7; j >= 0; j--) {    // Do eight times.
       mask = -(crc->value & 1);
       crc->value = (crc->value >> 1) ^ (0xEDB88320 & mask);
   }
   i = i + 1;
}

int main(int argc, char *argv[])
{
    // TODO:

    int input;
    int length = 0;
    char err;
    FILE* file = stdin;
    //boolean array for arguments [-md5] [-c16] [-c32] [-xor] [-hex] [-f file]
    //(0 if arg wasn't loaded/1 if was)
    int args[] = {0,0,0,0,0,0};

    //Ak užívateľ nezadá žiadny parameter
    if (argc < 1){
        fprintf(stderr, "no argument input");
        return 1;
    }

    for (int i = 1; i < argc; i++){
        err = 1;
        if (strcmp(argv[i], "-f") == 0){
            file = fopen(argv[i+1], "r");
            if (file == NULL) {
                fprintf(stderr, "could not open the file\n");
                return 1;
            }
            i++;
            continue;
            err = 0;
        }
        if (strcmp(argv[i], "-md5") == 0){ args[0] = 1; err = 0; }
        if (strcmp(argv[i], "-c16") == 0){ args[1] = 1; err = 0; }
        if (strcmp(argv[i], "-c32") == 0){ args[2] = 1; err = 0; }
        if (strcmp(argv[i], "-xor") == 0){ args[3] = 1; err = 0; }
        if (strcmp(argv[i], "-hex") == 0){ args[4] = 1; err = 0; }
        //zadá neplatný parameter
        if (err == 1){
            fprintf(stderr, "your argument is invalid\n");
            return 2;
        }
    }
    //nezadá ani jeden parameter určujúci ktorý hash sa má počítať
    if ((args[0] == 0) && (args[1] == 0) && (args[2] == 0) && (args[3] == 0)){
        fprintf(stderr, "no hash argument match");
        return 3;
    }
    // CRC16
    crc16_context crc16;
    crc16_init(&crc16);

    // XOR
    xorStruct xorh;
    xorh.value = 0;

    // MD5
    MD5_CTX md5;
    unsigned char result[16];
    MD5_Init(&md5);

    // CRC32
    crcStruct crc;
    crc.value = 0xFFFFFFFF;

    // loop
    while ((input = fgetc(file)) != EOF){
        crc16_update(&crc16, input);
        hashXor(&xorh, input);
        MD5_Update(&md5, &input, sizeof(char));
        crc32b(input, &crc);
        length += 1;
    }
    MD5_Final(result, &md5);

    // output
    if (args[4] == 0){
        printf("Length: %d bytes\n", length);
        if (args[3] == 1) printf("XOR: %u\n", xorh.value);
        if (args[1] == 1) printf("CRC-16: %u\n", crc16.crc);
        if (args[2] == 1) printf("CRC-32: %u\n", ~crc.value);
        if (args[0] == 1){
            printf("MD5: ");
            for (int i = 0; i < 16; i++) printf("%02x", result[i]);
            printf("\n");
        }
    }
    if (args[4] == 1){
        printf("Length: %d bytes\n", length);
        if (args[3] == 1) printf("XOR: 0x%02x\n", xorh.value);
        if (args[1] == 1) printf("CRC-16: 0x%04x\n", crc16.crc);
        if (args[2] == 1) printf("CRC-32: 0x%08x\n", ~crc.value);
        if (args[0] == 1){
            printf("MD5: ");
            for (int i = 0; i < 16; i++) printf("%02x", result[i]);
            printf("\n");
        }
    }
    if (file != stdin) fclose(file);

    return 0;
}

