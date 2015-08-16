
#include "madt.h"

struct madt_table *madt_base;

void init_madt(void)
{
	int i = 0, rec_len;
	struct madt_record *record;

	madt_base = (struct madt_table *)find_acpi_table("APIC");
	if (!madt_base) {
		printf("Error: madt table was not found.\n");
		return;
	}
	if (!do_sdt_checksum(&madt_base->hdr)) {
		printf("Error: madt checksum failed.\n");
		return;
	}
	record = &madt_base->base;
	rec_len = (madt_base->hdr.len - sizeof(struct acpi_sdthdr)) - 8;
	
	while (i < rec_len) {
		i += record->len;
		record->next = (unsigned int)record + record->len;
		record = record->next;
	}
	record->next = 0;
#ifdef __ACPI_DEBUG__
	printf("madt successfully added\n");
#endif
}

unsigned int dual_8259_installed(void)
{
	return madt_base->flags;
}

unsigned int find_num_processors(void)
{
	int num = 0;
	struct madt_record *record;

	record = &madt_base->base;
	while (record) {
		if (record->type == PROCESSOR_LOCAL_APIC)
			num++;
		record = record->next;
	}
	return num;
}

unsigned int find_num_io_apic(void)
{
	int num = 0;
	struct madt_record *record;

	record = &madt_base->base;
	while (record) {
		if (record->type == IO_APIC)
			num++;
		record = record->next;
	}
	return num;
}

unsigned int find_apic_base(void)
{
	return madt_base->ctrl_addr;
}

struct madt_record *find_record_base(void)
{
	return &madt_base->base;
}

struct madt_record *find_record(struct madt_record *record,
							    unsigned int flag)
{
	if (!record)
		return 0;

	while (record) {
		switch(flag) {
			case PROCESSOR_LOCAL_APIC:
				if (record->type == PROCESSOR_LOCAL_APIC)
					return record;
			break;
			case IO_APIC:
				if (record->type == IO_APIC)
					return record;
			break;
			case INTER_SRC_OVERRIDE:
				if (record->type == INTER_SRC_OVERRIDE)
					return record;
			break;
			case NON_MASK_IO_SOURCE:
				if (record->type == NON_MASK_IO_SOURCE)
					return record;
			break;
			case LOCAL_APIC_NMI:
				if (record->type == LOCAL_APIC_NMI)
					return record;
			break;
			case LOCAL_APIC_ADDR_OVERRIDE:
				if (record->type == LOCAL_APIC_ADDR_OVERRIDE)
					return record;
			break;
			case IO_SAPIC:
				if (record->type == IO_SAPIC)
					return record;
			break;
			case LOCAL_SAPIC:
				if (record->type == LOCAL_SAPIC)
					return record;
			break;
			case PLATFORM_IO_SRC:
				if (record->type == PLATFORM_IO_SRC)
					return record;
			break;
			case PROCESSOR_LOCAL_X2APIC:
				if (record->type == PROCESSOR_LOCAL_X2APIC)
					return record;
			break;
			case LOCAL_X2APIC_NMI:
				if (record->type == LOCAL_X2APIC_NMI)
					return record;
			break;
		}
		record = record->next;
	}
	return 0;
}