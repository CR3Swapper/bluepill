extern exit_handler : proc

.code
vmxexit_handler proc
	int 3 ; see if vmexit get called...
	push rax
	push rbx
	push rcx
	push rdx
	push rsi
	push rdi
	push rbp
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15

	sub rsp, 0100h ; 16 xmm registers... probably dont need to do all of them...
	movaps [rsp], xmm0
	movaps [rsp + 010h], xmm1
	movaps [rsp + 020h], xmm2
	movaps [rsp + 030h], xmm3
	movaps [rsp + 040h], xmm4
	movaps [rsp + 050h], xmm5
	movaps [rsp + 060h], xmm6
	movaps [rsp + 070h], xmm7
	movaps [rsp + 080h], xmm8
	movaps [rsp + 090h], xmm9
	movaps [rsp + 0A0h], xmm10
	movaps [rsp + 0B0h], xmm11
	movaps [rsp + 0C0h], xmm12
	movaps [rsp + 0D0h], xmm13
	movaps [rsp + 0E0h], xmm14
	movaps [rsp + 0F0h], xmm15

	mov rcx, rsp
	sub rsp, 20h
	call exit_handler
	add rsp, 20h

	movaps xmm0, [rsp]
	movaps xmm1, [rsp + 010h]
	movaps xmm2, [rsp + 020h]
	movaps xmm3, [rsp + 030h]
	movaps xmm4, [rsp + 040h]
	movaps xmm5, [rsp + 050h]
	movaps xmm6, [rsp + 060h]
	movaps xmm7, [rsp + 070h]
	movaps xmm8, [rsp + 080h]
	movaps xmm9, [rsp + 090h]
	movaps xmm10, [rsp + 0A0h]
	movaps xmm11, [rsp + 0B0h]
	movaps xmm12, [rsp + 0C0h]
	movaps xmm13, [rsp + 0D0h]
	movaps xmm14, [rsp + 0E0h]
	movaps xmm15, [rsp + 0F0h]
	add rsp, 0100h ; 16 xmm registers... probably dont need to do all of them...

	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rbp 
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rax

	vmresume
vmxexit_handler endp
end