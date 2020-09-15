# Makefile for user mode framework

GCC				:=riscv64-unknown-elf-gcc
SPIKE			:=spike
PKE				:=../obj/pke
OPT				:=-o
TARGET			:=riscv64
BIN_PATH		:=./bin
TARGET_PATH		:=${BIN_PATH}/${TARGET}

SRC_PATH		:=./src
SYSCALL_PATH	:=${SRC_PATH}/syscall
SYSCALL			:=${SYSCALL_PATH}/syscall.c

TEST_PATH		:=${SRC_PATH}/test

SOURCE			:=${TEST_PATH}/test_1.c

BINARY_FILE		:=${TARGET_PATH}/test_1

build: ${BINARY_FILE}

run: build ${PKE}
	@${SPIKE} ${PKE} ${BINARY_FILE}

${BINARY_FILE}: ${TARGET_PATH}
	@${GCC} ${SOURCE} ${SYSCALL} ${OPT} ${BINARY_FILE}

${TARGET_PATH}:
	@rm -rf ${TARGET_PATH}
	@mkdir ${TARGET_PATH}

${PKE}:
	# TODO

clean:
	@rm -rf ${TARGET_PATH}