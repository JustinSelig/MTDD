/* eeprom_device.h
 *
 * Justin S. Selig
 * Hardware Tier
 */

#include "eeprom_device.h"

#define LINE_LEN 3

//----------------------------------------------------------
// get_num_lines
//
// Helper function for checking that writes remain within bounds.
// Returns total number of lines in file file_name.
//----------------------------------------------------------
// @param[in]  : line_num - file line number indexed at 0
// @param[in]  : new_char - char (byte) to write
// @param[out] : int      - 0 on success
//
int get_num_lines(char *file_name)
{
    FILE *fp = fopen(file_name, "r");
    if (fp == NULL)
    {
        printf("Failed to open device\n");
        return -EIO;
    }
    int  result = 0;
    char ch;
    //iterate through whole file
    while(!feof(fp))
    {
        ch = fgetc(fp);
        //increment for each new line character encountered
        if(ch == '\n')
        {
            result++;
        }
    }
    fclose(fp);
    return result;
}

//Public specification in header
int eeprom_device_write(int line_num, char new_char)
{
    //check that write to line_num is allowed (within file)
    int num_file_lines = get_num_lines(DEVICE_FILE_NAME);
    if (num_file_lines < 0)
    {
        printf("Unable to get num file lines\n");
        return -EIO;
    }
    if (line_num > num_file_lines-1) //zero indexed
    {
        printf("Bad address: out of bounds\n");
        return -EFAULT;
    }

    //open eeprom data file for reading
    FILE *fd1 = fopen(DEVICE_FILE_NAME, "r");
    if (fd1 == NULL)
    {
        printf("Failed to open file\n");
        return -EIO;
    }

    //create a temporary file for writing old data and inserted datum
    char *temp = "device/temp.dat";
    FILE *fd2 = fopen(temp, "wb"); //write byte mode
    if (fd2 == NULL)
    {
        printf("Failed to open file\n");
        return -EIO;
    }

    //iterate through old file and copy contents to temp file with
    //user specified line replaced
    char str[LINE_LEN];
    int count = 0;
    while (!feof(fd1) && (count < num_file_lines))
    {
        fgets(str, LINE_LEN, fd1);
        if (count != line_num)
        {
            fprintf(fd2, "%s", str); //keep str on line count
        }
        else //found line to replace
        {
            fprintf(fd2, "%c\n", new_char); //replace str with new_char on line count
        }
        count++;
    }

    fclose(fd1);
    fclose(fd2);
    //delete old file, swap for new one
    remove(DEVICE_FILE_NAME); //temp file is now the primary
    rename(temp, DEVICE_FILE_NAME); //replace old file with new

    return 0; //success
}

//Public specification in header
int eeprom_device_read(int line_num, char *char_read)
{
    line_num++; //change indexing of line_num to comply with writes

    //check that read from line_num is allowed
    int num_file_lines = get_num_lines(DEVICE_FILE_NAME);
    if (num_file_lines < 0)
    {
        printf("Unable to get num file lines\n");
        return -EIO;
    }
    if (line_num > num_file_lines-1) //zero indexed
    {
        printf("Bad address: out of bounds\n");
        return -EFAULT;
    }

    //open file to read
    FILE *fd = fopen(DEVICE_FILE_NAME, "r");
    if (fd == NULL)
    {
        printf("Failed to open file\n");
        return -EIO;
    }

    //iterate through file line by line
    char str[LINE_LEN];
    int count = 0;
    do
    {
        //break when correct line found
        if (++count == line_num)
        {
            fgets(str, LINE_LEN, fd);
            break; //str will contain associate byte at line_num
        }
    } while((fscanf(fd, "%*[^\n]"), fscanf(fd, "%*c")) != EOF);
    fclose(fd);
    //error if count happened to go outside desired line location
    if (count != line_num)
    {
        printf("out of bounds read\n");
        return -EFAULT;
    }
    *char_read = str[0]; //character located in first index of string
    return 0;
}
