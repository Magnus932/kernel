
/*
 * The MADT describes all of the interrupt controllers in the system.
 * It can be used to enumerate the processors currently available
 * if needed. The defines below defines all of the records
 * that a MADT table can contain.

 * The table needs to be referenced by use of the RSDP pointer, which
 * is found in the main bios area ( Below 1mb ).
 */
#include "types.h"
#include "acpi.h"

#define PROCESSOR_LOCAL_APIC				0x00
#define IO_APIC								0x01
#define INTER_SRC_OVERRIDE					0x02
#define NON_MASK_IO_SOURCE					0x03
#define LOCAL_APIC_NMI						0x04
#define LOCAL_APIC_ADDR_OVERRIDE			0x05
#define IO_SAPIC							0x06
#define LOCAL_SAPIC							0x07
#define PLATFORM_IO_SRC						0x08
#define PROCESSOR_LOCAL_X2APIC				0x09
#define LOCAL_X2APIC_NMI					0x0a

struct madt_0 {
	uint8_t acpi_processor_id;
	uint8_t apic_id;
	uint32_t flags;
} __attribute__ ((packed));

struct madt_1 {
	uint8_t io_apic_id;
	uint8_t res;
	uint32_t io_apic_addr;
	uint32_t global_sys_io_base;
} __attribute__ ((packed));

struct madt_2 {
	uint8_t bus;
	uint8_t source;
	uint32_t global_sys_io;
	uint16_t flags;
};

struct madt_3 {
	uint16_t flags;
	uint32_t global_sys_io;
} __attribute__ ((packed));

struct madt_4 {
	uint8_t acpi_processor_id;
	uint16_t flags;
	uint8_t local_apic_lint;
};

struct madt_5 {
	uint16_t res;
	uint64_t local_apic_addr;
} __attribute__ ((packed));

struct madt_6 {
	uint8_t io_apic_id;
	uint8_t res;
	uint32_t global_sys_io_base;
	uint64_t io_sapic_addr;
} __attribute__ ((packed));

struct madt_7 {
	uint8_t acpi_processor_id;
	uint8_t local_sapic_id;
	uint8_t local_sapic_eid;
	char res[3];
	uint32_t flags;
	uint32_t acpi_processor_uid_val;
	char acpi_processor_uid_str[48];
} __attribute__ ((packed));

struct madt_8 {
	uint16_t flags;
	uint8_t io_type;
	uint8_t processor_id;
	uint8_t processor_eid;
	uint8_t io_sapic_vector;
	uint32_t global_sys_io;
	uint32_t platform_io_src_flags;
} __attribute__ ((packed));

struct madt_9 {
	uint16_t res;
	uint32_t x2apic_id;
	uint32_t flags;
	uint32_t acpi_processor_uid;
} __attribute__ ((packed));

struct madt_10 {
	uint16_t flags;
	uint32_t acpi_processor_uid;
	uint8_t local_x2apic_lint;
	char res[3];
} __attribute__ ((packed));

struct madt_record {
	uint8_t type;
	uint8_t len;
	union {
		struct madt_0 local_apic;
		struct madt_1 io_apic;
		struct madt_2 io_override;
		struct madt_3 non_mask_io_src;
		struct madt_4 local_apic_nmi;
		struct madt_5 local_apic_addr_override;
		struct madt_6 io_sapic;
		struct madt_7 local_sapic;
		struct madt_8 platform_io_src;
		struct madt_9 local_x2apic;
		struct madt_10 local_x2apic_nmi;
	} s;
	struct madt_record *next;
};

struct madt_table {
	struct acpi_sdthdr hdr;
	uint32_t ctrl_addr;
	uint32_t flags;
	struct madt_record base;
};

void init_madt(void);
unsigned int dual_8259_installed(void);
unsigned int find_num_processors(void);
unsigned int find_num_io_apic(void);
struct madt_record *find_record_base(void);
struct madt_record *find_record(struct madt_record *record,
								unsigned int flag);