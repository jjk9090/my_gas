	.text
	.comm	a,20,4
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
	bl	getint
	mov	r3, r0
	ldr	r2, .L5
	str	r3, [r2]
	bl	getint
	mov	r3, r0
	ldr	r2, .L5
	str	r3, [r2, #4]
	bl	getint
	mov	r3, r0
	ldr	r2, .L5
	str	r3, [r2, #8]
	bl	getint
	mov	r3, r0
	ldr	r2, .L5
	str	r3, [r2, #12]
	bl	getint
	mov	r3, r0
	ldr	r2, .L5
	str	r3, [r2, #16]
	mov	r3, #4
	str	r3, [fp, #-8]
	mov	r3, #0
	str	r3, [fp, #-12]
	b	.L2
.L3:
	ldr	r2, .L5
	ldr	r3, [fp, #-8]
	ldr	r3, [r2, r3, lsl #2]
	ldr	r2, [fp, #-12]
	add	r3, r2, r3
	str	r3, [fp, #-12]
	ldr	r3, [fp, #-8]
	sub	r3, r3, #1
	str	r3, [fp, #-8]
.L2:
	ldr	r3, [fp, #-8]
	cmp	r3, #1
	bgt	.L3
	ldr	r3, [fp, #-12]
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	pop	{fp, lr}
	bx	lr
.L6:
	.align	2
.L5:
	.word	a
	.size	main, .-main
    