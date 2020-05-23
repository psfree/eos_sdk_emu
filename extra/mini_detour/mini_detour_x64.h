#ifndef MINI_DETOUR_X64_H
#define MINI_DETOUR_X64_H

// // http://ref.x86asm.net/coder64.html

struct
{
    bool has_r_m;
    uint8_t base_size;
} s_opcodes[256] =
{
    {false, 0}, // 0x00
    {false, 0}, // 0x01
    {false, 0}, // 0x02
    {false, 0}, // 0x03
    {false, 0}, // 0x04
    {false, 0}, // 0x05
    {false, 0}, // 0x06
    {false, 0}, // 0x07
    {false, 0}, // 0x08
    {false, 0}, // 0x09
    {false, 0}, // 0x0a
    {false, 0}, // 0x0b
    {false, 0}, // 0x0c
    {false, 0}, // 0x0d
    {false, 0}, // 0x0e
    {false, 0}, // 0x0f
    {true , 2}, // 0x10 ADD 8 m8
    {false, 0}, // 0x11
    {false, 0}, // 0x12
    {false, 0}, // 0x13
    {false, 0}, // 0x14
    {false, 0}, // 0x15
    {false, 0}, // 0x16
    {false, 0}, // 0x17
    {false, 0}, // 0x18
    {false, 0}, // 0x19
    {false, 0}, // 0x1a
    {false, 0}, // 0x1b
    {false, 0}, // 0x1c
    {false, 0}, // 0x1d
    {false, 0}, // 0x1e
    {false, 0}, // 0x1f
    {false, 0}, // 0x20
    {false, 0}, // 0x21
    {false, 0}, // 0x22
    {false, 0}, // 0x23
    {false, 0}, // 0x24
    {false, 0}, // 0x25
    {false, 0}, // 0x26
    {false, 0}, // 0x27
    {false, 0}, // 0x28
    {false, 0}, // 0x29
    {false, 0}, // 0x2a
    {false, 0}, // 0x2b
    {false, 0}, // 0x2c
    {false, 0}, // 0x2d
    {false, 0}, // 0x2e
    {false, 0}, // 0x2f
    {false, 0}, // 0x30
    {false, 0}, // 0x31
    {false, 0}, // 0x32
    {false, 0}, // 0x33
    {false, 0}, // 0x34
    {false, 0}, // 0x35
    {false, 0}, // 0x36
    {false, 0}, // 0x37
    {false, 0}, // 0x38
    {false, 0}, // 0x39
    {false, 0}, // 0x3a
    {false, 0}, // 0x3b
    {false, 0}, // 0x3c
    {false, 0}, // 0x3d
    {false, 0}, // 0x3e
    {false, 0}, // 0x3f
    {false, 1}, // 0x40 REX
    {false, 1}, // 0x41 REX.B
    {false, 1}, // 0x42 REX.X
    {false, 1}, // 0x43 REX.XB
    {false, 1}, // 0x44 REX.R
    {false, 1}, // 0x45 REX.RB
    {false, 1}, // 0x46 REX.RX
    {false, 1}, // 0x47 REX.RXB
    {false, 1}, // 0x48 REX.W
    {false, 1}, // 0x49 REX.WB
    {false, 1}, // 0x4a REX.WX
    {false, 1}, // 0x4b REX.WXB
    {false, 1}, // 0x4c REX.WR
    {false, 1}, // 0x4d REX.WRB
    {false, 1}, // 0x4e REX.WRX
    {false, 1}, // 0x4f REX.WRXB
    {false, 1}, // 0x50 PUSH RAX
    {false, 1}, // 0x51 PUSH RCX
    {false, 1}, // 0x52 PUSH RDX
    {false, 1}, // 0x53 PUSH RBX
    {false, 1}, // 0x54 PUSH RSP
    {false, 1}, // 0x55 PUSH RBP
    {false, 1}, // 0x56 PUSH RSI
    {false, 1}, // 0x57 PUSH RDI
    {false, 1}, // 0x58 POP RAX
    {false, 1}, // 0x59 POP RCX
    {false, 1}, // 0x5a POP RDX
    {false, 1}, // 0x5b POP RBX
    {false, 1}, // 0x5c POP RSP
    {false, 1}, // 0x5d POP RBP
    {false, 1}, // 0x5e POP RSI
    {false, 1}, // 0x5f POP RDI
    {false, 0}, // 0x60 INVALID
    {false, 0}, // 0x61 INVALID
    {false, 0}, // 0x62 INVALID
    {true , 2}, // 0x63 MOVSXD
    {false, 0}, // 0x64
    {false, 0}, // 0x65
    {false, 0}, // 0x66
    {false, 0}, // 0x67
    {false, 5}, // 0x68 PUSH DWORD
    {false, 0}, // 0x69
    {false, 2}, // 0x6a PUSH BYTE
    {false, 0}, // 0x6b
    {false, 0}, // 0x6c
    {false, 0}, // 0x6d
    {false, 0}, // 0x6e
    {false, 0}, // 0x6f
    {false, 0}, // 0x70
    {false, 0}, // 0x71
    {false, 0}, // 0x72
    {false, 0}, // 0x73
    {false, 0}, // 0x74
    {false, 0}, // 0x75
    {false, 0}, // 0x76
    {false, 0}, // 0x77
    {false, 0}, // 0x78
    {false, 0}, // 0x79
    {false, 0}, // 0x7a
    {false, 0}, // 0x7b
    {false, 0}, // 0x7c
    {false, 0}, // 0x7d
    {false, 0}, // 0x7e
    {false, 0}, // 0x7f
    {true , 3}, // 0x80 ADD 	r/m8 	    imm8
    {true , 6}, // 0x81 ADD 	r/m16/32/64 imm16/32
    {false, 0}, // 0x82 INVALID
    {true , 3}, // 0x83 ADD 	r/m16/32 	imm8
    {true , 2}, // 0x84
    {false, 0}, // 0x85
    {false, 0}, // 0x86
    {false, 0}, // 0x87
    {true , 2}, // 0x88 MOV 	r/m8 	    r8
    {true , 2}, // 0x89 MOV 	r/m16/32/64	r16/32/64
    {true , 2}, // 0x8a MOV 	r8      	r/m8
    {true , 2}, // 0x8b MOV 	r16/32/64 	r/m16/32/64
    {true , 2}, // 0x8c MOV 	m16 	    Sreg
    {true , 2}, // 0x8d LEA 	r16/32/64 	m
    {true , 2}, // 0x8e MOV 	Sreg 	    r/m16
    {false, 2}, // 0x8f POP     r/m64/16
    {false, 1}, // 0x90 NOP
    {false, 0}, // 0x91
    {false, 0}, // 0x92
    {false, 0}, // 0x93
    {false, 0}, // 0x94
    {false, 0}, // 0x95
    {false, 0}, // 0x96
    {false, 0}, // 0x97
    {false, 0}, // 0x98
    {false, 0}, // 0x99
    {false, 0}, // 0x9a
    {false, 0}, // 0x9b
    {false, 0}, // 0x9c
    {false, 0}, // 0x9d
    {false, 0}, // 0x9e
    {false, 0}, // 0x9f
    {false, 0}, // 0xa0
    {false, 0}, // 0xa1
    {false, 0}, // 0xa2
    {false, 0}, // 0xa3
    {false, 0}, // 0xa4
    {false, 0}, // 0xa5
    {false, 0}, // 0xa6
    {false, 0}, // 0xa7
    {false, 0}, // 0xa8
    {false, 0}, // 0xa9
    {false, 0}, // 0xaa
    {false, 0}, // 0xab
    {false, 0}, // 0xac
    {false, 0}, // 0xad
    {false, 0}, // 0xae
    {false, 0}, // 0xaf
    {false, 0}, // 0xb0
    {false, 0}, // 0xb1
    {false, 0}, // 0xb2
    {false, 0}, // 0xb3
    {false, 0}, // 0xb4
    {false, 0}, // 0xb5
    {false, 0}, // 0xb6
    {false, 0}, // 0xb7
    {false, 0}, // 0xb8
    {false, 0}, // 0xb9
    {false, 0}, // 0xba
    {false, 0}, // 0xbb
    {false, 0}, // 0xbc
    {false, 0}, // 0xbd
    {false, 0}, // 0xbe
    {false, 0}, // 0xbf
    {false, 0}, // 0xc0
    {false, 0}, // 0xc1
    {false, 3}, // 0xc2 RETN_IMM16
    {false, 1}, // 0xc3 RET
    {false, 0}, // 0xc4
    {false, 0}, // 0xc5
    {false, 0}, // 0xc6
    {false, 0}, // 0xc7
    {false, 0}, // 0xc8
    {false, 1}, // 0xc9 LEAVE
    {false, 3}, // 0xca RETF_IMM16
    {false, 1}, // 0xcb RETF
    {false, 1}, // 0xcc INT
    {false, 2}, // 0xcd INT_IMM8
    {false, 1}, // 0xce INTO
    {false, 1}, // 0xcf IRET
    {false, 0}, // 0xd0
    {false, 0}, // 0xd1
    {false, 0}, // 0xd2
    {false, 0}, // 0xd3
    {false, 0}, // 0xd4
    {false, 0}, // 0xd5
    {false, 0}, // 0xd6
    {false, 0}, // 0xd7
    {false, 0}, // 0xd8
    {false, 0}, // 0xd9
    {false, 0}, // 0xda
    {false, 0}, // 0xdb
    {false, 0}, // 0xdc
    {false, 0}, // 0xdd
    {false, 0}, // 0xde
    {false, 0}, // 0xdf
    {false, 0}, // 0xe0
    {false, 0}, // 0xe1
    {false, 0}, // 0xe2
    {false, 0}, // 0xe3
    {false, 0}, // 0xe4
    {false, 0}, // 0xe5
    {false, 0}, // 0xe6
    {false, 0}, // 0xe7
    {false, 5}, // 0xe8 CALL
    {false, 5}, // 0xe9 JMP
    {false, 0}, // 0xea UNUSED
    {false, 2}, // 0xeb SHORT_JMP
    {false, 0}, // 0xec
    {false, 0}, // 0xed
    {false, 0}, // 0xee
    {false, 0}, // 0xef
    {false, 0}, // 0xf0
    {false, 0}, // 0xf1
    {false, 0}, // 0xf2
    {false, 0}, // 0xf3
    {false, 0}, // 0xf4
    {false, 0}, // 0xf5
    {false, 0}, // 0xf6
    {false, 0}, // 0xf7
    {false, 0}, // 0xf8
    {false, 0}, // 0xf9
    {false, 0}, // 0xfa
    {false, 0}, // 0xfb
    {false, 0}, // 0xfc
    {false, 0}, // 0xfd
    {false, 0}, // 0xfe
    {false, 0}, // 0xff
};

static constexpr auto mod_mask = 0xC0;
static constexpr auto register_addressing_mode = 0xC0;
static constexpr auto four_bytes_signed_displacement = 0x80;
static constexpr auto one_byte_signed_displacement = 0x40;

static constexpr auto rm_mask = 0x07;
static constexpr auto displacement_only_addressing = 0x05;
static constexpr auto sib_with_no_displacement = 0x04;
static constexpr auto register_indirect_addressing_mode = 0x00;

enum opcodes_e // Commonly used opcode in the beginning of functions
{
    ADD_8_r8 = 0x10,
    REX = 0x40, REX_B, REX_X, REX_XB, REX_R, REX_RB, REX_RX, REX_RXB,
    REX_W, REX_WB, REX_WX, REX_WXB, REX_WR, REX_WRB, REX_WRX, REX_WRXB,
    PUSH_RAX, PUSH_RCX, PUSH_RDX, PUSH_RBX, PUSH_RSP, PUSH_RBP, PUSH_RSI, PUSH_RDI,
    POP_RAX, POP_RCX, POP_RDX, POP_RBX, POP_RSP, POP_RBP, POP_RSI, POP_RDI,
    PUSH_D = 0x68, PUSH_B = 0x6a,
    ADD_Br_IMM8 = 0x80, ADD_Dr_IMM32, ADD_Dr_IMM8, TEST_Br_r8, TEST_Dr_r32,
    MOV_rm8_r8 = 0x88, MOV_rm32_r32, MOV_r8_rm8, MOV_r32_rm32,
    LEA = 0x8d,
    NOP = 0x90,
    CALL = 0xe8, JMP, LJMP, SHORT_JMP,
    RETN_IMM16 = 0xC2, RET,
    LEAVE = 0xC9, RETF_IMM16, RETF, INTERRUPT, INT_IMM8, INTO, IRET
};

const char* opcode_name(uint8_t opcode)
{
#define NAME(opcode_name) case opcode_name: return #opcode_name
    switch (opcode)
    {
        NAME(ADD_8_r8);
        NAME(REX); NAME(REX_B); NAME(REX_X); NAME(REX_XB); NAME(REX_R); NAME(REX_RB); NAME(REX_RX); NAME(REX_RXB);
        NAME(REX_W); NAME(REX_WB); NAME(REX_WX); NAME(REX_WXB); NAME(REX_WR); NAME(REX_WRB); NAME(REX_WRX); NAME(REX_WRXB);
        NAME(PUSH_RAX); NAME(PUSH_RCX); NAME(PUSH_RDX); NAME(PUSH_RBX); NAME(PUSH_RSP); NAME(PUSH_RBP); NAME(PUSH_RSI); NAME(PUSH_RDI);
        NAME(POP_RAX); NAME(POP_RCX); NAME(POP_RDX); NAME(POP_RBX); NAME(POP_RSP); NAME(POP_RBP); NAME(POP_RSI); NAME(POP_RDI);
        NAME(ADD_Br_IMM8); NAME(ADD_Dr_IMM32); NAME(ADD_Dr_IMM8); NAME(TEST_Br_r8); NAME(TEST_Dr_r32);
        NAME(MOV_rm8_r8); NAME(MOV_rm32_r32); NAME(MOV_r8_rm8); NAME(MOV_r32_rm32);
        NAME(LEA);
        NAME(NOP);
        NAME(CALL); NAME(JMP); NAME(SHORT_JMP);
        NAME(RETN_IMM16); NAME(RET);
        NAME(LEAVE); NAME(RETF_IMM16); NAME(RETF); NAME(INTERRUPT); NAME(INT_IMM8); NAME(INTO); NAME(IRET);
    }
#undef NAME
    return "no registered";
}

#endif // MINI_DETOUR_X64_H
