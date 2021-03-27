	.text
	.globl	superblock_d
	.bss
	.align 32
	.type	superblock_d, @object
	.size	superblock_d, 4096

superblock_d:
	.zero	4096

	.globl	cfs
	.align 32
	.type	cfs, @object
	.size	cfs, 32

cfs:
	.zero	32


	.section	.rodata

.LC0:
	.string	"mkfs: no device specified"

.LC1:
	.string	"\nusage: \n\tmkfs <device>"

.LC2:
	.string	"mkfs.c"

.LC3:
	.string	"dev != NULL"

.LC4:
	.string	"fd > 0"


	.text
	.globl	main
	.type	main, @function

main:
.LFB6:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
	movl	%edi, -20(%rbp)
	movq	%rsi, -32(%rbp)
	cmpl	$1, -20(%rbp)
	jne	.L2
	leaq	.LC0(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	leaq	.LC1(%rip), %rdi
	call	puts@PLT
	movl	$1, %edi
	call	exit@PLT
.L2:
	movq	-32(%rbp), %rax
	addq	$8, %rax
	movq	(%rax), %rax
	movq	%rax, %rdi
	call	strlen@PLT
	addq	$1, %rax
	movq	%rax, %rdi
	call	malloc@PLT
	movq	%rax, -8(%rbp)
	cmpq	$0, -8(%rbp)
	jne	.L3
	leaq	__PRETTY_FUNCTION__.2(%rip), %rcx
	movl	$37, %edx
	leaq	.LC2(%rip), %rsi
	leaq	.LC3(%rip), %rdi
	call	__assert_fail@PLT
.L3:
	movq	-32(%rbp), %rax
	addq	$8, %rax
	movq	(%rax), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	strcpy@PLT
	movq	-8(%rbp), %rax
	movl	$2, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	open@PLT
	movl	%eax, -12(%rbp)
	cmpl	$0, -12(%rbp)
	jg	.L4
	leaq	__PRETTY_FUNCTION__.2(%rip), %rcx
	movl	$42, %edx
	leaq	.LC2(%rip), %rsi
	leaq	.LC4(%rip), %rdi
	call	__assert_fail@PLT
.L4:
	movl	-12(%rbp), %eax
	movl	%eax, %edi
	call	mkfs
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	free@PLT
	movl	-12(%rbp), %eax
	movl	%eax, %edi
	call	close@PLT
	movl	$0, %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	main, .-main
	.globl	mkfs
	.type	mkfs, @function
mkfs:
.LFB7:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movl	%edi, -36(%rbp)
	movl	-36(%rbp), %eax
	movl	$2, %edx
	movl	$0, %esi
	movl	%eax, %edi
	call	lseek@PLT
	movq	%rax, -24(%rbp)
	movq	-24(%rbp), %rax
	shrq	$12, %rax
	movq	%rax, -16(%rbp)
	movl	-36(%rbp), %eax
	movl	$0, %edx
	movl	$0, %esi
	movl	%eax, %edi
	call	lseek@PLT
	movq	-16(%rbp), %rax
	movl	%eax, %edx
	movl	-36(%rbp), %eax
	movl	%edx, %esi
	movl	%eax, %edi
	call	setup_sb
	cltq
	movq	%rax, -8(%rbp)
	movq	-16(%rbp), %rax
	movl	%eax, %edx
	movq	-8(%rbp), %rax
	movl	%eax, %ecx
	movl	-36(%rbp), %eax
	movl	%ecx, %esi
	movl	%eax, %edi
	call	setup_bbm
	movq	-16(%rbp), %rax
	movl	%eax, %edx
	movq	-16(%rbp), %rax
	salq	$12, %rax
	shrq	$9, %rax
	shrq	$6, %rax
	movl	%eax, %ecx
	movl	-36(%rbp), %eax
	movl	%ecx, %esi
	movl	%eax, %edi
	call	setup_ibm
	movl	$0, %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	mkfs, .-mkfs
	.section	.rodata
.LC5:
	.string	"blocks count: %u\n"
.LC6:
	.string	"inodes count: %u\n"
.LC7:
	.string	"first data block: %u\n"
	.text
	.globl	setup_sb
	.type	setup_sb, @function
setup_sb:
.LFB8:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	

	/* store arguments */
	/*
		-4(%rbp): fd
		-8(%rbp): cblocks
	*/
	movl	%edi, -4(%rbp)
	movl	%esi, -8(%rbp)

	/* superblock_d.magic = SB_MAGIC */
	movl	$20090427, 512+superblock_d(%rip)

	/* superblock_d.cblocks = cblocks */
	movl	-8(%rbp), %eax
	movl	%eax, 516+superblock_d(%rip)

	/* printf("blocks count: %u\n", cblocks) */
	movl	-8(%rbp), %eax
	movl	%eax, %esi
	leaq	.LC5(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT

	/* superblock_d.block_bitmap = BBM_START */
	movl	$1, 520+superblock_d(%rip)

	/* superblock_d.cinodes = INODES_COUNT */
	/* %eax = cblocks */
	movl	-8(%rbp), %eax

	/* `sal` means $12 is signed */
	sall	$12, %eax
	shrl	$9, %eax
	shrl	$6, %eax
	movl	%eax, 528+superblock_d(%rip)

	movl	-8(%rbp), %eax
	shrl	$3, %eax
	addl	$4095, %eax
	andl	$-4096, %eax
	addl	$2, %eax
	leal	(%rax,%rax), %edx
	movl	-8(%rbp), %eax
	sall	$12, %eax
	shrl	$9, %eax
	shrl	$6, %eax
	sall	$6, %eax
	addl	$4095, %eax
	andl	$-4096, %eax
	addl	%edx, %eax
	addl	$2, %eax
	movl	%eax, 532+superblock_d(%rip)

	movl	-8(%rbp), %eax
	sall	$12, %eax
	shrl	$9, %eax
	shrl	$6, %eax
	movl	%eax, %eax
	movq	%rax, %rsi
	leaq	.LC6(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	movl	-8(%rbp), %eax
	shrl	$3, %eax
	addl	$4095, %eax
	andl	$-4096, %eax
	addl	$2, %eax
	addl	%eax, %eax
	addl	$1, %eax
	movl	%eax, %edx
	movl	-8(%rbp), %eax
	sall	$12, %eax
	shrl	$9, %eax
	shrl	$6, %eax
	movl	%eax, %eax
	salq	$6, %rax
	addq	$4095, %rax
	andl	$4294963200, %eax
	addq	%rdx, %rax
	addq	$1, %rax
	movq	%rax, %rsi
	leaq	.LC7(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	movl	-4(%rbp), %eax
	movl	$0, %ecx
	movl	$4096, %edx
	leaq	superblock_d(%rip), %rsi
	movl	%eax, %edi
	call	pwrite@PLT
	movl	-8(%rbp), %eax
	shrl	$3, %eax
	addl	$4095, %eax
	andl	$-4096, %eax
	addl	$2, %eax
	leal	(%rax,%rax), %edx
	movl	-8(%rbp), %eax
	sall	$12, %eax
	shrl	$9, %eax
	shrl	$6, %eax
	sall	$6, %eax
	addl	$4095, %eax
	andl	$-4096, %eax
	addl	%edx, %eax
	addl	$1, %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE8:
	.size	setup_sb, .-setup_sb
	.section	.rodata
.LC8:
	.string	"bbm != NULL"
	.text
	.globl	setup_bbm
	.type	setup_bbm, @function
setup_bbm:
.LFB9:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, -20(%rbp)
	movl	%esi, -24(%rbp)
	movl	%edx, -28(%rbp)
	movl	-28(%rbp), %eax
	shrl	$12, %eax
	addl	$4095, %eax
	andl	$-4096, %eax
	sall	$12, %eax
	movl	%eax, %eax
	movq	%rax, %rdi
	call	malloc@PLT
	movq	%rax, -8(%rbp)
	cmpq	$0, -8(%rbp)
	jne	.L11
	leaq	__PRETTY_FUNCTION__.1(%rip), %rcx
	movl	$141, %edx
	leaq	.LC2(%rip), %rsi
	leaq	.LC8(%rip), %rdi
	call	__assert_fail@PLT
.L11:
	movq	-8(%rbp), %rax
	movq	%rax, 8+cfs(%rip)
	movq	$0, -16(%rbp)
	jmp	.L12
.L13:
	movq	-16(%rbp), %rax
	movl	%eax, %ecx
	movq	-8(%rbp), %rax
	movl	$1, %edx
	movl	%ecx, %esi
	movq	%rax, %rdi
	call	bitmap_set@PLT
	addq	$1, -16(%rbp)
.L12:
	movl	-24(%rbp), %eax
	cmpq	%rax, -16(%rbp)
	jb	.L13
	movl	-28(%rbp), %eax
	shrl	$12, %eax
	addl	$4095, %eax
	andl	$-4096, %eax
	sall	$12, %eax
	movl	%eax, %edx
	movq	-8(%rbp), %rsi
	movl	-20(%rbp), %eax
	movl	$4096, %ecx
	movl	%eax, %edi
	call	pwrite@PLT
	movl	$0, %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE9:
	.size	setup_bbm, .-setup_bbm
	.section	.rodata
.LC9:
	.string	"ibm != NULL"
	.text
	.globl	setup_ibm
	.type	setup_ibm, @function
setup_ibm:
.LFB10:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$312, %rsp
	.cfi_offset 3, -24
	movl	%edi, -308(%rbp)
	movl	%esi, -312(%rbp)
	movl	%edx, -316(%rbp)
	movq	%fs:40, %rax
	movq	%rax, -24(%rbp)
	xorl	%eax, %eax
	movl	-312(%rbp), %eax
	shrl	$3, %eax
	addl	$4095, %eax
	andl	$-4096, %eax
	sall	$12, %eax
	movl	%eax, %eax
	movq	%rax, %rdi
	call	malloc@PLT
	movq	%rax, -296(%rbp)
	cmpq	$0, -296(%rbp)
	jne	.L16
	leaq	__PRETTY_FUNCTION__.0(%rip), %rcx
	movl	$170, %edx
	leaq	.LC2(%rip), %rsi
	leaq	.LC9(%rip), %rdi
	call	__assert_fail@PLT
.L16:
	movq	-296(%rbp), %rax
	movq	%rax, 16+cfs(%rip)
	movq	-296(%rbp), %rax
	movl	$1, %edx
	movl	$0, %esi
	movq	%rax, %rdi
	call	bitmap_set@PLT
	movq	-296(%rbp), %rax
	movl	$1, %edx
	movl	$1, %esi
	movq	%rax, %rdi
	call	bitmap_set@PLT
	movl	$0, -288(%rbp)
	movl	$0, -284(%rbp)
	movw	$1215, -280(%rbp)
	movl	$0, -278(%rbp)
	movw	$0, -274(%rbp)
	leaq	-288(%rbp), %rax
	addq	$16, %rax
	movl	$32, %edx
	movl	$0, %esi
	movq	%rax, %rdi
	call	memset@PLT
	leaq	-288(%rbp), %rax
	addq	$48, %rax
	movl	$16, %edx
	movl	$0, %esi
	movq	%rax, %rdi
	call	memset@PLT
	movl	$1, -224(%rbp)
	movl	$0, -220(%rbp)
	movw	$1215, -216(%rbp)
	movl	$0, -214(%rbp)
	movw	$0, -210(%rbp)
	leaq	-224(%rbp), %rax
	addq	$16, %rax
	movl	$32, %edx
	movl	$0, %esi
	movq	%rax, %rdi
	call	memset@PLT
	leaq	-288(%rbp), %rax
	addq	$48, %rax
	movl	$16, %edx
	movl	$0, %esi
	movq	%rax, %rdi
	call	memset@PLT
	movq	-288(%rbp), %rax
	movq	-280(%rbp), %rdx
	movq	%rax, -160(%rbp)
	movq	%rdx, -152(%rbp)
	movq	-272(%rbp), %rax
	movq	-264(%rbp), %rdx
	movq	%rax, -144(%rbp)
	movq	%rdx, -136(%rbp)
	movq	-256(%rbp), %rax
	movq	-248(%rbp), %rdx
	movq	%rax, -128(%rbp)
	movq	%rdx, -120(%rbp)
	movq	-240(%rbp), %rax
	movq	-232(%rbp), %rdx
	movq	%rax, -112(%rbp)
	movq	%rdx, -104(%rbp)
	leaq	-160(%rbp), %rax
	addq	$64, %rax
	movq	-224(%rbp), %rcx
	movq	-216(%rbp), %rbx
	movq	%rcx, (%rax)
	movq	%rbx, 8(%rax)
	movq	-208(%rbp), %rcx
	movq	-200(%rbp), %rbx
	movq	%rcx, 16(%rax)
	movq	%rbx, 24(%rax)
	movq	-192(%rbp), %rcx
	movq	-184(%rbp), %rbx
	movq	%rcx, 32(%rax)
	movq	%rbx, 40(%rax)
	movq	-176(%rbp), %rcx
	movq	-168(%rbp), %rbx
	movq	%rcx, 48(%rax)
	movq	%rbx, 56(%rax)
	movl	-316(%rbp), %eax
	shrl	$12, %eax
	addl	$4095, %eax
	andl	$-4096, %eax
	movl	%eax, %edx
	movl	-312(%rbp), %eax
	shrl	$3, %eax
	addl	$4095, %eax
	andl	$-4096, %eax
	addl	%edx, %eax
	addl	$3, %eax
	sall	$12, %eax
	movl	%eax, %edx
	leaq	-160(%rbp), %rsi
	movl	-308(%rbp), %eax
	movq	%rdx, %rcx
	movl	$128, %edx
	movl	%eax, %edi
	call	pwrite@PLT
	movl	$0, %eax
	movq	-24(%rbp), %rbx
	subq	%fs:40, %rbx
	je	.L18
	call	__stack_chk_fail@PLT
.L18:
	movq	-8(%rbp), %rbx
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE10:
	.size	setup_ibm, .-setup_ibm
	.section	.rodata
	.type	__PRETTY_FUNCTION__.2, @object
	.size	__PRETTY_FUNCTION__.2, 5
__PRETTY_FUNCTION__.2:
	.string	"main"
	.align 8
	.type	__PRETTY_FUNCTION__.1, @object
	.size	__PRETTY_FUNCTION__.1, 10
__PRETTY_FUNCTION__.1:
	.string	"setup_bbm"
	.align 8
	.type	__PRETTY_FUNCTION__.0, @object
	.size	__PRETTY_FUNCTION__.0, 10
__PRETTY_FUNCTION__.0:
	.string	"setup_ibm"
	.ident	"GCC: (GNU) 10.2.0"
	.section	.note.GNU-stack,"",@progbits
