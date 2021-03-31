extern exit_handler : proc
extern vmresume_failure : proc

.code
vmxlaunch_processor proc
	pushfq				; vmlaunch sets some flags if an error happens...

	mov rcx, 0681Ch		; VMCS_GUEST_RSP
	vmwrite rcx, rsp	; current rsp pointer...

	mov rcx, 0681Eh		; VMCS_GUEST_RIP
	lea rdx, done		;
	vmwrite rcx, rdx	; return C0FFEE on success...
	vmlaunch

	pushfq				; push rflags to the stack then put it into rax...
	pop rax				;

	popfq				; restore rflags back to what it was in the c++ code...
	ret

done:
	popfq				; restore flags and return back to c++ code...
	mov rax, 0C0FFEEh
	ret
vmxlaunch_processor endp

vmxexit_handler proc
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

	sub rsp, 0108h ; 16 xmm registers... and +8 bytes for alignment...
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

	movups xmm0, [rsp]
	movups xmm1, [rsp + 010h]
	movups xmm2, [rsp + 020h]
	movups xmm3, [rsp + 030h]
	movups xmm4, [rsp + 040h]
	movups xmm5, [rsp + 050h]
	movups xmm6, [rsp + 060h]
	movups xmm7, [rsp + 070h]
	movups xmm8, [rsp + 080h]
	movups xmm9, [rsp + 090h]
	movups xmm10, [rsp + 0A0h]
	movups xmm11, [rsp + 0B0h]
	movups xmm12, [rsp + 0C0h]
	movups xmm13, [rsp + 0D0h]
	movups xmm14, [rsp + 0E0h]
	movups xmm15, [rsp + 0F0h]
	add rsp, 0108h ; 16 xmm registers... and +8 bytes for alignment...

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
	pop rbx
	pop rax

	vmresume
	call vmresume_failure
	hlt
vmxexit_handler endp
end