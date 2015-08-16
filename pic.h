
#define MASTER_PIC_BASE			0x20
#define SLAVE_PIC_BASE			0xa0
#define OFFSET_ICW				0x00
#define OFFSET_OCW				0x01

#define MASTER_ICW				(MASTER_PIC_BASE + OFFSET_ICW)
#define MASTER_OCW				(MASTER_PIC_BASE + OFFSET_OCW)
#define SLAVE_ICW				(SLAVE_PIC_BASE + OFFSET_ICW)
#define SLAVE_OCW				(SLAVE_PIC_BASE + OFFSET_OCW)

/* ICW1 */
#define ICW4_NEEDED				0x01
#define NO_ICW4					0x00
#define SINGLE_MODE				0x02
#define CASCADE_MODE			0x00
#define INTERVAL_4				0x04
#define INTERVAL_8				0x00
#define LEVEL_TRIGGER			0x08
#define EDGE_TRIGGER			0x00
#define INIT_PIC				0x10

/* ICW3 Master device */
#define SLAVE_ON_IR0			0x01
#define SLAVE_ON_IR1			0x02
#define SLAVE_ON_IR2			0x04
#define SLAVE_ON_IR3			0x08
#define SLAVE_ON_IR4			0x10
#define SLAVE_ON_IR5			0x20
#define SLAVE_ON_IR6			0x40
#define SLAVE_ON_IR7			0x80

/* ICW3 Slave device */
#define I_AM_SLAVE0				0x00
#define I_AM_SLAVE1				0x01
#define I_AM_SLAVE2				0x02
#define I_AM_SLAVE3				0x03
#define I_AM_SLAVE4				0x04
#define I_AM_SLAVE5				0x05
#define I_AM_SLAVE6				0x06
#define I_AM_SLAVE7				0x07

/* ICW4 */
#define MODE_8086_88			0x01
#define MODE_MCS_80_85			0x00
#define AUTO_EOI				0x02
#define NORMAL_EOI				0x00
#define BUFF_MODE_SLAVE			0x04
#define BUFF_MODE_MASTER		0x00
#define BUFF_MODE 				0x08
#define NON_BUFF_MODE			0x00
#define SFNM_MODE				0x10
#define NO_SFNM_MODE			0x00

/* OCW1 */
#define IR_0					0x01
#define IR_1					0x02
#define IR_2					0x04
#define IR_3					0x08
#define IR_4 					0x10
#define IR_5					0x20
#define IR_6					0x40
#define IR_7					0x80
#define MASK_PIC				0xff

/* OCW2 */
#define NON_SPEC_EOI			0x20
#define SPEC_EOI 				0x60
#define ROT_NON_SPEC_EOI 		0x0a
#define ROT_AUTO_EOI_SET		0x80
#define ROT_AUTO_EOI_CLEAR		0x00
#define ROT_SPEC_EOI 			0xe0
#define SET_PRIORITY			0x0c
#define NO_OPERATION			0x40

/* OCW3 */
#define READ_IRR				0x0a
#define READ_ISR				0x0b
#define POLL_COMMAND			0x0c
#define RESET_S_MASK			0x48
#define SET_S_MASK				0x68

#define MASTER_VECTOR			0x20
#define SLAVE_VECTOR			0x28

#define MASTER_ICW1 			(ICW4_NEEDED | CASCADE_MODE | INTERVAL_8 | \
								 EDGE_TRIGGER | INIT_PIC)
#define MASTER_ICW3				(SLAVE_ON_IR2)
#define MASTER_ICW4				(MODE_8086_88 | NORMAL_EOI | NON_BUFF_MODE | \
							     NO_SFNM_MODE)

#define SLAVE_ICW1				(ICW4_NEEDED | CASCADE_MODE | INTERVAL_8 | \
								 EDGE_TRIGGER | INIT_PIC)
#define SLAVE_ICW3				(I_AM_SLAVE2)
#define SLAVE_ICW4				(MODE_8086_88 | NORMAL_EOI | NON_BUFF_MODE | \
								 NO_SFNM_MODE)

void init_pics(unsigned int master_vec, unsigned int slave_vec);
void pic_enable_irq(unsigned int irq);
void pic_disable_irq(unsigned int irq);
void pic_mask_master(void);
void pic_mask_slave(void);
short read_irr(void);
short read_isr(void);