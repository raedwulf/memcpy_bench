.global _memcpy_movsb

.text
_memcpy_movsb:
	movq %rdi, %rax
	movq %rdx, %rcx
	rep movsb
	ret
