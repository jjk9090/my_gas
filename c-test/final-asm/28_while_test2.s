	.text
	.align	2
	.global	FourWhile
	.arm
	.type	FourWhile, %function
FourWhile:
	@ Function supports interworking.
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #20
	mov	r3, #5
	str	r3, [fp, #-8]
	mov	r3, #6
	str	r3, [fp, #-12]
	mov	r3, #7
	str	r3, [fp, #-16]
	mov	r3, #10
	str	r3, [fp, #-20]
	b	.L2
.L9:
	ldr	r3, [fp, #-8]
	add	r3, r3, #3
	str	r3, [fp, #-8]
	b	.L3
.L8:
	ldr	r3, [fp, #-12]
	add	r3, r3, #1
	str	r3, [fp, #-12]
	b	.L4
.L7:
	ldr	r3, [fp, #-16]
	sub	r3, r3, #1
	str	r3, [fp, #-16]
	b	.L5
.L6:
	ldr	r3, [fp, #-20]
	add	r3, r3, #3
	str	r3, [fp, #-20]
.L5:
	ldr	r3, [fp, #-20]
	cmp	r3, #19
	ble	.L6
	ldr	r3, [fp, #-20]
	sub	r3, r3, #1
	str	r3, [fp, #-20]
.L4:
	ldr	r3, [fp, #-16]
	cmp	r3, #7
	beq	.L7
	ldr	r3, [fp, #-16]
	add	r3, r3, #1
	str	r3, [fp, #-16]
.L3:
	ldr	r3, [fp, #-12]
	cmp	r3, #9
	ble	.L8
	ldr	r3, [fp, #-12]
	sub	r3, r3, #2
	str	r3, [fp, #-12]
.L2:
	ldr	r3, [fp, #-8]
	cmp	r3, #19
	ble	.L9
	ldr	r2, [fp, #-12]
	ldr	r3, [fp, #-20]
	add	r2, r2, r3
	ldr	r3, [fp, #-8]
	add	r2, r2, r3
	ldr	r3, [fp, #-16]
	add	r3, r2, r3
	mov	r0, r3
	add	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
	.size	FourWhile, .-FourWhile
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
	bl	FourWhile
	mov	r3, r0
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	pop	{fp, lr}
	bx	lr
	.size	main, .-main
    