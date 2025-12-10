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
include /home/dode/dev/avrenv/avrenv-tx/Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/7eff1f7e/avrenv.o \
	${OBJECTDIR}/_ext/7eff1f7e/bme688.o \
	${OBJECTDIR}/_ext/3d12a1ed/bme68x.o \
	${OBJECTDIR}/_ext/7eff1f7e/data.o \
	${OBJECTDIR}/_ext/7eff1f7e/ens160.o \
	${OBJECTDIR}/_ext/7eff1f7e/i2c.o \
	${OBJECTDIR}/_ext/7eff1f7e/pa1616s.o \
	${OBJECTDIR}/_ext/7eff1f7e/rfm.o \
	${OBJECTDIR}/_ext/7eff1f7e/sdc.o \
	${OBJECTDIR}/_ext/7eff1f7e/spi.o \
	${OBJECTDIR}/_ext/7eff1f7e/usart.o \
	${OBJECTDIR}/_ext/7eff1f7e/utils.o


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
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/avrenv-tx

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/avrenv-tx: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/avrenv-tx ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/7eff1f7e/avrenv.o: /home/dode/dev/avrenv/avrenv-tx/avrenv.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7eff1f7e
	$(COMPILE.c) -g -DBAUD=9600 -DBAUDRATE=9600 -DF_CPU=10000000UL -DLORA=1 -D__AVR_AVR64EA28__ -D__flash=volatile -I. -I/home/dode/dev -I/usr/local/include/dode -std=c99 -o ${OBJECTDIR}/_ext/7eff1f7e/avrenv.o /home/dode/dev/avrenv/avrenv-tx/avrenv.c

${OBJECTDIR}/_ext/7eff1f7e/bme688.o: /home/dode/dev/avrenv/avrenv-tx/bme688.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7eff1f7e
	$(COMPILE.c) -g -DBAUD=9600 -DBAUDRATE=9600 -DF_CPU=10000000UL -DLORA=1 -D__AVR_AVR64EA28__ -D__flash=volatile -I. -I/home/dode/dev -I/usr/local/include/dode -o ${OBJECTDIR}/_ext/7eff1f7e/bme688.o /home/dode/dev/avrenv/avrenv-tx/bme688.c

${OBJECTDIR}/_ext/3d12a1ed/bme68x.o: /home/dode/dev/avrenv/avrenv-tx/bme68x/bme68x.c
	${MKDIR} -p ${OBJECTDIR}/_ext/3d12a1ed
	$(COMPILE.c) -g -DBAUD=9600 -DBAUDRATE=9600 -DF_CPU=10000000UL -DLORA=1 -D__AVR_AVR64EA28__ -D__flash=volatile -I. -I/home/dode/dev -I/usr/local/include/dode -o ${OBJECTDIR}/_ext/3d12a1ed/bme68x.o /home/dode/dev/avrenv/avrenv-tx/bme68x/bme68x.c

${OBJECTDIR}/_ext/7eff1f7e/data.o: /home/dode/dev/avrenv/avrenv-tx/data.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7eff1f7e
	$(COMPILE.c) -g -DBAUD=9600 -DBAUDRATE=9600 -DF_CPU=10000000UL -DLORA=1 -D__AVR_AVR64EA28__ -D__flash=volatile -I. -I/home/dode/dev -I/usr/local/include/dode -o ${OBJECTDIR}/_ext/7eff1f7e/data.o /home/dode/dev/avrenv/avrenv-tx/data.c

${OBJECTDIR}/_ext/7eff1f7e/ens160.o: /home/dode/dev/avrenv/avrenv-tx/ens160.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7eff1f7e
	$(COMPILE.c) -g -DBAUD=9600 -DBAUDRATE=9600 -DF_CPU=10000000UL -DLORA=1 -D__AVR_AVR64EA28__ -D__flash=volatile -I. -I/home/dode/dev -I/usr/local/include/dode -o ${OBJECTDIR}/_ext/7eff1f7e/ens160.o /home/dode/dev/avrenv/avrenv-tx/ens160.c

${OBJECTDIR}/_ext/7eff1f7e/i2c.o: /home/dode/dev/avrenv/avrenv-tx/i2c.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7eff1f7e
	$(COMPILE.c) -g -DBAUD=9600 -DBAUDRATE=9600 -DF_CPU=10000000UL -DLORA=1 -D__AVR_AVR64EA28__ -D__flash=volatile -I. -I/home/dode/dev -I/usr/local/include/dode -o ${OBJECTDIR}/_ext/7eff1f7e/i2c.o /home/dode/dev/avrenv/avrenv-tx/i2c.c

${OBJECTDIR}/_ext/7eff1f7e/pa1616s.o: /home/dode/dev/avrenv/avrenv-tx/pa1616s.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7eff1f7e
	$(COMPILE.c) -g -DBAUD=9600 -DBAUDRATE=9600 -DF_CPU=10000000UL -DLORA=1 -D__AVR_AVR64EA28__ -D__flash=volatile -I. -I/home/dode/dev -I/usr/local/include/dode -o ${OBJECTDIR}/_ext/7eff1f7e/pa1616s.o /home/dode/dev/avrenv/avrenv-tx/pa1616s.c

${OBJECTDIR}/_ext/7eff1f7e/rfm.o: /home/dode/dev/avrenv/avrenv-tx/rfm.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7eff1f7e
	$(COMPILE.c) -g -DBAUD=9600 -DBAUDRATE=9600 -DF_CPU=10000000UL -DLORA=1 -D__AVR_AVR64EA28__ -D__flash=volatile -I. -I/home/dode/dev -I/usr/local/include/dode -o ${OBJECTDIR}/_ext/7eff1f7e/rfm.o /home/dode/dev/avrenv/avrenv-tx/rfm.c

${OBJECTDIR}/_ext/7eff1f7e/sdc.o: /home/dode/dev/avrenv/avrenv-tx/sdc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7eff1f7e
	$(COMPILE.c) -g -DBAUD=9600 -DBAUDRATE=9600 -DF_CPU=10000000UL -DLORA=1 -D__AVR_AVR64EA28__ -D__flash=volatile -I. -I/home/dode/dev -I/usr/local/include/dode -o ${OBJECTDIR}/_ext/7eff1f7e/sdc.o /home/dode/dev/avrenv/avrenv-tx/sdc.c

${OBJECTDIR}/_ext/7eff1f7e/spi.o: /home/dode/dev/avrenv/avrenv-tx/spi.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7eff1f7e
	$(COMPILE.c) -g -DBAUD=9600 -DBAUDRATE=9600 -DF_CPU=10000000UL -DLORA=1 -D__AVR_AVR64EA28__ -D__flash=volatile -I. -I/home/dode/dev -I/usr/local/include/dode -o ${OBJECTDIR}/_ext/7eff1f7e/spi.o /home/dode/dev/avrenv/avrenv-tx/spi.c

${OBJECTDIR}/_ext/7eff1f7e/usart.o: /home/dode/dev/avrenv/avrenv-tx/usart.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7eff1f7e
	$(COMPILE.c) -g -DBAUD=9600 -DBAUDRATE=9600 -DF_CPU=10000000UL -DLORA=1 -D__AVR_AVR64EA28__ -D__flash=volatile -I. -I/home/dode/dev -I/usr/local/include/dode -o ${OBJECTDIR}/_ext/7eff1f7e/usart.o /home/dode/dev/avrenv/avrenv-tx/usart.c

${OBJECTDIR}/_ext/7eff1f7e/utils.o: /home/dode/dev/avrenv/avrenv-tx/utils.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7eff1f7e
	$(COMPILE.c) -g -DBAUD=9600 -DBAUDRATE=9600 -DF_CPU=10000000UL -DLORA=1 -D__AVR_AVR64EA28__ -D__flash=volatile -I. -I/home/dode/dev -I/usr/local/include/dode -o ${OBJECTDIR}/_ext/7eff1f7e/utils.o /home/dode/dev/avrenv/avrenv-tx/utils.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:
