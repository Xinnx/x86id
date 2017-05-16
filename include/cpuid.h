#pragma once

#include <stdint.h>
#include <stdlib.h>

// change to 1 to enable a bit more output on the terminal
#define DEBUG_OUTPUT 1

// MinGW doesn't have uint?
typedef unsigned int uint;

struct cpufeat_eax {
  uint reserved1  : 4;    // 31:28
  uint ExtFamily  : 8;    // 27:20
  uint ExtModel   : 4;    // 19:16
  uint reserved2  : 2;    // 15:14
  uint CPUType    : 2;    // 13:12
  uint BaseFamily : 4;    // 11:8
  uint BaseModel  : 4;    // 7:4
  uint Stepping   : 4;    // 3:0
};

struct cpufeat_ebx {
  uint LocalApicId           : 8;           // 31:24
  uint LogicalProcessorCount : 8;           // 23:16
  uint CLFlush               : 8;           // 15:8
  uint eight_BitBrandId      : 8;           // 7:0
};

struct cpufeat_ecx {
  uint RAZ        : 1;        //Used by hypervisor to indicate guest status
  uint RDRAND     : 1;
  uint F16C       : 1;
  uint AVX        : 1;
  uint OSXSAVE    : 1;
  uint XSAVE      : 1;
  uint AES        : 1;
  uint reserved1  : 1;
  uint POPCNT     : 1;
  uint MOVEBE     : 1;
  uint reserved2  : 1;
  uint SSE42      : 1;
  uint SSE41      : 1;
  uint reserved3  : 5;
  uint CMPXCHG16B : 1;
  uint FMA        : 1;
  uint reserved4  : 2;
  uint SSSE3      : 1;
  uint reserved5  : 5;
  uint MONITOR    : 1;
  uint reserved6  : 1;
  uint PCLMULQDQ  : 1;
  uint SSE3       : 1;
};

struct cpufeat_edx {
  uint reserved1 : 3;
  uint HTT       : 1;
  uint reserved2 : 1;
  uint SSE2      : 1;
  uint SSE       : 1;
  uint FXSR      : 1;
  uint MMX       : 1;
  uint reserved3 : 3;
  uint CLFSH     : 1;
  uint reserved4 : 1;
  uint PSE36     : 1;
  uint PAT       : 1;
  uint CMOV      : 1;
  uint MCA       : 1;
  uint PGE       : 1;
  uint MTRR      : 1;
  uint SysEnterSysExit : 1;
  uint reserved5 : 1;
  uint APIC      : 1;
  uint CMPXCHG8B : 1;
  uint MCE       : 1;
  uint PAE       : 1;
  uint MSR       : 1;
  uint TSC       : 1;
  uint PSE       : 1;
  uint DE        : 1;
  uint VME       : 1;
  uint FPU       : 1;
};

/*
  These unions are to assist with assigning values to the bit fields
  this allows us to use the bitfield as a bitfield or a 32-bit uint
  HOWEVER, when first assigning the values of the fields you should
  assign them individually by using the structure directly NOT the
  integer value. ! IT IS PROVIDED FOR EASE OF USE ONLY !
*/

typedef union {
  struct cpufeat_eax b;
  uint32_t i;
} u_eax;

typedef union {
  struct cpufeat_ebx b;
  uint32_t i;
} u_ebx;

typedef union {
  struct cpufeat_ecx b;
  uint32_t i;
} u_ecx;

typedef union {
  struct cpufeat_edx b;
  uint32_t i;
} u_edx;

typedef union {
  uint64_t quad;
  uint32_t d_word[2];
  uint16_t word[4];
  uint8_t byte[8];
} u_register_64_t;

typedef union {
  uint32_t d_word;
  uint16_t word[2];
  uint8_t byte[4];
} u_register_32_t;

struct cpuinfo_s {
  // Stardard specifies a maximum length of 12 characters
  // cpuid does not add null terminator to string
  char cpuvend[13];
  char brandstring[48];
  uint32_t flevel;
  uint32_t ext_flevel;
  uint8_t model;
  uint8_t family;

  u_eax u_model_info;
  u_ebx u_misc_info;
  u_ecx u_feature1_info;
  u_edx u_feature2_info;
};

void getbrandstring(struct cpuinfo_s* cpuinfo);
void getbasicinfo(struct cpuinfo_s* cpuinfo);
void getcputype(struct cpuinfo_s* cpuinfo);
int checkcpuid(void);
uint32_t get_extended_leaf(void);
void getcpucache_leaf2();
void getcpucache_leaf4();
