# PHASE 1
First, look at the assembly code

```objdump -d bomb```

Find the `phase_1` assembly code

```
0000000000400ee0 <phase_1>:
  400ee0:       48 83 ec 08             sub    $0x8,%rsp
  400ee4:       be 00 24 40 00          mov    $0x402400,%esi
  400ee9:       e8 4a 04 00 00          call   401338 <strings_not_equal>
  400eee:       85 c0                   test   %eax,%eax
  400ef0:       74 05                   je     400ef7 <phase_1+0x17>
  400ef2:       e8 43 05 00 00          call   40143a <explode_bomb>
  400ef7:       48 83 c4 08             add    $0x8,%rsp
  400efb:       c3                      ret
```
It compares your input with the preset string at memory address 0x402400.
So use gdb and print the strings located at 0x402400.

```(gdb) x/s 0x402400```

# PHASE2
```
0000000000400efc <phase_2>:
  400efc:       55                      push   %rbp
  400efd:       53                      push   %rbx
  400efe:       48 83 ec 28             sub    $0x28,%rsp
  400f02:       48 89 e6                mov    %rsp,%rsi
  400f05:       e8 52 05 00 00          call   40145c <read_six_numbers>
  400f0a:       83 3c 24 01             cmpl   $0x1,(%rsp)
  400f0e:       74 20                   je     400f30 <phase_2+0x34>
  400f10:       e8 25 05 00 00          call   40143a <explode_bomb>
  400f15:       eb 19                   jmp    400f30 <phase_2+0x34>
  400f17:       8b 43 fc                mov    -0x4(%rbx),%eax
  400f1a:       01 c0                   add    %eax,%eax
  400f1c:       39 03                   cmp    %eax,(%rbx)
  400f1e:       74 05                   je     400f25 <phase_2+0x29>
  400f20:       e8 15 05 00 00          call   40143a <explode_bomb>
  400f25:       48 83 c3 04             add    $0x4,%rbx
  400f29:       48 39 eb                cmp    %rbp,%rbx
  400f2c:       75 e9                   jne    400f17 <phase_2+0x1b>
  400f2e:       eb 0c                   jmp    400f3c <phase_2+0x40>
  400f30:       48 8d 5c 24 04          lea    0x4(%rsp),%rbx
  400f35:       48 8d 6c 24 18          lea    0x18(%rsp),%rbp
  400f3a:       eb db                   jmp    400f17 <phase_2+0x1b>
  400f3c:       48 83 c4 28             add    $0x28,%rsp
  400f40:       5b                      pop    %rbx
  400f41:       5d                      pop    %rbp
  400f42:       c3                      ret    
```
* Call `read_six_numbers`: Reads 6 integers and stores them on the stack (starting at `%rsp`, i.e. the 1th-6th number address: `(%rsp)`,
  `0x4 (%rsp)`, `0x8 (%rsp)`, `0x10 (%rsp)`, `0x14 (%rsp)`, `0x18 (%rsp)`) 
* `cmpl $0x1, (%rsp)`: Checks if the first operand is 1; otherwise, terminates.
* Initialization loop:
  * `%rbx` points to the second number (`0x4 (%rsp)`)
  * `%rbp` points to the end of the array (`0x18 (%rsp)`, i.e., after the sixth element)
* Loop (400f17):
  * Subtract the previous number (`%rbx-4`) and store the result in `%eax`.
  * Add once: `add %eax, %eax`(i.e., multiply by 2).
  * If it is not equal to the current value (`%rbx`), an explosion occurs.
  * Then `%rbx` moves to the next number until all six numbers are compared.


# PHASE 3
```
0000000000400f43 <phase_3>:
  400f43:       48 83 ec 18             sub    $0x18,%rsp
  400f47:       48 8d 4c 24 0c          lea    0xc(%rsp),%rcx
  400f4c:       48 8d 54 24 08          lea    0x8(%rsp),%rdx
  400f51:       be cf 25 40 00          mov    $0x4025cf,%esi
  400f56:       b8 00 00 00 00          mov    $0x0,%eax
  400f5b:       e8 90 fc ff ff          call   400bf0 <__isoc99_sscanf@plt>
  400f60:       83 f8 01                cmp    $0x1,%eax
  400f63:       7f 05                   jg     400f6a <phase_3+0x27>
  400f65:       e8 d0 04 00 00          call   40143a <explode_bomb>
  400f6a:       83 7c 24 08 07          cmpl   $0x7,0x8(%rsp)
  400f6f:       77 3c                   ja     400fad <phase_3+0x6a>
  400f71:       8b 44 24 08             mov    0x8(%rsp),%eax
  400f75:       ff 24 c5 70 24 40 00    jmp    *0x402470(,%rax,8)
  400f7c:       b8 cf 00 00 00          mov    $0xcf,%eax
  400f81:       eb 3b                   jmp    400fbe <phase_3+0x7b>
  400f83:       b8 c3 02 00 00          mov    $0x2c3,%eax
  400f88:       eb 34                   jmp    400fbe <phase_3+0x7b>
  400f8a:       b8 00 01 00 00          mov    $0x100,%eax
  400f8f:       eb 2d                   jmp    400fbe <phase_3+0x7b>
  400f91:       b8 85 01 00 00          mov    $0x185,%eax
  400f96:       eb 26                   jmp    400fbe <phase_3+0x7b>
  400f98:       b8 ce 00 00 00          mov    $0xce,%eax
  400f9d:       eb 1f                   jmp    400fbe <phase_3+0x7b>
  400f9f:       b8 aa 02 00 00          mov    $0x2aa,%eax
  400fa4:       eb 18                   jmp    400fbe <phase_3+0x7b>
  400fa6:       b8 47 01 00 00          mov    $0x147,%eax
  400fab:       eb 11                   jmp    400fbe <phase_3+0x7b>
  400fad:       e8 88 04 00 00          call   40143a <explode_bomb>
  400fb2:       b8 00 00 00 00          mov    $0x0,%eax
  400fb7:       eb 05                   jmp    400fbe <phase_3+0x7b>
  400fb9:       b8 37 01 00 00          mov    $0x137,%eax
  400fbe:       3b 44 24 0c             cmp    0xc(%rsp),%eax
  400fc2:       74 05                   je     400fc9 <phase_3+0x86>
  400fc4:       e8 71 04 00 00          call   40143a <explode_bomb>
  400fc9:       48 83 c4 18             add    $0x18,%rsp
  400fcd:       c3                      ret
```

## Check Input
`lea    0xc(%rsp), %rcx`   ; The 4th parameter points to the storage location of the second integer.

`lea    0x8(%rsp), %rdx`   ; The 3rd parameter points to the storage location of the second integer.

`mov    $0x4025cf, %esi`   ; Second parameter: Format string address

`mov    $0x0, %eax`        ; variable argument convention, indicating zero floating-point arguments

Print the string at address 0x4025cf

`(gdb) x/s 0x4025cf`

The result is `"%d %d"`, so we need to enter two integers separated by spaces.

## Check 1st argument
`cmpl   $0x7, 0x8(%rsp)`

* `0x8 (%rsp)` is the first integer (denoted as num1).
* `cmpl $0x7, num1`: unsigned comparison of num1 and 7. 
  * If num1 is negative (e.g., -1), its complement will be interpreted as a large unsigned number, 
  inevitably exceeding 7, which may trigger an overflow.
  * If num1 is 0 to 7 (inclusive), the program does not jump and continues execution.
* Therefore, num1 must be one of 0, 1, 2, 3, 4, 5, 6, or 7.

## Check 2nd argument
```
mov 0x8 (%rsp),%eax; Load num1 into%eax
jmp    *0x402470(,%rax,8)
```
* `mov 0x8 (%rsp),%eax`: `%eax` = num1 (32-bit, but the high 32 bits are implicitly cleared since the subsequent index is in 64-bit mode).
* `jmp *0x402470(,%rax,8)`：
  * This is an indirect memory jump.
  * Calculate `address = 0x402470 + %rax * 8`.
  * Use the 8-byte value at this address as the target address and jump to it.
* 0x402470 is the base address of a jump table containing eight 64-bit code addresses corresponding to case 0 through case 7.

View the redirect table content with GDB:

`(gdb) x/8gx 0x402470`

The output is:  
```
0x402470:       0x0000000000400f7c      0x0000000000400fb9
0x402480:       0x0000000000400f83      0x0000000000400f8a
0x402490:       0x0000000000400f91      0x0000000000400f98
0x4024a0:       0x0000000000400f9f      0x0000000000400fa6
```
It means that ：
* If num1 = 0, jump to 0x400f7c
* If num1 = 1, jump to 0x400fb9
* ...
* If num1 = 7, jump to 0x400fa6

Each branch (except case 1 which executes sequentially) sets `%eax` to a specific constant 
and then jumps to the unified exit address 0x400fbe.

```
cmp    0xc(%rsp), %eax
je     400fc9 <phase_3+0x86>
call   40143a <explode_bomb>
```
* `0xc (%rsp)` is the second integer (denoted as num2).
* `cmp num2, %eax`: compares num2 against the previously set branch target value `%eax`.
* If the value matches, jump to the function termination and return safely; otherwise, detonate the bomb.

Therefore, the second number must equal the preset constant value for the corresponding branch.

So there exist 7 valid combinations:
```
num1   %eax     num2 (decimal)
0	0xcf	207
1	0x137	311
2	0x2c3	707
3	0x100	256
4	0x185	389
5	0xce	206
6	0x2aa	682
7	0x147	327
```

# PHASE 4

```
000000000040100c <phase_4>:
40100c:       48 83 ec 18             sub    $0x18,%rsp
401010:       48 8d 4c 24 0c          lea    0xc(%rsp),%rcx
401015:       48 8d 54 24 08          lea    0x8(%rsp),%rdx
40101a:       be cf 25 40 00          mov    $0x4025cf,%esi
40101f:       b8 00 00 00 00          mov    $0x0,%eax
401024:       e8 c7 fb ff ff          call   400bf0 <__isoc99_sscanf@plt>
401029:       83 f8 02                cmp    $0x2,%eax
40102c:       75 07                   jne    401035 <phase_4+0x29>
40102e:       83 7c 24 08 0e          cmpl   $0xe,0x8(%rsp)
401033:       76 05                   jbe    40103a <phase_4+0x2e>
401035:       e8 00 04 00 00          call   40143a <explode_bomb>
40103a:       ba 0e 00 00 00          mov    $0xe,%edx
40103f:       be 00 00 00 00          mov    $0x0,%esi
401044:       8b 7c 24 08             mov    0x8(%rsp),%edi
401048:       e8 81 ff ff ff          call   400fce <func4>
40104d:       85 c0                   test   %eax,%eax
40104f:       75 07                   jne    401058 <phase_4+0x4c>
401051:       83 7c 24 0c 00          cmpl   $0x0,0xc(%rsp)
401056:       74 05                   je     40105d <phase_4+0x51>
401058:       e8 dd 03 00 00          call   40143a <explode_bomb>
40105d:       48 83 c4 18             add    $0x18,%rsp
401061:       c3                      ret
```
## Check Input
It behaves like phase3: 
* The formatted string address 0x4025cf matches Phase 3, containing `"%d %d"`, so we need to enter two integers as well.
* The parsed values are stored in `0x8 (%rsp)` and `0xc (%rsp)`.


## Check 1st argument
```
cmpl   $0xe, 0x8(%rsp)
```
* Use jbe (unsigned less than or equal to) to check if the first integer is <= 0xe (14).
* If the value is greater than 14 or negative (negative values are interpreted as greater than 14 without sign), the bomb will explode.
* The first number must be between 0 and 14 (inclusive).


```
mov $0xe, %edx; Third parameter: Right boundary = 14
mov $0x0, %esi; Second parameter: Left boundary = 0
mov 0x8 (%rsp), %edi; First parameter: The first number to be input
call   400fce <func4>
```
Call `func4(x, 0, 14)`, where x is the first integer input by the user.

```
test   %eax, %eax
jne    401058 <phase_4+0x4c>
```
Check if the return value of func4 is 0. If not 0, trigger bomb. 
Therefore, find the value of x that satisfies `func4(x,0,14) == 0`.

### func4

```
0000000000400fce <func4>:
  400fce:       48 83 ec 08             sub    $0x8,%rsp
  400fd2:       89 d0                   mov    %edx,%eax
  400fd4:       29 f0                   sub    %esi,%eax
  400fd6:       89 c1                   mov    %eax,%ecx
  400fd8:       c1 e9 1f                shr    $0x1f,%ecx
  400fdb:       01 c8                   add    %ecx,%eax
  400fdd:       d1 f8                   sar    %eax
  400fdf:       8d 0c 30                lea    (%rax,%rsi,1),%ecx
  400fe2:       39 f9                   cmp    %edi,%ecx
  400fe4:       7e 0c                   jle    400ff2 <func4+0x24>
  400fe6:       8d 51 ff                lea    -0x1(%rcx),%edx
  400fe9:       e8 e0 ff ff ff          call   400fce <func4>
  400fee:       01 c0                   add    %eax,%eax
  400ff0:       eb 15                   jmp    401007 <func4+0x39>
  400ff2:       b8 00 00 00 00          mov    $0x0,%eax
  400ff7:       39 f9                   cmp    %edi,%ecx
  400ff9:       7d 0c                   jge    401007 <func4+0x39>
  400ffb:       8d 71 01                lea    0x1(%rcx),%esi
  400ffe:       e8 cb ff ff ff          call   400fce <func4>
  401003:       8d 44 00 01             lea    0x1(%rax,%rax,1),%eax
  401007:       48 83 c4 08             add    $0x8,%rsp
  40100b:       c3                      ret
```

```
mov    %edx, %eax        ; eax = edx (high)
sub    %esi, %eax        ; eax = high - low
mov    %eax, %ecx        ; ecx = eax
shr    $0x1f, %ecx       ; Perform a 31-bit right shift onecx to extract the sign bit (0 or 1)
add    %ecx, %eax        ; eax +=ecx (if the original difference is negative, add 1 to correct rounding to zero)
sar    %eax              ; eax is shifted right by 1 bit, equivalent toeax = (high-low + (sign?1:0)) / 2
lea   (%rax,%rsi, 1),%ecx; where ecx = rax + rsi = mid + low, i.e., mid-point
```

This calculates the midpoint of an interval, equivalent to the C language `int mid = (high - low) / 2 + low;`.
Negative numbers are processed to ensure rounding to zero.

```
cmp %edi,%ecx           ; Compare mid with target (edi)
jle 400ff2 <func4+0x24> ; if mid <= target, jump
lea-0x1 (%rcx),%edx     ; otherwise edx = mid-1
call 400fce <func4>     ; recursive call of func4(target, low, mid-1)
add %eax,%eax           ; return valueeax = 2 * recursive return value
```

Branch 1: mid > target
* Update the right boundary high = mid-1
* Recursive call func4, return the value multiplied by 2

```
mov    $0x0, %eax               ; eax = 0
cmp    %edi,%ecx                ; Recompare mid with target
jge    401007 <func4+0x39>      ; if mid >= target, jump back (at this point eax=0)
lea    0x1 (%rcx), %esi         ; otherwise esi = mid + 1
call   400fce <func4>           ; recursive call of func4(target, mid+1, high)
lea    0x1(%rax,%rax, 1), %eax  ; eax = 2 * recursive return value + 1

```
Branch 2: mid <= target
* Set eax to 0 first (to prepare for equal conditions)
* If mid >= target, it means mid == target, and 0 is returned directly.
* Otherwise, mid < target:
  * Update the left boundary low = mid + 1
  * Recursive call. The return value is calculated as 2 * recursive return value + 1

C Language Equivalent Implementation of Func4
```
int func4(int target, int low, int high) {
    int mid = (high-low) / 2 + low; // Modified zero-division rounding method
    if (mid > target) {
        return 2 * func4(target, low, mid - 1);
    } else if (mid < target) {
        return 2 * func4(target, mid + 1, high) + 1;
    } else {
        return 0;
    }
}
```

Find the x value that returns 0
* The initial interval is [0,14], with the midpoint mid = (14-0)/2 + 0 = 7.
* If enter 7: mid == target, it immediately returns 0, meeting the requirement.
* If the input is < 7, execute the mid > target branch and recursively traverse to the left half region [0,6].
  * To make the total return value 0, there must be 2 * left recursive return values = 0, meaning the left recursion returns 0.
  * Midpoint of the left hemisphere = (6-0)/2 + 0 = 3.
    * If enter 3 and mid == target, return 0 to exit function call.
    * If the input is less than 3, recursively return to [0,2] with midpoint 1.
      * If enter 1, it returns 0. 
      * If enter 0, recursively return to [0,0] (midpoint 0) and return 0.
      * If enter 2, take the right branch 2 * recursive return value + 1, cannot get 0.
    * If the input is greater than 3 (e.g., 4, 5, 6), the system branches to the right and returns an odd value, which cannot be 0.
* Similarly, if the input exceeds 7, the system executes the mid < target branch, 
recursively traversing to the right half-region [8,14]. 
The return value must contain +1, resulting in an odd number and thus cannot be 0.

Therefore, only the following inputs can return 0: 0, 1, 3, and 7.

## Check 2nd argument
```
cmpl   $0x0, 0xc(%rsp)
je     40105d <phase_4+0x51>
call   40143a <explode_bomb>
```
Check if the second integer equals 0. If not 0, the bomb will explode.
The second number must be fixed to 0.

# PHASE 5
```
0000000000401062 <phase_5>:
  401062:       53                      push   %rbx
  401063:       48 83 ec 20             sub    $0x20,%rsp
  401067:       48 89 fb                mov    %rdi,%rbx
  40106a:       64 48 8b 04 25 28 00    mov    %fs:0x28,%rax
  401071:       00 00 
  401073:       48 89 44 24 18          mov    %rax,0x18(%rsp)
  401078:       31 c0                   xor    %eax,%eax
  40107a:       e8 9c 02 00 00          call   40131b <string_length>
  40107f:       83 f8 06                cmp    $0x6,%eax
  401082:       74 4e                   je     4010d2 <phase_5+0x70>
  401084:       e8 b1 03 00 00          call   40143a <explode_bomb>
  401089:       eb 47                   jmp    4010d2 <phase_5+0x70>
  40108b:       0f b6 0c 03             movzbl (%rbx,%rax,1),%ecx
  40108f:       88 0c 24                mov    %cl,(%rsp)
  401092:       48 8b 14 24             mov    (%rsp),%rdx
  401096:       83 e2 0f                and    $0xf,%edx
  401099:       0f b6 92 b0 24 40 00    movzbl 0x4024b0(%rdx),%edx
  4010a0:       88 54 04 10             mov    %dl,0x10(%rsp,%rax,1)
  4010a4:       48 83 c0 01             add    $0x1,%rax
  4010a8:       48 83 f8 06             cmp    $0x6,%rax
  4010ac:       75 dd                   jne    40108b <phase_5+0x29>
  4010ae:       c6 44 24 16 00          movb   $0x0,0x16(%rsp)
  4010b3:       be 5e 24 40 00          mov    $0x40245e,%esi
  4010b8:       48 8d 7c 24 10          lea    0x10(%rsp),%rdi
  4010bd:       e8 76 02 00 00          call   401338 <strings_not_equal>
  4010c2:       85 c0                   test   %eax,%eax
  4010c4:       74 13                   je     4010d9 <phase_5+0x77>
  4010c6:       e8 6f 03 00 00          call   40143a <explode_bomb>
  4010cb:       0f 1f 44 00 00          nopl   0x0(%rax,%rax,1)
  4010d0:       eb 07                   jmp    4010d9 <phase_5+0x77>
  4010d2:       b8 00 00 00 00          mov    $0x0,%eax
  4010d7:       eb b2                   jmp    40108b <phase_5+0x29>
  4010d9:       48 8b 44 24 18          mov    0x18(%rsp),%rax
  4010de:       64 48 33 04 25 28 00    xor    %fs:0x28,%rax
  4010e5:       00 00 
  4010e7:       74 05                   je     4010ee <phase_5+0x8c>
  4010e9:       e8 42 fa ff ff          call   400b30 <__stack_chk_fail@plt>
  4010ee:       48 83 c4 20             add    $0x20,%rsp
  4010f2:       5b                      pop    %rbx
  4010f3:       c3                      ret
```

```
call   40131b <string_length>
cmp    $0x6, %eax
je     4010d2 <phase_5+0x70>
call   40143a <explode_bomb>
jmp    4010d2 <phase_5+0x70>
```
* Calls string_length to calculate the length of the user-input string (result in `%eax`).
* `cmp $0x6, %eax` checks if the length equals 6. If not equal, detonate the bomb.
* If the value equals 6, proceed the code.

```
mov    $0x0, %eax
```
Set `%rax` to 0 as the loop index i, then jump to the first instruction 40108b in the loop body.

```
movzbl (%rbx, %rax, 1), %ecx
```
* `%rbx` is the starting address of the user-input string, and `%rax` is the current index i.
* Calculate the address `%rbx + %rax * 1` and read one byte (character).
* `movzbl`: zero-extend a byte to a 32-bit long word and stores it in `%ecx`.The lower 8 bits of `%ecx` represent the i-th character of the input string.

```
mov    %cl, (%rsp)
```
Write the ASCII code of the character (only the lower 8 bits `%cl`) to the low byte of the stack top (`%rsp`).

```
mov    (%rsp), %rdx
```
Then load the byte just written into the stack (along with the original 7-byte garbage on the stack) as a 64-bit value into `%rdx`. 
Since only the low bytes are considered, the high bits are irrelevant.

```
and    $0xf, %edx
```
Key operation: Perform a bitwise AND operation between `%edx` and 0xF (binary 1111), 
extracting the lower 4 bits of the character's ASCII code, 
with the result ranging from 0 to 15. This is equivalent to index = ch & 0xF.

```
movzbl 0x4024b0(%rdx), %edx
```
Using the low 4 bits of `%rdx` as the index, 
read a byte from the lookup table starting at address 0x4024b0 
and store it in `%edx` with zero extension.

```
mov    %dl, 0x10(%rsp, %rax, 1)
```
* Write the character retrieved from the lookup table (`%dl`) to the stack buffer starting at position `0x10 (%rsp)` with an offset of `%rax` (i.e., the i-th position).
* After the loop completes, a 6-byte string of table-lookup characters will be generated at `0x10 (%rsp)`.

```
add    $0x1, %rax
cmp    $0x6, %rax
jne    40108b <phase_5+0x29>
```
* Increment the index i by 1 and check if it reaches 6.
* If it is not 6, return to 40108b and proceed to process the next character.
* Execute the loop 6 times, converting the input string's 6 characters sequentially to generate a new 6-character string and pushing it onto the stack.

```
 movb   $0x0, 0x16(%rsp)
```
Write an empty character '\0' at the end of the string conversion stack (0x10 + 6 = 0x16) to form a valid C string.

```
mov    $0x40245e, %esi
lea    0x10(%rsp), %rdi
call   401338 <strings_not_equal>
```
* `%esi` points to the preset target string (address 0x40245e).
* `%rdi` points to the conversion string we just constructed on the stack.
* Call `strings_not_equal` to compare if they are the same. If the values differ, return a non-zero value and detonate the bomb.

View key data 
```
(gdb) x/s 0x40245e # Target string
(gdb) x/16c 0x4024b0 # Lookup table
```

Based on the data viewed:
* The target string address 0x40245e contains `"flyers"`
* Find the table address 0x4024b0 with content `"maduiersnfotvbyl"`

Index correspondence relationship
```
Target character Index in lookup table (decimal) Index (hexadecimal) Required ASCII low 4 bits
f	9	0x9	0x9
l	15	0xF	0xF
y	14	0xE	0xE
e	5	0x5	0x5
r	6	0x6	0x6
s	7	0x7	0x7
```
Construct input string
* We need 6 printable characters whose ASCII code's lower 4 bits match the specified index values.
* The most straightforward approach is to utilize the correspondence between lowercase letters a through o:
  * Index 0x9: Character 'i' (ASCII 0x69) or 'y' (ASCII 0x79), etc. 
  * Index 0xF: Character 'o' (ASCII 0x6F) or '_' (ASCII 0x5F), etc. 
  * Index 0xE: Character 'n' (ASCII 0x6E) or '~' (ASCII 0x7E), etc. 
  * Index 0x5: Character 'e' (ASCII 0x65) or 'u' (ASCII 0x75), etc. 
  * Index 0x6: Character 'f' (ASCII 0x66) or 'v' (ASCII 0x76), etc. 
  * Index 0x7: Character 'g' (ASCII 0x67) or 'w' (ASCII 0x77), etc.

A simple and memorable combination is `ionefg`.

# PHASE 6
```
00000000004010f4 <phase_6>:
  4010f4:       41 56                   push   %r14
  4010f6:       41 55                   push   %r13
  4010f8:       41 54                   push   %r12
  4010fa:       55                      push   %rbp
  4010fb:       53                      push   %rbx
  4010fc:       48 83 ec 50             sub    $0x50,%rsp
  401100:       49 89 e5                mov    %rsp,%r13
  401103:       48 89 e6                mov    %rsp,%rsi
  401106:       e8 51 03 00 00          call   40145c <read_six_numbers>
  40110b:       49 89 e6                mov    %rsp,%r14
  40110e:       41 bc 00 00 00 00       mov    $0x0,%r12d
  401114:       4c 89 ed                mov    %r13,%rbp
  401117:       41 8b 45 00             mov    0x0(%r13),%eax
  40111b:       83 e8 01                sub    $0x1,%eax
  40111e:       83 f8 05                cmp    $0x5,%eax
  401121:       76 05                   jbe    401128 <phase_6+0x34>
  401123:       e8 12 03 00 00          call   40143a <explode_bomb>
  401128:       41 83 c4 01             add    $0x1,%r12d
  40112c:       41 83 fc 06             cmp    $0x6,%r12d
  401130:       74 21                   je     401153 <phase_6+0x5f>
  401132:       44 89 e3                mov    %r12d,%ebx
  401135:       48 63 c3                movslq %ebx,%rax
  401138:       8b 04 84                mov    (%rsp,%rax,4),%eax
  40113b:       39 45 00                cmp    %eax,0x0(%rbp)
  40113e:       75 05                   jne    401145 <phase_6+0x51>
  401140:       e8 f5 02 00 00          call   40143a <explode_bomb>
  401145:       83 c3 01                add    $0x1,%ebx
  401148:       83 fb 05                cmp    $0x5,%ebx
  40114b:       7e e8                   jle    401135 <phase_6+0x41>
  40114d:       49 83 c5 04             add    $0x4,%r13
  401151:       eb c1                   jmp    401114 <phase_6+0x20>
  401153:       48 8d 74 24 18          lea    0x18(%rsp),%rsi
  401158:       4c 89 f0                mov    %r14,%rax
  40115b:       b9 07 00 00 00          mov    $0x7,%ecx
  401160:       89 ca                   mov    %ecx,%edx
  401162:       2b 10                   sub    (%rax),%edx
  401164:       89 10                   mov    %edx,(%rax)
  401166:       48 83 c0 04             add    $0x4,%rax
  40116a:       48 39 f0                cmp    %rsi,%rax
  40116d:       75 f1                   jne    401160 <phase_6+0x6c>
  40116f:       be 00 00 00 00          mov    $0x0,%esi
  401174:       eb 21                   jmp    401197 <phase_6+0xa3>
  401176:       48 8b 52 08             mov    0x8(%rdx),%rdx
  40117a:       83 c0 01                add    $0x1,%eax
  40117d:       39 c8                   cmp    %ecx,%eax
  40117f:       75 f5                   jne    401176 <phase_6+0x82>
  401181:       eb 05                   jmp    401188 <phase_6+0x94>
  401183:       ba d0 32 60 00          mov    $0x6032d0,%edx
  401188:       48 89 54 74 20          mov    %rdx,0x20(%rsp,%rsi,2)
  40118d:       48 83 c6 04             add    $0x4,%rsi
  401191:       48 83 fe 18             cmp    $0x18,%rsi
  401195:       74 14                   je     4011ab <phase_6+0xb7>
  401197:       8b 0c 34                mov    (%rsp,%rsi,1),%ecx
  40119a:       83 f9 01                cmp    $0x1,%ecx
  40119d:       7e e4                   jle    401183 <phase_6+0x8f>
  40119f:       b8 01 00 00 00          mov    $0x1,%eax
  4011a4:       ba d0 32 60 00          mov    $0x6032d0,%edx
  4011a9:       eb cb                   jmp    401176 <phase_6+0x82>
  4011ab:       48 8b 5c 24 20          mov    0x20(%rsp),%rbx
  4011b0:       48 8d 44 24 28          lea    0x28(%rsp),%rax
  4011b5:       48 8d 74 24 50          lea    0x50(%rsp),%rsi
  4011ba:       48 89 d9                mov    %rbx,%rcx
  4011bd:       48 8b 10                mov    (%rax),%rdx
  4011c0:       48 89 51 08             mov    %rdx,0x8(%rcx)
  4011c4:       48 83 c0 08             add    $0x8,%rax
  4011c8:       48 39 f0                cmp    %rsi,%rax
  4011cb:       74 05                   je     4011d2 <phase_6+0xde>
  4011cd:       48 89 d1                mov    %rdx,%rcx
  4011d0:       eb eb                   jmp    4011bd <phase_6+0xc9>
  4011d2:       48 c7 42 08 00 00 00    movq   $0x0,0x8(%rdx)
  4011d9:       00 
  4011da:       bd 05 00 00 00          mov    $0x5,%ebp
  4011df:       48 8b 43 08             mov    0x8(%rbx),%rax
  4011e3:       8b 00                   mov    (%rax),%eax
  4011e5:       39 03                   cmp    %eax,(%rbx)
  4011e7:       7d 05                   jge    4011ee <phase_6+0xfa>
  4011e9:       e8 4c 02 00 00          call   40143a <explode_bomb>
  4011ee:       48 8b 5b 08             mov    0x8(%rbx),%rbx
  4011f2:       83 ed 01                sub    $0x1,%ebp
  4011f5:       75 e8                   jne    4011df <phase_6+0xeb>
  4011f7:       48 83 c4 50             add    $0x50,%rsp
  4011fb:       5b                      pop    %rbx
  4011fc:       5d                      pop    %rbp
  4011fd:       41 5c                   pop    %r12
  4011ff:       41 5d                   pop    %r13
  401201:       41 5e                   pop    %r14
  401203:       c3                      ret
 ```

```
mov    %rsp, %r13
mov    %rsp, %rsi
call   40145c <read_six_numbers>
```
* `%r13` Save the stack pointer `%rsp` as the base address for subsequent integer accesses.
* The `%rsi` parameter, serving as the second argument to `read_six_numbers`, specifies the starting address of an array containing six integers 
(the first argument being the input string, already stored in `%rdi`).
* The `read_six_numbers` function stores six user-input integers in the first 24 bytes of (`%rsp`).

```
mov    %rsp, %r14
mov    $0x0, %r12d
```
* `%r14` also saves the stack top address, which will later serve as the base address for arrays.
* `%r12d` Used as an outer loop counter, initialized to 0.

## STEP1
Outer loop (traversing each input value):
```
mov    %r13, %rbp
mov    0x0(%r13), %eax
sub    $0x1, %eax
cmp    $0x5, %eax
```
* `%r13` points to the integer being checked (initially at the top of the stack, pointing to the first number).
* `mov 0x0 (%r13), %eax`: Reads the current integer into `%eax`.
* `sub $0x1, %eax`: decrement the value by 1.
* `cmp $0x5, %eax`: Checks if the result after decrementing by 1 is <= 5 (unsigned comparison jbe).
  * If the result after subtracting 1 is greater than 5, it indicates that the original number is < 1 or > 6, triggering the explosion.
* Therefore, each input must satisfy 1 <= x <= 6.

```
add $0x1,%r12d; Increment the outer counter by 1
cmp $0x6,%r12d; Have you checked all six numbers?
je 401153 <phase_6+0x5f>; if checked, proceed to the second stage
```
Inner loop (checks if the current number repeats with the next number):

```
mov %r12d, %ebx             ; ebx = inner loop index starting from r12d
movslq %ebx, %rax           ; Symbol extended to 64-bit index
mov (%rsp, %rax, 4),%eax     ; fetch the number of ebx element 
cmp %eax, 0x0 (%rbp)         ; compare with the current outer layer count
jne 401145 <phase_6+0x51>   ; if unequal, continue
call 40143a <explode_bomb>  ; if equal, detonate
add $0x1,%ebx               ; Increment the inner index by 1
cmp $0x5,%ebx               ; Has reached the 5th comparison (i.e., the last comparison)
jle 401135 <phase_6+0x41>   ; if ≤ 5, continue comparison
```
* `%rbp` still points to the outermost current integer.
* `%ebx` starts from `%r12d` and sequentially checks each subsequent integer to see if it equals the outer layer count.
If a repetition occurs, it triggers an explosion.
* Therefore, the six numbers must be distinct from each other.

```
add $0x4,%r13               ; Move the outer pointer to the next integer
jmp 401114 <phase_6+0x20>   ; Continue outer loop
```
Upon completion of the outer cycle, we obtained an arrangement of numbers from 1 to 6.

## STEP2
```
lea 0x18(%rsp),%rsi    ; rsi = array end address (after the 6th element)
mov %r14, %rax         ; rax = array starting address (%r14 saves stack top)
mov $0x7, %ecx         ; ecx = 7
```
loop body ：

```
mov %ecx, %edx                  ; edx = 7
sub (%rax),%edx                 ; edx = 7-current number
mov %edx, (%rax)                ; Save the result back to the original location
add $0x4, %rax                  ; move pointer to next integer
cmp %rsi, %rax                  ; Has it reached the end?
jne 401160 <phase_6+0x6c>       ; continue if not reached
```
Iterate through the 6 integers on the stack and execute x = 7-x one by one.

## STEP3

At this stage, the transformed digital values serve as "index" to retrieve corresponding node addresses from the linked list, 
which are then sequentially stored into the pointer array on the stack (starting at position 0x20 (%rsp)).

```
mov $0x0, %esi   ; esi = outer index i = 0
jmp 401197 <phase_6+0xa3>
```

Main loop entry (401197):
```
mov (%rsp, %rsi, 1), %ecx       ; where ecx = the current transformed value (denoted as v)
cmp $0x1, %ecx                  ; v <= 1?
jle 401183 <phase_6+0x8f>       ; if v <= 1, proceed with processing
```

If v > 1, traverse the linked list to locate the v-th node:
```
mov $0x1,%eax            ; eax = counter starting from 1
mov $0x6032d0, %edx      ; edx = address of the first node in the linked list
jmp 401176 <phase_6+0x82>
```

Inner loop (traversing the linked list):
```
mov 0x8(%rdx), %rdx         ; rdx = rdx->next
add $0x1,%eax               ; Increment the counter by 1
cmp %ecx,%eax               ; Has it reached v?
jne 401176 <phase_6+0x82>   ; if not reached, continue
jmp 401188 <phase_6+0x94>
```
Starting from node1, move the next pointer each time until the counter equals v. 
At this point, `%rdx` points to node v.

Handle cases where v ≤ 1 (directly take the first node):
```
mov $0x6032d0, %edx  ; edx = primary node address
```

Storage node pointer:
```
mov %rdx, 0x20(%rsp, %rsi, 2); store in pointer array
```
* `0x20 (%rsp)` is the starting address of the pointer array, with each element being 8 bytes.
* The `%rsi` represents the outermost index.

```
add $0x4, %rsi              ; i += 4 (byte offset, equivalent to index increment by 1)
cmp $0x18, %rsi             ; Have processed all 6 numbers?（6*4=24=0x18）
je  4011ab <phase_6+0xb7>   ; proceed to the next stage upon completion
401197:  ...Continue loop
```
After the loop completes, six node pointers are stored at stack address `0x20(%rsp)`, with their order determined by the transformed values.

## STEP4
```
mov 0x20 (%rsp), %rbx        ; rbx = first node pointer
lea 0x28 (%rsp), %rax        ; rax = address of the second pointer
lea 0x50 (%rsp), %rsi        ; rsi = pointer array end (boundary)
mov %rbx, %rcx               ; rcx = Address of the current node
```
Loop (rearrange next pointer):
```
mov (%rax), %rdx         ; rdx = pointer to the next node
mov %rdx, 0x8(%rcx)      ; next of current node = next node
add $0x8, %rax           ; rax points to the next pointer address
cmp %rsi, %rax           ; Has it reached the end?
je  4011d2 <phase_6+0xde>
mov %rdx, %rcx           ; Current node = Next node
jmp 4011bd <phase_6+0xc9>
```
* Traverse the pointer array and set the next pointer of each node to point to the next node in the array.
* Effect: Links nodes in the array in order.

```
movq $0x0, 0x8(%rdx); next of the last node = NULL
```

## STEP5
```
mov $0x5, %ebp     ; loop counter = 5 (comparing 5 times)
```

loop body:
```
mov 0x8 (%rbx), %rax        ; rax = next of current node
mov (%rax), %eax            ; eax = next->value
cmp %eax, (%rbx)            ; compares the current node value with the next node value
jge 4011ee <phase_6+0xfa>   ; If current >= next, continue; otherwise, explode
call 40143a <explode_bomb>
mov 0x8(%rbx), %rbx         ; rbx = rbx->next
sub $0x1,%ebp               ; Counter decrement by 1
jne 4011df <phase_6+0xeb>
```
* The linked list must be sorted in non-strict descending order by node value 
(the previous node is greater than or equal to the next node).
* If ascending order is detected, it will trigger the bomb.

Summary of overall logic
* Enter 6 distinct integers from 1 to 6. 
* Each number x is replaced with 7-x. 
* Based on the replaced value v, retrieve the v-th node from the linked list 
at address 0x6032d0 and store it in the pointer array in the input order.
* Re-link nodes in array order. 
* Check if the linked list node values are in descending order. If so, proceed.


View the first address 0x6032d0 and subsequent nodes in the linked list in GDB:
```
(gdb) x/12gx 0x6032d0
```
Output:
```
0x6032d0 <node1>:  0x000000010000014c    0x00000000006032e0
0x6032e0 <node2>:  0x00000002000000a8    0x00000000006032f0
0x6032f0 <node3>:  0x000000030000039c    0x0000000000603300
0x603300 <node4>:  0x00000004000002b3    0x0000000000603310
0x603310 <node5>:  0x00000005000001dd    0x0000000000603320
0x603320 <node6>:  0x00000006000001bb    0x0000000000000000
```
Each node occupies 16 bytes:
* The first 8 bytes: the lower 4 bytes are the node value (int), and the upper 4 bytes are the node number (usually ignored).
* The last 8 bytes: a pointer to the next node.
* Extract values for each node:
```
Node value (hexadecimal) Value (decimal)
node1	0x14c	332
node2	0x0a8	168
node3	0x39c	924
node4	0x2b3	691
node5	0x1dd	477
node6	0x1bb	443
```

Linked list node values must be sorted in descending order. Sort nodes by value in descending order:
```
Sort position node value
1	node3	924
2	node4	691
3	node5	477
4	node6	443
5	node1	332
6	node2	168
```
* Therefore, the linked list should be ordered by node numbers as follows: 3 -> 4 -> 5 -> 6 -> 1 -> 2.
* The six input numbers will first be converted to x = 7 - x. 
* Reverse input numbers:
```
x	3	4	5	6	1	2
7-x	4	3	2	1	6	5
```
Therefore, the six digits to be entered should be: `4 3 2 1 6 5`.




