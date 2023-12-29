	.text
	.comm	a,4,4
	.comm	b,4,4
	.align	2
	.global	main
	.arm
	.type	main, %function
main:
	@ Function supports interworking.
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	push	{fp, lr}
	add	fp, sp, #4
	bl	getint
	mov	r3, r0
	ldr	r2, .L4
	str	r3, [r2]
	bl	getint
	mov	r3, r0
	ldr	r2, .L4+4
	str	r3, [r2]
	ldr	r3, .L4
	ldr	r2, [r3]
	ldr	r3, .L4+4
	ldr	r3, [r3]
	cmp	r2, r3
	bge	.L2
	mov	r3, #1
	b	.L3
.L2:
	mov	r3, #0
.L3:
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	pop	{fp, lr}
	bx	lr
.L5:
	.align	2
.L4:
	.word	a
	.word	b
	.size	main, .-main
    