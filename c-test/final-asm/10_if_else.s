	.text
	.comm	a,4,4
	.align	2
	.global	main
	.arm
	.type	main, %function
main:
	@ Function supports interworking.
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	ldr	r3, .L4
	mov	r2, #10
	str	r2, [r3]
	ldr	r3, .L4
	ldr	r3, [r3]
	cmp	r3, #0
	ble	.L2
	mov	r3, #1
	b	.L3
.L2:
	mov	r3, #0
.L3:
	mov	r0, r3
	add	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
.L5:
	.align	2
.L4:
	.word	a
	.size	main, .-main
    