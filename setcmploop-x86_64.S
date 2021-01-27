#define FUNC(name) \
	.global name; \
	.hidden name; \
	.type   name,@function; \
	.align  16; \
	name:

#define ENDF(name) \
	.size   name,.-name

#define Pv   %rdi
#define Pend %rsi
#define Rv   %rdx
#define Rend %rcx
#define Rval %r8d
#define Pval %r9d
#define Addr %r9 // clobbers Pval
#define Le   %r10d
#define Ge   %r11d

#define Px0  %xmm1
#define Px1  %xmm2
#define Rx   %xmm3

#define L(name) .L##name
#define C(name) .L##name##__LINE__
#define L2(name) .L2##name
#define C2(name) .L2##name##__LINE__

FUNC(setcmploop)
	mov Pend,%rax
	shr $5,%rax
	cmp Rend,%rax
	lea (Pv,Pend,4),Pend
	lea (Rv,Rend,4),Rend
	mov (Rv),Rval
	mov (Pv),Pval
	mov $8,Le
	mov $8,Ge
	jae L2(cmp)

	// Probe Pv with step=2.
L(cmp):
	cmp Rval,Pval
	jb L(lt)
L(ge):
	add $4,Rv
	cmp Rval,Pval
	jne L(gt)
C(eq):
	add $4,Pv
	cmp Rv,Rend
	je L(breakR)
	mov (Rv),Rval
	mov (Pv),Pval
	cmp Pv,Pend
	jne L(cmp)
	jmp L(breakP)
L(lt2):
	cmp Rval,4*4(Pv)
	jae L(found4)
	lea 6*4(Pv),Addr
	cmp Rval,6*4(Pv)
	jae L(found6)
	mov Addr,Pv
L(lt):
	cmp Rval,2*4(Pv)
	jb L(lt2)
C(found2):
	cmp Rval,1*4(Pv)
	lea 2*4(Pv),Addr
	lea 1*4(Pv),Pv
	cmovb Addr,Pv
	xor Le,Le
	mov (Pv),Pval
	cmp Pv,Pend
	jne L(ge)
	jmp L(breakP)
L(found4):
	cmp Rval,3*4(Pv)
	lea 4*4(Pv),Addr
	lea 3*4(Pv),Pv
	cmovb Addr,Pv
	xor Le,Le
	mov (Pv),Pval
	cmp Pv,Pend
	jne L(ge)
	jmp L(breakP)
L(found6):
	cmp Rval,5*4(Pv)
	lea 5*4(Pv),Pv
	cmovb Addr,Pv
	xor Le,Le
	mov (Pv),Pval
	cmp Pv,Pend
	jne L(ge)
	jmp L(breakP)
L(gt):
	xor Ge,Ge
	cmp Rv,Rend
	je L(breakR)
	mov (Rv),Rval
	jmp L(cmp)

	// Probe Pv with step=8.
L2(cmp):
	cmp Rval,Pval
	jb L2(lt)
L2(ge):
	add $4,Rv
	cmp Rval,Pval
	jne L2(gt)
C2(eq):
	add $4,Pv
	cmp Rv,Rend
	je L(breakR)
	mov (Rv),Rval
	mov (Pv),Pval
	cmp Pv,Pend
	jne L2(cmp)
	jmp L(breakP)
L2(lt2):
	cmp Rval,16*4(Pv)
	jae L2(found16)
	lea 24*4(Pv),Addr
	cmp Rval,24*4(Pv)
	jae L2(found24)
	mov Addr,Pv
L2(lt):
	cmp Rval,8*4(Pv)
	jb L2(lt2)
C2(found8):
	movdqu 1*4(Pv),Px0
	movdqu 5*4(Pv),Px1
	movd Rval,Rx
	pshufd $0,Rx,Rx
	movdqa Rx,%xmm0
	pcmpgtd Px0,Rx
	pcmpgtd Px1,%xmm0
	packssdw %xmm0,Rx
	pmovmskb Rx,%eax
	lea 1*4(Pv),Pv
	xor Le,Le
	not %eax
	tzcnt %eax,%eax
	lea (Pv,%rax,2),Pv
	mov (Pv),Pval
	cmp Pv,Pend
	jne L2(ge)
	jmp L(breakP)
L2(found16):
	movdqu 9*4(Pv),Px0
	movdqu 13*4(Pv),Px1
	movd Rval,Rx
	pshufd $0,Rx,Rx
	movdqa Rx,%xmm0
	pcmpgtd Px0,Rx
	pcmpgtd Px1,%xmm0
	packssdw %xmm0,Rx
	pmovmskb Rx,%eax
	lea 10*4(Pv),Pv
	xor Le,Le
	not %eax
	tzcnt %eax,%eax
	lea (Pv,%rax,2),Pv
	mov (Pv),Pval
	cmp Pv,Pend
	jne L2(ge)
	jmp L(breakP)
L2(found24):
	movdqu 17*4(Pv),Px0
	movdqu 21*4(Pv),Px1
	movd Rval,Rx
	pshufd $0,Rx,Rx
	movdqa Rx,%xmm0
	pcmpgtd Px0,Rx
	pcmpgtd Px1,%xmm0
	packssdw %xmm0,Rx
	pmovmskb Rx,%eax
	lea 17*4(Pv),Pv
	xor Le,Le
	not %eax
	tzcnt %eax,%eax
	lea (Pv,%rax,2),Pv
	mov (Pv),Pval
	cmp Pv,Pend
	jne L2(ge)
	jmp L(breakP)
L2(gt):
	xor Ge,Ge
	cmp Rv,Rend
	je L(breakR)
	mov (Rv),Rval
	jmp L2(cmp)

L(breakP):
	xor %eax,%eax
	cmp Rv,Rend
	cmovne %eax,Ge
	jmp L(ret)
L(breakR):
	xor %eax,%eax
	cmp Pv,Pend
	cmovne %eax,Le
L(ret):
	lea (Le,Ge,2),%ecx
	mov $0x01fffe,%eax
	shr %cl,%eax
	movsbl %al,%eax
	ret
ENDF(setcmploop)
