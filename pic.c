
#include "pic.h"

void init_pics(unsigned int master_vec, unsigned int slave_vec)
{
	outb(MASTER_ICW, MASTER_ICW1);
	outb(MASTER_OCW, master_vec);
	outb(MASTER_OCW, MASTER_ICW3);
	outb(MASTER_OCW, MASTER_ICW4);

	outb(SLAVE_ICW, SLAVE_ICW1);
	outb(SLAVE_OCW, slave_vec);
	outb(SLAVE_OCW, SLAVE_ICW3);
	outb(SLAVE_OCW, SLAVE_ICW4);

	outb(MASTER_ICW, NON_SPEC_EOI);
	outb(SLAVE_ICW, NON_SPEC_EOI);
}

void pic_enable_irq(unsigned int irq)
{
	if (irq < 8)
		outb(MASTER_OCW, inb(MASTER_OCW) &~ (1 << irq));
	else {
		irq -= 8;
		outb(SLAVE_OCW, inb(SLAVE_OCW) &~ (1 << irq));
	}
}

void pic_disable_irq(unsigned int irq)
{
	if (irq < 8)
		outb(MASTER_OCW, inb(MASTER_OCW) | (1 << irq));
	else {
		irq -= 8;
		outb(SLAVE_OCW, inb(SLAVE_OCW) | (1 << irq));
	}
}

void pic_mask_master(void)
{
	outb(MASTER_OCW, MASK_PIC);
}

void pic_mask_slave(void)
{
	outb(SLAVE_OCW, MASK_PIC);
}

short read_irr(void)
{
	outb(MASTER_ICW, READ_IRR);
	outb(SLAVE_ICW, READ_IRR);

	return (inb(SLAVE_ICW) << 8) | inb(MASTER_ICW);
}

short read_isr(void)
{
	outb(MASTER_ICW, READ_ISR);
	outb(SLAVE_ICW, READ_ISR);

	return (inb(SLAVE_ICW) << 8) | inb(MASTER_ICW);
}