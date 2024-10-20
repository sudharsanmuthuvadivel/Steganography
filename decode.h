#ifndef DECODE_H
#define DECODE_H

#include "types.h" //Contain user defined types
#include "common.h"

/* Get file extention size by decoding 32 bytes */
#define DECODE_FILE_EXTN_SIZE 32

/* Get file size by decoding 32 bytes (in this case both are same)*/
#define DECIODE_FILE_SIZE (DECODE_FILE_EXTN_SIZE)

/*
* Structure to store information required for 
decoding stego image to new file.
Info about output and information data is also stored.
*/

typedef struct _DecodeInfo
{
    /*Encoded image(stego image) info*/
    char *bmp_image_fname;
    FILE *fptr_bmp_image;
    uint image_data_size;
    char image_data[MAX_IMAGE_BUF_SIZE];

    /*Decoded file (output file) info*/
    char *decode_fname;
    FILE *fptr_decode_file;
    char decode_file_extn[MAX_FILE_SUFFIX];
    char decode_data[MAX_SECRET_BUF_SIZE];
} DecodeInfo;

/*Decode function prototype*/

/*perform decoding*/
Status do_decoding(DecodeInfo* decode_info);

/*Read and validate Decode args from argv*/
Status read_and_validate_decode_args(char *argv[], DecodeInfo* decode_info);

/*Open all required files for decoding*/
Status open_decode_files(DecodeInfo* decode_info);

/*decode magic string from encoded file*/
Status decode_magic_string(DecodeInfo* decode_info);

/*decode lsb bit from image data*/
Status decode_lsb_to_byte(char* image_data , char* decode_data);

/*Reusable funtion to find decode file extention size and file size */
Status decode_extn_and_file_size(DecodeInfo* decode_info);

/*Decode file extention*/
Status decode_file_extn(int extn_size, DecodeInfo* decode_info);

/*Function to decode the  data */
Status decode_data_to_file(DecodeInfo* decode_info);

#endif