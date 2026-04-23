################################################################################
# \file Makefile
# \version 1.0
#
# \brief
# Makefile for configuring the Cortex-M55 core project
#
################################################################################
# \copyright
# (c) 2025-2026, Infineon Technologies AG, or an affiliate of Infineon
# Technologies AG.  SPDX-License-Identifier: Apache-2.0
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

include ../common.mk

################################################################################
# Basic Configuration
################################################################################

# Name of application (used to derive name of final linked file).
#
# If APPNAME is edited, ensure to update or regenerate launch
# configurations for your IDE.
APPNAME=proj_cm55

# If set to "true" or "1", display full command-lines when building.
VERBOSE=

################################################################################
# Advanced Configuration
################################################################################

DEFINES+=PROJECT_PREFIX=$(DEEPCRAFT_PROJECT_NAME)

ifeq ($(DEEPCRAFT_PROJECT_NAME), LED_Demo)
    DEFINES+=USE_LED_DEMO
endif

#Uncomment to print MCPS (Voice-Assistant only)
#DEFINES+=SHOW_MCPS

# Enable optional code that is ordinarily disabled by default.
#
# Available components depend on the specific targeted hardware and firmware
# in use. In general, if you have
#
#    COMPONENTS=foo bar
#
# ... then code in directories named COMPONENT_foo and COMPONENT_bar will be
# added to the build
#
COMPONENTS+=FREERTOS CMSIS_DSP U55 RTOS_AWARE

# Lift of components for ML-middleware
COMPONENTS+=ML_TFLM ML_INT8x8 MW_MDL_VA 
COMPONENTS +=MW_MDL_DSNS MW_MDL_DSES

# Check which kit is being used
ifeq (APP_KIT_PSE84_AI, $(TARGET))
    DEFINES+=USE_KIT_PSE84_AI
endif 

# Like COMPONENTS, but disable optional code that was enabled by default.
DISABLE_COMPONENTS=

CORE=CM55
CORE_NAME=CM55_0

# By default the build system automatically looks in the Makefile's directory
# tree for source code and builds it. The SOURCES variable can be used to
# manually add source code to the build process from a location not searched
# by default, or otherwise not found by the build system.
SOURCES+=

# Like SOURCES, but for include directories. Value should be paths to
# directories (without a leading -I).
INCLUDES+=

# Add additional defines to the build process (without a leading -D).
DEFINES+=CY_RETARGET_IO_CONVERT_LF_TO_CRLF CY_RTOS_AWARE 

# List of defines for ML-middleware
DEFINES+=TF_LITE_STATIC_MEMORY

DEFINES+=CY_ML_MODEL_MEM=.cy_socmem_data
DEFINES+=CY_ML_MODEL_MEM_VID=app_code_main

NPU=U55
DEFINES+=ETHOSU55

# List of defines for DEEPCRAFT Voice Assistant
DEFINES+=ENABLE_IFX_SOD ENABLE_IFX_PRE_PROCESS_HPF ENABLE_IFX_VA_WWD ENABLE_IFX_VA_CMD

# Add necessary defines for the DEEPCRAFT Audio Enhancement
DEFINES+= DSNS_ENABLE_MTB_ML USE_MTB_ML
DEFINES+=ENABLE_AFE_MW_SUPPORT

ifeq (ENABLED, $(USE_AUDIO_ENHANCEMENT))
    DEFINES+=USE_AUDIO_ENHANCEMENT
    #AFE Tuning enablement
    DEFINES += CY_AFE_ENABLE_TUNING_FEATURE
    DEFINES +=AFE_AUDIO_PROCESSING_TASK_PRIORITY=4
endif

#For USB building
COMPONENTS += EMUSB
COMPONENTS += USBD_BASE

DEFINES+=ENABLE_XIP_LITTLEFS_ON_SAME_NOR_FLASH
DEFINES+=LFS_NO_ASSERT

ifeq (ENABLED,$(CONFIG_VOICE_ID))

    DEFINES+=ENABLE_VOICE_ID

    ifeq ($(TOOLCHAIN),LLVM_ARM)
        DEFINES+=APP_MSP_STACK_SIZE=0x4000
    else
        LDFLAGS+= --predefine="-DAPP_MSP_STACK_SIZE=0x4000"
    endif
    
    ifeq ($(TOOLCHAIN),LLVM_ARM)
        COMPONENTS+=LLVM_VID
    endif
    
    ifeq ($(TOOLCHAIN),ARM)
        COMPONENTS+=ARM_VID
    endif
    DEFINES+=RUN_FLOAT_FE
    DEFINES+=ENABLE_IFX_FE    
endif


# Include the Audio Voice Core library FULL or LIMITED version
ifeq (FULL, $(CONFIG_VOICE_CORE_MODE))
    COMPONENTS+=AVC_FULL
endif
ifeq (LIMITED, $(CONFIG_VOICE_CORE_MODE))
    COMPONENTS+=AVC_DEMO
endif


# Select softfp or hardfp floating point. Default is softfp.
VFP_SELECT+=

# Additional / custom C compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
CFLAGS+=

# Additional / custom C++ compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
CXXFLAGS+=

# Additional / custom assembler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
ASFLAGS+=

# Additional / custom linker flags.
LDFLAGS+=

# Additional / custom libraries to link in to the application.
LDLIBS+=

ifeq ($(TOOLCHAIN),LLVM_ARM)
    ifeq ($(CONFIG),Debug)
        CFLAGS+=-O1
    else
        CFLAGS+=
    endif
else
    CFLAGS+=
endif 

# Path to the linker script to use (if empty, use the default linker script).
LINKER_SCRIPT=

# Custom pre-build commands to run.
PREBUILD=

# Custom post-build commands to run.
POSTBUILD=


################################################################################
# Paths
################################################################################

# Relative path to the project directory (default is the Makefile's directory).
#
# This controls where automatic source code discovery looks for code.
CY_APP_PATH=

# Relative path to the shared repo location.
#
# All .mtb files have the format, <URI>#<COMMIT>#<LOCATION>. If the <LOCATION> 
# field begins with $$ASSET_REPO$$, then the repo is deposited in the path 
# specified by the CY_GETLIBS_SHARED_PATH variable. The default location is one 
# directory level above the current app directory. This is used with 
# CY_GETLIBS_SHARED_NAME variable, which specifies the directory name.
CY_GETLIBS_SHARED_PATH=../..

# Directory name of the shared repo location.
#
CY_GETLIBS_SHARED_NAME=mtb_shared

CY_VA_PATH=$(SEARCH_voice-assistant)
CY_AVC_PATH=$(SEARCH_audio-voice-core)
CY_AFE_PATH=$(SEARCH_audio-front-end)
CY_SOD_PATH=$(SEARCH_speech-onset-detection)

#CY_IGNORE += ${CY_VA_PATH}
#CY_IGNORE += ${CY_AVC_PATH}
#CY_IGNORE += ${CY_AFE_PATH}
#CY_IGNORE += ${CY_SOD_PATH}

include $(CY_TOOLS_DIR)/make/start.mk
