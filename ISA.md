# Table of Contents
<blockquote>
	<ol>
		<li><a href="#intro">Introduction</a>
		<li><a href="#registers">Registers</a>
			<ol>
				<li><a href="#reg-st">Status Register</a></li>
			</ol>
		</li>
		<li><a href="#prog">Programs</a>
			<ol>
				<li><a href="#prog-meta">Metadata Section</a></li>
				<li><a href="#prog-symtab">Symbol Table</a></li>
				<li><a href="#prog-data">Data Section</a></li>
				<li><a href="#prog-code">Code Section</a></li>
			</ol>
		</li>
		<li><a href="#rings">Rings</a></li>
		<li><a href="#interrupts">Interrupts</a>
			<ol>
				<li><a href="#int-system"><code>SYSTEM</code></a>
				<li><a href="#int-timer"><code>TIMER</code></a>
				<li><a href="#int-protec"><code>PROTEC</code></a>
			</ol>
		</li>
		<li><a href="#format">Instruction Format</a>
			<ol>
				<li><a href="#format-r">R-Type Instructions</a></li>
				<li><a href="#format-i">I-Type Instructions</a></li>
				<li><a href="#format-j">J-Type Instructions</a></li>
			</ol>
		</li>
		<li><a href="#operations">Operations</a>
			<ol>
				<li><a href="#ops-math-r">Math (R-Types)</a>
					<ol>
						<li><a href="#op-add">Add</a> (<code>add</code>)</li>
						<li><a href="#op-sub">Subtract</a> (<code>sub</code>)</li>
						<li><a href="#op-mult">Multiply</a> (<code>mult</code>)</li>
						<li><a href="#op-multu">Multiply Unsigned</a> (<code>multu</code>)</li>
						<li><a href="#op-sll">Shift Left Logical</a> (<code>sll</code>)</li>
						<li><a href="#op-srl">Shift Right Logical</a> (<code>srl</code>)</li>
						<li><a href="#op-sra">Shift Right Arithmetic</a> (<code>sra</code>)</li>
						<li><a href="#op-mod">Modulo</a> (<code>mod</code>)</li>
					</ol>
				</li>
				<li><a href="#ops-logic-r">Logic (R-Types)</a>
					<ol>
						<li><a href="#op-and">Bitwise AND</a>    (<code>and</code>)</li>
						<li><a href="#op-nand">Bitwise NAND</a>  (<code>nand</code>)</li>
						<li><a href="#op-nor">Bitwise NOR</a>    (<code>nor</code>)</li>
						<li><a href="#op-not">Bitwise NOT</a>    (<code>not</code>)</li>
						<li><a href="#op-or">Bitwise OR</a>      (<code>or</code>)</li>
						<li><a href="#op-xnor">Bitwise XNOR</a>  (<code>xnor</code>)</li>
						<li><a href="#op-xor">Bitwise XOR</a>    (<code>xor</code>)</li>
						<li><a href="#op-land">Logical AND</a>   (<code>land</code>)</li>
						<li><a href="#op-lnand">Logical NAND</a> (<code>lnand</code>)</li>
						<li><a href="#op-lnor">Logical NOR</a>   (<code>lnor</code>)</li>
						<li><a href="#op-lnot">Logical NOT</a>   (<code>lnot</code>)</li>
						<li><a href="#op-lor">Logical OR</a>     (<code>lor</code>)</li>
						<li><a href="#op-lxnor">Logical XNOR</a> (<code>lxnor</code>)</li>
						<li><a href="#op-lxor">Logical XOR</a>   (<code>lxor</code>)</li>
					</ol>
				</li>
				<li><a href="#ops-math-i">Math (I-Types)</a>
					<ol>
						<li><a href="#op-addi">Add Immediate</a> (<code>addi</code>)</li>
						<li><a href="#op-subi">Subtract Immediate</a> (<code>subi</code>)</li>
						<li><a href="#op-multi">Multiply Immediate</a> (<code>multi</code>)</li>
						<li><a href="#op-multui">Multiply Unsigned Immediate</a> (<code>multui</code>)</li>
						<li><a href="#op-slli">Shift Left Logical Immediate</a> (<code>slli</code>)</li>
						<li><a href="#op-srli">Shift Right Logical Immediate</a> (<code>srli</code>)</li>
						<li><a href="#op-srai">Shift Right Arithmetic Immediate</a> (<code>srai</code>)</li>
						<li><a href="#op-modi">Modulo Immediate</a> (<code>modi</code>)</li>
					</ol>
				</li>
				<li><a href="#ops-logic-i">Logic (I-Types)</a>
					<ol>
						<li><a href="#op-andi">Bitwise AND Immediate</a>   (<code>andi</code>)</li>
						<li><a href="#op-nandi">Bitwise NAND Immediate</a> (<code>nandi</code>)</li>
						<li><a href="#op-nori">Bitwise NOR Immediate</a>   (<code>nori</code>)</li>
						<li><a href="#op-ori">Bitwise OR Immediate</a>     (<code>ori</code>)</li>
						<li><a href="#op-xnori">Bitwise XNOR Immediate</a> (<code>xnori</code>)</li>
						<li><a href="#op-xori">Bitwise XOR Immediate</a>   (<code>xori</code>)</li>
					</ol>
				</li>
				<li><a href="#ops-comp-r">Comparisons (R-Types)</a>
					<ol>
						<li><a href="#op-cmp">Compare</a> (<code>cmp</code>)</li>
						<li><a href="#op-sl">Set on Less Than</a> (<code>sl</code>)</li>
						<li><a href="#op-sle">Set on Less Than or Equal</a> (<code>sle</code>)</li>
						<li><a href="#op-seq">Set on Equal</a> (<code>seq</code>)</li>
						<li><a href="#op-sge">Set on Greater Than or Equal</a> (<code>sge</code>)</li>
						<li><a href="#op-sg">Set on Greater Than</a> (<code>sg</code>)</li>
						<li><a href="#op-slu">Set on Less Than Unsigned</a> (<code>slu</code>)</li>
						<li><a href="#op-sleu">Set on Less Than or Equal Unsigned</a> (<code>sleu</code>)</li>
						<li><a href="#op-sgeu">Set on Greater Than or Equal Unsigned</a> (<code>sgeu</code>)</li>
						<li><a href="#op-sgu">Set on Greater Than Unsigned</a> (<code>sgu</code>)</li>
					</ol>
				</li>
				<li><a href="#ops-comp-i">Comparisons (I-Types)</a>
					<ol>
						<li><a href="#op-cmpi">Compare Immediate</a> (<code>cmpi</code>)</li>
						<li><a href="#op-sli">Set on Less Than Immediate</a> (<code>sli</code>)</li>
						<li><a href="#op-slei">Set on Less Than or Equal Immediate</a> (<code>slei</code>)</li>
						<li><a href="#op-seqi">Set on Equal Immediate</a> (<code>seqi</code>)</li>
						<li><a href="#op-sgei">Set on Greater Than or Equal Immediate</a> (<code>sgei</code>)</li>
						<li><a href="#op-sgi">Set on Greater Than Immediate</a> (<code>sgi</code>)</li>
						<li><a href="#op-slui">Set on Less Than Unsigned Immediate</a> (<code>slui</code>)</li>
						<li><a href="#op-sleui">Set on Less Than or Equal Unsigned Immediate</a> (<code>sleui</code>)</li>
						<li><s><a href="#op-sgeui">Set on Greater Than or Equal Unsigned Immediate</a> (<code>sgeui</code>)</s></li>
						<li><s><a href="#op-sgui">Set on Greater Than Unsigned Immediate</a> (<code>sgui</code>)</s></li>
					</ol>
				</li>
				<li><a href="#ops-jump-j">Jumps (J-Types)</a>
					<ol>
						<li><a href="#op-j">Jump</a> (<code>j</code>)</li>
						<li><a href="#op-jc">Jump Conditional</a> (<code>jc</code>)</li>
					</ol>
				</li>
				<li><a href="#ops-jump-r">Jumps (R-Types)</a>
					<ol>
						<li><a href="#op-jr">Jump to Register</a> (<code>jr</code>)</li>
						<li><a href="#op-jrc">Jump to Register Conditional</a> (<code>jrc</code>)</li>
						<li><a href="#op-jrl">Jump to Register and Link</a> (<code>jrl</code>)</li>
						<li><a href="#op-jrlc">Jump to Register and Link Conditional</a> (<code>jrlc</code>)</li>
					</ol>
				</li>
				<li><a href="#ops-mem-r">Memory (R-Types)</a>
					<ol>
						<li><a href="#op-c">Copy</a>           (<code>c</code>)</li>
						<li><a href="#op-l">Load</a>           (<code>l</code>)</li>
						<li><a href="#op-s">Store</a>          (<code>s</code>)</li>
						<li><a href="#op-cb">Copy Byte</a>     (<code>cb</code>)</li>
						<li><a href="#op-lb">Load Byte</a>     (<code>lb</code>)</li>
						<li><a href="#op-sb">Store Byte</a>    (<code>sb</code>)</li>
						<li><a href="#op-ch">Copy Halfword</a>     (<code>ch</code>)</li>
						<li><a href="#op-lh">Load Halfword</a>     (<code>lh</code>)</li>
						<li><a href="#op-sh">Store Halfword</a>    (<code>sh</code>)</li>
						<li><a href="#op-spush">Stack Push</a> (<code>spush</code>)</li>
						<li><a href="#op-spop">Stack Pop</a>   (<code>spop</code>)</li>
					</ol>
				</li>
				<li><a href="#ops-mem-i">Memory (I-Types)</a>
					<ol>
						<li><a href="#op-li">Load Immediate</a> (<code>li</code>)</li>
						<li><a href="#op-si">Store Immediate</a> (<code>si</code>)</li>
						<li><a href="#op-lni">Load Indirect Immediate</a> (<code>lni</code>)</li>
						<li><a href="#op-lbi">Load Byte Immediate</a> (<code>lbi</code>)</li>
						<li><a href="#op-sbi">Store Byte Immediate</a> (<code>sbi</code>)</li>
						<li><a href="#op-lbni">Load Byte Indirect Immediate</a> (<code>lbni</code>)</li>
						<li><a href="#op-set">Set</a> (<code>set</code>)</li>
						<li><a href="#op-lui">Load Upper Immediate</a> (<code>lui</code>)</li>
					</ol>
				</li>
				<li><a href="#ops-special">Special Instructions</a>
					<ol>
						<li><a href="#op-ext">External</a> (<code>ext</code>)</li>
						<li><a href="#op-int">Interrupt</a> (<code>int</code>)</li>
						<li><a href="#op-rit">Register Interrupt Table</a> (<code>rit</code>)</li>
						<li><a href="#op-time">Set Timer</a> (<code>time</code>)</li>
						<li><a href="#op-timei">Set Timer Immediate</a> (<code>timei</code>)</li>
						<li><a href="#op-ring">Change Ring</a> (<code>ring</code>)</li>
					</ol>
				</li>
				<li><a href="#ops-pseudo">Pseudoinstructions</a>
					<ol>
						<li><a href="#op-mv">Move</a> (<code>mv</code>)</li>
						<li><a href="#op-ret">Return</a> (<code>ret</code>)</li>
						<li><a href="#op-push">Push</a> (<code>push</code>)</li>
						<li><a href="#op-pop">Pop</a> (<code>pop</code>)</li>
						<li><a href="#op-jeq">Jump if Equal</a> (<code>jeq</code>)</li>
					</ol>
				</li>
			</ol>
		</li>
		<li><a href="#ext">Externals</a>
			<ol>
				<li><a href="#ext-printr">Print Register</a> (<code>printr</code>)</li>
				<li><a href="#ext-halt">Halt</a> (<code>halt</code>)</li>
				<li><a href="#ext-eval">Evaluate String</a> (<code>eval</code>)</li>
				<li><a href="#ext-prc">Print Character</a> (<code>prc</code>)</li>
				<li><a href="#ext-prd">Print Decimal</a> (<code>prd</code>)</li>
				<li><a href="#ext-prx">Print Hexadecimal</a> (<code>prx</code>)</li>
			</ol>
		</li>
	</ol>
</blockquote>

# <a name="intro"></a>Introduction

The VM emulates WhySA, a custom RISC instruction set that may or may not actually be theoretically implementable as real hardware. This instruction set has 64-bit word length, but the memory addressability is 32 bits.

# <a name="registers"></a>Registers
There are 128 registers. Their purposes are pretty much stolen from MIPS:

| Number   | Name         | Purpose                                     |
|----------|--------------|---------------------------------------------|
| 0        | `$0`         | Always contains zero.                       |
| 1        | `$g`         | Global area pointer (start of data segment) |
| 2        | `$sp`        | Stack pointer.                              |
| 3        | `$fp`        | Frame pointer.                              |
| 4        | `$rt`        | Return address.                             |
| 5        | `$lo`        | Stores the lower half of a mult result.     |
| 6        | `$hi`        | Stores the upper half of a mult result.     |
| 7–22     | `$r0`–`$rf`  | Contains return values.                     |
| 23–38    | `$a0`–`$af`  | Contains arguments for subroutines.         |
| 39–61    | `$t0`–`$t16` | Temporary registers.                        |
| 62–84    | `$s0`–`$s16` | Saved registers.                            |
| 85–100   | `$k0`–`$kf`  | Kernel registers.                           |
| 101      | `$st`        | Status register.                            |
| 102–117  | `$m0`–`$mf`  | Reserved for use by the assembler.          |
| 118–121  | `$f0`–`$f3`  | Floating point return values.               |
| 122–127  | `$e0`–`$e5`  | Contains data about exceptions.             |

## <a name="reg-st"></a>Status Register
The `$st` register stores flag bits. Currently, this includes the results of arithmetic instructions. In ascending order of significance, these are:
<ol>
	<li><code>Z</code> (zero):
		Whether the last arithmetic result was zero (for <code>cmp</code>/<code>cmpi</code>, whether the compared values are equal)
	</li>
	<li><code>N</code> (negative):
		Whether the result of the last arithmetic result was negative (for <code>cmp</code>/<code>cmpi</code>, whether the left value was less than the right value)
	</li>
	<li><code>C</code> (carry):
		Whether the result of an addition was truncated. (Currently not implemented for subtraction.)
	</li>
	<li><code>O</code> (overflow):
		Whether the addition of two positive signed numbers had a negative result due to overflow.
	</li>
</ol>

These status numbers are used in conditional branches, but they can also be accessed by programs. Writing to the `$st` register is possible, but strange behavior may occur as a result.

# <a name="prog"></a>Programs

Programs are divided into four sections: metadata, symbol table, code and data. The <a href="#prog-meta">metadata section</a> contains information about the program. The <a href="#prog-symtab">symbol table</a> contains the names, locations and types of all visible symbols. The <a href="#prog-code">code section</a> consists of executable code. The <a href="#prog-data">data section</a> contains data, unsurprisingly.

## <a name="prog-meta"></a>Metadata Section
The metadata section is a block of data at the beginning of the program that contains the beginning addresses of the other sections. The first value in this section represents the beginning address of the symbol table, and is therefore equivalent to the size of the metadata section.

* `0x00`: Address of the beginning of the [symbol table](#prog-symtab).
* `0x01`: Address of the beginning of the [code section](#prog-code).
* `0x02`: Address of the beginning of the [data section](#prog-data).
* `0x03`: Total size of the program.
* `0x04`–`0x05`: ORCID of the author (represented with ASCII).
* `0x06`–`...`: Program name, version string and author name of the program (represented with null-terminated ASCII).
	* Example: given a program name `"Example"`, version string `"4"` and author name `"Kai Tamkun"`, this will be `0x4578616d706c6500` `0x34004b6169205461` `0x6d6b756e00000000`.

### Assembler syntax
<pre>
#meta
author: "Kai Tamkun"
orcid: "0000-0001-7405-6654"
name: "Example"
version: "4"
</pre>

## <a name="prog-symtab"></a>Symbol Table Section
The symbol table contains a list of debug symbols. Each debug symbol is assigned a numeric ID equal to the CRC64 hash of its name. Each symbol is encoded in the table as a variable number of words. The upper half of the first is the numeric ID. The next 16 bits comprise symbol type, while the lowest 16 bits comprise the length (in words) of the symbol's name. The second is the symbol's offset (its position relative to the start of the code section). The remaining words encode the symbol's name. The length of the name in words is equal to the ceiling of the 1/8 of the symbol name's length in characters. Any extra bytes in the last word are null.

## <a name="prog-code"></a>Code Section
The code section consists of executable code. This is the only section of the code that the program counter is expected to point to.

## <a name="prog-data"></a>Data Section
The data section contains non-code program data. Execution is not expected to occur in the data section, but there is no error checking to prevent it.

### Assembler syntax
Variables and their values are declared (once again) with JSON-like markup:

<pre>
#data
some_string: "this is an example."
some_number: 42
</pre>

# <a name="rings"></a>Rings
WhySA has support for four protection rings, just like x86. Ring 0 is called kernel mode and ring 3 is called user mode; rings 1 and 2 are currently unused. 

# <a name="interrupts"></a>Interrupts
Interrupts can be triggered by software or by the VM itself. Whenever an interrupt is triggered, `$e0` is set to the address of the next instruction (i.e., 8 bytes after the current program counter) and `$e1` is set to the current <a href="#rings">ring</a> at the time the interrupt occurred. Interrupt handlers are expected to deal with properly resuming normal program execution after finishing up.

## <a name="int-system"></a>1: `SYSTEM`
The `SYSTEM` interrupt is a software-triggered interrupt handled by the operating system. It can be called from any ring and causes a switch to kernel mode.

## <a name="int-timer"></a>2: `TIMER`
The `TIMER` interrupt is a hardware-triggered interrupt caused when the hardware timer expires. It's exclusive to kernel mode. This is to prevent unprivileged code from interfering with schedulers; operating systems can implement their own mechanisms to expose timer functionality to lower-privileged code.

## <a name="int-protec"></a>3: `PROTEC`
The `PROTEC` interrupt is a hardware-triggered interrupt caused when a called instruction attempts to do something not possible within the current <a href="#rings">ring</a>. This switches to kernel mode.


# <a name="format"></a>Instruction Format
Like much of this instruction set, the formatting for instructions is copied from MIPS with a few modifications (for example, instructions are 64 bits long in this instruction set, as opposed to 32 for MIPS64).

## <a name="format-r"></a>R-Type Instructions
R-type instructions perform computations with multiple registers.

| Range       | 63–52 (12)  | 51–45 (7) | 44–38 (7) | 37–31 (7) | 30–18 (13) | 17–14 (4)  | 13-12 (2)    | 11–0 (12) |
|------------:|:-----------:|:---------:|:---------:|:---------:|:----------:|:----------:|:------------:|:---------:|
| **Purpose** | Opcode      | rt        | rs        | rd        | Unused     | Conditions | Linker flags | Function  |

## <a name="format-i"></a>I-Type Instructions
I-type instructions perform computations with registers and an immediate value.

| Range       | 63–52 (12) | 51–48 (4)  | 47–46 (2)    | 45–39 (7) | 38–32 (7) | 31–0 (32)       |
|------------:|:----------:|:----------:|:------------:|:---------:|:---------:|:---------------:|
| **Purpose** | Opcode     | Conditions | Linker flags | rs        | rd        | Immediate Value |

## <a name="format-j"></a>J-Type Instructions
J-type instructions point the program counter to a given address under certain circumstances.

|   Range | 63–52 (12) | 51–45 (7) | 44   | 43–38 (6) | 37–34 (4)  | 33–32 (2)    | 31–0 (32) |
|--------:|:----------:|:---------:|:----:|:---------:|:----------:|:------------:|:---------:|
| Purpose | Opcode     | rs        | Link | Unused    | Conditions | Linker flags | Address   |

If the link bit is set, the current value of the program counter will be stored in `$rt`, the return address register.

## <a name="condbits"></a>Condition Bits
For operations that support conditions, the condition bits indicate what combination of [ALU flags](#reg-st)
are required for the operation to occur.

<ul>
	<li><code>0xxx</code>: Conditions disabled</li>
	<li><code>1000</code>: Positive (<code>!N & !Z</code>)</li>
	<li><code>1001</code>: Negative</li>
	<li><code>1010</code>: Zero</li>
	<li><code>1011</code>: Nonzero</li>
</ul>

# <a name="operations"></a>Operations

## <a name="ops-math-r"></a>Math (R-Types)

### <a name="op-add"></a>Add (`add`)
> `$rs + $rt -> $rd` or `$rd += $rt`  
> `000000000001` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `000000000000`

Adds the values in `rs` and `rt` and stores the result in `rd`.

### <a name="op-sub"></a>Subtract (`sub`)
> `$rs - $rt -> $rd` or `$rd -= $rt`  
> `000000000001` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `000000000001`

Subtracts the value in `rt` from the value in `rs` and stores the result in `rd`.

### <a name="op-mult"></a>Multiply (`mult`)
> `$rs * $rt`  
> `000000000001` `ttttttt` `sssssss` `0000000` `0000000000000` `......` `000000000010`

Multiplies the value in `rs` by the value in `rt` and stories the upper half in [`HI`](#hi-lo) and the lower half in [`LO`](#hi-lo).

### <a name="op-multu"></a>Multiply Unsigned (`multu`)
> `$rs * $rt /u`  
> `000000000001` `ttttttt` `sssssss` `0000000` `0000000000000` `......` `000000000101`

Multiplies the value in `rs` by the value in `rt` (treating both as unsigned values) and stories the upper half in [`HI`](#hi-lo) and the lower half in [`LO`](#hi-lo).

### <a name="op-sll"></a>Shift Left Logical (`sll`)
> `$rs << $rt -> $rd` or `$rd <<= $rt`  
> `000000000001` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `000000000110`

Logically shifts the value in `rs` to the left by a number of bits equal to the value in `rt` and stores the result in `rd`.

### <a name="op-srl"></a>Shift Right Logical (`srl`)
> `$rs >>> $rt -> $rd` or `$rd >>>= $rt`  
> `000000000001` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `000000000111`

Logically shifts the value in `rs` to the right by a number of bits equal to the value in `rt` and stores the result in `rd`.

### <a name="op-sra"></a>Shift Right Arithmetic (`sra`)
> `$rs >> $rt -> $rd` or `$rd >>= $rt`  
> `000000000001` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `000000001000`

Arithmetically shifts the value in `rs` to the left by a number of bits equal to the value in `rt` and stores the result in `rd`.

### <a name="op-mod"></a>Modulo (`mod`)
> `$rs % $rt -> $rd` or `$rs %= $rt`  
> `000000000001` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `000000001001`

Computes the `rt`-modulo of `rs` and stores the result in `rd`.

## <a name="ops-logic-r"></a>Logic (R-Types)

### <a name="op-and"></a>Bitwise AND (`and`)
> `$rs & $rt -> $rd` or `$rd &= $rt`  
> `000000000010` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `000000000000`
   
Computes the bitwise AND of `rs` and `rt` and stores the result in `rd`.

### <a name="op-nand"></a>Bitwise NAND (`nand`)
> `$rs ~& $rt -> $rd` or `$rd ~&= $rt`  
> `000000000010` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `000000000001`
   
Computes the bitwise NAND of `rs` and `rt` and stores the result in `rd`.

### <a name="op-nor"></a>Bitwise NOR (`nor`)
> `$rs ~| $rt -> $rd` or `$rd ~|= $rt`  
> `000000000010` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `000000000010`
   
Computes the bitwise NOR of `rs` and `rt` and stores the result in `rd`.

### <a name="op-not"></a>Bitwise NOT (`not`)
> `~$rs -> $rd` or `~$rs.`  
> `000000000010` `0000000` `sssssss` `ddddddd` `0000000000000` `......` `000000000011`
   
Computes the bitwise NOT of `rs` and stores the result in `rd`.

### <a name="op-or"></a>Bitwise OR (`or`)
> `$rs | $rt -> $rd` or `$rd |= $rt`  
> `000000000010` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `000000000100`
   
Computes the bitwise OR of `rs` and `rt` and stores the result in `rd`.

### <a name="op-xnor"></a>Bitwise XNOR (`xnor`)
> `$rs ~x $rt -> $rd` or `$rd ~x= $rt`  
> `000000000010` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `000000000101`
   
Computes the bitwise XNOR of `rs` and `rt` and stores the result in `rd`.

### <a name="op-xor"></a>Bitwise XOR (`xor`)
> `$rs x $rt -> $rd` or `$rd x= $rt`  
> `000000000010` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `000000000110`

Computes the bitwise XOR of `rs` and `rt` and stores the result in `rd`.

### <a name="op-land"></a>Logical AND (`land`)
> `$rs && $rt -> $rd` or `$rd &&= $rt`  
> `000000000010` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `000000001000`
   
Computes the logical AND of `rs` and `rt` and stores the result in `rd`.

### <a name="op-lnand"></a>Logical NAND (`lnand`)
> `$rs !&& $rt -> $rd` or `$rd !&&= $rt`  
> `000000000010` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `000000001001`
   
Computes the logical NAND of `rs` and `rt` and stores the result in `rd`.

### <a name="op-lnor"></a>Logical NOR (`lnor`)
> `$rs !|| $rt -> $rd` or `$rd !||= $rt`  
> `000000000010` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `000000001010`
   
Computes the logical NOR of `rs` and `rt` and stores the result in `rd`.

### <a name="op-lnot"></a>Logical NOT (`lnot`)
> `!$rs -> $rd` or `!$rs.`  
> `000000000010` `0000000` `sssssss` `ddddddd` `0000000000000` `......` `000000001011`
   
Computes the logical NOT of `rs` and stores the result in `rd`.

### <a name="op-lor"></a>Logical OR (`lor`)
> `$rs || $rt -> $rd` or `$rd ||= $rt`  
> `000000000010` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `000000001100`
   
Computes the logical OR of `rs` and `rt` and stores the result in `rd`.

### <a name="op-lxnor"></a>Logical XNOR (`lxnor`)
> `$rs !xx $rt -> $rd` or `$rd !xx= $rt`  
> `000000000010` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `000000001101`
   
Computes the logical XNOR of `rs` and `rt` and stores the result in `rd`.

### <a name="op-lxor"></a>Logical XOR (`lxor`)
> `$rs xx $rt -> $rd` or `$rd xx= $rt`  
> `000000000010` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `000000001110`

Computes the logical XOR of `rs` and `rt` and stores the result in `rd`.

## <a name="ops-math-i"></a>Math (I-Types)

### <a name="op-addi"></a>Add Immediate (`addi`)
> `$rs + imm -> $rd` or `$rd += imm`  
> `000000000011` `......` `sssssss` `ddddddd` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

Adds the value in `rs` and a constant and stores the result in `rd`.

### <a name="op-subi"></a>Subtract Immediate (`subi`)
> `$rs - imm -> $rd` or `$rd -= imm`  
> `000000000100` `......` `sssssss` `ddddddd` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

Subtracts a constant from the value in `rs` and stores the result in `rd`.

### <a name="op-multi"></a>Multiply Immediate (`multi`)
> `$rs * imm`  
> `000000000101` `......` `sssssss` `0000000` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

Multiplies the value in `rs` by a constant and stories the upper half in [`HI`](#hi-lo) and the lower half in [`LO`](#hi-lo).

### <a name="op-multui"></a>Multiply Unsigned Immediate (`multui`)
> `$rs * imm /u`  
> `000000011000` `......` `sssssss` `0000000` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

Multiplies the value in `rs` by a constant (treating both as unsigned values) and stories the upper half in [`HI`](#hi-lo) and the lower half in [`LO`](#hi-lo).

### <a name="op-slli"></a>Shift Left Logical Immediate (`slli`)
> `$rs << imm -> $rd` or `$rd <<= imm`  
> `000000100010` `......` `sssssss` `ddddddd` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

Logically shifts the value in `rs` to the left by a number of bits equal to `imm` and stores the result in `rd`.

### <a name="op-srli"></a>Shift Right Logical Immediate (`srli`)
> `$rs >>> imm -> $rd` or `$rd >>>= imm`  
> `000000100011` `......` `sssssss` `ddddddd` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

Logically shifts the value in `rs` to the right by a number of bits equal to `imm` and stores the result in `rd`.

### <a name="op-srai"></a>Shift Right Arithmetic Immediate (`srai`)
> `$rs >> imm -> $rd` or `$rd >>= imm`  
> `000000100100` `......` `sssssss` `ddddddd` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

Arithmetically shifts the value in `rs` to the right by a number of bits equal to `imm` and stores the result in `rd`.

### <a name="op-modi"></a>Modulo Immediate (`modi`)
> `$rs % imm -> $rd` or `$rd %= imm`  
> `000000011110` `......` `sssssss` `ddddddd` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

Computes the `imm`-modulo of `rs` and stores the result in `rd`.

## <a name="ops-logic-i"></a>Logic (I-Types)

### <a name="op-andi"></a>Bitwise AND Immediate (`andi`)
> `$rs & imm -> $rd` or `$rd &= imm`  
> `000000000110` `......` `sssssss` `ddddddd` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

Computes the bitwise AND of `rs` and a constant and stores the result in `rd`.

### <a name="op-nandi"></a>Bitwise NAND Immediate (`nandi`)
> `$rs ~& imm -> $rd` or `$rd ~&= imm`  
> `000000000111` `......` `sssssss` `ddddddd` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

Computes the bitwise NAND of `rs` and a constant and stores the result in `rd`.

### <a name="op-nori"></a>Bitwise NOR Immediate (`nori`)
> `$rs ~| imm -> $rd` or `$rd ~|= imm`  
> `000000001000` `......` `sssssss` `ddddddd` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`
   
Computes the bitwise NOR of `rs` and a constant and stores the result in `rd`.

### <a name="op-ori"></a>Bitwise OR Immediate (`ori`)
> `$rs | imm -> $rd` or `$rd |= imm`  
> `000000001001` `......` `sssssss` `ddddddd` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`
   
Computes the bitwise OR of `rs` and a constant and stores the result in `rd`.

### <a name="op-xnori"></a>Bitwise XNOR Immediate (`xnori`)
> `$rs ~x imm -> $rd` or `$rd ~x= imm`  
> `000000001010` `......` `sssssss` `ddddddd` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`
   
Computes the bitwise XNOR of `rs` and a constant and stores the result in `rd`.

### <a name="op-xori"></a>Bitwise XOR Immediate (`xori`)
> `$rs x imm -> $rd` or `$rd x= imm`  
> `000000001011` `......` `sssssss` `ddddddd` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

Computes the bitwise XOR of `rs` and a constant and stores the result in `rd`.

## <a name="ops-comp-r"></a>Comparisons (R-Types)

### <a name="op-cmp"></a>Compare (`cmp`)
> `$rs ~ $rt`  
> `000000001110` `ttttttt` `sssssss` `.......` `0000000000000` `......` `000000000101`

Compares the value in `rs` to the value in `rt` and updates the [status register](#reg-st).

### <a name="op-sl"></a>Set on Less Than (`sl`)
> `$rs < $rt -> $rd`  
> `000000001110` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `000000000000`

If the value in `rs` is less than the value in `rt`, `rd` is set to 1; otherwise, `rd` is set to 0.

### <a name="op-sle"></a>Set on Less Than or Equal (`sle`)
> `$rs <= $rt -> $rd`  
> `000000001110` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `000000000001`

If the value in `rs` is less than or equal to the value in `rt`, `rd` is set to 1; otherwise, `rd` is set to 0.

### <a name="op-seq"></a>Set on Equal (`seq`)
> `$rs == $rt -> $rd`  
> `000000001110` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `000000000010`

If the value in `rs` is equal to the value in `rt`, `rd` is set to 1; otherwise, `rd` is set to 0.

### <a href="#op-sge">Set on Greater Than or Equal</a> (`sge`)

### <a href="#op-sg">Set on Greater Than</a> (`sg`)

### <a name="op-slu"></a>Set on Less Than Unsigned (`slu`)
> `$rs < $rt -> $rd /u`  
> `000000001110` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `000000000011`

If the value in `rs` is less than the value in `rt` (treating both as unsigned values), `rd` is set to 1; otherwise, `rd` is set to 0.

### <a name="op-sleu"></a>Set on Less Than or Equal Unsigned (`sleu`)
> `$rs <= $rt -> $rd /u`  
> `000000001110` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `000000000100`

If the value in `rs` is less than or equal to the value in `rt` (treating both as unsigned values), `rd` is set to 1; otherwise, `rd` is set to 0.

## <a name="ops-comp-i"></a>Comparisons (I-Types)

### <a name="op-sli"></a>Set on Less Than Immediate (`sli`)
> `$rs < imm -> $rd`  
> `000000011001` `......` `sssssss` `ddddddd` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

If the value in `rs` is less than `imm`, `rd` is set to 1; otherwise, `rd` is set to 0.

### <a name="op-slei"></a>Set on Less Than or Equal Immediate (`slei`)
> `$rs <= imm -> $rd`  
> `000000011010` `......` `sssssss` `ddddddd` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

If the value in `rs` is less than or equal to `imm`, `rd` is set to 1; otherwise, `rd` is set to 0.

### <a name="op-seqi"></a>Set on Equal Immediate (`seqi`)
> `$rs == imm -> $rd`  
> `000000011011` `......` `sssssss` `ddddddd` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

If the value in `rs` is equal to `imm`, `rd` is set to 1; otherwise, `rd` is set to 0.

### <a href="#op-sgeu">Set on Greater Than or Equal Unsigned</a> (`sgeu`)

### <a href="#op-sgu">Set on Greater Than Unsigned</a> (`sgu`)

### <a name="op-sgi"></a>Set on Greater Than Immediate (`sgi`)
> `$rs > imm -> $rd`  
> `000000101001` `......` `sssssss` `ddddddd` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

If the value in `rs` is greater than `imm`, `rd` is set to 1; otherwise, `rd` is set to 0.

### <a name="op-sgei"></a>Set on Greater Than or Equal Immediate (`sgei`)
> `$rs >= imm -> $rd`  
> `000000101010` `......` `sssssss` `ddddddd` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

If the value in `rs` is greater than or equal to `imm`, `rd` is set to 1; otherwise, `rd` is set to 0.

### <a name="op-slui"></a>Set on Less Than Unsigned Immediate (`slui`)
> `$rs < imm -> $rd /u`  
> `000000011100` `......` `sssssss` `ddddddd` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

If the value in `rs` is less than `imm` (treating both as unsigned values), `rd` is set to 1; otherwise, `rd` is set to 0.

### <a name="op-sleui"></a>Set on Less Than or Equal Unsigned Immediate (`sleui`)
> `$rs <= imm -> $rd /u`  
> `000000011101` `......` `sssssss` `ddddddd` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

If the value in `rs` is less than or equal to `imm` (treating both as unsigned values), `rd` is set to 1; otherwise, `rd` is set to 0.

### <a href="#op-sgeui">Set on Greater Than or Equal Unsigned Immediate</a> (`sgeui`)

### <a href="#op-sgui">Set on Greater Than Unsigned Immediate</a> (`sgui`)

## <a name="ops-jump-j"></a>Jumps (J-Types)

### <a name="op-j"></a>Jump (`j`)
> `: label` or `: imm`  
> `:: label` or `:: imm` (linking variant)  
> `000000001111` `0000000` `0000000` `cccc` `..` `aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`

Supports conditions:
> `+:`/`+::`: jump if positive  
> `-:`/`-::`: jump if negative  
> `0:`/`0::`: jump if zero  
> `*:`/`*::`: jump if nonzero  

Jumps to the address of a given label or directly to a given address.

### <a name="op-jc"></a>Jump Conditional (`jc`)
> `: label if $rs` or `: imm if $rs`  
> `:: label if $rs` or `:: imm if $rs` (linking variant)  
> `000000010000` `sssssss` `0000000` `cccc` `..` `aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`

Jumps to the address of a given label or directly to a given address, provided the value in `rs` is nonzero.

## <a name="ops-jump-r"></a>Jumps (R-Types)

### <a name="op-jr"></a>Jump to Register (`jr`)
> `: $rd`  
> `000000010001` `0000000` `0000000` `ddddddd` `0000000000000` `......` `000000000000`  
> Supports conditions

Jumps to the address stored in `rd`.

### <a name="op-jrc"></a>Jump to Register Conditional (`jrc`)
> `: $rd if $rs`  
> `000000010001` `0000000` `sssssss` `ddddddd` `0000000000000` `......` `000000000001`  

Jumps to the address stored in `rd`, provided the value in `rs` is nonzero.

### <a name="op-jrl"></a>Jump to Register and Link (`jrl`)
> `:: $rd`  
> `000000010001` `0000000` `0000000` `ddddddd` `0000000000000` `......` `000000000010`  
> Supports conditions

Stores the address of the next instruction in `$rt` and jumps to the address stored in `rd`.

### <a name="op-jrlc"></a>Jump to Register and Link Conditional (`jrlc`)
> `:: $rd if $rs`  
> `000000010001` `0000000` `sssssss` `ddddddd` `0000000000000` `......` `000000000011`

Stores the address of the next instruction in `$rt` and jumps to the address stored in `rd`, provided the value in `rs` is nonzero.

## <a name="ops-mem-r"></a>Memory (R-Types)

### <a name="op-c"></a>Copy (`c`)
> `[$rs] -> [$rd]`  
> `000000010010` `0000000` `sssssss` `ddddddd` `0000000000000` `......` `000000000000`

Copies the word beginning at the memory address pointed to by `rs` into memory beginning at the address pointed to by `rd`.

### <a name="op-l"></a>Load (`l`)
> `[$rs] -> $rd`  
> `000000010010` `0000000` `sssssss` `ddddddd` `0000000000000` `......` `000000000001`

Loads the word beginning at the memory address pointed to by `rs` into `rd`.

### <a name="op-s"></a>Store (`s`)
> `$rs -> [$rd]`  
> `000000010010` `0000000` `sssssss` `ddddddd` `0000000000000` `......` `000000000010`

Stores the value of `rs` into memory beginning at the address pointed to by `rd`.

### <a name="op-cb"></a>Copy Byte (`cb`)
> `[$rs] -> [$rd] /b`  
> `000000010010` `0000000` `sssssss` `ddddddd` `0000000000000` `......` `000000000011`

Copies the byte stored at the memory address pointed to by `rs` into the memory address pointed to by `rd`.

### <a name="op-lb"></a>Load Byte (`lb`)
> `[$rs] -> $rd /b`  
> `000000010010` `0000000` `sssssss` `ddddddd` `0000000000000` `......` `000000000100`

Loads the byte stored at the memory address pointed to by `rs` into `rd`.

### <a name="op-sb"></a>Store Byte (`sb`)
> `$rs -> [$rd] /b`  
> `000000010010` `0000000` `sssssss` `ddddddd` `0000000000000` `......` `000000000101`

Stores the lowest 8 bits of `rs` into the memory address pointed to by `rd`.

### <a name="op-ch"></a>Copy Halfword (`ch`)
> `[$rs] -> [$rd] /h`  
> `000000010010` `0000000` `sssssss` `ddddddd` `0000000000000` `......` `000000000110`

Copies the halfword stored at the memory address pointed to by `rs` into the memory address pointed to by `rd`.

### <a name="op-lh"></a>Load Halfword (`lh`)
> `[$rs] -> $rd /h`  
> `000000010010` `0000000` `sssssss` `ddddddd` `0000000000000` `......` `000000000111`

Loads the halfword stored at the memory address pointed to by `rs` into `rd`.

### <a name="op-sh"></a>Store Halfword (`sh`)
> `$rs -> [$rd] /h`  
> `000000010010` `0000000` `sssssss` `ddddddd` `0000000000000` `......` `000000001000`

Stores the lowest 32 bits of `rs` into the memory address pointed to by `rd`.

### <a name="op-spush"></a>Stack Push (`spush`)
> `[ $rs`  
> `000000010010` `0000000` `sssssss` `0000000` `0000000000000` `......` `000000000110`

Copies the word at `rs` into the stack and adjusts the stack pointer.  
See also: <a href="#op-push">push pseudoinstruction</a>

### <a name="op-spop"></a>Stack Pop (`spop`)
> `] $rd`  
> `000000010010` `0000000` `ddddddd` `0000000` `0000000000000` `......` `000000000111`

Adjusts the stack pointer and copies the word at the stack pointer into `rd`.  
See also: <a href="#op-pop">pop pseudoinstruction</a>

## <a name="ops-mem-i"></a>Memory (I-Types)

### <a name="op-li"></a>Load Immediate (`li`)
> `[imm] -> $rd`  
> `000000010011` `......` `0000000` `ddddddd` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

Loads the word beginning at address `imm` into `rd`.

### <a name="op-si"></a>Store Immediate (`si`)
> `$rs -> [imm]`  
> `000000010100` `......` `sssssss` `0000000` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

Stores the value of `rs` into memory beginning at address `imm`.

### <a name="op-lbi"></a>Load Byte Immediate (`lbi`)
> `[imm] -> $rd /b`  
> `000000100101` `......` `0000000` `ddddddd` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

Loads the byte at address `imm` into `rd`.

### <a name="op-sbi"></a>Store Byte Immediate (`sbi`)
> `$rs -> [imm] /b`  
> `000000100110` `......` `sssssss` `0000000` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

Stores the lowest 8 bits of `rs` into memory at address `imm`.

### <a name="op-lni"></a>Load Indirect Immediate (`lni`)
> `[imm] -> [$rd]`  
> `000000100111` `......` `sssssss` `0000000` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

Copies the word stored in memory at address `imm` into the memory beginning at the address pointed to by `rd`.

### <a name="op-lbni"></a>Load Byte Indirect Immediate (`lbni`)
> `[imm] -> [$rd] /b`  
> `000000101000` `......` `sssssss` `0000000` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

Copies the byte stored in memory at address `imm` into the memory address pointed to by `rd`.

### <a name="op-set"></a>Set (`set`)
> `imm -> $rd`  
> `000000010101` `......` `0000000` `ddddddd` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

Sets a register to the given immediate value.

### <a name="op-lui"></a>Load Upper Immediate (`lui`)
> `lui: imm -> $rd`  
> `000000001101` `......` `0000000` `ddddddd` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

Loads an immediate value into the upper half of the word at `rd`. The lower half is not affected.

## <a name="ops-special"></a>Special Instructions

### <a name="op-ext"></a>External (`ext`)
> (varies; see <a href="#ext">Externals</a>)  
> `000000011111` `ttttttt` `sssssss` `ddddddd` `0000000000000` `......` `xxxxxxxxxxxx`

Performs a special instruction, typically for interaction with the world outside the VM.

### <a name="op-int"></a>Interrupt (`int`)
> `int imm`  
> `000000100000` `......` `......` `......` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

Performs an interrupt. If no interrupt table has been registered, nothing interesting happens.

### <a name="op-rit"></a>Register Interrupt Table (`rit`)
> `rit imm`  
> `000000100001` `......` `......` `......` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

Registers the interrupt table. Takes a pointer to a table in the data section. Valid only in kernel mode; will cause the machine to halt if called in user mode.

### <a name="op-time"></a>Set Timer (`time`)
> `time $rs`  
> `000000110000` `.......` `sssssss` `.......` `0000000000000` `......` `000000000000`

Sets the hardware timer to the number stored in `rs` (in microseconds), canceling any previous timer. Requires kernel mode. Sub-millisecond precision may be unsupported or inaccurate. Once the timer expires, a <a href="#int-timer">timer interrupt</a> occurs.

### <a name="op-timei"></a>Set Timer Immediate (`timei`)
> `time imm`  
> `000000110001` `......` `......` `......` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

Sets the hardware timer to the number stored in `imm` (in microseconds), canceling any previous timer. Requires kernel mode. Sub-millisecond precision may be unsupported or inaccurate. Once the timer expires, a <a href="#int-timer">timer interrupt</a> occurs.

### <a name="op-ring"></a>Change Ring (`ring`)
> `ring $rs`  
> `000000110010` `.......` `sssssss` `.......` `0000000000000` `......` `000000000000`

Sets the <a href="#rings">protection ring</a> to the value stored in `rs`. A <a href="#int-protec">protection interrupt</a> will occur if the indicated ring is lower than the current ring to prevent privilege escalation.

### <a name="op-ring"></a>Change Ring Immediate (`ring`)
> `ring imm`  
> `000000110011` `......` `......` `......` `iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii`

Sets the <a href="#rings">protection ring</a> to `imm`. A <a href="#int-protec">protection interrupt</a> will occur if the indicated ring is lower than the current ring to prevent privilege escalation.

## <a name="ops-pseudo"></a>Pseudoinstructions

### <a name="op-mv"></a>Move (`mv`)
> `$rs -> $rd`

Copies the value of `rs` into `rd`.

Translation:  
<code>$rs [|](#op-or) $0 -> $rd</code>

### <a name="op-ret"></a>Return (`ret`)
> `ret`

Jumps to the return address.

Translation:  
<code>[:](#op-jr) $r</code>

### <a name="op-push"></a>Push (`push`)
> `[ $x $y $z ...`

Pushes the value of the specified register(s) to the stack. Acts as a shorthand for calling
<a href="#op-spush">spush</a> on multiple registers.

Translation for each register in order:  
<code><a href="#op-spush">[</a> $rs</code>

### <a name="op-pop"></a>Pop (`pop`)
> `] $x $y $z`

Pops the value(s) at the top of the stack and stores the value(s) in the specified register(s).
Acts as a shorthand for calling <a href="#op-spop">spop</a> on multiple registers.

Translation for each register in order:
<code><a href="#op-spop">]</a> $rs</code>

### <a name="op-jeq"></a>Jump if Equal (`jeq`)
> `: $rd if $rs == $rt`

If the value in `rs` is equal to the value in `rt`, jumps to the address stored in `rd` (or to the address of `var`). (Modifies `m0`.)

Translation:  
<code>$rs [==](#op-seq) $rt -> $m0</code>  
<code>[:](#op-jrc) $rd if $m0</code>

> `: label if $rs == $rt`

If the value in `rs` is equal to the value in `rt`, jumps to `label`. (Modifies `m0` and `m1`.)

Translation:  
<code>$rs [==](#op-seq) $rt -> $m0</code>  
<code>[label] [->](#op-li) $m1</code>  
<code>[:](#op-jrc) $m1 if $m0</code>

### <a name="op-sge"></a>Set on Greater Than or Equal (`sge`)
> `$rs >= $rt -> $rd`

If the value in `rs` is greather than or equal to the value in `rt`, `rd` is set to 1; otherwise, `rd` is set to 0.

Translation:  
<code>$rt [<=](#op-sle) $rs -> $rd</code>

### <a name="op-sg"></a>Set on Greater Than (`sg`)
> `$rs > $rt -> $rd`

If the value in `rs` is greather than the value in `rt`, `rd` is set to 1; otherwise, `rd` is set to 0.

Translation:  
<code>$rt [<](#op-sl) $rs -> $rd</code>

### <a name="op-sgeu"></a>Set on Greater Than or Equal Unsigned (`sgeu`)
> `$rs >= $rt -> $rd /u`

If the value in `rs` is greather than or equal to the value in `rt` (treating both as unsigned values), `rd` is set to 1; otherwise, `rd` is set to 0.

Translation:  
<code>$rt [<=](#op-sleu) $rs -> $rd</code>

### <a name="op-sgu"></a>Set on Greater Than Unsigned (`sgu`)
> `$rs > $rt -> $rd /u`

If the value in `rs` is greather than the value in `rt` (treating both as unsigned values), `rd` is set to 1; otherwise, `rd` is set to 0.

Translation:  
<code>$rt [<](#op-slu) $rs -> $rd /u</code>

### <a name="op-sgeui"></a><s>Set on Greater Than or Equal Unsigned Immediate</s> (`sgeui`)
> `$rs >= imm -> $rd /u`

If the value in `rs` is greather than or equal to `imm` (treating both as unsigned values), `rd` is set to 1; otherwise, `rd` is set to 0. Currently unimplemented.

### <a name="op-sgui"></a><s>Set on Greater Than Unsigned Immediate</s> (`sgui`)
> `$rs > imm -> $rd /u`

If the value in `rs` is greather than `imm` (treating both as unsigned values), `rd` is set to 1; otherwise, `rd` is set to 0. Currently unimplemented.

# <a name="ext"></a>Externals

### <a name="ext-printr"></a>Print Register
Syntax: `<print $rs>`  
Function value: `000000000001`

Prints the value stored in `rs` to the console.

### <a name="ext-halt"></a>Halt
Syntax: `<halt>`  
Function value: `000000000010`

Halts the VM.

### <a name="ext-eval"></a>Evaluate String
Syntax: `<eval $rs>`  
Function value: `000000000011`

Evaluates the string beginning at the address stored in `rs`. Unimplemented.

### <a name="ext-prc"></a>Print Character
Syntax: `<prc $rs>`  
Function value: `000000000100`

Prints the character stored in `rs` to the console.

### <a name="ext-prd"></a>Print Decimal
Syntax: `<prd $rs>`  
Function value: `000000000101`

Prints the number stored in `rs` to the console as a decimal.

### <a name="ext-prx"></a>Print Hexadecimal
Syntax: `<prx $rs>`  
Function value: `000000000110`

Prints the number stored in `rs` to the console as a hexadecimal.