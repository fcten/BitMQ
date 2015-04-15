#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/common/wbt_config.o \
	${OBJECTDIR}/src/common/wbt_connection.o \
	${OBJECTDIR}/src/common/wbt_event.o \
	${OBJECTDIR}/src/common/wbt_file.o \
	${OBJECTDIR}/src/common/wbt_heap.o \
	${OBJECTDIR}/src/common/wbt_list.o \
	${OBJECTDIR}/src/common/wbt_log.o \
	${OBJECTDIR}/src/common/wbt_memory.o \
	${OBJECTDIR}/src/common/wbt_module.o \
	${OBJECTDIR}/src/common/wbt_rbtree.o \
	${OBJECTDIR}/src/common/wbt_string.o \
	${OBJECTDIR}/src/common/wbt_time.o \
	${OBJECTDIR}/src/http/wbt_http.o \
	${OBJECTDIR}/src/http/wbt_http_consts.o \
	${OBJECTDIR}/src/os/linux/wbt_process.o \
	${OBJECTDIR}/src/os/linux/wbt_sigsegv.o \
	${OBJECTDIR}/src/webit.o


# C Compiler Flags
CFLAGS=-ldl

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/webit

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/webit: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${CC} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/webit ${OBJECTFILES} ${LDLIBSOPTIONS} ${CFLAGS} 

${OBJECTDIR}/src/common/wbt_config.o: src/common/wbt_config.c 
	${MKDIR} -p ${OBJECTDIR}/src/common
	${RM} $@.d
	$(COMPILE.c) -O2 -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/common/wbt_config.o src/common/wbt_config.c

${OBJECTDIR}/src/common/wbt_connection.o: src/common/wbt_connection.c 
	${MKDIR} -p ${OBJECTDIR}/src/common
	${RM} $@.d
	$(COMPILE.c) -O2 -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/common/wbt_connection.o src/common/wbt_connection.c

${OBJECTDIR}/src/common/wbt_event.o: src/common/wbt_event.c 
	${MKDIR} -p ${OBJECTDIR}/src/common
	${RM} $@.d
	$(COMPILE.c) -O2 -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/common/wbt_event.o src/common/wbt_event.c

${OBJECTDIR}/src/common/wbt_file.o: src/common/wbt_file.c 
	${MKDIR} -p ${OBJECTDIR}/src/common
	${RM} $@.d
	$(COMPILE.c) -O2 -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/common/wbt_file.o src/common/wbt_file.c

${OBJECTDIR}/src/common/wbt_heap.o: src/common/wbt_heap.c 
	${MKDIR} -p ${OBJECTDIR}/src/common
	${RM} $@.d
	$(COMPILE.c) -O2 -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/common/wbt_heap.o src/common/wbt_heap.c

${OBJECTDIR}/src/common/wbt_list.o: src/common/wbt_list.c 
	${MKDIR} -p ${OBJECTDIR}/src/common
	${RM} $@.d
	$(COMPILE.c) -O2 -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/common/wbt_list.o src/common/wbt_list.c

${OBJECTDIR}/src/common/wbt_log.o: src/common/wbt_log.c 
	${MKDIR} -p ${OBJECTDIR}/src/common
	${RM} $@.d
	$(COMPILE.c) -O2 -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/common/wbt_log.o src/common/wbt_log.c

${OBJECTDIR}/src/common/wbt_memory.o: src/common/wbt_memory.c 
	${MKDIR} -p ${OBJECTDIR}/src/common
	${RM} $@.d
	$(COMPILE.c) -O2 -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/common/wbt_memory.o src/common/wbt_memory.c

${OBJECTDIR}/src/common/wbt_module.o: src/common/wbt_module.c 
	${MKDIR} -p ${OBJECTDIR}/src/common
	${RM} $@.d
	$(COMPILE.c) -O2 -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/common/wbt_module.o src/common/wbt_module.c

${OBJECTDIR}/src/common/wbt_rbtree.o: src/common/wbt_rbtree.c 
	${MKDIR} -p ${OBJECTDIR}/src/common
	${RM} $@.d
	$(COMPILE.c) -O2 -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/common/wbt_rbtree.o src/common/wbt_rbtree.c

${OBJECTDIR}/src/common/wbt_string.o: src/common/wbt_string.c 
	${MKDIR} -p ${OBJECTDIR}/src/common
	${RM} $@.d
	$(COMPILE.c) -O2 -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/common/wbt_string.o src/common/wbt_string.c

${OBJECTDIR}/src/common/wbt_time.o: src/common/wbt_time.c 
	${MKDIR} -p ${OBJECTDIR}/src/common
	${RM} $@.d
	$(COMPILE.c) -O2 -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/common/wbt_time.o src/common/wbt_time.c

${OBJECTDIR}/src/http/wbt_http.o: src/http/wbt_http.c 
	${MKDIR} -p ${OBJECTDIR}/src/http
	${RM} $@.d
	$(COMPILE.c) -O2 -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/http/wbt_http.o src/http/wbt_http.c

${OBJECTDIR}/src/http/wbt_http_consts.o: src/http/wbt_http_consts.c 
	${MKDIR} -p ${OBJECTDIR}/src/http
	${RM} $@.d
	$(COMPILE.c) -O2 -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/http/wbt_http_consts.o src/http/wbt_http_consts.c

${OBJECTDIR}/src/os/linux/wbt_process.o: src/os/linux/wbt_process.c 
	${MKDIR} -p ${OBJECTDIR}/src/os/linux
	${RM} $@.d
	$(COMPILE.c) -O2 -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/os/linux/wbt_process.o src/os/linux/wbt_process.c

${OBJECTDIR}/src/os/linux/wbt_sigsegv.o: src/os/linux/wbt_sigsegv.c 
	${MKDIR} -p ${OBJECTDIR}/src/os/linux
	${RM} $@.d
	$(COMPILE.c) -O2 -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/os/linux/wbt_sigsegv.o src/os/linux/wbt_sigsegv.c

${OBJECTDIR}/src/webit.o: src/webit.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -O2 -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/webit.o src/webit.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/webit

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
