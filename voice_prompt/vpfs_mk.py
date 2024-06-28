#!/usr/bin/python -tt
# Copyright 2016-2024, Cypress Semiconductor Corporation (an Infineon company) or
# an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
#
# This software, including source code, documentation and related
# materials ("Software") is owned by Cypress Semiconductor Corporation
# or one of its affiliates ("Cypress") and is protected by and subject to
# worldwide patent protection (United States and foreign),
# United States copyright laws and international treaty provisions.
# Therefore, you may use this Software only as provided in the license
# agreement accompanying the software package from which you
# obtained this Software ("EULA").
# If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
# non-transferable license to copy, modify, and compile the Software
# source code solely for use in connection with Cypress's
# integrated circuit products.  Any reproduction, modification, translation,
# compilation, or representation of this Software except as specified
# above is prohibited without the express written permission of Cypress.
#
# Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
# reserves the right to make changes to the Software without notice. Cypress
# does not assume any liability arising out of the application or use of the
# Software or any product or circuit described in the Software. Cypress does
# not authorize its products for use in any products where a malfunction or
# failure of the Cypress product may reasonably be expected to result in
# significant property damage, injury or death ("High Risk Product"). By
# including Cypress's product in a High Risk Product, the manufacturer
# of such system or application assumes all risk of such use and in doing
# so agrees to indemnify Cypress against all liability.
# Test Program to test the Audio Insert feature


# The following Python modules are required
import sys
import os

# Usage
def Usage(str=None):
    if str:
        print str
    print 'Usage:", sys.argv[0], " OPTIONS FILE...'
    print '''OPTIONS
  -f format         Input Files format [1:VPC8, 2:RawPCM, 3:ADPCM]
  -o output_file    Binary file system
  -v level          Verbose/Debug mode
  -h                display this help text and exit
 '''

# Check the parameters (passed on the Command Line)
def CheckParameter(param):
    try:
        sys.argv.index(param)
        return True
    except:
        return False

# Exit
def Exit(error):
    if error:
        sys.exit(1)
    else:
        sys.exit(0)

# Convert a 32 bit value to a list of bytes (in Little Endian format)
def u32_to_bytes(u32):
    bytes_list = []
    bytes_list.append(u32 & 0xFF)
    bytes_list.append((u32 >> 8 ) & 0xFF)
    bytes_list.append((u32 >> 16 ) & 0xFF)
    bytes_list.append((u32 >> 24 ) & 0xFF)
    return bytes_list

# Some default values
verbose = 0

# Main function
def main(argv):

    # Check parameters
    if CheckParameter('-h'):
        Usage()
        exit(True)

    if CheckParameter('-v'):
        verbose = int(sys.argv[sys.argv.index('-v')+1])
        print 'verbose:', verbose

    if CheckParameter('-f'):
        files_format = int(sys.argv[sys.argv.index('-f')+1])
        if (files_format == 1):
            print 'files_format:', files_format, ' => VoicePromptCodec 8kHz'
        elif (files_format == 2):
            print 'files_format:', files_format, ' => Row PCM (S16, 8kHz, Mono)'
        elif (files_format == 3):
            print 'files_format:', files_format, ' => ADPCM'
        else:
            Usage('Unsupported format:' + str(files_format))
            Exit(True)
    else:
        Usage("format parameter missing")
        exit(False)

    if CheckParameter('-o'):
        output_file = sys.argv[sys.argv.index('-o')+1]
        print 'output_file:{:s}' .format(output_file)
    else:
        Usage("output file parameter missing")
        exit(False)

    # Extract the list of input files
    input_file_list = []
    arg_index = 1
    total_files_size = 0
    while arg_index < len(argv):
        if (argv[arg_index] == '-f'):
            arg_index += 1
        elif (argv[arg_index] == '-o'):
            arg_index += 1
        elif (argv[arg_index] == '-v'):
            arg_index += 1
        else:
            # Check if the file exists
            if os.path.isfile(argv[arg_index]) == False:
                print 'Err: Cannot open ' + argv[arg_index]
                Exit(True)
            # Update the totsal file size ithe the size of thsi file
            total_files_size += os.path.getsize(argv[arg_index])
            input_file_list.append(argv[arg_index])
        arg_index += 1

    # Count the number of Input files
    nb_input_files = len(input_file_list)

    # Calculate the File system size
    fs_size = total_files_size + nb_input_files * 8

    if (verbose >= 1):
        print 'Input files:' + ' ' .join(input_file_list)
        print 'nb input files:', nb_input_files

    if (verbose >= 2):
        print 'total_files_size:', total_files_size
        print 'fs_size:', fs_size
        print 'to_bytes(fs_size):', u32_to_bytes(fs_size)

    # open the output_file in Write/Binary mode
    ofd = open(output_file, 'w+b')

    # File System Signature = VPFS (Voice Prompt File System)
    fs_signature = ['V', 'P', 'F', 'S']
    binary_format = bytearray(fs_signature)
    ofd.write(binary_format)

    # File System Version (1.0)
    fs_version = bytearray(u32_to_bytes(0x00010000))
    ofd.write(fs_version)

    # File System Length
    binary_format = bytearray(u32_to_bytes(fs_size))
    ofd.write(binary_format)

    for input_file in input_file_list:
        if (verbose >= 1):
            print 'processing input file:' + input_file

        # Get the size of the file
        file_size = os.path.getsize(input_file)
        if (verbose >= 1):
            print 'file size:', file_size

        # Add the File Header (4 bytes format)
        file_size += 4

        # Write the File Length
        binary_format = bytearray(u32_to_bytes(file_size))
        ofd.write(binary_format)

        # Write the File Format
        binary_format = bytearray(u32_to_bytes(files_format))
        ofd.write(binary_format)

        # open the input_file in read/Binary mode. We already checked that it exists
        ifd = open(input_file, "rb")

        # Read the file content
        file_content = ifd.read(file_size)

        # write it in the File System file
        ofd.write(file_content)

        ifd.close()

    # close the Output file
    ofd.close()

if __name__ == "__main__":
    main(sys.argv)
