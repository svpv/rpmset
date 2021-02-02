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

#define L(name) .Lsse2##name
#define C(name) .Lsse2##name##__LINE__

FUNC(setcmploop_sse2)
	mov Pend,%rax
	shr $5,%rax
	cmp Rend,%rax
	lea (Pv,Pend,4),Pend
	lea (Rv,Rend,4),Rend
	mov (Rv),Rval
	mov (Pv),Pval
	mov $8,Le
	mov $8,Ge
	jb setcmploop_step2
	cmpl $0,-4(Rend)
	js setcmploop_step4
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
	cmp Rval,18*4(Pv)
	jae L(found18)
	lea 27*4(Pv),Addr
	cmp Rval,27*4(Pv)
	jae L(found27)
	mov Addr,Pv
L(lt):
	cmp Rval,9*4(Pv)
	jb L(lt2)
C(found9):
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
	cmp Pend,Pv
	jb L(ge)
	jmp L(breakP)
L(found18):
	movdqu 10*4(Pv),Px0
	movdqu 14*4(Pv),Px1
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
	cmp Pend,Pv
	jb L(ge)
	jmp L(breakP)
L(found27):
	movdqu 19*4(Pv),Px0
	movdqu 23*4(Pv),Px1
	movd Rval,Rx
	pshufd $0,Rx,Rx
	movdqa Rx,%xmm0
	pcmpgtd Px0,Rx
	pcmpgtd Px1,%xmm0
	packssdw %xmm0,Rx
	pmovmskb Rx,%eax
	lea 19*4(Pv),Pv
	xor Le,Le
	not %eax
	tzcnt %eax,%eax
	lea (Pv,%rax,2),Pv
	mov (Pv),Pval
	cmp Pend,Pv
	jb L(ge)
	jmp L(breakP)
L(gt):
	xor Ge,Ge
	cmp Rv,Rend
	je L(breakR)
	mov (Rv),Rval
	jmp L(cmp)

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
ENDF(setcmploop_sse2)

#undef L
#undef C
#define L(name) .Lavx2##name
#define C(name) .Lavx2##name##__LINE__

FUNC(setcmploop_avx2)
	mov Pend,%rax
	shr $5,%rax
	cmp Rend,%rax
	lea (Pv,Pend,4),Pend
	lea (Rv,Rend,4),Rend
	mov (Rv),Rval
	mov (Pv),Pval
	mov $8,Le
	mov $8,Ge
	jb setcmploop_step2
	cmpl $0,-4(Rend)
	js setcmploop_step4
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
	cmp Rval,18*4(Pv)
	jae L(found18)
	lea 27*4(Pv),Addr
	cmp Rval,27*4(Pv)
	jae L(found27)
	mov Addr,Pv
L(lt):
	cmp Rval,9*4(Pv)
	jb L(lt2)
C(found9):
	vmovdqu 1*4(Pv),%ymm0
	movd Rval,%xmm1
	vpbroadcastd %xmm1,%ymm1
	vpcmpgtd %ymm0,%ymm1,%ymm1
	vpmovmskb %ymm1,%eax
	lea 1*4(Pv),Pv
	xor Le,Le
	not %rax
	tzcnt %rax,%rax
	lea (Pv,%rax,1),Pv
	mov (Pv),Pval
	cmp Pend,Pv
	jb L(ge)
	jmp L(breakP)
L(found18):
	vmovdqu 10*4(Pv),%ymm0
	movd Rval,%xmm1
	vpbroadcastd %xmm1,%ymm1
	vpcmpgtd %ymm0,%ymm1,%ymm1
	vpmovmskb %ymm1,%eax
	lea 10*4(Pv),Pv
	xor Le,Le
	not %rax
	tzcnt %rax,%rax
	lea (Pv,%rax,1),Pv
	mov (Pv),Pval
	cmp Pend,Pv
	jb L(ge)
	jmp L(breakP)
L(found27):
	vmovdqu 19*4(Pv),%ymm0
	movd Rval,%xmm1
	vpbroadcastd %xmm1,%ymm1
	vpcmpgtd %ymm0,%ymm1,%ymm1
	vpmovmskb %ymm1,%eax
	lea 19*4(Pv),Pv
	xor Le,Le
	not %rax
	tzcnt %rax,%rax
	lea (Pv,%rax,1),Pv
	mov (Pv),Pval
	cmp Pend,Pv
	jb L(ge)
	jmp L(breakP)
L(gt):
	xor Ge,Ge
	cmp Rv,Rend
	je L(breakR)
	mov (Rv),Rval
	jmp L(cmp)

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
ENDF(setcmploop_avx2)

#undef L
#undef C
#define L(name) .Lstep2##name
#define C(name) .Lstep2##name##__LINE__

FUNC(setcmploop_step2)
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
ENDF(setcmploop_step2)

#undef L
#undef C
#define L(name) .Lstep4##name
#define C(name) .Lstep4##name##__LINE__

FUNC(setcmploop_step4)
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
	cmp Rval,8*4(Pv)
	jae L(found8)
	lea 12*4(Pv),Addr
	cmp Rval,12*4(Pv)
	jae L(found12)
	mov Addr,Pv
L(lt):
	cmp Rval,4*4(Pv)
	jb L(lt2)
C(found4):
	cmp Rval,2*4(Pv)
	lea 4*4(Pv),Addr
	lea 2*4(Pv),Pv
	cmovb Addr,Pv
	xor Le,Le
	lea -4(Pv),Addr
	cmp Rval,-4(Pv)
	cmovae Addr,Pv
	mov (Pv),Pval
	cmp Pv,Pend
	jne L(ge)
	jmp L(breakP)
L(found8):
	cmp Rval,6*4(Pv)
	lea 8*4(Pv),Addr
	lea 6*4(Pv),Pv
	cmovb Addr,Pv
	xor Le,Le
	lea -4(Pv),Addr
	cmp Rval,-4(Pv)
	cmovae Addr,Pv
	mov (Pv),Pval
	cmp Pv,Pend
	jne L(ge)
	jmp L(breakP)
L(found12):
	cmp Rval,10*4(Pv)
	lea 10*4(Pv),Pv
	cmovb Addr,Pv
	xor Le,Le
	lea -4(Pv),Addr
	cmp Rval,-4(Pv)
	cmovae Addr,Pv
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
ENDF(setcmploop_step4)
