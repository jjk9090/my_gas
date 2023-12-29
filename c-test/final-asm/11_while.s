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
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	ldr	r3, .L5
	mov	r2, #0
	str	r2, [r3]
	ldr	r3, .L5+4
	mov	r2, #3
	str	r2, [r3]
	b	.L2
.L3:
	ldr	r3, .L5
	ldr	r2, [r3]
	ldr	r3, .L5+4
	ldr	r3, [r3]
	add	r3, r2, r3
	ldr	r2, .L5
	str	r3, [r2]
	ldr	r3, .L5+4
	ldr	r3, [r3]
	sub	r3, r3, #1
	ldr	r2, .L5+4
	str	r3, [r2]
.L2:
	ldr	r3, .L5+4
	ldr	r3, [r3]
	cmp	r3, #0
	bgt	.L3
	ldr	r3, .L5
	ldr	r3, [r3]
	mov	r0, r3
	add	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
.L6:
	.align	2
.L5:
	.word	b
	.word	a
	.size	main, .-main
    