#include "mini_detour.h"

#include <algorithm>
#include <list>
#include <cstdint>

#include <iostream>

#if defined(WIN64) || defined(_WIN64) || defined(__MINGW64__)
#define __WINDOWS_64__
#define __64BITS__
#elif defined(WIN32) || defined(_WIN32) || defined(__MINGW32__)
#define __WINDOWS_32__
#define __32BITS__
#endif

#if defined(__WINDOWS_32__) || defined(__WINDOWS_64__)
#define __WINDOWS__
#endif

#if defined(__linux__) || defined(linux)
#if defined(__x86_64__)
#define __LINUX_64__
#define __64BITS__
#else
#define __LINUX_32__
#define __32BITS__
#endif
#endif

#if defined(__LINUX_32__) || defined(__LINUX_64__)
#define __LINUX__
#endif

#if defined(__APPLE__)
#if defined(__x86_64__)
#define __APPLE_64__
#define __64BITS__
#else
#define __APPLE_32__
#define __32BITS__
#endif
#endif

#if defined(__WINDOWS__)

#elif defined(__LINUX__)
#include <sys/mman.h>
#include <unistd.h>

#elif defined(__APPLE__)
#include <mach/mach_init.h>
#include <mach/vm_map.h>

#endif

//------------------------------------------------------------------------------//
// Helper funcs
//------------------------------------------------------------------------------//
constexpr int addr_size = sizeof(void*);
constexpr int absolute_addr_size = addr_size;
constexpr int relative_addr_size = sizeof(int32_t);
#ifdef __64BITS__
constexpr int absolute_jmp_size = absolute_addr_size + 6; // FF 25 00 00 00 00       JMP [RIP+6]
                                                          // XX XX XX XX XX XX XX XX Address to jump to
#else
constexpr int absolute_jmp_size = absolute_addr_size + 2; // PUSH XX XX XX XX
                                                          // RET
#endif
constexpr int relative_jmp_size = relative_addr_size + 1; // E9 XX XX XX XX    JMP Relative Addr

#ifdef __64BITS__
#include "mini_detour_x64.h"

#else
#include "mini_detour_x86.h"

#endif

#pragma pack(push, 1)

struct trampoline_x86_t
{
    uint8_t trampolineBytes[14 + absolute_jmp_size]; // function opcodes + jmp to original function
    uint8_t originalBytes[14];       // original function opcodes
    uint8_t hookJump[absolute_jmp_size];             // jump to hook addr, needed because of relative jump overflow
    uint8_t nOriginalBytes;          // number of original function bytes bkp
    uint8_t* originalAddr;           // Where to restore bytes
};

struct trampoline_x64_t
{
    uint8_t trampolineBytes[20 + absolute_jmp_size]; // function opcodes + jmp to original function
    uint8_t originalBytes[20];        // original function opcodes
    uint8_t hookJump[absolute_jmp_size];             // jump to hook addr, needed because of relative jump overflow
    uint8_t nOriginalBytes;           // number of original function bytes bkp
    uint8_t* originalAddr;            // Where to restore bytes
};

#if defined(__32BITS__)
typedef trampoline_x86_t trampoline_t;
#else
typedef trampoline_x64_t trampoline_t;
#endif

struct trampoline_region_t
{
    uint8_t numTrampolines; // current trampolines allocated
    trampoline_t* trampolines_start; // start pointer of current region trampolines
    trampoline_t* next_free_trampoline; // next free trampoline in region
};

#pragma pack(pop)

struct transaction_t
{
    bool attach;
    void** ppOriginalFunc;
    trampoline_t* trampoline;
};

static std::list<trampoline_region_t> trampoline_regions;

static bool transaction_started = false;
static std::list<transaction_t> cur_transaction;

#if defined(__LINUX__)
enum mem_protect_rights
{
    mem_r = PROT_READ,
    mem_w = PROT_WRITE,
    mem_x = PROT_EXEC,
    mem_rw = PROT_WRITE | PROT_READ,
    mem_rx = PROT_WRITE | PROT_EXEC,
    mem_rwx = PROT_WRITE | PROT_READ | PROT_EXEC,
};

inline size_t page_size()
{
    static size_t _page_size = 0;
    if (_page_size == 0)
    {
        _page_size = sysconf(_SC_PAGESIZE);
    }
    return _page_size;
}

inline bool mem_protect(void* addr, size_t size, size_t rights)
{
    return mprotect(addr, size, rights) == 0;
}

inline void* memory_alloc(void* address_hint, size_t size, mem_protect_rights rights)
{
    // TODO: Here find a way to allocate moemry near the address_hint.
    // Sometimes you get address too far for a relative jmp
    return mmap(address_hint, size, rights, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

inline void memory_free(void* mem_addr, size_t size)
{
    munmap(mem_addr, size);
}

int flush_instruction_cache(void* pBase, size_t size)
{
    return 1;
}

#elif defined(__WINDOWS__)
enum mem_protect_rights
{
    mem_r = PAGE_READONLY,
    mem_w = PAGE_READWRITE,
    mem_x = PAGE_EXECUTE,
    mem_rw = PAGE_READWRITE,
    mem_rx = PAGE_EXECUTE_READ,
    mem_rwx = PAGE_EXECUTE_READWRITE,
};

inline size_t page_size()
{
    static size_t _page_size = 0;
    if (_page_size == 0)
    {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        _page_size = sysInfo.dwPageSize;
    }
    return _page_size;
}

inline bool mem_protect(void* addr, size_t size, size_t rights)
{
    DWORD oldProtect;
    return VirtualProtect(addr, size, rights, &oldProtect) != FALSE;
}

inline void* memory_alloc(void* address_hint, size_t size, mem_protect_rights rights)
{
    MEMORY_BASIC_INFORMATION mbi;
    ZeroMemory(&mbi, sizeof(mbi));

    HANDLE hProcess = GetCurrentProcess();

    PBYTE pbBase = (PBYTE)address_hint;
    PBYTE pbLast = pbBase;
    for (;; pbLast = (PBYTE)mbi.BaseAddress + mbi.RegionSize) {

        ZeroMemory(&mbi, sizeof(mbi));
        if (VirtualQueryEx(hProcess, (PVOID)pbLast, &mbi, sizeof(mbi)) == 0) {
            if (GetLastError() == ERROR_INVALID_PARAMETER) {
                break;
            }
            break;
        }
        // Usermode address space has such an unaligned region size always at the
        // end and only at the end.
        //
        if ((mbi.RegionSize & 0xfff) == 0xfff) {
            break;
        }

        // Skip anything other than a pure free region.
        //
        if (mbi.State != MEM_FREE) {
            continue;
        }

        // Use the max of mbi.BaseAddress and pbBase, in case mbi.BaseAddress < pbBase.
        PBYTE pbAddress = (PBYTE)mbi.BaseAddress > pbBase ? (PBYTE)mbi.BaseAddress : pbBase;

        // Round pbAddress up to the nearest MM allocation boundary.
        const DWORD_PTR mmGranularityMinusOne = (DWORD_PTR)(0x10000 - 1);
        pbAddress = (PBYTE)(((DWORD_PTR)pbAddress + mmGranularityMinusOne) & ~mmGranularityMinusOne);

        for (; pbAddress < (PBYTE)mbi.BaseAddress + mbi.RegionSize; pbAddress += 0x10000) {
            PBYTE pbAlloc = (PBYTE)VirtualAllocEx(hProcess, pbAddress, size,
                MEM_RESERVE | MEM_COMMIT, rights);
            if (pbAlloc == nullptr) {
                continue;
            }
            return pbAlloc;
        }
    }

    return nullptr;
}

void memory_free(void* mem_addr, size_t size)
{
    VirtualFree(mem_addr, 0, MEM_RELEASE);
}

int flush_instruction_cache(void* pBase, size_t size)
{
    return FlushInstructionCache(GetCurrentProcess(), pBase, size);
}

#elif defined(__APPLE__)
enum mem_protect_rights
{
    mem_r = PROT_READ,
    mem_w = PROT_WRITE,
    mem_x = PROT_EXEC,
    mem_rw = PROT_WRITE | PROT_READ,
    mem_rx = PROT_WRITE | PROT_EXEC,
    mem_rwx = PROT_WRITE | PROT_READ | PROT_EXEC,
};

inline size_t page_size()
{
    static size_t _page_size = 0;
    if (_page_size == 0)
    {
        _page_size = sysconf(_SC_PAGESIZE);
    }
    return _page_size;
}

inline bool mem_protect(void* addr, size_t size, size_t rights)
{
    return mprotect(addr, size, rights) == 0;
}

inline void* memory_alloc(void* address_hint, size_t size, mem_protect_rights rights)
{
    //vm_address_t address = (vm_address_t)address_hint;
    //vm_offset_t byteSize = (vm_offset_t)size;

    //kern_return_t result = vm_allocate((vm_map_t)mach_task_self(),
    //                                    &address,
    //                                    byteSize,
    //                                    VM_FLAGS_FIXED);

#ifdef __64BITS__
    return mmap(address_hint, size, rights, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#else
    return mmap(address_hint, size, rights, MAP_PRIVATE | MAP_32BIT | MAP_ANONYMOUS, -1, 0);
#endif
}

inline void memory_free(void* mem_addr, size_t size)
{
    munmap(mem_addr, size);
    //kern_return_t result = vm_deallocate((vm_map_t)mach_task_self(),
    //                                    &address,
    //
}

int flush_instruction_cache(void* pBase, size_t size)
{
    return 1;
}

#endif

inline size_t region_size()
{
    return page_size();
}

static uint8_t max_trampolines_in_region = region_size() / sizeof(trampoline_t);

inline void* library_address_by_handle(void* library)
{
    return (library == nullptr ? nullptr : *reinterpret_cast<void**>(library));
}

inline size_t page_align(size_t size, size_t page_size)
{
    return (size + (page_size - 1)) & (((size_t)-1) ^ (page_size - 1));
}

inline void* page_addr(void* addr, size_t page_size)
{
    return reinterpret_cast<void*>(reinterpret_cast<size_t>(addr)& (((size_t)-1) ^ (page_size - 1)));
}

inline uint8_t* relative_addr_to_absolute(int32_t rel_addr, uint8_t* code_addr)
{
    return code_addr + rel_addr + 5;
}

////////////////////////////////////////////////////
/// Tiny disasm

bool is_opcode_terminating_function(uint8_t opcode)
{
    switch (opcode)
    {
        case LEAVE: case RET: case RETN_IMM16:
        case RETF: case RETF_IMM16: case INTERRUPT:
        case INT_IMM8: case INTO: case IRET:
            return true;
    }
    return false;
}

bool is_opcode_filler(uint8_t opcode)
{
    switch (opcode)
    {
        case NOP:
            return true;
    }
    return false;
}

bool read_mod_reg_rm_opcode(uint8_t** ppCode)
{
    uint8_t* pCode = *ppCode;
    // MOD-REG-R/M Byte
    //  7 6    5 4 3    2 1 0 - bits
    //[ MOD ][  REG  ][  R/M  ]
    switch (pCode[1] & mod_mask) // Check MOD to know how many bytes we have after this opcode
    {
        case register_addressing_mode      : *ppCode += s_opcodes[*pCode].base_size; break; // register addressing mode [opcode] [R/M] [XX]
        case four_bytes_signed_displacement:
            return false; // There is a relative displacement, can't move this opcode
            //switch (pCode[1] & rm_mask)
            //{
            //    case sib_with_no_displacement: *ppCode += s_opcodes[*pCode].base_size + 5; break; // address mode byte + 4 bytes displacement
            //    default                      : *ppCode += s_opcodes[*pCode].base_size + 4; break; // 4 bytes displacement
            //}
            //break;

        case one_byte_signed_displacement:
            return false; // There is a displacement, can't move this opcode
            //switch (pCode[1] & rm_mask)
            //{
            //    case sib_with_no_displacement: *ppCode += s_opcodes[*pCode].base_size + 2; break; // address mode byte + 1 byte displacement
            //    default: *ppCode += s_opcodes[*pCode].base_size + 1; break; // 1 byte displacement
            //}
            //break;

        default:
            switch (pCode[1] & rm_mask)
            {
                case displacement_only_addressing:
                    return false; // There is a displacement, can't move this opcode
                    //*ppCode += s_opcodes[*pCode].base_size + 4; break; // 4 bytes Displacement only addressing mode
                case sib_with_no_displacement         : *ppCode += s_opcodes[*pCode].base_size + 1; break; // SIB with no displacement
                case register_indirect_addressing_mode: // Register indirect addressing mode
                default: *ppCode += s_opcodes[*pCode].base_size;
            }
    }
    return true;
}

int find_space_for_trampoline(uint8_t** func, int bytes_needed, bool ignore_jump)
{
    if (func == nullptr)
        return -1;

    int code_len = 0;
    bool search = true;
    uint8_t* pCode = *func;
    while (search) // Find opcodes size and try to find at least 5 bytes for our JMP
    {
        const char* name = opcode_name(*pCode);
        printf("%s %x\n", name, (int)*pCode);

        if (is_opcode_terminating_function(*pCode))
            break;

        if (s_opcodes[*pCode].has_r_m)
        {
            search = read_mod_reg_rm_opcode(&pCode);
        }
        else if (s_opcodes[*pCode].base_size)
        {
            switch (*pCode)
            {
#ifdef __64BITS__
            case REX: case REX_B: case REX_X: case REX_XB: case REX_R: case REX_RB: case REX_RX: case REX_RXB:
            case REX_W: case REX_WB: case REX_WX: case REX_WXB: case REX_WR: case REX_WRB: case REX_WRX: case REX_WRXB:
                pCode += s_opcodes[*pCode].base_size;
                continue; // REX works only with the next opcode, don't stop searching after a REX
#endif
            case JMP:
            case CALL:
                // JMP or CALL is forbidden for trampolines

                if (pCode == *func)
                {// If jmp is the first opcode, we can override it
                    pCode += s_opcodes[*pCode].base_size;
                }
                else
                {
                    if (ignore_jump)
                    {
                        pCode += s_opcodes[*pCode].base_size;
                    }
                }

            case LJMP: case SHORT_JMP:
                search = false;
                break;

            default:
                pCode += s_opcodes[*pCode].base_size;
            }
        }
        else
        {
            switch (*pCode)
            {
                //case EXTENDED:
                //    //std::cerr << "IMPORT_JUMP is not handled" << std::endl;
                //    if (pCode[1] == 0x25) // This is an imported function
                //    { // Get the true function call
                //        //pCode = (uint8_t*)*(pCode+2);
                //        //startCode = pCode;
                //        // For now disable this case
                //        if (!ignore_jump)
                //            search = false;
                //    }
                //    else
                //    {
                //        if (!ignore_jump)
                //            search = false;
                //    }
                //    break;

            default:
                //PRINT_DEBUG("Unknown opcode %02X\n", (int)*pCode);
                search = false;
            }
        }

        while (is_opcode_filler(*pCode))
        {// Eat filler opcodes like NOP
            ++pCode;
        }

        if ((pCode - *func) >= bytes_needed && search)
        {
            search = false;
        }
    }

    return pCode - *func;
}

///////////////////////////////////////////
// Tiny asm

inline uint8_t* gen_relative_jmp(uint8_t* opcode_addr, uint8_t* dest)
{
    *opcode_addr++ = JMP;
    *reinterpret_cast<int32_t*>(opcode_addr) = (int32_t)(dest - (opcode_addr + relative_addr_size));
    return opcode_addr + addr_size;
}

#ifdef __64BITS__
inline uint8_t* gen_absolute_jmp(uint8_t* opcode_addr, uint8_t* dest)
{
    *opcode_addr++ = 0xFF; // JMP
    *opcode_addr++ = 0x25; // RIP+6
    *opcode_addr++ = 0x00; //
    *opcode_addr++ = 0x00; //
    *opcode_addr++ = 0x00; //
    *opcode_addr++ = 0x00; //
    *reinterpret_cast<void**>(opcode_addr) = (void*)dest;
    return opcode_addr + addr_size;
}

#else

inline uint8_t* gen_absolute_jmp(uint8_t* opcode_addr, uint8_t* dest)
{
    *opcode_addr++ = PUSH_D; // PUSH DWORD
    *reinterpret_cast<void**>(opcode_addr) = (void*)dest; // XX XX XX XX
    opcode_addr += addr_size;
    *opcode_addr++ = RET;  // RET (uses the last stack value)
    return opcode_addr;
}

#endif

trampoline_t* alloc_new_trampoline_region(void* hint_addr, bool limit_to_2gb)
{
    trampoline_region_t region;
    trampoline_t* trampoline = nullptr;
    uint8_t* addr = (uint8_t*)hint_addr;

    for (int i = 0; i < 100; ++i)
    {
        trampoline = reinterpret_cast<trampoline_t*>(memory_alloc(addr, region_size(), mem_protect_rights::mem_rwx));
        if (!limit_to_2gb || std::abs((int64_t)trampoline - (int64_t)addr) <= 0x7FFFFFFF)
            break;

        addr += 0x100000;
        memory_free(trampoline, region_size());
        trampoline = nullptr;
    }
    if (trampoline)
    {
        region.numTrampolines = 0;
        // allocate new trampoline right in the middle of memory so relative jump can access any function
        region.trampolines_start = trampoline;
        // Fill the region with 0
        std::fill(reinterpret_cast<uint8_t*>(region.trampolines_start), reinterpret_cast<uint8_t*>(region.trampolines_start) + region_size(), 0);
        region.next_free_trampoline = region.trampolines_start;
        // Protect trampoline region memory
        mem_protect(region.trampolines_start, region_size(), mem_protect_rights::mem_rx);

        trampoline_regions.push_back(region);
    }

    return trampoline;
}

trampoline_t* get_free_trampoline(void* originalFuncAddr, bool limit_to_2gb)
{
    if (!transaction_started)
        return nullptr;

    trampoline_t* res = nullptr;
    auto it = std::find_if(trampoline_regions.begin(), trampoline_regions.end(), [originalFuncAddr](trampoline_region_t& region) {
        std::cout << "Testing trampoline %p" << region.trampolines_start << std::endl;
        if (region.numTrampolines == max_trampolines_in_region || // If the trampoline region is full
            std::abs((int64_t)region.trampolines_start - (int64_t)originalFuncAddr) > 0x7FFFFFFFul) // Or the trampoline address isn't in the relative jmp range (max-int32_t)
            return false; // Don't select it
        return true; // We have a free trampoline to use
        });

    if (it == trampoline_regions.end())
    {
        if (alloc_new_trampoline_region(originalFuncAddr, limit_to_2gb) == nullptr)
            return nullptr;

        it = --trampoline_regions.end();
    }
    res = it->next_free_trampoline;

    trampoline_t* next_new_trampoline = res + 1;
    if (it->numTrampolines != max_trampolines_in_region)
    {
        while (next_new_trampoline->nOriginalBytes != 0)
        {
            ++next_new_trampoline;
        }
    }
    else
    {
        next_new_trampoline = nullptr;
    }
    it->next_free_trampoline = next_new_trampoline;

    ++it->numTrampolines;

    return res;
}

void clear_trampoline(trampoline_region_t& region, trampoline_t* trampoline)
{
    --region.numTrampolines;

    std::fill(reinterpret_cast<uint8_t*>(trampoline), reinterpret_cast<uint8_t*>(trampoline + 1), 0);
    if (region.next_free_trampoline == nullptr || region.next_free_trampoline > trampoline)
        region.next_free_trampoline = trampoline;
}

inline bool is_page_inside_region(void* page, trampoline_region_t& region)
{
    if (page >= region.trampolines_start && page <= (region.trampolines_start + region_size()))
        return true;
    return false;
}

//------------------------------------------------------------------------------//

int mini_detour::update_thread(threadid_t thread_id)
{
    return 0;
}

int mini_detour::transaction_begin()
{
    if (transaction_started)
        return -1;

    transaction_started = true;

    return 0;
}

int mini_detour::transaction_abort()
{
    if (!transaction_started)
        return -1;

    for (auto& i : cur_transaction)
    {
        trampoline_t* trampoline = i.trampoline;
        void* page_start = page_addr(reinterpret_cast<void*>(trampoline), page_size());
        auto it = std::find_if(trampoline_regions.begin(), trampoline_regions.end(), [page_start](trampoline_region_t& region) {
            if (is_page_inside_region(page_start, region))
                return true;
            return false;
            });
        if (it != trampoline_regions.end())
        {
            clear_trampoline(*it, trampoline);
        }
    }
    cur_transaction.clear();
    transaction_started = false;

    return 0;
}

int mini_detour::transaction_commit()
{
    if (!transaction_started)
        return -1;

    for (auto& i : cur_transaction)
    {
        void** ppOriginalFunc = i.ppOriginalFunc;
        trampoline_t* trampoline = i.trampoline;
        int res;

        if (i.attach)
        {
            void* originalFunctionPage = page_addr(*ppOriginalFunc, page_size());

            // Allow write on the original func
            res = mem_protect(originalFunctionPage, page_size() * 2, mem_protect_rights::mem_rwx);

            // Write a  jump to the trampoline relative or absolute address
            if (trampoline->nOriginalBytes >= absolute_jmp_size)
            {
                std::cout << "Rewritting with an absolute jump" << std::endl;
                // We've got place for an absolute jump
                gen_absolute_jmp(reinterpret_cast<uint8_t*>(*ppOriginalFunc), trampoline->hookJump);
            }
            else
            {
                std::cout << "Rewritting with a relative jump" << std::endl;
                // The trampoline jump should be next to the original function for a relative jump
                gen_relative_jmp(reinterpret_cast<uint8_t*>(*ppOriginalFunc), trampoline->hookJump);
            }

            // Remove write permission
            res = mem_protect(originalFunctionPage, page_size() * 2, mem_protect_rights::mem_rx);

            *ppOriginalFunc = (void*)(trampoline->trampolineBytes);
        }
        else
        {
            void* trampoline_page = page_addr(reinterpret_cast<void*>(trampoline), page_size());

            *ppOriginalFunc = trampoline->originalAddr;

            void* originalFunctionPage = page_addr(*ppOriginalFunc, page_size());

            // Allow write on the original func
            res = mem_protect(originalFunctionPage, page_size() * 2, mem_protect_rights::mem_rwx);

            // Write the original opcodes
            std::copy(trampoline->originalBytes, trampoline->originalBytes + trampoline->nOriginalBytes,
                reinterpret_cast<uint8_t*>(*ppOriginalFunc));

            // Remove write permission
            res = mem_protect(originalFunctionPage, page_size() * 2, mem_protect_rights::mem_rx);

            // Allow write on trampoline page
            res = mem_protect(trampoline_page, page_size(), mem_protect_rights::mem_rwx);

            clear_trampoline(*reinterpret_cast<trampoline_region_t*>(trampoline_page), trampoline);

            // Remove write permission
            res = mem_protect(trampoline_page, page_size(), mem_protect_rights::mem_rx);
        }
    }
    cur_transaction.clear();
    transaction_started = false;

    return 0;
}

int mini_detour::unhook_func(void** ppOriginalFunc, void* _hook)
{
    if (!transaction_started)
        return -EPERM;

    if (ppOriginalFunc == nullptr || _hook == nullptr || *ppOriginalFunc == nullptr)
        return -EINVAL;

    trampoline_t* trampoline = reinterpret_cast<trampoline_t*>(*ppOriginalFunc);
    void* page_start = page_addr(reinterpret_cast<void*>(trampoline), page_size());
    auto it = std::find_if(trampoline_regions.begin(), trampoline_regions.end(), [page_start](trampoline_region_t& region) {
        if (is_page_inside_region(page_start, region))
            return true;
        return false;
        });
    if (it != trampoline_regions.end())
    {
        cur_transaction.push_back({ false, ppOriginalFunc, trampoline });
    }

    return -EINVAL;
}

int hook_func(void** ppOriginalFunc, void* _hook, bool replace)
{
    if (!transaction_started)
        return -EPERM;

    if (ppOriginalFunc == nullptr || _hook == nullptr || *ppOriginalFunc == nullptr)
        return -EINVAL;

    uint8_t* hook = reinterpret_cast<uint8_t*>(_hook);
    // Try to find enought place for absolute jump
    int code_len = find_space_for_trampoline((uint8_t**)ppOriginalFunc, absolute_jmp_size, replace);
    uint8_t* pOriginalFunc = (uint8_t*)*ppOriginalFunc;

    // If we got less space than we need for a relative jump, the function is too short anyway.
    if (code_len < relative_jmp_size)
        return -ENOSPC;

    // Allocate the trampoline or get the next free trampoline
    trampoline_t* trampoline = get_free_trampoline(*ppOriginalFunc, code_len < absolute_jmp_size);
    if (trampoline == nullptr)
        return -EFAULT;

    uint8_t* pTrampolineCode = trampoline->trampolineBytes;

    void* trampoline_page = page_addr(trampoline, page_size());

    // Enable write to the trampoline region
    mem_protect(trampoline_page, page_size(), mem_protect_rights::mem_rw);
    // Create jmp to hook
    //gen_relative_jmp(trampoline->hookJump, hook);
    gen_absolute_jmp(trampoline->hookJump, hook);
    // Copy original opcodes
    trampoline->nOriginalBytes = code_len;
    std::copy(pOriginalFunc, pOriginalFunc + code_len, trampoline->originalBytes);
    if (*pOriginalFunc == JMP)
    {// The first opcode is a jmp, so compute the absolute address and make an absolute jmp instead
        uint8_t* func_abs_addr = relative_addr_to_absolute(*(int32_t*)(pOriginalFunc + 1), pOriginalFunc);
        gen_absolute_jmp(pTrampolineCode, func_abs_addr);
    }
    else
    {
        std::copy(trampoline->originalBytes, trampoline->originalBytes + code_len, pTrampolineCode);
        pTrampolineCode += code_len;
        // Create the absolute jmp to original
        pTrampolineCode = gen_absolute_jmp(pTrampolineCode, pOriginalFunc + code_len);
    }

    trampoline->originalAddr = pOriginalFunc;

    // Disable trampoline region write
    mem_protect(trampoline_page, page_size(), mem_protect_rights::mem_rx);

    cur_transaction.push_back({ true, ppOriginalFunc, trampoline });

    return 0;
}

int mini_detour::detour_func(void** ppOriginalFunc, void* _hook)
{
    return hook_func(ppOriginalFunc, _hook, false);
}

int mini_detour::replace_func(void* pOriginalFunc, void* _hook)
{
    return hook_func(&pOriginalFunc, _hook, true);
}

/* ------ DOCUMENTATION ------
http://www.c-jump.com/CIS77/CPU/x86/lecture.html                <- some help to understand [MOD][REG][R/M] (see paragraph #6)
http://shell-storm.org/online/Online-Assembler-and-Disassembler <- online assembler
http://ref.x86asm.net/coder32.html                              <- opcodes reference

X86

push ebx     : 0x53
sub  esp ??  : 0x83 0xEC 0x??
call ????????: 0xE8 0x?? 0x?? 0x?? 0x??


// relative jmp: ???????? = dst_addr - curr_addr - 5
jmp ???????? : 0xe9 0x?? 0x?? 0x?? 0x??
destination = 0x8dba8
jmp location: 0x91995 - opcodes: e9 0e c2 ff ff
0e c2 ff ff = 0x8dba8 - 0x91995 - 5

// short jmp: ?? = dst_addr - curr_addr - 2
jmp short ??: 0xeb 0x??
destination = 0x91964
jmp location: 0x9198f - opcodes: 0xeb 0xd3
d3 = 0x91964 - 0x9198f - 2

X64
Reuse x86 relative jmp method to jmp to trampoline
From trampoline make an absolute jmp

Example:
Trampoline Addr 0x20000:
FuncToHook 0x10000:

FuncToHook Code:
0x90 NOP
0x90 NOP
0x90 NOP
0x90 NOP
0x90 NOP

Hook The Func:
FuncToHook Code:
0xE9 JMP
0xFB Relative Hook Addr
0xFF Relative Hook Addr
0x00 Relative Hook Addr
0x00 Relative Hook Addr


*/
