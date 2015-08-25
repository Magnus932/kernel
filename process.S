
.text
	.globl copy_page_physical
	copy_page_physical:
		pushl %ebx
		pushf					# Push EFLAGS, so we can pop it and re-enable interrupts
								# later.
		cli						# Disable interrupts, so we arent interrupted.
								# Load these in BEFORE we disable paging.
		movl 12(%esp), %ebx		# Source address.
		movl 16(%esp), %ecx		# Destination address.

		movl %cr0, %edx			# Get the control register.
		and $0x7fffffff, %edx	# Clear PG bit ( Bit 31 ).
		movl %edx, %cr0			# Disable paging.

		movl $1024, %edx		# 1024 * 4 bytes = 4096 bytes to copy.

		loop:
			movl (%ebx), %eax	# Get the word at the source address.
			movl %eax, (%ecx)	# Store it at the dest address.
			addl $0x04, %ebx	# Source address += 4.
			addl $0x04, %ecx	# Dest address += 4.
			dec %edx			# Decrement counter.
			jnz loop			# Jump if ZF flag is not set.

		movl %cr0, %edx			# Get the control register again.
		or $0x80000000, %edx	# Set the PG bit ( Bit 31 ).
		movl %edx, %cr0			# Enable paging.

		popf					# Pop EFLAGS back.
		popl %ebx
		ret

	.globl read_eip
	read_eip:
		popl %eax
		jmp *%eax