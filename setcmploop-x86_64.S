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
	mov $1,Le
	mov $1,Ge
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
	je L(break)
	mov (Rv),Rval
	mov (Pv),Pval
	cmp Pv,Pend
	jne L(cmp)
	jmp L(break)
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
	jmp L(break)
L(found4):
	cmp Rval,3*4(Pv)
	lea 4*4(Pv),Addr
	lea 3*4(Pv),Pv
	cmovb Addr,Pv
	xor Le,Le
	mov (Pv),Pval
	cmp Pv,Pend
	jne L(ge)
	jmp L(break)
L(found6):
	cmp Rval,5*4(Pv)
	lea 5*4(Pv),Pv
	cmovb Addr,Pv
	xor Le,Le
	mov (Pv),Pval
	cmp Pv,Pend
	jne L(ge)
	jmp L(break)
L(gt):
	xor Ge,Ge
	cmp Rv,Rend
	je L(break)
	mov (Rv),Rval
	jmp L(cmp)

	// Probe Pv with step=4.
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
	je L(break)
	mov (Rv),Rval
	mov (Pv),Pval
	cmp Pv,Pend
	jne L2(cmp)
	jmp L(break)
L2(lt2):
	cmp Rval,8*4(Pv)
	jae L2(found8)
	lea 12*4(Pv),Addr
	cmp Rval,12*4(Pv)
	jae L2(found12)
	mov Addr,Pv
L2(lt):
	cmp Rval,4*4(Pv)
	jb L2(lt2)
C2(found4):
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
	jne L2(ge)
	jmp L(break)
L2(found8):
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
	jne L2(ge)
	jmp L(break)
L2(found12):
	cmp Rval,10*4(Pv)
	lea 10*4(Pv),Pv
	cmovb Addr,Pv
	xor Le,Le
	lea -4(Pv),Addr
	cmp Rval,-4(Pv)
	cmovae Addr,Pv
	mov (Pv),Pval
	cmp Pv,Pend
	jne L2(ge)
	jmp L(break)
L2(gt):
	xor Ge,Ge
	cmp Rv,Rend
	je L(break)
	mov (Rv),Rval
	jmp L2(cmp)

L(break):
	xor %eax,%eax
	cmp Pv,Pend
	cmovne %eax,Le
	cmp Rv,Rend
	cmovne %eax,Ge

	test Le,Ge
	jz 3f
	ret
3:
	test Le,Le
	jz 4f
	mov $-1,%eax
	ret
4:
	test Ge,Ge
	jz 5f
	mov $1,%eax
	ret
5:
	mov $-2,%eax
	ret
ENDF(setcmploop)
