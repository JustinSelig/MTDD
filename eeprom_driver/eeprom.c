/* eeprom.c
 *
 * Justin S. Selig
 * System Tier
 */

#include "eeprom.h"
#include "device/eeprom_device.h"


//----------------------------------------------------------
// calc_first_write
//
// Calculates bytes to write in first page where roofline
// of first page write should be remaining space.
//----------------------------------------------------------
// @param[in]  : size - total number of bytes to write
// @param[in]  : page_space - bytes remaining in first page
// @param[out] : int - num bytes to write in first page
//
int calc_first_write(int size, int page_space)
{
    return (size > page_space) ? page_space : size;
}

//----------------------------------------------------------
// calc_last_write
//
// Calculates bytes to write in last page.
//----------------------------------------------------------
// @param[in]  : size - total number of bytes to write
// @param[in]  : first_write_size - bytes written in first page
// @param[in]  : page_size_bytes - num of bytes in one page
// @param[out] : int - num bytes to write in last page
//
int calc_last_write(int size, int first_write_size, int page_size_bytes)
{
    if (size > first_write_size)
    {
        return (size - first_write_size) % page_size_bytes;
    }
    else
    {
        return 0;
    }
}

//----------------------------------------------------------
// calc_total_writes
//
// Calculates number of intermediate writes the size of pages
// and returns total page writes including first and last.
//----------------------------------------------------------
// @param[in]  : size - total number of bytes to write
// @param[in]  : first_write_size - bytes written in first page
// @param[in]  : page_size_bytes - num of bytes in one page
// @param[out] : int - num bytes to write in last page
//
int calc_total_writes(int size, int first_write_size, int page_size_bytes)
{
    int       inter_num_writes = 0;
    int       total_num_writes = 0;
    const int first_plus_last  = 2;
    if (size > first_write_size)
    {
        inter_num_writes = (size - first_write_size) / page_size_bytes;
        total_num_writes = inter_num_writes + first_plus_last;
    }
    else
    {
        total_num_writes = 1;
    }
    return total_num_writes;
}

//----------------------------------------------------------
// check_input_errors
//
// Check user specified arguments for errors.
//----------------------------------------------------------
// @param[in]  : dev    - process independent device struct
// @param[in]  : offset - base relative write/read location
// @param[in]  : size   - number of bytes
// @param[in]  : buf    - data buffer
// @param[out] : int    - 0 on success
//
int check_input_errors(eeprom_dev_t *dev, uint32_t offset, int size, char * buf)
{
    //device not specified
    if (dev == NULL)
    {
        return -ENODEV;
    }
    //device specified, fields unspecified
    if (!(dev->mutex && dev->fault_handler))
    {
        return -EINVAL;
    }
    //properties struct uninitialized
    if (!dev->properties.device_size_words)
    {
        return -EINVAL;
    }
    //invalid offset
    if (offset < 0)
    {
        dev->fault_handler("Illegal offset specified");
    }

    return 0; //success
}

//Public specification in header
int eeprom_write(eeprom_dev_t *dev, uint32_t offset, int size, char * buf)
{
    //scrub user input
    int e = check_input_errors(dev, offset, size, buf);
    if (e < 0)
    {
        return e;
    }

    //write loop variables
    uint32_t write_size;          //current number of bytes to write
    uint32_t page;                //page counter
    uint32_t cur_addr;            //current address during transaction
    uint32_t total_byte_counter;  //counter across entire buffer
    uint8_t  page_byte_counter;   //byte transmission counter 0-32
    int      result;              //error code or succcessful transmission
    char     err[1024];           //string holding fault handler error

    //calculate effective address from base, check boundaries
    const uint8_t  page_size_bytes   = dev->properties.page_size_bytes;
    const uint32_t device_size_words = dev->properties.device_size_words;
    const uint32_t base_addr         = dev->properties.base_address;
    const uint32_t effective_addr    = base_addr + offset;
    //memory should be zero-indexed: [base, words-1]
    if ((effective_addr < base_addr) || (effective_addr > device_size_words-1))
    {
        snprintf(err, sizeof(err), "Bad address %i, bounds are [%i, %i]",
            effective_addr, base_addr, device_size_words-1);
        dev->fault_handler(err);
    }

    //calculate remaining space available in page
    //variables holding data transaction sizes
    uint32_t page_space  =
        (((effective_addr/page_size_bytes) + 1)*page_size_bytes) - effective_addr;
    uint32_t first_write_size = calc_first_write(size, page_space);
    uint32_t total_num_writes = calc_total_writes(size, first_write_size, page_size_bytes);
    uint32_t last_write_size  = calc_last_write(size, first_write_size, page_size_bytes);

    //start page access transmissions
    cur_addr = effective_addr;
    total_byte_counter = 0;

    //lock reentrant code protecting shared resource
    pthread_mutex_lock((pthread_mutex_t*)(dev->mutex));
    for (page = 0; page < total_num_writes; page++)
    {
        if (page == 0)                         //first page
        {
            write_size = first_write_size;
        }
        else if (page == (total_num_writes-1)) //last page
        {
            write_size = last_write_size;
        }
        else                                   //intermediate page
        {
            write_size = page_size_bytes;
        }

        page_byte_counter = 0;
        while (page_byte_counter < write_size)
        {
            //For typical page accesses, address would be sent once over i2c
            //followed by a serial stream of byte data. But here, address is
            //being incremented since eeprom file i/o device doesn't hold state.
            result = eeprom_device_write(cur_addr, buf[total_byte_counter]);
            if (result < 0)
            {
                pthread_mutex_unlock((pthread_mutex_t*)(dev->mutex));
                snprintf(err, sizeof(err),
                    "Failed transmission on byte %i", total_byte_counter);
                dev->fault_handler(err);
            }
            page_byte_counter++;
            total_byte_counter++;
            cur_addr++;
        }
    }
    pthread_mutex_unlock((pthread_mutex_t*)(dev->mutex));

    return 0; //success
}

//Public specification in header
int eeprom_read(eeprom_dev_t *dev, uint32_t offset, int size, char * buf)
{
    //scrub user input
    int e = check_input_errors(dev, offset, size, buf);
    if (e < 0)
    {
        return e;
    }
    //read loop variables
    int  byte_counter; //current byte being read
    char err[1024];   //string holding fault handler error

    //calculate effective address from base, check boundaries
    const uint32_t device_size_words = dev->properties.device_size_words;
    const uint32_t base_addr         = dev->properties.base_address;
    const uint32_t effective_addr    = base_addr + offset;
    if ((effective_addr < base_addr) || (effective_addr > device_size_words-1))
    {
        snprintf(err, sizeof(err),
            "Bad offset address, bounds are [%i, %i]", base_addr, device_size_words-1);
        dev->fault_handler(err);
    }

    //lock reentrant code protecting shared resource
    pthread_mutex_lock((pthread_mutex_t*)(dev->mutex));
    for (byte_counter = 0; byte_counter < size; byte_counter++)
    {
        int res = eeprom_device_read(effective_addr+byte_counter, &buf[byte_counter]);
        if (res < 0)
        {
            pthread_mutex_unlock((pthread_mutex_t*)(dev->mutex));
            snprintf(err, sizeof(err), "Failed read on byte %i", byte_counter);
            dev->fault_handler(err);
        }
    }
    pthread_mutex_unlock((pthread_mutex_t*)(dev->mutex));

    return 0; //success
}
