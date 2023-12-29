	.text
	.align	2
	.global	main
	.arm
	.type	main, %function
main:
	@ Function supports interworking.
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #12
	mov	r3, #10
	str	r3, [fp, #-8]
	b	.L2
.L5:
	ldr	r3, [fp, #-8]
	cmp	r3, #5
	ble	.L3
	ldr	r3, [fp, #-8]
	sub	r3, r3, #1
	str	r3, [fp, #-8]
	b	.L2
.L3:
	ldr	r3, [fp, #-8]
	b	.L4
.L2:
	ldr	r3, [fp, #-8]
	cmp	r3, #0
	bgt	.L5
	ldr	r3, [fp, #-8]
.L4:
	mov	r0, r3
	add	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
	.size	main, .-main
    