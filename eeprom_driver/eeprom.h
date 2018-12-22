/* eeprom.h
 *
 * Justin S. Selig
 * System Tier
 */

#ifndef _eeprom_h
#define _eeprom_h

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>

//Model-specific hardware device struct
typedef struct eeprom_dev_properties
{
    // Base Address
    uint8_t base_address;

    // Total Memory Size in Bits (64 Kb)
    uint32_t device_size_bits;

    // Total Memory Size in 8-bit Words (ie. length of file)
    uint16_t device_size_words;

    // Word Size
    uint8_t word_size_bits;

    // Page Size in Bytes
    uint8_t page_size_bytes;

} eeprom_dev_properties_t;


//Device struct per driver
typedef struct eeprom_dev
{
    //device mutex
    pthread_mutex_t *mutex;

    //properties struct
    eeprom_dev_properties_t properties;

    //pointer to device fault handler
    //Takes string error as argument
    void (*fault_handler)(char*);

    //device id - used mostly for debugging purposes
    int id;

} eeprom_dev_t;


//----------------------------------------------------------
// eeprom_write
//
// Write to EEPROM Device:
// Performs device-independent page calculations and initiates
// page write transaction. Emulates i2c bus communication but
// instead of separating address and data, sends both at once.
//----------------------------------------------------------
// @param[in]  : dev    - process independent device struct
// @param[in]  : offset - base relative write location
// @param[in]  : size   - number of bytes to write
// @param[in]  : buf    - user specified data buffer
// @param[out] : int    - 0 on success
//
int eeprom_write(eeprom_dev_t *dev, uint32_t offset, int size, char * buf);


//----------------------------------------------------------
// eeprom_read
//
// Read from EEPROM Device
// Reads single stride byte aligned data from device and stores
// in user specified buffer.
//----------------------------------------------------------
// @param[in]  : dev    - process independent device struct
// @param[in]  : offset - base relative read location
// @param[in]  : size   - number of bytes to read
// @param[in]  : buf    - read data buffer
// @param[out] : int    - 0 on success
//
int eeprom_read(eeprom_dev_t *dev, uint32_t offset, int size, char * buf);


#endif
