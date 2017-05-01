#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>


#include "../include/cpuid.h"

#if !(__x86_64__ || __i386__)
#error this program will only work on x86 processors
#endif


// calls cpuid leafs 0x8000000[2:4] to obtain the cpu name
// IE. "       Intel(R) Core(TM) i5-2540M CPU @ 2.60GHz\0"
// returns 4 characters in each register [eax,ebx,ecx,edx]
// for a total of 48 characters -- The string is null terminated
void
getbrandstring(struct cpuinfo_s *cpuinfo) {

    char eax_ret[4], ebx_ret[4], ecx_ret[4], edx_ret[4];
    char str[48] = {'\0'};
    //0x80000002, 0x80000003, 0x80000004 all need to be called
    for (uint32_t i = 0x80000002; i < 0x80000005; i++) {

        asm("xor %%ebx, %%ebx;"
                "xor %%ecx, %%ecx;"
                "xor %%edx, %%edx;"
                "cpuid;"
        : "=a"(eax_ret), "=b"(ebx_ret), "=c"(ecx_ret), "=d"(edx_ret)
        : "a"(i)
        :
        );

        // TODO: Make this into a util.h method
        // varargs of char* cstr's; returns -> char*
        strncat(str, eax_ret, 4);
        strncat(str, ebx_ret, 4);
        strncat(str, ecx_ret, 4);
        strncat(str, edx_ret, 4);
    }

    //
    // Strips leading spaces from vendor model string
    //
    char *curr = &str[0];
    size_t org_len = strlen(str);
    uint space_count = 0;
    while (!isalpha(*curr)) {
        curr++;
        space_count++;
    }
    size_t total_len = (org_len - space_count);

    memcpy(cpuinfo->brandstring, (str + space_count), total_len);
}

//
// Calls the 1st basic leaf using cpuid
// fills cpuinfo structure with base/ext model and 
// family as well as stepping and cputype
void
getbasicinfo(struct cpuinfo_s *cpuinfo) {

    uint32_t eax_ret, ebx_ret, ecx_ret, edx_ret;

    asm("mov $0x1, %%eax;"
            "xor %%ebx, %%ebx;"
            "xor %%ecx, %%ecx;"
            "xor %%edx, %%edx;"
            "cpuid;"
    : "=a"(eax_ret), "=b"(ebx_ret), "=c"(ecx_ret), "=d"(edx_ret) /* output */
    : /* input */
    : /* clobber */
    );

    cpuinfo->u_model_info.b.reserved1 = ((eax_ret & 0xF0000000) >> 28);
    cpuinfo->u_model_info.b.ExtFamily = ((eax_ret & 0x0FF00000) >> 20);
    cpuinfo->u_model_info.b.ExtModel = ((eax_ret & 0x000F0000) >> 16);
    cpuinfo->u_model_info.b.reserved2 = ((eax_ret & 0x0000C000) >> 14);
    cpuinfo->u_model_info.b.CPUType = ((eax_ret & 0x00003000) >> 12);
    cpuinfo->u_model_info.b.BaseFamily = ((eax_ret & 0x00000F00) >> 8);
    cpuinfo->u_model_info.b.BaseModel = ((eax_ret & 0x000000F0) >> 4);
    cpuinfo->u_model_info.b.Stepping = (eax_ret & 0x0000000F);


    if (cpuinfo->u_model_info.b.BaseFamily == 0xF) {
        //ExtFamily is reserved in this case.
        cpuinfo->family = (uint8_t) cpuinfo->u_model_info.b.BaseFamily;
    } else {
        cpuinfo->family = (uint8_t) (cpuinfo->u_model_info.b.BaseFamily +
                                     cpuinfo->u_model_info.b.ExtFamily);
    }


    // Model Calculation
    if (!(cpuinfo->u_model_info.b.BaseFamily == 0xF
          || cpuinfo->u_model_info.b.BaseFamily == 0x6)) {
        // ExtModel is reserved in this case
        cpuinfo->model = (uint8_t) cpuinfo->u_model_info.b.BaseModel;
    } else {
        cpuinfo->model =
                (uint8_t) (((cpuinfo->u_model_info.b.ExtModel) << 4) +
                           cpuinfo->u_model_info.b.BaseModel);
    }

    if (DEBUG_OUTPUT == 1) {
        printf("EAX: 0x%.8X\t EBX: 0x%.8X\n", eax_ret, ebx_ret);
        printf("ECX: 0x%.8X\t EDX: 0x%.8X\n\n", ecx_ret, edx_ret);
    }

}

// the 0h leaf of cpuid also returns the highest supported function
// in the eax register
void
getcputype(struct cpuinfo_s *cpuinfo) {
    // order of string is ebx, edx, ecx
    char vendor1[4], vendor2[4], vendor3[4];
    asm("xor %%eax, %%eax;"
            "cpuid;"
    : "=a"(cpuinfo->flevel), "=b"(vendor1), "=c"(vendor2), "=d"(vendor3) /* output */
    : /* input */
    : /* clobbered */
    );
    cpuinfo->ext_flevel = get_extended_leaf();

    // TODO: Another str cat funtion to move to util.h
    strncat(cpuinfo->cpuvend, vendor1, 4);
    strncat(cpuinfo->cpuvend, vendor3, 4);
    strncat(cpuinfo->cpuvend, vendor2, 4);
}

// returns non-zero if cpuid instruction is available
int
checkcpuid() {

    uint64_t rflags_updated, rflags_original;

    asm("pushf;"
            "pop %%rax;"
            "mov %%rax, %%rbx;"
            "xor $0x00200000, %%eax;"
            "push %%rax;"
            "popf;"
            "pushf;"
            "pop %%rax;"
    : "=a"(rflags_updated), "=b"(rflags_original) /* output */
    );

    // Some code inspection utilities will incorrectly report that these are always equal
    // because they cannot determine the state of these varibles from the above inline assembly
    if (rflags_updated == rflags_original) {
        // If we could not set the 21st bit of the rFLAGS register
        // cpuid is not supported so we return "false" here
        return 0;
    } else {
        return 1;
    }
}

uint32_t get_extended_leaf() {
    uint32_t ext_leaf_max = 0;

    asm("mov $0x80000000, %%eax;"
            "cpuid;"
    : "=a"(ext_leaf_max) /* Output */
    : /* Input */
    : "%ebx", "%ecx", "%edx" /* clobber */
    );

    return ext_leaf_max;
}

int
main() {

    if (checkcpuid()) {

        struct cpuinfo_s *cpuinfo = calloc(1,sizeof(struct cpuinfo_s));
        cpuinfo->flevel = 0x0;
        cpuinfo->ext_flevel = 0x0;
        cpuinfo->cpuvend[0] = '\0';
        strcpy(cpuinfo->brandstring, "UNK\0");

        printf("testing -> %s\n", cpuinfo->brandstring);

        getcputype(cpuinfo);
        getbasicinfo(cpuinfo);
        getbrandstring(cpuinfo);

        printf("\n   Highest basic leaf: 0x%.2X\n", cpuinfo->flevel);
        printf("Highest extended leaf: 0x%.8X\n", cpuinfo->ext_flevel);
        printf("     Processor vendor: %s\n", cpuinfo->cpuvend);
        printf("         Brand string: %s\n\n", cpuinfo->brandstring);
        printf(" -- Processor Information --\n");
        printf("     Base Model: 0x%.2X\n", cpuinfo->u_model_info.b.BaseModel);
        printf("    Base Family: 0x%.2X\n", cpuinfo->u_model_info.b.BaseFamily);
        printf(" Extended Model: 0x%.2X\n", cpuinfo->u_model_info.b.ExtModel);
        printf("Extended Family: 0x%.2X\n", cpuinfo->u_model_info.b.ExtFamily);
        printf(" Processor Type: 0x%.2X\n", cpuinfo->u_model_info.b.CPUType);
        printf("          Model: 0x%.2X\n", cpuinfo->model);
        printf("         Family: 0x%.2X\n", cpuinfo->family);
        printf("       Stepping: 0x%.2X\n", cpuinfo->u_model_info.b.Stepping);


        // TODO: the Misc and feature registers
        // TODO: Do a complete implementation of the Intel spec
        // TODO: After Intel spec is complete, Do AMD spec

        free(cpuinfo);
    } else {
        printf("CPUID instruction is not supported...exiting\n");
        exit(-1);
    }
    return 0;
}
