	.text
	.align	2
	.global	ifWhile
	.arm
	.type	ifWhile, %function
ifWhile:
	@ Function supports interworking.
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #12
	mov	r3, #0
	str	r3, [fp, #-8]
	mov	r3, #3
	str	r3, [fp, #-12]
	ldr	r3, [fp, #-8]
	cmp	r3, #5
	bne	.L6
	b	.L3
.L4:
	ldr	r3, [fp, #-12]
	add	r3, r3, #2
	str	r3, [fp, #-12]
.L3:
	ldr	r3, [fp, #-12]
	cmp	r3, #2
	beq	.L4
	ldr	r3, [fp, #-12]
	add	r3, r3, #25
	str	r3, [fp, #-12]
	b	.L5
.L7:
	ldr	r3, [fp, #-12]
	lsl	r3, r3, #1
	str	r3, [fp, #-12]
	ldr	r3, [fp, #-8]
	add	r3, r3, #1
	str	r3, [fp, #-8]
.L6:
	ldr	r3, [fp, #-8]
	cmp	r3, #4
	ble	.L7
.L5:
	ldr	r3, [fp, #-12]
	mov	r0, r3
	add	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
	.size	ifWhile, .-ifWhile
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
	bl	ifWhile
	mov	r3, r0
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	pop	{fp, lr}
	bx	lr
	.size	main, .-main
    