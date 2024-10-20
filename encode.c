#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height, bit_per_pixel;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    fseek(fptr_image, 2, SEEK_CUR);
    fread(&bit_per_pixel, sizeof(short), 1, fptr_image);
    printf("bit per pixel %hu\n", bit_per_pixel / 8); // 24 bit/ 8 = 3 byte

    rewind(fptr_image);

    // Return image capacity
    return width * height * (bit_per_pixel / 8);
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);
        return e_failure;
    }

    // No failure return e_success
    return e_success;
}
Status read_and_validate_encode_args(char **argv, EncodeInfo *encode_info)
{
    if (argv[2] != NULL && strcmp(strstr(argv[2], "."), ".bmp") == 0)
        encode_info->src_image_fname = argv[2];
    else
        return e_failure;

    if (argv[3] != NULL && strcmp(strstr(argv[3], "."), ".txt") == 0)
        encode_info->secret_fname = argv[3];
    else
        return e_failure;

    if (argv[4] != NULL)
        encode_info->stego_image_fname = argv[4];
    else
        encode_info->stego_image_fname = "stego.bmp";

    return e_success;
}
Status check_capacity(EncodeInfo *encode_info)
{
    encode_info->image_capacity = get_image_size_for_bmp(encode_info->fptr_src_image);
    encode_info->size_secret_file = get_file_size(encode_info->fptr_secret);
    //                       2  + secret_file extention size(int 4) + secret file extention ".txt" (4) + sizeof secret file (int 4)
    if (encode_info->image_capacity > (54 + (strlen(MAGIC_STRING) + 4 + 4 + 4 + encode_info->size_secret_file) * 8))
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}
uint get_file_size(FILE *fptr_secret)
{
    fseek(fptr_secret, 0, SEEK_END);
    return ftell(fptr_secret);
}
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char header[54];
    fseek(fptr_src_image, 0, SEEK_SET);
    fread(header, 54, 1, fptr_src_image);
    fwrite(header, 54, 1, fptr_dest_image);
    return e_success;
}
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    unsigned int mask = 1 << 7;
    for (int i = 0; i < 8; i++)
    {
        // data & with mask will give a bit from MSB of  the data
        // Bring the data to end of lsb by right shift
        // clear the lsb bit from image buffer data
        // or them together to get encoded data
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data & mask) >> (7 - i));
        mask = mask >> 1;
    }
    return e_success;
}
Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image, EncodeInfo *encode_info)
{
    // fetch 8 bytes each time from source image untill the size of data
    for (int i = 0; i < size; i++)
    {
        // read 8bytes from beautiful.bmp
        fread(encode_info->image_data, 8, sizeof(char), fptr_src_image);
        // call encode_byte_to_lsb to encode the data
        encode_byte_to_lsb(data[i], encode_info->image_data); // image string and image buffer data
        // after encoding write encoded data to setgo.bmp
        fwrite(encode_info->image_data, 8, sizeof(char), fptr_stego_image);
    }
    return e_success;
}

Status encode_magic_string(const char *magic_str, EncodeInfo *encode_info)
{
    // every string encoding needs to call encode_data_to image
    encode_data_to_image(magic_str, strlen(magic_str), encode_info->fptr_src_image, encode_info->fptr_stego_image, encode_info);
    return e_success;
}
Status encode_size_to_lsb(char* image_buffer, int size)
{
    unsigned int mask = 1 << 31;
    for (int i = 0; i < 32; i++)
    {
        // data(size) & with mask will give a bit from MSB of  the data
        // Bring the data to end of lsb by right shift
        // clear the lsb bit from image buffer data
        // or them together to get encoded data
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((size & mask) >> (31 - i));
        mask >>= 1;
    }
    return e_success;
}
Status encode_secret_file_ext_size(int size, FILE* fptr_src, FILE* fptr_stego)
{
    //read 32 bytes of rgb data from beautiful.bmp
    char image_buffer[32];
    fread(image_buffer, 32, sizeof(char), fptr_src);
    //reusable function to encode the size
    encode_size_to_lsb(image_buffer, size);
    fwrite(image_buffer, 32, sizeof(char), fptr_stego);
    return e_success;
}
Status encode_secret_file_extn(const char* file_extn, EncodeInfo* encode_info)
{
    file_extn = ".txt";
    encode_data_to_image(file_extn, strlen(".txt"), encode_info->fptr_src_image, encode_info->fptr_stego_image, encode_info);
    return e_success;
}
Status encode_secret_file_size(long int file_size, EncodeInfo* encode_info)
{
    //read 32 bytes of rgb data from beautiful.bmp
    char image_buffer[32];
    fread(image_buffer, 32, sizeof(char), encode_info->fptr_src_image);
    //reusable function to encode the size
    encode_size_to_lsb(image_buffer, file_size);
    fwrite(image_buffer, 32, sizeof(char), encode_info->fptr_stego_image);
    return e_success;
}
Status encode_secret_file_data(EncodeInfo* encode_info)
{
    char ch;
    //bring the file pointer to the starting position
    rewind(encode_info->fptr_secret);
    for(int i = 0; i < encode_info->size_secret_file; i++)
    {
        //read 8bytes of rgb data from beautiful.bmp
        fread(encode_info->image_data, 8, sizeof(char), encode_info->fptr_src_image);
        fread(&ch, 1, sizeof(char), encode_info->fptr_secret);
        encode_byte_to_lsb(ch, encode_info->image_data);
        fwrite(encode_info->image_data, 8, sizeof(char), encode_info->fptr_stego_image);
    }
    return e_success;
}
Status copy_remaining_img_data(FILE* src_image, FILE* stego_image)
{
    char ch;
    while(fread(&ch, 1, sizeof(char), src_image) > 0)
    {
        fwrite(&ch, 1, sizeof(char), stego_image);
    }
    return e_success;
}
Status do_encoding(EncodeInfo *encode_info)
{
    // call rest all encoding function
    if (open_files(encode_info) == e_success)
    {
        printf("INFO: ## Successfully opened all required files ##\n");
        printf("INFO: ## Encoding procedure Started ##\n");
        printf("INFO: ## Check %s capacity ##\n", encode_info->src_image_fname);
        if (check_capacity(encode_info) == e_success)
        {
            printf("INFO: ## Check %s capacity successfull ##\n", encode_info->src_image_fname);
            if (copy_bmp_header(encode_info->fptr_src_image, encode_info->fptr_stego_image) == 0)
            {
                printf("INFO: ## Copy header successfully ##\n");
                if (encode_magic_string(MAGIC_STRING, encode_info) == e_success)
                {
                    printf("INFO: ## Magic string successfully encoded ##\n");
                    if (encode_secret_file_ext_size(strlen(".txt"), encode_info->fptr_src_image, encode_info->fptr_stego_image) == e_success)
                    {
                        printf("INFO: ## Secret file extention size successfully encoded ##\n");
                        if(encode_secret_file_extn(encode_info->extn_secret_file, encode_info) == e_success)
                        {
                            printf("INFO: ## Secret file extention successfully encoded ##\n");
                            if(encode_secret_file_size(encode_info->size_secret_file, encode_info) == e_success)
                            {
                                printf("INFO: ## Secret file size successfully encoded ##\n");
                                if(encode_secret_file_data(encode_info) == e_success)
                                {
                                    printf("INFO: ## Secret file data successfully encoded ##\n");
                                    if(copy_remaining_img_data(encode_info->fptr_src_image, encode_info->fptr_stego_image) == e_success)
                                    {
                                        printf("INFO: ## Remaining bytes successfully copied ##\n");
                                    }
                                    else
                                    {
                                        printf("Failed to copy remaing bytes");
                                    }
                                }
                                else
                                {
                                    printf("Failed to encode secret file data\n");
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf("Failed to encode secret file size\n");
                                e_failure;
                            }
                        }
                        else
                        {
                            printf("Failed to encode secret file extention\n");
                            e_failure;
                        }
                    }
                    else
                    {
                        printf("Failed to encode secret file extention size\n");
                        e_failure;
                    }
                }
                else
                {
                    printf("Failed to encode magic string\n");
                    return e_failure;
                }
            }
            else
            {
                printf("Failed to copy the header info\n");
                return e_failure;
            }
        }
        else
        {
            printf("Don't have enough capacity to encode the data!\n");
            return e_failure;
        }
    }
    else
    {
        printf("Failed to open the files\n");
        return e_failure;
    }
    return e_success;
}
