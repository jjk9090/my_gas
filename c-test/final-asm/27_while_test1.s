	.text
	.align	2
	.global	doubleWhile
	.arm
	.type	doubleWhile, %function
doubleWhile:
	@ Function supports interworking.
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #12
	mov	r3, #5
	str	r3, [fp, #-8]
	mov	r3, #7
	str	r3, [fp, #-12]
	b	.L2
.L5:
	ldr	r3, [fp, #-8]
	add	r3, r3, #30
	str	r3, [fp, #-8]
	b	.L3
.L4:
	ldr	r3, [fp, #-12]
	add	r3, r3, #6
	str	r3, [fp, #-12]
.L3:
	ldr	r3, [fp, #-12]
	cmp	r3, #99
	ble	.L4
	ldr	r3, [fp, #-12]
	sub	r3, r3, #100
	str	r3, [fp, #-12]
.L2:
	ldr	r3, [fp, #-8]
	cmp	r3, #99
	ble	.L5
	ldr	r3, [fp, #-12]
	mov	r0, r3
	add	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
	.size	doubleWhile, .-doubleWhile
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
	bl	doubleWhile
	mov	r3, r0
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	pop	{fp, lr}
	bx	lr
	.size	main, .-main
    