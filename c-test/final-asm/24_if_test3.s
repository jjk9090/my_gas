	.text
	.align	2
	.global	ififElse
	.arm
	.type	ififElse, %function
ififElse:
	@ Function supports interworking.
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #12
	mov	r3, #5
	str	r3, [fp, #-8]
	mov	r3, #10
	str	r3, [fp, #-12]
	ldr	r3, [fp, #-8]
	cmp	r3, #5
	bne	.L2
	ldr	r3, [fp, #-12]
	cmp	r3, #10
	bne	.L3
	mov	r3, #25
	str	r3, [fp, #-8]
	b	.L2
.L3:
	ldr	r3, [fp, #-8]
	add	r3, r3, #15
	str	r3, [fp, #-8]
.L2:
	ldr	r3, [fp, #-8]
	mov	r0, r3
	add	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
	.size	ififElse, .-ififElse
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
	bl	ififElse
	mov	r3, r0
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	pop	{fp, lr}
	bx	lr
	.size	main, .-main
