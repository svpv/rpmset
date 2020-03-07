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

FUNC(setcmploop)
	lea (Pv,Pend,4),Pend
	lea (Rv,Rend,4),Rend
	mov (Rv),Rval
	mov (Pv),Pval
	mov $1,Le
	mov $1,Ge
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
