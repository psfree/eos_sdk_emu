#ifndef MINI_DETOUR_X86_H
#define MINI_DETOUR_X86_H

// http://ref.x86asm.net/coder32.html

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
    {false, 1}, // 0x40 INC eax
    {false, 1}, // 0x41 INC ecx
    {false, 1}, // 0x42 INC edx
    {false, 1}, // 0x43 INC ebx
    {false, 1}, // 0x44 INC esp
    {false, 1}, // 0x45 INC ebp
    {false, 1}, // 0x46 INC esi
    {false, 1}, // 0x47 INC edi
    {false, 1}, // 0x48 DEC eax
    {false, 1}, // 0x49 DEC ecx
    {false, 1}, // 0x4a DEC edx
    {false, 1}, // 0x4b DEC ebx
    {false, 1}, // 0x4c DEC esp
    {false, 1}, // 0x4d DEC ebp
    {false, 1}, // 0x4e DEC esi
    {false, 1}, // 0x4f DEC edi
    {false, 1}, // 0x50 PUSH eax
    {false, 1}, // 0x51 PUSH ecx
    {false, 1}, // 0x52 PUSH edx
    {false, 1}, // 0x53 PUSH ebx
    {false, 1}, // 0x54 PUSH esp
    {false, 1}, // 0x55 PUSH ebp
    {false, 1}, // 0x56 PUSH esi
    {false, 1}, // 0x57 PUSH edi
    {false, 1}, // 0x58 POP eax
    {false, 1}, // 0x59 POP ecx
    {false, 1}, // 0x5a POP edx
    {false, 1}, // 0x5b POP ebx
    {false, 1}, // 0x5c POP esp
    {false, 1}, // 0x5d POP ebp
    {false, 1}, // 0x5e POP esi
    {false, 1}, // 0x5f POP edi
    {false, 0}, // 0x60
    {false, 0}, // 0x61
    {false, 0}, // 0x62
    {false, 0}, // 0x63
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
    {true , 3}, // 0x80 MOV
    {true , 6}, // 0x81 ADD
    {true , 3}, // 0x82
    {true , 3}, // 0x83
    {true , 2}, // 0x84 TEST_8
    {true , 2}, // 0x85 TEST
    {true , 2}, // 0x86 XCHG_8
    {true , 2}, // 0x87 XCHG
    {true , 2}, // 0x88 MOV_8
    {true , 2}, // 0x89 MOV
    {true , 2}, // 0x8a MOV_R8_B
    {true , 2}, // 0x8b MOV_R32_D
    {true , 2}, // 0x8c MOV_32_ES
    {true , 2}, // 0x8d LEA
    {true , 2}, // 0x8e MOV_ES_32
    {false, 2}, // 0x8f POP
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
    {false, 2}, // 0xb0 MOV_AL
    {false, 2}, // 0xb1 MOV_CL
    {false, 2}, // 0xb2 MOV_DL
    {false, 2}, // 0xb3 MOV_BL
    {false, 2}, // 0xb4 MOV_AH
    {false, 2}, // 0xb5 MOV_CH
    {false, 2}, // 0xb6 MOV_DH
    {false, 2}, // 0xb7 MOV_BH
    {false, 5}, // 0xb8 MOV_EAX
    {false, 5}, // 0xb9 MOV_ECX
    {false, 5}, // 0xba MOV_EDX
    {false, 5}, // 0xbb MOV_EBX
    {false, 5}, // 0xbc MOV_ESP
    {false, 5}, // 0xbd MOV_EBP
    {false, 5}, // 0xbe MOV_ESI
    {false, 5}, // 0xbf MOV_EDI
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
    {false, 0}, // 0xea LJMP
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
    {false, 6}, // 0xff EXTENDED
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
    INC_EAX = 0x40, INC_ECX, INC_EDX, INC_EBX, INC_ESP, INC_EBP, INC_ESI, INC_EDI,
    DEC_EAX, DEC_ECX, DEC_EDX, DEC_EBX, DEC_ESP, DEC_EBP, DEC_ESI, DEC_EDI,
    PUSH_EAX, PUSH_ECX, PUSH_EDX, PUSH_EBX, PUSH_ESP, PUSH_EBP, PUSH_ESI, PUSH_EDI,
    POP_EAX, POP_ECX, POP_EDX, POP_EBX, POP_ESP, POP_EBP, POP_ESI, POP_EDI,
    PUSH_D = 0x68, PUSH_B = 0x6a,

    R8_IMM8 = 0x80, R32_IMM32, R8_IMM8_2, R32_IMM8,
    TEST_8, TEST,
    XCHG_8, XCHG,
    MOV_8, MOV,
    MOV_8_B, MOV_32_D, MOV_32_ES,
    LEA,
    MOV_ES_32, POP,
    NOP,
    MOV_AL = 0xB0, MOV_CL, MOV_DL, MOV_BL, MOV_AH, MOV_CH, MOV_DH, MOV_BH,
    MOV_EAX, MOV_ECX, MOV_EDX, MOV_EBX, MOV_ESP, MOV_EBP, MOV_ESI, MOV_EDI,
    RETN_IMM16 = 0xC2, RET,
    LEAVE = 0xC9, RETF_IMM16, RETF, INTERRUPT, INT_IMM8, INTO, IRET,
    CALL = 0xE8, // 5 bytes don't process
    JMP, // 5 bytes don't process
    LJMP, // 7 bytes, don't process
    SHORT_JMP, // 2 bytes don't process
    EXTENDED = 0xFF,
};

const char* opcode_name(uint8_t opcode)
{
#define NAME(opcode_name) case opcode_name: return #opcode_name
    switch (opcode)
    {
        NAME(ADD_8_r8);
        NAME(INC_EAX); NAME(INC_ECX); NAME(INC_EDX); NAME(INC_EBX); NAME(INC_ESP); NAME(INC_EBP); NAME(INC_ESI); NAME(INC_EDI);
        NAME(DEC_EAX); NAME(DEC_ECX); NAME(DEC_EDX); NAME(DEC_EBX); NAME(DEC_ESP); NAME(DEC_EBP); NAME(DEC_ESI); NAME(DEC_EDI);
        NAME(PUSH_EAX); NAME(PUSH_ECX); NAME(PUSH_EDX); NAME(PUSH_EBX); NAME(PUSH_ESP); NAME(PUSH_EBP); NAME(PUSH_ESI); NAME(PUSH_EDI);
        NAME(POP_EAX); NAME(POP_ECX); NAME(POP_EDX); NAME(POP_EBX); NAME(POP_ESP); NAME(POP_EBP); NAME(POP_ESI); NAME(POP_EDI);
        NAME(R8_IMM8); NAME(R32_IMM32); NAME(R8_IMM8_2); NAME(R32_IMM8);
        NAME(TEST_8); NAME(TEST); NAME(XCHG_8); NAME(XCHG); NAME(MOV_8); NAME(MOV);
        NAME(MOV_8_B); NAME(MOV_32_D); NAME(MOV_32_ES);
        NAME(LEA);
        NAME(MOV_ES_32); NAME(POP);
        NAME(NOP);
        NAME(MOV_AL); NAME(MOV_CL); NAME(MOV_DL); NAME(MOV_BL); NAME(MOV_AH); NAME(MOV_CH); NAME(MOV_DH); NAME(MOV_BH);
        NAME(MOV_EAX); NAME(MOV_ECX); NAME(MOV_EDX); NAME(MOV_EBX); NAME(MOV_ESP); NAME(MOV_EBP); NAME(MOV_ESI); NAME(MOV_EDI);
        NAME(RETN_IMM16); NAME(RET); NAME(LEAVE); NAME(RETF_IMM16); NAME(RETF);
        NAME(INTERRUPT); NAME(INT_IMM8); NAME(INTO); NAME(CALL); NAME(JMP); NAME(LJMP); NAME(SHORT_JMP);
        NAME(EXTENDED);
    }
#undef NAME
    return "no registered";
}

#endif // MINI_DETOUR_X86_H
