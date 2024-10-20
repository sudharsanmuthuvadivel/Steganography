/*
Title: 	Programme to encode secret file data to source image file by creating new image file with option '-e' and decode data from new image file to new text file
Date: 	25.08.2024
Author: SUDHARSAN M
Input:	For option 'e'
        a) Source image file name
        b) Secret text file name
        c) Output image file name
        For option 'd'
        a) Stego image file name
        b) Output text file name
Output:	Data in secret file copied to Output text file
*/

#include <stdio.h>
#include "encode.h"
#include "types.h"
#include "decode.h"
#include <string.h>
int main(int argc, char **argv)
{
    if (check_operation_type(argv) == e_encode)
    {
        EncodeInfo encode_info;
        printf("INFO: ## Selected Encoding ##\n");
        if (read_and_validate_encode_args(argv, &encode_info) == e_success)
        {
            printf("INFO: ## Read and validate arguments for Encoding is successfull ##\n");
            if(do_encoding(&encode_info) == e_success)
            {
                fclose(encode_info.fptr_src_image);
                fclose(encode_info.fptr_secret);
                fclose(encode_info.fptr_stego_image);
                printf("INFO: ## Encoding Done Successfully ##\n");
            }
            else
            {
                printf("Failed to Encode the data\n");
            }
        }
        else
            printf("Failed to read and validate arguments!\n");
    }
    else if (check_operation_type(argv) == e_decode)
    {
        DecodeInfo decode_info;
        printf("INFO: ## Selected Decoding ##\n");
        if(read_and_validate_decode_args(argv, &decode_info) == e_success)
        {
            printf("INFO: ## Read and validate arguments for Decoding is successfull ##\n");
            if(do_decoding(&decode_info) == e_success)
            {
                fclose(decode_info.fptr_bmp_image);
                fclose(decode_info.fptr_decode_file);
                printf("INFO: ## Decoding successfull ##\n");

            }
            else
            {
                printf("Failed to decode the data\n");
            }
        }
        else{
            printf("Failed to read and validate arguments\n");
            printf("Decoding : ./a.out -d stego.bmp output.txt\n");

        }

    }
    else
    {
        printf("\nInvalid Option!\n------------------User Reference-------------------------------------------\n");
        printf("Encoding : ./a.out -e beautiful.bmp secret.txt stego.bmp\n");
        printf("Decoding : ./a.out -d stego.bmp output.txt\n");
        printf("---------------------------------------------------------------------------\n");
    }

    return 0;
}

OperationType check_operation_type(char **argv)
{
    if (argv[0] != NULL && strcmp(argv[0], "./a.out") == 0 && argv[1] != NULL)
    {
        if (strcmp(argv[1], "-e") == 0)
            return e_encode;
        else if (strcmp(argv[1], "-d") == 0)
            return e_decode;
    }
    else
        return e_unsupported;
}
