#!/usr/bin/env python

"""
Script to quickly fill data file with default text.
Simulates clean erase/memory wipe.
Ensures file i/o safety via use of 'with'
"""
#with open('eeprom.dat', 'w') as file:
#    for i in range(8192):
#	    file.write('0xFF\n')

with open('eeprom.dat', 'wb') as file:
    #content = b"\x44\n\x33\n" * 10 #10 instances of ascii 'D3'
    content = b"\xFF\n" * 8192 #default erased state
    file.write(content)
	
