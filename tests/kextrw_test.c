#include <libkextrw.h>

#include <mach-o/loader.h>
#include <stdint.h>
#include <stdio.h>

#define BCMP_26A434 0xFFFFFE000BA4370CULL
#define BCMP_PROLOGUE 0xD503245FU

int main(void)
{
    if (kextrw_init() == -1) {
        fprintf(stderr, "Failed to initialize KextRW\n");
        return 1;
    }

    const char *error = NULL;
    uint64_t left = 0;
    uint64_t right = 0;

    uint64_t kernelBase = get_kernel_base();
    struct mach_header_64 header = { 0 };
    if (!kernelBase ||
        kreadbuf(kernelBase, &header, sizeof(header)) != KERN_SUCCESS ||
        header.magic != MH_MAGIC_64 || header.filetype != MH_EXECUTE) {
        error = "kernel base read failed";
        goto out;
    }

    uint64_t bcmp = kslide(BCMP_26A434);
    if (kread32(bcmp) != BCMP_PROLOGUE) {
        error = "kernel call target does not match build 26A434";
        goto out;
    }

    left = kalloc(sizeof(uint64_t));
    right = kalloc(sizeof(uint64_t));
    if (!left || !right) {
        error = "kernel allocation failed";
        goto out;
    }

    uint64_t expected = 0x4B65787452573237ULL;
    uint64_t changed = expected ^ 1;
    uint64_t observed = 0;

    if (kwritebuf(left, &expected, sizeof(expected)) != KERN_SUCCESS ||
        kwritebuf(right, &expected, sizeof(expected)) != KERN_SUCCESS ||
        kreadbuf(left, &observed, sizeof(observed)) != KERN_SUCCESS ||
        observed != expected) {
        error = "virtual read/write failed";
        goto out;
    }

    uint64_t leftPhysical = kvtophys(left);
    uint64_t rightPhysical = kvtophys(right);
    if (!leftPhysical || !rightPhysical || physread64(leftPhysical) != expected) {
        error = "physical read failed";
        goto out;
    }

    physwrite64(rightPhysical, changed);
    if (kread64(right) != changed) {
        error = "physical read/write failed";
        goto out;
    }

    uint64_t args[] = { left, right, sizeof(expected) };
    if (kcall(bcmp, args, 3) == 0 ||
        kwritebuf(right, &expected, sizeof(expected)) != KERN_SUCCESS ||
        kcall(bcmp, args, 3) != 0) {
        error = "kernel call failed";
        goto out;
    }

    printf("Kernel base:  0x%016llx\n", kernelBase);
    printf("Kernel slide: 0x%016llx\n", gKernelSlide);
    printf("PASS: virtual and physical read/write, allocation, translation, and kernel call\n");

out:
    if (right) kfree(right, sizeof(uint64_t));
    if (left) kfree(left, sizeof(uint64_t));
    kextrw_deinit();

    if (error) {
        fprintf(stderr, "FAIL: %s\n", error);
        return 1;
    }
    return 0;
}
