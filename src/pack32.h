#ifndef PACK32_H
#define PACK32_H

#include <inttypes.h>

uint32_t packRR(uint8_t op, uint8_t rd, uint8_t rs1, uint8_t rs2);
uint32_t packRI(uint8_t op, uint8_t rd, uint8_t rs1, uint16_t imm);
uint32_t packMEM(uint8_t op, uint8_t rd, uint16_t addr);
uint32_t packJ(uint8_t op, uint16_t addr);
uint32_t packBR(uint8_t op, uint8_t rs1, uint8_t rs2, uint16_t addr); // branch instructions use rd as rs1
uint32_t packHLT(uint8_t op);

#endif
