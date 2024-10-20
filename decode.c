#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "common.h"
#include "decode.h"


/*Function definition*/
/* Get File name and validate
Input : Stego Image file name and Output filr name
Output: Store the above file name
Return Value : e_success or e_failure , file errors*/

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decode_info)
{
    //open 
    if (argv[2] != NULL && strcmp(strstr(argv[2], "."), ".bmp") == 0)
        decode_info->bmp_image_fname = argv[2];
    else
        return e_failure;

    if (argv[3] != NULL)
    {
        decode_info->decode_fname = argv[3];
        printf("INFO: ## Output file name mentioned, File name is %s\n", decode_info->decode_fname);
    }
    else
    {
        decode_info->decode_fname = "decode.txt";
        printf("INFO: ## Output file name not mentioned, So default output file name is %s\n", decode_info->decode_fname);
    }
    return e_success;
}
/* Get File pointers for i/p and o/p files
 * Inputs: Stego Image file, Output file
 * Ouput: File pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_decode_files(DecodeInfo *decode_info)
{
    //open Stego Image file 
    decode_info->fptr_bmp_image = fopen(decode_info->bmp_image_fname, "r");

    //Error handling
    if (decode_info->fptr_bmp_image == NULL)
    {
        printf("Error : Faield to open %s file\n", decode_info->bmp_image_fname);
        return e_failure;
    }
    // OPen Output file 
    decode_info->fptr_decode_file = fopen(decode_info->decode_fname, "w");

    //Error Handling
    if (decode_info->fptr_decode_file == NULL)
    {
        printf("Error: failed to open %s file\n", decode_info->decode_fname);
        return e_failure;
    }

    return e_success;
}
/* Decodes the magic string from stego image
 * Input: 	Magic string and File info of input and output
 * Output:	Proceed decoing if magic string if found or else stop decoding
 * Return:	e_success or e_failure
 */
Status decode_magic_string(DecodeInfo *decode_info)
{
    fseek(decode_info->fptr_bmp_image, 54, SEEK_SET);
    uint i;
    for (i = 0; i < strlen(MAGIC_STRING); i++)
    {
        for (int j = 0; j < MAX_IMAGE_BUF_SIZE; j++)
        {
            fread(&decode_info->image_data[j], sizeof(char), 1, decode_info->fptr_bmp_image);
            if (ferror(decode_info->fptr_bmp_image))
            {
                fprintf(stderr, "Error: Reading data from image failed\n");
                clearerr(decode_info->fptr_bmp_image);
                return e_failure;
            }
            if(decode_lsb_to_byte(decode_info->decode_data + i, decode_info->image_data) != e_success)
            {
                fprintf(stderr, "Error: Decoding LSB to byte failed\n");
                return e_failure;
            }
        }
        
    }
    if(decode_info->decode_data[0] == '#' && decode_info->decode_data[1] == '*')
    {
        return e_success;
    }
    else {
        printf("Error: Magic string mismatch!\n");
        return e_failure;
    }
    
}
/* Function to find Output File extention size and Encoded secret Data size 
* Input: File info of stego image and output file
* Output: Decode the extenstion size and Secret data size from stego image and store in image_data_size
* Return: e_success or e_failure
*/
Status decode_extn_and_file_size(DecodeInfo* decode_info)
{
    char file_size[DECIODE_FILE_SIZE];
    fread(file_size, sizeof(char), DECIODE_FILE_SIZE, decode_info->fptr_bmp_image);
    //Error Handling
    if (ferror(decode_info->fptr_bmp_image))
        {
            fprintf(stderr, "Error: Reading data from image is failed\n");
            clearerr(decode_info->fptr_bmp_image);
            return e_failure;
        }

    decode_info->image_data_size = 0;
    for(int i = 0; i < DECODE_FILE_EXTN_SIZE; i++)
    {
        decode_info->image_data_size <<= 1;
        decode_info->image_data_size |= (uint)(file_size[i] & 0x01);
    }
    return e_success;
}
/* Decode File Extenstion From Stego Image 
 * Input: Extenstion Size and File info of stego image
 * Output: Decodes the file extenstion and store in decode_file_extn
 * Return: e_success or e_failure
 */
Status decode_file_extn(int size, DecodeInfo* decode_info)
{
    for(uint i = 0; i < size; i++)
    {
        fread(decode_info->image_data, sizeof(char), MAX_IMAGE_BUF_SIZE, decode_info->fptr_bmp_image);

        //Error handling
        if(ferror(decode_info->fptr_bmp_image))
        {
            fprintf(stderr, "Error: Reading data from image is failed\n");
            clearerr(decode_info->fptr_bmp_image);
            return e_failure;
        }
        if(decode_lsb_to_byte(decode_info->decode_data, decode_info->image_data) == e_success)
        {
            decode_info ->decode_file_extn[i] = decode_info->decode_data[0];
        }
        else{
            return e_failure;
        }
    }
    decode_info->decode_file_extn[size] = '\0';
    printf("File extension is %s\n", decode_info->decode_file_extn);

    return e_success;
}
/* Get lsb bit from stego image byte 
 * Input: Image_data array and decode_data character
 * Output: Decode the image_data and stores the 1 byte data in decode_data
 * Return: e_success or e_failure
 */
Status decode_lsb_to_byte(char *decode_data, char *image_data)
{
    
        decode_data[0] = 0;
        for (int j = 0; j < MAX_IMAGE_BUF_SIZE; j++)
        {
            decode_data[0] <<= 1;
            decode_data[0] = decode_data[0] | (image_data[j] & 0x01);
        }

    return e_success;
}
/* Decode file data from stego image
 * Input: FILE info of stego image and output decode file
 * Output: Write decode data in the output file
 * Return: e_success or e_failure
 */
Status decode_data_to_file(DecodeInfo* decode_info)
{
    for(int i  = 0; i < decode_info->image_data_size; i++)
    {
        fread(decode_info->image_data, sizeof(char), MAX_IMAGE_BUF_SIZE, decode_info->fptr_bmp_image);
        if(ferror(decode_info ->fptr_bmp_image))
        {
            fprintf(stderr, "Error: Reading data from image is failed\n");
            clearerr(decode_info->fptr_bmp_image);
            return e_failure;
        }
        if(decode_lsb_to_byte(decode_info->decode_data, decode_info->image_data) == e_success)
        {
            fwrite(decode_info->decode_data, sizeof(char), 1, decode_info->fptr_decode_file);
            if(ferror(decode_info ->fptr_decode_file))
            {
                fprintf(stderr, "Error: Writing data to output file is failed\n");
                clearerr(decode_info->fptr_decode_file);
                return e_failure;
            }
        }
        else{
            printf("Error : %s function failed\n", "decode_lsb_to_byte");
            return e_failure;
        }
    }
    return e_success;
}
/* Decoding stego image to another file
 * Input: File info of stego image and output file
 * Output: Decoded message copied in output file
 * Return: e_success or e_failure, on file error
 */
Status do_decoding(DecodeInfo *decode_info)
{
    //Open All Required Files
    if (open_decode_files(decode_info) == e_success)
    {
        printf("INFO: ## Open all required files for decoding ##\n");
        printf("INFO: ## Decoding procedure started ##\n");

        //Decoding Magic String Signature
        if (decode_magic_string(decode_info) == e_success)
        {
            printf("INFO: ## Magic string successfully decoded ##\n");

            //Decode Output File Extention Size
            if(decode_extn_and_file_size(decode_info) == e_success)
            {
                printf("INFO: ## Secret file extension size successfully decoded ##\n");

                //Decode Output file Extention
                if(decode_file_extn(decode_info->image_data_size, decode_info) == e_success)
                {
                    printf("INFO: ## Secret file extension successfully decoded ##\n");
                    printf("INFO: ## Check secret file size ##\n");

                    //Decode Encoded data size 
                    if(decode_extn_and_file_size(decode_info) == e_success)
                    {
                        printf("INFO: ## Secret file size decoded successfully ##\n");
                         //Decode Secret data from Encode image file
                        if(decode_data_to_file(decode_info) == e_success)
                        {
                            printf("INFO: ## Secret data successfully decoded ##\n");
                        }
                        else
                        {
                            printf("Error : Failed to decode secret data\n");
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("Error: Failed to decode secret file size\n");
                        return e_failure;
                    }
                    
                }
                else
                {
                    printf("Error: Failed to decode file extention\n");
                    return e_failure;
                }
   
            }
            else{
                printf("Error: Failed to decode file extension\n");
                return e_failure;
            }
        }
        else
        {
            printf("Error: Failed to decode magic string\n");
            return e_failure;
        }
    }
    else
    {
        printf("Error : Failed to open all required files\n");
        return e_failure;
    }
    return e_success;
}
