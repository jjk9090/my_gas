	.text
	.comm	a,4,4
	.comm	b,4,4
	.global	__aeabi_idivmod
	.align	2
	.global	main
	.arm
	.type	main, %function
main:
	@ Function supports interworking.
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	push	{fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #8
	ldr	r3, .L3
	mov	r2, #10
	str	r2, [r3]
	ldr	r3, .L3+4
	mov	r2, #3
	str	r2, [r3]
	ldr	r3, .L3
	ldr	r3, [r3]
	ldr	r2, .L3+4
	ldr	r2, [r2]
	mov	r1, r2
	mov	r0, r3
	bl	__aeabi_idivmod
	mov	r3, r1
	str	r3, [fp, #-8]
	ldr	r3, [fp, #-8]
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	pop	{fp, lr}
	bx	lr
.L4:
	.align	2
.L3:
	.word	a
	.word	b
	.size	main, .-main
    