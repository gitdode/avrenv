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
CC=avr-gcc
CCC=avr-g++
CXX=avr-g++
FC=gfortran
AS=avr-as

# Macros
CND_PLATFORM=AVR-Linux
CND_DLIB_EXT=so
CND_CONF=Custom
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include /home/dode/dev/avrenv/avrenv-rx/Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/7eff1f40/avrenv.o \
	${OBJECTDIR}/_ext/7eff1f40/data.o \
	${OBJECTDIR}/_ext/7eff1f40/rfm.o \
	${OBJECTDIR}/_ext/7eff1f40/spi.o \
	${OBJECTDIR}/_ext/7eff1f40/tft.o \
	${OBJECTDIR}/_ext/7eff1f40/usart.o \
	${OBJECTDIR}/_ext/7eff1f40/utils.o


# C Compiler Flags
CFLAGS=

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
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/avrenv-rx

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/avrenv-rx: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/avrenv-rx ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/7eff1f40/avrenv.o: /home/dode/dev/avrenv/avrenv-rx/avrenv.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7eff1f40
	$(COMPILE.c) -g -DBAUD=9600 -DBAUDRATE=9600 -DBGR=1 -DDISPLAY_HEIGHT=240 -DDISPLAY_WIDTH=320 -DF_CPU=10000000UL -DHFLIP=1 -DINVERT=0 -DRFM=69 -DVFLIP=1 -D__AVR_AVR64EA28__ -D__flash=volatile -I. -I/home/dode/dev -std=c99 -o ${OBJECTDIR}/_ext/7eff1f40/avrenv.o /home/dode/dev/avrenv/avrenv-rx/avrenv.c

${OBJECTDIR}/_ext/7eff1f40/data.o: /home/dode/dev/avrenv/avrenv-rx/data.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7eff1f40
	$(COMPILE.c) -g -DBAUD=9600 -DBAUDRATE=9600 -DBGR=1 -DDISPLAY_HEIGHT=240 -DDISPLAY_WIDTH=320 -DF_CPU=10000000UL -DHFLIP=1 -DINVERT=0 -DRFM=69 -DVFLIP=1 -D__AVR_AVR64EA28__ -D__flash=volatile -I. -I/home/dode/dev -o ${OBJECTDIR}/_ext/7eff1f40/data.o /home/dode/dev/avrenv/avrenv-rx/data.c

${OBJECTDIR}/_ext/7eff1f40/rfm.o: /home/dode/dev/avrenv/avrenv-rx/rfm.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7eff1f40
	$(COMPILE.c) -g -DBAUD=9600 -DBAUDRATE=9600 -DBGR=1 -DDISPLAY_HEIGHT=240 -DDISPLAY_WIDTH=320 -DF_CPU=10000000UL -DHFLIP=1 -DINVERT=0 -DRFM=69 -DVFLIP=1 -D__AVR_AVR64EA28__ -D__flash=volatile -I. -I/home/dode/dev -o ${OBJECTDIR}/_ext/7eff1f40/rfm.o /home/dode/dev/avrenv/avrenv-rx/rfm.c

${OBJECTDIR}/_ext/7eff1f40/spi.o: /home/dode/dev/avrenv/avrenv-rx/spi.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7eff1f40
	$(COMPILE.c) -g -DBAUD=9600 -DBAUDRATE=9600 -DBGR=1 -DDISPLAY_HEIGHT=240 -DDISPLAY_WIDTH=320 -DF_CPU=10000000UL -DHFLIP=1 -DINVERT=0 -DRFM=69 -DVFLIP=1 -D__AVR_AVR64EA28__ -D__flash=volatile -I. -I/home/dode/dev -o ${OBJECTDIR}/_ext/7eff1f40/spi.o /home/dode/dev/avrenv/avrenv-rx/spi.c

${OBJECTDIR}/_ext/7eff1f40/tft.o: /home/dode/dev/avrenv/avrenv-rx/tft.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7eff1f40
	$(COMPILE.c) -g -DBAUD=9600 -DBAUDRATE=9600 -DBGR=1 -DDISPLAY_HEIGHT=240 -DDISPLAY_WIDTH=320 -DF_CPU=10000000UL -DHFLIP=1 -DINVERT=0 -DRFM=69 -DVFLIP=1 -D__AVR_AVR64EA28__ -D__flash=volatile -I. -I/home/dode/dev -o ${OBJECTDIR}/_ext/7eff1f40/tft.o /home/dode/dev/avrenv/avrenv-rx/tft.c

${OBJECTDIR}/_ext/7eff1f40/usart.o: /home/dode/dev/avrenv/avrenv-rx/usart.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7eff1f40
	$(COMPILE.c) -g -DBAUD=9600 -DBAUDRATE=9600 -DBGR=1 -DDISPLAY_HEIGHT=240 -DDISPLAY_WIDTH=320 -DF_CPU=10000000UL -DHFLIP=1 -DINVERT=0 -DRFM=69 -DVFLIP=1 -D__AVR_AVR64EA28__ -D__flash=volatile -I. -I/home/dode/dev -o ${OBJECTDIR}/_ext/7eff1f40/usart.o /home/dode/dev/avrenv/avrenv-rx/usart.c

${OBJECTDIR}/_ext/7eff1f40/utils.o: /home/dode/dev/avrenv/avrenv-rx/utils.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7eff1f40
	$(COMPILE.c) -g -DBAUD=9600 -DBAUDRATE=9600 -DBGR=1 -DDISPLAY_HEIGHT=240 -DDISPLAY_WIDTH=320 -DF_CPU=10000000UL -DHFLIP=1 -DINVERT=0 -DRFM=69 -DVFLIP=1 -D__AVR_AVR64EA28__ -D__flash=volatile -I. -I/home/dode/dev -o ${OBJECTDIR}/_ext/7eff1f40/utils.o /home/dode/dev/avrenv/avrenv-rx/utils.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:
