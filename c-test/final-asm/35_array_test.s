	.text
	.comm	field,8,4
	.align	2
	.global	func
	.arm
	.type	func, %function
func:
	@ Function supports interworking.
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #12
	str	r0, [fp, #-8]
	ldr	r3, .L3
	ldr	r3, [r3]
	rsb	r3, r3, #3
	lsl	r3, r3, #2
	ldr	r2, [fp, #-8]
	add	r3, r2, r3
	ldr	r3, [r3]
	mov	r0, r3
	add	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
.L4:
	.align	2
.L3:
	.word	field
	.size	func, .-func
	.align	2
	.global	main
	.arm
	.type	main, %function
main:
	@ Function supports interworking.
	@ args = 0, pretend = 0, frame = 24
	@ frame_needed = 1, uses_anonymous_args = 0
	push	{fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #24
	ldr	r3, .L7
	mov	r2, #1
	str	r2, [r3]
	ldr	r3, .L7
	mov	r2, #2
	str	r2, [r3, #4]
	mvn	r3, #0
	str	r3, [fp, #-24]
	ldr	r3, [fp, #-24]
	sub	r3, r3, #2
	str	r3, [fp, #-20]
	ldr	r3, [fp, #-20]
	str	r3, [fp, #-8]
	mov	r3, #16
	str	r3, [fp, #-16]
	sub	r3, fp, #24
	mov	r0, r3
	bl	func
	mov	r3, r0
	add	r2, r3, #2
	ldr	r3, [fp, #-8]
	add	r3, r2, r3
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	pop	{fp, lr}
	bx	lr
.L8:
	.align	2
.L7:
	.word	field
	.size	main, .-main
    