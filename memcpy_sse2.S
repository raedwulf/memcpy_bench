.global _memcpy_sse2

.text
_memcpy_sse2:
	cmpq $256, %rcx
	jb less_than_256
loop_256:
	movdqu   0(%rsi), %xmm0
	movdqu  16(%rsi), %xmm1
	movdqu  32(%rsi), %xmm2
	movdqu  48(%rsi), %xmm3
	movdqu  64(%rsi), %xmm4
	movdqu  80(%rsi), %xmm5
	movdqu  96(%rsi), %xmm6
	movdqu 112(%rsi), %xmm7
	movdqu 128(%rsi), %xmm8
	movdqu 144(%rsi), %xmm9
	movdqu 160(%rsi), %xmm10
	movdqu 176(%rsi), %xmm11
	movdqu 192(%rsi), %xmm12
	movdqu 208(%rsi), %xmm13
	movdqu 224(%rsi), %xmm14
	movdqu 240(%rsi), %xmm15
	movntdq %xmm0 ,   0(%rdi)
	movntdq %xmm1 ,  16(%rdi)
	movntdq %xmm2 ,  32(%rdi)
	movntdq %xmm3 ,  48(%rdi)
	movntdq %xmm4 ,  64(%rdi)
	movntdq %xmm5 ,  80(%rdi)
	movntdq %xmm6 ,  96(%rdi)
	movntdq %xmm7 , 112(%rdi)
	movntdq %xmm8 , 128(%rdi)
	movntdq %xmm9 , 144(%rdi)
	movntdq %xmm10, 160(%rdi)
	movntdq %xmm11, 176(%rdi)
	movntdq %xmm12, 192(%rdi)
	movntdq %xmm13, 208(%rdi)
	movntdq %xmm14, 224(%rdi)
	movntdq %xmm15, 240(%rdi)
	subq $256, %rcx
	jae loop_256
less_than_256:
	movq %rdi, %rax
	movq %rdx, %rcx
	rep movsb
	ret
