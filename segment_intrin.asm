.code
readfs proc
	mov rax, fs
	ret
readfs endp

readgs proc
	mov rax, gs
	ret
readgs endp

reades proc
	mov rax, es
	ret
reades endp

readds proc
	mov rax, ds
	ret
readds endp

readss proc
	mov rax, ss
	ret
readss endp

readcs proc
	mov rax, cs
	ret
readcs endp

readtr proc
	str ax
	ret
readtr endp

readldt proc
	sldt ax
	ret
readldt endp
end