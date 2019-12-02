#***************************************************************************
#	Copyright (c) 2019, Xilinx, Inc.
#	All rights reserved.

#	Redistribution and use in source and binary forms, with or without modification, 
#	are permitted provided that the following conditions are met:

#	1. Redistributions of source code must retain the above copyright notice, 
#	this list of conditions and the following disclaimer.

#	2. Redistributions in binary form must reproduce the above copyright notice, 
#	this list of conditions and the following disclaimer in the documentation 
#	and/or other materials provided with the distribution.

#	3. Neither the name of the copyright holder nor the names of its contributors 
#	may be used to endorse or promote products derived from this software 
#	without specific prior written permission.

#	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
#	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
#	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
#	IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
#	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
#	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
#	HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
#	OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
#	EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#***************************************************************************/



#-------------------------------------------------------------------------------
# Board specific variables
#-------------------------------------------------------------------------------
PLATFORM 	= zed #zcu104
CLK_VAL        ?= 4 
DMCLK_VAL      ?= ${CLK_VAL}
#PLATFORM_T     ?= ZCU102-RV

#-------------------------------------------------------------------------------
# Enables for Lib generation
#-------------------------------------------------------------------------------
LIB_GEN_ENA    ?= 0


#-------------------------------------------------------------------------------
# Enables URAM for ZCU104
#-------------------------------------------------------------------------------
URAM_ENA := $(shell echo $(PLATFORM) | grep "zcu104" | wc -l)

ifeq ($(URAM_ENA), 1)
   URAM_ENA_FLAG := -DXF_USE_URAM=true
else
   URAM_ENA_FLAG := -DXF_USE_URAM=false
endif

#-------------------------------------------------------------------------------
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Function related variables
#-------------------------------------------------------------------------------
# Note: Valid values for FUNC_TYPE are "imgproc / core / features"
FUNC_TYPE      := imgproc
FUNC_TYPE1     := video
FUNC_NAME      := xf_pyr_dense_optical_flow
FUNC_NAME_1    := xf_pyr_down
FUNC_NAME_2    := xf_pyr_dense_optical_flow_wrapper
FUNC_DIR       := lkdensepyrof_videoinput
HW_TOP_NAME    := "xf::pyrDown <0,  480, 640, 1,${URAM_ENA}>"
HW_TOP_NAME_1  := "xf::densePyrOpticalFlow<4, 50,5,8,5,0,480,640,1,${URAM_ENA}>"
EXECUTABLE     := ipOF.elf

#-------------------------------------------------------------------------------
# Internal Path variables
#-------------------------------------------------------------------------------
XLNX_SRC_PATH  ?= $(shell pwd)/../..
WORK_PATH      := $(shell pwd)
PROJ_TOP_PATH  := ${XLNX_SRC_PATH}
TB_SRC_PATH    := ${PROJ_TOP_PATH}/examples/${FUNC_DIR}
SRC_PATH       := ${PROJ_TOP_PATH}/include/${FUNC_TYPE}
SRC_PATH1       := ${PROJ_TOP_PATH}/include/${FUNC_TYPE1}

#-------------------------------------------------------------------------------
# Source files
#-------------------------------------------------------------------------------
APPSOURCES := ${TB_SRC_PATH}/${FUNC_NAME}_tb.cpp ${TB_SRC_PATH}/${FUNC_NAME}_accel.cpp

#-------------------------------------------------------------------------------
# Library/Include paths
#-------------------------------------------------------------------------------
IDIRS = -I. -I../../../arm_opencv_249/include -I../../../arm_other_libs/include -I${PROJ_TOP_PATH}/include
LDIRS =  -L../../../arm_opencv_249/lib -L../../../arm_other_libs/lib
LLIBS ?= -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect  -lopencv_flann -llzma -ltiff -lpng16 -lz -ljpeg -ldl -lrt -lwebp  -lopencv_flann -lavcodec -lavformat -lavutil -lswscale -lx264


#-------------------------------------------------------------------------------
# sds++/c++ Flags
#-------------------------------------------------------------------------------
DEBUG_FLAG  :=#-ggdb
VERBOSE     :=#-verbose
OLEVEL      := -O0
WARN_FLAGS  := -Wall -Wno-unused-label -Wno-unused-variable -Wno-unused-function# -Wno-int-in-bool-context -Wno-mismatched-tags -Wno-unknown-warning-option -Wno-unneeded-internal-declaration -Wno-extra-tokens

SDSCC_FLAGS := -sds-pf ${PLATFORM} ${URAM_ENA_FLAG}\
               -sds-hw ${HW_TOP_NAME} ${TB_SRC_PATH}/${FUNC_NAME}_accel.cpp -files ${SRC_PATH}/${FUNC_NAME_1}.hpp -clkid ${CLK_VAL} -sds-end \
			   -sds-hw ${HW_TOP_NAME_1} ${TB_SRC_PATH}/${FUNC_NAME}_accel.cpp -files ${SRC_PATH1}/${FUNC_NAME_2}.hpp -clkid ${CLK_VAL} -sds-end \
			   -dmclkid ${DMCLK_VAL} \
			   -xp "vivado_prop:run.impl_1.{STEPS.OPT_DESIGN.ARGS.MORE OPTIONS}={-directive Explore}" \
			   -xp "vivado_prop:run.impl_1.{STEPS.PHYS_OPT_DESIGN.ARGS.MORE OPTIONS}={-directive Explore}" \
			   -xp "vivado_prop:run.impl_1.{STEPS.ROUTE_DESIGN.ARGS.MORE OPTIONS}={-directive Explore}" \
			   -xp "vivado_prop:run.impl_1.{STEPS.PHYS_OPT_DESIGN.ARGS.MORE OPTIONS}={-directive Explore}"


CFLAGS := ${DEBUG_FLAG} ${IDIRS} ${OLEVEL} ${WARN_FLAGS} -DHLS_NO_XIL_FPO_LIB -hls-target 1  #-std=c++11 # -mstrict-align 
LFLAGS := ${DEBUG_FLAG} ${LDIRS} ${LLIBS} 
 
ifeq (${LIB_GEN_ENA}, 1)
  CFLAGS += -FPIC
  LFLAGS += -shared
endif

#-------------------------------------------------------------------------------
# Tool variables
#-------------------------------------------------------------------------------
CC := sds++

#-------------------------------------------------------------------------------
# Internal variables
#-------------------------------------------------------------------------------
SRC_FILES := $(notdir $(APPSOURCES))
OBJECTS   := ${SRC_FILES:%.cpp=${WORK_PATH}/%.o}

#-------------------------------------------------------------------------------
# Target(s) declaration:
#-------------------------------------------------------------------------------
# Default
# ------------------------
.PHONY: all
all: ${EXECUTABLE}

.PHONY: obj
obj: ${WORK_PATH}/${FUNC_NAME}_accel.o

# Linking
# ------------------------
${EXECUTABLE}: ${OBJECTS}
	${CC} ${SDSCC_FLAGS} ${OBJECTS} ${LFLAGS} -o $@

# Dependencies
# Note: Include only if exists
# ------------------------
#-include ${DEPS}

# Compiling
# ------------------------
${WORK_PATH}/%.o: ${TB_SRC_PATH}/%.cpp
	${CC} ${SDSCC_FLAGS} -c  ${CFLAGS} $< -o $@

# House keeping
# ------------------------
clean:
	${RM} ${EXECUTABLE} ${OBJECTS}

ultraclean: clean
	${RM} -rf _sds sd_card ${EXECUTABLE}.bit ${EXECUTABLE} ${OBJECTS} *.s

.PHONY: copy
copy:
	@echo "Makefile got changed, so this target (copy) is obsolete."
