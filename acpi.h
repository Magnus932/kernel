
/*
 * ACPI ( Advanced Configuration and Power Interface ) is a Power
 * management and configuration standard for the PC, developed by
 * Intel, Microsoft and Toshiba. ACPI allows the operating system to control
 * the amount of power each hardware device is given. It can also be used to
 * control or check thermal zones ( temperature sensors, fan speeds, etc.. )
 * and many other things.

 * There are two main parts to the ACPI interface. The first part is the tables
 * used by the OS for configuration during boot. The MADT ( Multiple APIC Description Table )
 * is a nice example, which you can read to figure out everything you need about
 * the interrupt controllers present in your system. The second part is the runtime ACPI
 * environment which consists of AML ( ACPI Machine Language ) bytecode.

 * There is not always ACPI is present on a system, the bios and chipset must co-op
 * in being compatible for it. To verify that ACPI is supported, the CPUID instruction
 * must be used.
 */
#include "types.h"

#define MAIN_BIOS_BASE				0x000E0000
#define MAIN_BIOS_END				0x000FFFFF
#define ACPI_VERSION_1				0x00
#define ACPI_VERSION_2				0x01

#define RSDP_SIGNATURE				"RSD PTR "
#define __ACPI_DEBUG__				0x00

struct rsdp_v1 {
	char signature[8];
	uint8_t checksum;
	char oemid[6];
	uint8_t revision;
	uint32_t rsdt_addr;
} __attribute__ ((packed));

struct rsdp_v2 {
	struct rsdp_v1 v1;
	uint32_t len;
	uint64_t xsdt_addr;
	uint8_t extended_csum;
	uint8_t reserved[3];
} __attribute__ ((packed));

struct acpi_sdthdr {
	char signature[4];
	uint32_t len;
	uint8_t revision;
	uint8_t checksum;
	char oemid[6];
	char oem_tbl_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
};

unsigned int locate_rsdp(void);
unsigned int acpi_version(void);
unsigned int do_rsdp_checksum_v1(void);
unsigned int do_rsdp_checksum_v2(void);
void init_rsdp(void);
unsigned int do_sdt_checksum(struct acpi_sdthdr *sdt);
void locate_rsdt(void);
void init_rsdt(void);
void setup_acpi_ptrs(void);
unsigned int find_acpi_table_32(unsigned int offset,
								const char *signature, int len);
unsigned long find_acpi_table_64(unsigned int offset,
								const char *signature, int len);
unsigned long find_acpi_table(const char *signature);

