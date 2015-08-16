
#include "gdt.h"
#include "registers.h"

void add_gdt_entry(struct gdt_descr *desc, unsigned int base,
				   unsigned int limit, unsigned char access,
				   unsigned char flags)
{
	desc->limit_low = limit & 0xffff;
	desc->base_low = base & 0xffff;
	desc->base_med = (base >> 16) & 0xff;
	desc->access = access;
	desc->limit_high = (limit >> 16) & 0xff;
	desc->flags = flags;
	desc->base_high = (base >> 24) & 0xff;
}

void load_gdt(void)
{
	struct gdt_ptr ptr;

	ptr.size = sizeof(gdt_base) - 1;
	ptr.offset = (unsigned int)gdt_base;
	LOAD_GDT(&ptr);
	
	__asm__ __volatile__ ("jmp %0, $load_selectors" : : 
						  "i" (create_selector(GDT_ENTRY_DPL0_CS, SELECTOR_GDT,
						  					   SELECTOR_RPL0)));
	__asm__ __volatile__ ("load_selectors:");
	SET_DS(create_selector(GDT_ENTRY_DPL0_DS, SELECTOR_GDT, SELECTOR_RPL0));
	SET_ES(create_selector(GDT_ENTRY_DPL0_DS, SELECTOR_GDT, SELECTOR_RPL0));   
	SET_SS(create_selector(GDT_ENTRY_DPL0_DS, SELECTOR_GDT, SELECTOR_RPL0));
	SET_FS(0);
	SET_GS(0);

	tss_flush();
}

void init_gdt(void)
{
	/*
	 * Initialize the code segment at ring level
	 * zero.
	 */
	add_gdt_entry(&gdt_base[GDT_ENTRY_DPL0_CS], 0x00, 0xfffff,
				  SEGMENT_DPL0_CS, SEGMENT_FLAG);
	/*
	 * Initialize the data segment at ring level
	 * zero.
	 */
	add_gdt_entry(&gdt_base[GDT_ENTRY_DPL0_DS], 0x00, 0xfffff,
				  SEGMENT_DPL0_DS, SEGMENT_FLAG);
	/*
	 * Initialize the code segment at ring level
	 * three.
	 */
	add_gdt_entry(&gdt_base[GDT_ENTRY_DPL3_CS], 0x00, 0xfffff,
				  SEGMENT_DPL3_CS, SEGMENT_FLAG);
	/*
	 * Initialize the data segment at ring level
	 * three.
	 */
	add_gdt_entry(&gdt_base[GDT_ENTRY_DPL3_DS], 0x00, 0xfffff,
				  SEGMENT_DPL3_DS, SEGMENT_FLAG);
	write_tss(5, 0x10, 0x00);

	add_gdt_entry(&gdt_base[0x06], 0x00, 0xfffff, SEGMENT_DPL1_CS, SEGMENT_FLAG);
	add_gdt_entry(&gdt_base[0x07], 0x00, 0xfffff, SEGMENT_DPL1_DS, SEGMENT_FLAG);
}