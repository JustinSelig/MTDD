/* eeprom_device.h
 *
 * Justin S. Selig
 * Hardware Tier
 */

#ifndef _eeprom_device_h
#define _eeprom_device_h

#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#define DEVICE_FILE_NAME "device/eeprom.dat"

//----------------------------------------------------------
// eeprom_device_write
//
// Fakes an EEPROM I2C write transaction by writing byte to
// file DEVICE_FILE_NAME at line_num. This function creates a
// temporary file with the new datum inserted, then removes
// the old file. Mutex required due to reentrant code.
//----------------------------------------------------------
// @param[in]  : line_num - file line number indexed at 0
// @param[in]  : new_char - char (byte) to write
// @param[out] : int      - 0 on success
//
int eeprom_device_write(int line_num, char new_char);


//----------------------------------------------------------
// eeprom_device_read
//
// Fakes an EEPROM I2C read transaction by reading from file
// DEVICE_FILE_NAME at line_num and stores associated byte in
// user specified char buffer array location. Mutex required
// due to reentrant code.
//----------------------------------------------------------
// @param[in]  : line_num  - file line number indexed at 0
// @param[in]  : char_read - pointer to location in buffer array
// @param[out] : int       - 0 on success
//
int eeprom_device_read(int line_num, char *char_read);


#endif
