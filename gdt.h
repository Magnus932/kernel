
#ifndef KERNEL_GDT_H
#define KERNEL_GDT_H

#define GDT_SIZE					0x08
#define GDT_ENTRY_NULL				0x00
#define GDT_ENTRY_DPL0_CS			0x01
#define GDT_ENTRY_DPL0_DS			0x02
#define GDT_ENTRY_DPL3_CS			0x03
#define GDT_ENTRY_DPL3_DS			0x04

#define SEGMENT_ACCESSED			0x01
#define SEGMENT_RW					0x02
#define SEGMENT_RDONLY				0x00
#define SEGMENT_EXECREAD			0x02
#define SEGMENT_EXECONLY			0x00
#define SEGMENT_BUSY				0x02
#define SEGMENT_N_BUSY				0x00
#define SEGMENT_EXP_DOWN			0x04
#define SEGMENT_EXP_UP				0x00
#define SEGMENT_CONFORMING			0x04
#define SEGMENT_N_CONFORMING		0x00
#define SEGMENT_CODE				0x08
#define SEGMENT_DATA				0x00
#define SEGMENT_SYSTEM				0x00
#define SEGMENT_CODE_DATA			0x10
#define SEGMENT_DPL0				(0 << 5)
#define SEGMENT_DPL1				(1 << 5)
#define SEGMENT_DPL2				(2 << 6)
#define SEGMENT_DPL3				(3 << 5)
#define SEGMENT_PRESENT				(1 << 7)
#define SEGMENT_N_PRESENT			0x00

#define SEGMENT_AVAILABLE			0x01
/* IA32M/IA32EM is used only in IA-32e mode */
#define SEGMENT_IA32M 				0x00
#define SEGMENT_IA32EM 				0x02
#define SEGMENT_SIZE32 				0x04
#define SEGMENT_SIZE16 				0x00
#define SEGMENT_P_BOUNDARY			0x08
#define SEGMENT_B_BOUNDARY			0x00

#define SEGMENT_DPL0_CS				(SEGMENT_EXECREAD | SEGMENT_N_CONFORMING | SEGMENT_CODE | \
									 SEGMENT_CODE_DATA | SEGMENT_DPL0 | SEGMENT_PRESENT)
#define SEGMENT_DPL0_DS				(SEGMENT_RW | SEGMENT_EXP_UP | SEGMENT_DATA | \
									 SEGMENT_CODE_DATA | SEGMENT_DPL0 | SEGMENT_PRESENT)

#define SEGMENT_DPL1_CS				(SEGMENT_EXECREAD | SEGMENT_N_CONFORMING | SEGMENT_CODE | \
									 SEGMENT_CODE_DATA | SEGMENT_DPL1 | SEGMENT_PRESENT)
#define SEGMENT_DPL1_DS				(SEGMENT_RW | SEGMENT_EXP_UP | SEGMENT_DATA | \
									 SEGMENT_CODE_DATA | SEGMENT_DPL1 | SEGMENT_PRESENT)

#define SEGMENT_DPL2_CS				(SEGMENT_EXECREAD | SEGMENT_N_CONFORMING | SEGMENT_CODE | \
									 SEGMENT_CODE_DATA | SEGMENT_DPL2 | SEGMENT_PRESENT)
#define SEGMENT_DPL2_DS				(SEGMENT_RW | SEGMENT_EXP_UP | SEGMENT_DATA | \
									 SEGMENT_CODE_DATA | SEGMENT_DPL2 | SEGMENT_PRESENT)

#define SEGMENT_DPL3_CS 			(SEGMENT_EXECREAD | SEGMENT_N_CONFORMING | SEGMENT_CODE | \
								     SEGMENT_CODE_DATA | SEGMENT_DPL3 | SEGMENT_PRESENT)
#define SEGMENT_DPL3_DS 			(SEGMENT_RW | SEGMENT_EXP_UP | SEGMENT_DATA | \
								     SEGMENT_CODE_DATA | SEGMENT_DPL3 | SEGMENT_PRESENT)

#define SEGMENT_TSS_DPL0			(SEGMENT_ACCESSED | SEGMENT_N_BUSY | SEGMENT_CODE | \
									 SEGMENT_DPL0 | SEGMENT_PRESENT)
#define SEGMENT_FLAG 				(SEGMENT_SIZE32 | SEGMENT_P_BOUNDARY)
#define SEGMENT_FLAG_TSS			(SEGMENT_B_BOUNDARY)

#define SELECTOR_RPL0				0x00
#define SELECTOR_RPL1				0x01
#define SELECTOR_RPL2				0x02
#define SELECTOR_RPL3				0x03
#define SELECTOR_GDT				0x00
#define SELECTOR_LDT				0x04
#define create_selector(index, ti, rpl) ((unsigned short)((index << 3) | ti | rpl))

struct gdt_descr {
	unsigned int limit_low:16;
	unsigned int base_low:16;
	unsigned int base_med:8;
	unsigned int access:8;
	unsigned int limit_high:4;
	unsigned int flags:4;
	unsigned int base_high:8;
};

struct gdt_descr gdt_base[GDT_SIZE] __attribute__ ((aligned(4096)));
struct gdt_ptr {
	unsigned short size;
	unsigned int offset;
} __attribute__ ((packed));

void add_gdt_entry(struct gdt_descr *desc, unsigned int base,
				   unsigned int limit, unsigned char access,
				   unsigned char flags);
void load_gdt(void);
void init_gdt(void);

/*
 * Conserning the P ( Segment-Present ) flag;
 * When this flag is clear, the operating system is free to
 * use the locations marked "Available" to store its own data in the descriptor,
 * such as information regarding the whereabouts of the missing segment.
 * 0..31 available.
 * 31..37 available.
 * 16..31 available.
 */

/* When the S ( descriptor type) flag in a segment descriptor is clear, the
 * descriptor type is a system descriptor. The processor recognizes the following
 * types of system descriptors:
 * 1. Local descriptor-table ( LDT ) segment descriptor.
 * 2. Task-state segment ( TSS ) descriptor.
 * 3. Call-gate descriptor.
 * 4. Interrupt-gate descriptor.
 * 5. Trap-gate descriptor.
 * 6. Task-gate descriptor.
 */
#endif
