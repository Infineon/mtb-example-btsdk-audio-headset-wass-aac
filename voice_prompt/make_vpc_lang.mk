#
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
#
#
# This makefile is not supposed to be called manually (but it can).
# This makefile is called by the makefile for every 'language folder' found.
#
# This makefile must be called with LDIR parameter set to the name of a language folder.
# For example:
# make -f make_vpc_lang.mk LDIR=en-us
#

# Encoder Quality 0:Economy 1:HighQuality
QUALITY = 1

# time Scale Modification
TIME_SCALE_MODIFICATION = 0

# Check that the LDIR parameter is set
ifndef LDIR
  $(error Parameter LDIR not set)
else
  $(info Processing '$(LDIR)/' folder)
endif

TARGET = $(LDIR)/vpfs_vpc.bin

# The built-in 'sort' function does not correctly sort files (e.g. '10-file1' and '2-file1')
# Let's define a function based on the shell sort program
sp :=
sp += # add space
my-sort = $(shell echo -e $(subst $(sp),'\n',$2) | sort $1 --key=1,1 -)

# Search all the .wav files located in the <LDIR>/wav file
WAV_FILES1 =  $(wildcard $(LDIR)/wav/*.wav)
# Remove the <LDIR>/wav of every file
WAV_FILES2 = $(subst $(LDIR)/wav/, , $(WAV_FILES1))
# Remove the .wav extension of every file
WAV_FILES3 = $(basename $(WAV_FILES2))

# 'sort' the wav files
WAV_FILES_SORTED = $(call my-sort, -n, $(WAV_FILES3))

# build a sorted list of .vpc file
VPC_FILES1 = $(call my-sort, -n, $(WAV_FILES3))
VPC_FILES2 =  $(addsuffix .vpc, $(VPC_FILES1))
VPC_FILES3 =  $(addprefix $(LDIR)/vpc/, $(VPC_FILES2))

PCM_FILE1:=$(VPC_FILES3:.vpc=.pcm)

# Rule to Encode a .wav file into a .vpc file
$(LDIR)/vpc/%.vpc : $(LDIR)/wav/%.wav
	$(info Encoding $< to $@. Be patient...)
	$(info QUALITY:$(QUALITY))
	$(info TIME_SCALE_MODIFICATION:$(TIME_SCALE_MODIFICATION))
	@mkdir -p $(@D)
	./vpc_encoder.exe $< $@ $(@:.vpc=.pcm) $(QUALITY) $(TIME_SCALE_MODIFICATION)
	rm $(@:.vpc=.pcm)

# Rule to build the VPC File System
$(TARGET): $(VPC_FILES3)
	$(info Creating Voice Prompt File System $@)
	python ./vpfs_mk.py -v 1 -f 1 -o $@ $(VPC_FILES3)

all: $(TARGET)

clean:
	@rm -f $(PCM_FILE1) $(VPC_FILES3) $(TARGET)
