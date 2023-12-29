	.text
	.comm	n,4,4
	.comm	a,40,4
	.align	2
	.global	main
	.arm
	.type	main, %function
main:
	@ Function supports interworking.
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 1, uses_anonymous_args = 0
	push	{fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #16
	bl	getint
	mov	r3, r0
	ldr	r2, .L6
	str	r3, [r2]
	ldr	r3, .L6
	ldr	r3, [r3]
	cmp	r3, #10
	ble	.L2
	mov	r3, #1
	b	.L3
.L2:
	mov	r3, #0
	str	r3, [fp, #-12]
	ldr	r3, [fp, #-12]
	str	r3, [fp, #-8]
	b	.L4
.L5:
	bl	getint
	mov	r2, r0
	ldr	r1, .L6+4
	ldr	r3, [fp, #-12]
	str	r2, [r1, r3, lsl #2]
	ldr	r2, .L6+4
	ldr	r3, [fp, #-12]
	ldr	r3, [r2, r3, lsl #2]
	ldr	r2, [fp, #-8]
	add	r3, r2, r3
	str	r3, [fp, #-8]
	ldr	r3, [fp, #-12]
	add	r3, r3, #1
	str	r3, [fp, #-12]
.L4:
	ldr	r3, .L6
	ldr	r3, [r3]
	ldr	r2, [fp, #-12]
	cmp	r2, r3
	blt	.L5
	ldr	r0, [fp, #-8]
	bl	putint
	mov	r3, #10
	str	r3, [fp, #-16]
	ldr	r0, [fp, #-16]
	bl	putch
	ldr	r3, [fp, #-8]
.L3:
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	pop	{fp, lr}
	bx	lr
.L7:
	.align	2
.L6:
	.word	n
	.word	a
	.size	main, .-main
    