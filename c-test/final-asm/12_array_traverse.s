	.text
	.comm	a,48,4
	.align	2
	.global	main
	.arm
	.type	main, %function
main:
	@ Function supports interworking.
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #20
	mov	r3, #0
	str	r3, [fp, #-8]
	mov	r3, #0
	str	r3, [fp, #-12]
	b	.L2
.L6:
	ldr	r3, [fp, #-8]
	str	r3, [fp, #-16]
	b	.L3
.L5:
	ldr	r3, [fp, #-16]
	cmp	r3, #3
	bgt	.L4
	ldr	r2, [fp, #-8]
	ldr	r3, [fp, #-16]
	sub	r3, r2, r3
	cmp	r3, #2
	bgt	.L4
	ldr	r2, [fp, #-8]
	ldr	r3, [fp, #-16]
	sub	r3, r2, r3
	ldr	r1, .L8
	lsl	r2, r3, #2
	ldr	r3, [fp, #-16]
	add	r3, r2, r3
	ldr	r2, [fp, #-12]
	str	r2, [r1, r3, lsl #2]
	mov	r3, #1
	str	r3, [fp, #-12]
.L4:
	ldr	r3, [fp, #-16]
	sub	r3, r3, #1
	str	r3, [fp, #-16]
.L3:
	ldr	r3, [fp, #-16]
	cmp	r3, #0
	bge	.L5
	ldr	r3, [fp, #-8]
	add	r3, r3, #1
	str	r3, [fp, #-8]
.L2:
	ldr	r3, [fp, #-8]
	cmp	r3, #5
	ble	.L6
	mov	r3, #0
	mov	r0, r3
	add	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
.L9:
	.align	2
.L8:
	.word	a
	.size	main, .-main
    