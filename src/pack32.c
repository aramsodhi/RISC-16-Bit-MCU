#include <inttypes.h>

/*   
0000 0000 0000 0000 |  0  | 000 0 | 000 0 |  0  | 00 0000 |
       imm 16       | N/A |  rs1  |  rd   |  i  |   op    |
*/

uint32_t packRR(uint8_t op, uint8_t rd, uint8_t rs1, uint8_t rs2) {
    return (uint32_t)((rs2 & 0xF) << 16)
            | (uint32_t)((rs1 & 0xF) << 11)
            | (uint32_t)((rd & 0xF) << 7)
            | (uint32_t)(op & 0x3F);
}

uint32_t packRI(uint8_t op, uint8_t rd, uint8_t rs1, uint16_t imm) {
    return (uint32_t)(imm << 16)
            | (uint32_t)((rs1 & 0xF) << 11)
            | (uint32_t)((rd & 0xF) << 7)
            | (uint32_t)(0x40)
            | (uint32_t)(op & 0x3F);
}

uint32_t packMEM(uint8_t op, uint8_t rd, uint16_t addr) {
    return (uint32_t)(addr << 16)
        | (uint32_t)((rd & 0xF) << 7)
        | (uint32_t)(0x40)
        | (uint32_t)(op & 0x3F);
}

uint32_t packJ(uint8_t op, uint16_t addr) {
    return (uint32_t)(addr << 16)
        | (uint32_t)(0x40)
        | (uint32_t)(op & 0x3F);
}

// branch instructions use rd as rs1
uint32_t packBR(uint8_t op, uint8_t rs1, uint8_t rs2, uint16_t addr) {
    return (uint32_t)(addr << 16)
        | (uint32_t)((rs2 & 0xF) << 11)
        | (uint32_t)((rs1 & 0xF) << 7)
        | (uint32_t)(0x40)
        | (uint32_t)(op & 0x3F);
}

uint32_t packHLT(uint8_t op) {
    return (uint32_t)(op & 0x3F);
}
