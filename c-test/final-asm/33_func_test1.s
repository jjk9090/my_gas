	.text
	.comm	a,4,4
	.align	2
	.global	myFunc
	.arm
	.type	myFunc, %function
myFunc:
	@ Function supports interworking.
	@ args = 0, pretend = 0, frame = 24
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #28
	str	r0, [fp, #-16]
	str	r1, [fp, #-20]
	str	r2, [fp, #-24]
	mov	r3, #2
	str	r3, [fp, #-16]
	mov	r3, #0
	str	r3, [fp, #-8]
	ldr	r3, [fp, #-8]
	cmp	r3, #0
	beq	.L4
	mov	r3, #0
	b	.L3
.L5:
	ldr	r3, [fp, #-20]
	sub	r3, r3, #1
	str	r3, [fp, #-20]
.L4:
	ldr	r3, [fp, #-20]
	cmp	r3, #0
	bgt	.L5
	ldr	r2, [fp, #-16]
	ldr	r3, [fp, #-20]
	add	r3, r2, r3
.L3:
	mov	r0, r3
	add	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
	.size	myFunc, .-myFunc
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
	ldr	r3, .L8
	mov	r2, #3
	str	r2, [r3]
	mov	r2, #1
	mov	r1, #2
	mov	r0, #1
	bl	myFunc
	str	r0, [fp, #-8]
	ldr	r3, .L8
	ldr	r2, [r3]
	ldr	r3, [fp, #-8]
	add	r3, r2, r3
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	pop	{fp, lr}
	bx	lr
.L9:
	.align	2
.L8:
	.word	a
	.size	main, .-main
    