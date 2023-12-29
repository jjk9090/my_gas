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
	ldr	r2, .L5
	str	r3, [r2]
	bl	getint
	mov	r3, r0
	ldr	r2, .L5+4
	str	r3, [r2]
	ldr	r3, .L5
	ldr	r3, [r3]
	cmp	r3, #0
	bne	.L2
	ldr	r3, .L5+4
	ldr	r3, [r3]
	cmp	r3, #0
	beq	.L3
.L2:
	mov	r3, #1
	b	.L4
.L3:
	mov	r3, #0
.L4:
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	pop	{fp, lr}
	bx	lr
.L6:
	.align	2
.L5:
	.word	a
	.word	b
	.size	main, .-main
    