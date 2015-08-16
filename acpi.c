
#include "acpi.h"

unsigned int rsdp_base;

/*
 * The RSDP is either located within the first 1 KB of the EBDA
 * ( Extended Bios Data Area ), or in the memory region from
 * 0x000e0000 to 0x000FFFFF ( The Main Bios Area ).
 * Todo: Make it search through the EBDA if the signature is
 * not found in the MBA.
 */ 
unsigned int locate_rsdp(void)
{
	unsigned int main_bios_base = MAIN_BIOS_BASE;

	while (main_bios_base < MAIN_BIOS_END) {
		if (strncmp((unsigned int *)main_bios_base, RSDP_SIGNATURE, 8)) {
			rsdp_base = main_bios_base;
			return 1;
		}
		/* The RSDP signature is on a 16 byte boundary */
		main_bios_base += 16;
	}
	return 0;
}

unsigned int acpi_version(void)
{
	struct rsdp_v1 *r = (struct rsdp_v1 *)rsdp_base;
	
	return r->revision;
}

unsigned int do_rsdp_checksum_v1(void)
{
	int i;
	unsigned char sum = 0;
	struct rsdp_v1 *rsdp;

	rsdp = (struct rsdp_v1 *)rsdp_base;

	for (i = 0; i < sizeof(struct rsdp_v1); i++)
		sum += ((char *)rsdp)[i];

	return sum == 0;
}

unsigned int do_rsdp_checksum_v2(void)
{
	int i;
	unsigned char sum = 0;
	struct rsdp_v2 *rsdp;

	rsdp = (struct rsdp_v2 *)rsdp_base;

	for (i = 0; i < sizeof(struct rsdp_v2); i++)
		sum += ((char *)rsdp)[i];

	return sum == 0;
}

void init_rsdp(void)
{
	int revision;

	if (!locate_rsdp()) {
		printf("Error: failed to find rsdp\n");
		return;
	}
	revision = acpi_version();

	switch(revision) {
		case ACPI_VERSION_1:
			if (!do_rsdp_checksum_v1()) {
				printf("Error: rsdp 1.0 checksum failed.\n");
				return;
			}
		break;
		case ACPI_VERSION_2:
			if (!do_rsdp_checksum_v2()) {
				printf("Error: rsdp 2.0 checksum failed.\n");
				return;
			}
		break;
		default:
			printf("Error: ACPI version: %d not supported\n", revision);
			return;
		break;
	}
#ifdef __ACPI_DEBUG__
	printf("rsdp successfully added\n");
#endif
}

unsigned long rsdt_base;

unsigned int do_sdt_checksum(struct acpi_sdthdr *sdt)
{
	int i;
	unsigned char sum = 0;

	for (i = 0; i < sdt->len; i++)
		sum += ((char *)sdt)[i];

	return sum == 0;
}

void locate_rsdt(void)
{
	if (!acpi_version())
		rsdt_base = ((struct rsdp_v1 *)rsdp_base)->rsdt_addr;
	else
		rsdt_base = ((struct rsdp_v2 *)rsdp_base)->xsdt_addr;
}

void init_rsdt(void)
{
	struct acpi_sdthdr *rsdt;

	locate_rsdt();
	rsdt = (struct acpi_sdthdr *)rsdt_base;
	if (!do_sdt_checksum(rsdt)) {
		printf("Error: rsdt checksum failed.\n");
		return;
	}
#ifdef __ACPI_DEBUG__
	printf("rsdt successfully added\n");
#endif
}

void setup_acpi_ptrs(void)
{
	init_rsdp();
	init_rsdt();
}

unsigned int find_acpi_table_32(unsigned int offset,
							    const char *signature, int len)
{
	struct acpi_sdthdr *sdt;
	int i, num_entries;

	num_entries = (len - sizeof(struct acpi_sdthdr)) / 4;

	for (i = 0; i < num_entries; i++) {
		sdt = (struct acpi_sdthdr *)*(unsigned int *)offset;
		if (strncmp(sdt->signature, signature, 4))
			return offset;

		offset += 4;
	}
	return 0;
}

unsigned long find_acpi_table_64(unsigned int offset,
								 const char *signature, int len)
{
	struct acpi_sdthdr *sdt;
	int i, num_entries;

	num_entries = (len - sizeof(struct acpi_sdthdr)) / 8;

	for (i = 0; i < num_entries; i++) {
		sdt = (struct acpi_sdthdr *)*(unsigned int *)offset;
		if (strncmp(sdt->signature, signature, 4))
			return offset;

		offset += 8;
	}
	return 0;
}

unsigned long find_acpi_table(const char *signature)
{
	struct acpi_sdthdr *rsdt;
	unsigned int offset;
	unsigned int retval;

	rsdt = (struct acpi_sdthdr *)rsdt_base;
	offset = rsdt_base + sizeof(struct acpi_sdthdr);

	if (!acpi_version())
		retval = find_acpi_table_32(offset, signature, rsdt->len);
	else
		retval = find_acpi_table_64(offset, signature, rsdt->len);
	return *(unsigned long *)retval;
}
