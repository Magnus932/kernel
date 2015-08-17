
#include <types.h>

uint8_t inb(uint16_t port)
{
	uint8_t retval;

	__asm__ __volatile__ ("inb %1, %0" : "=a" (retval) : "d" (port));
	return retval;
}

void outb(uint16_t port, uint8_t data)
{
	__asm__ __volatile__ ("outb %1, %0" : : "d" (port), "a" (data));
}

uint16_t inw(uint16_t port)
{
	uint16_t retval;

	__asm__ __volatile__ ("inw %1, %0" : "=a" (retval) : "d" (port));
}

void outw(uint16_t port, uint16_t data)
{
	__asm__ __volatile__ ("outw %1, %0" : : "d" (port), "a" (data));
}

void io_delay(void)
{
	const uint16_t DELAY_PORT = 0x80;

	__asm__ __volatile__ ("outb %%al, %0" : : "dN" (DELAY_PORT));
}

/*
	Highest byte (attribute byte): background color in most significant four
	bits, and the foreground color in the least significant four bits.

	Lowest byte (data)
*/


/*
	VGA (Video Graphics Array) is a standard model for video cards that provides
	an interface for translating output(data) to the computer monitor.

	Now today VGA is a computer display standard, and nearly every video card
	supports the VGA model.

	Standard graphics modes:
		- 640x480 in 16 colors.

		- 640x350 in 16 colors.

		- 640x200 in 16 colors.

		- 320x200 in 4 or 16 colors.

		- 320x200 in 256 colors.

	Standard text modes:
		- 80x25 character display, rendered with a 9x16 pixel font, with an
		  effective resolution of 720x400 in 16 colors.

		- 40x25, using the same font grid, for an effective resolution of 360x400
*/