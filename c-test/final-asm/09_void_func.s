	.text
	.comm	a,4,4
	.comm	b,4,4
	.comm	c,4,4
	.align	2
	.global	add
	.arm
	.type	add, %function
add:
	@ Function supports interworking.
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #12
	str	r0, [fp, #-8]
	str	r1, [fp, #-12]
	ldr	r2, [fp, #-8]
	ldr	r3, [fp, #-12]
	add	r3, r2, r3
	ldr	r2, .L3
	str	r3, [r2]
	nop
	add	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
.L4:
	.align	2
.L3:
	.word	c
	.size	add, .-add
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
	ldr	r3, .L7
	mov	r2, #3
	str	r2, [r3]
	ldr	r3, .L7+4
	mov	r2, #2
	str	r2, [r3]
	ldr	r3, .L7
	ldr	r3, [r3]
	ldr	r2, .L7+4
	ldr	r2, [r2]
	mov	r1, r2
	mov	r0, r3
	bl	add
	ldr	r3, .L7+8
	ldr	r3, [r3]
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	pop	{fp, lr}
	bx	lr
.L8:
	.align	2
.L7:
	.word	a
	.word	b
	.word	c
	.size	main, .-main
    