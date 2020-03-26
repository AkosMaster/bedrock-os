section .kernel_entry
global _start

[bits 32]

_start:

cli ; disabling interrupts, just to be sure (gdt_setup will need this)
call enable_VM
lgdt [gdt_descriptor]
call reload_segments

k_free_mem_addr equ 0x10000
block_size equ 0x1000
mov esp, 0xC0000000 + k_free_mem_addr + block_size - 16 ; set new esp to first kmalloc block

    [extern kernel_main] ; Define calling point. Must have same name as kernel.c 'main' function
    push dword 0x0
    jmp kernel_main ; Calls the C function. The linker will know where it is placed in memory
    jmp $

%include "boot/gdt.asm"

enable_VM:
    VM_FLAGS_SUPERVISOR equ 3
    VM_FLAGS_USER equ 7
    PAGE_DIR_ADDRESS equ 0x80000
    PAGE_TABLE_ADDRESS equ PAGE_DIR_ADDRESS + 0x1000

    ; setup VM:
    call identity_map_first_mb	; page table for first mb created at PAGE_TABLE_ADDRESS

    call register_1mb_page_table	; register the table both to 0x0 and 0xC0000000
					; directory is at 0x10000
    ; now enable VM:

    mov eax, PAGE_DIR_ADDRESS
    mov cr3, eax			; load page directory

    mov eax, cr0
    or eax, 0x80000001
    mov cr0, eax			; enable VM

    ret

identity_map_first_mb:

    mov eax, 0
    mov ebx, PAGE_TABLE_ADDRESS	; first page table phys. address
    mov ecx, 0x0	; start mapping from this address

    add ecx, VM_FLAGS_SUPERVISOR	; supervisor flags (7 would be user)

    .loop:

        mov [ebx], ecx

        add eax, 1
	add ebx, 4
	add ecx, 0x1000
	cmp eax, 1024
        jl .loop

    ret

register_1mb_page_table:
    mov eax, PAGE_DIR_ADDRESS	; address of page directory
    
    mov [eax+0], dword PAGE_TABLE_ADDRESS + VM_FLAGS_SUPERVISOR ; register page table with supervisor flags
    mov [eax+0x300*4], dword PAGE_TABLE_ADDRESS + VM_FLAGS_SUPERVISOR ; register to 0xC0000000
    ret

reload_segments:
   ; Reload CS register containing code selector:
   JMP   0x08:.reload_CS ; 0x08 points at the new code selector
.reload_CS:
   ; Reload data segment registers:
   MOV   AX, 0x10 ; 0x10 points at the new data selector
   MOV   DS, AX
   MOV   ES, AX
   MOV   FS, AX
   MOV   GS, AX
   MOV   SS, AX
   RET

; Declare constants for the multiboot header.
MBALIGN  equ  1 << 0            ; align loaded modules on page boundaries
MEMINFO  equ  1 << 1            ; provide memory map
FLAGS    equ  MBALIGN | MEMINFO ; this is the Multiboot 'flag' field
MAGIC    equ  0x1BADB002        ; 'magic number' lets bootloader find the header
CHECKSUM equ -(MAGIC + FLAGS)   ; checksum of above, to prove we are multiboot

;section .multiboot
align 4
	dd MAGIC
	dd FLAGS
	dd CHECKSUM
