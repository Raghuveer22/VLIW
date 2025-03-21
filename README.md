# VLIW-Processor
A basic VLIW processor with an integrated compiler, implemented in Verilog and Python.

## test.vsm
An input file containing sequential instructions.

## Compiler/main.py
The primary script that reads test.vsm, reorders instructions into packets to maximize functional unit usage, and resolves RAW, WAR, and WAW hazards.

## About The Processor
- 3-stage pipeline
- Executes 6 instructions per cycle
- Features 6 functional units: IntAdd, IntMul, FPAdd, FPMul, LogicUnit, MemoryUnit
- Register-based addressing for all units except Memory, which uses immediate operands
- No support for Jump/Branch instructions
- Memory divided into 6kB Instruction memory and 4kB Data memory

## Processor.v
The core Verilog module integrating all units, memory, and registers, modified by reorder.py.

## Output Explanation
The input file contains sequential instructions like ADD, SUB, MUL, etc. The compiler reorders them into packets, ensuring no hazards (RAW, WAR, WAW) and optimal unit usage. It builds a register dependency graph, uses breadth-first search, and handles special cases like variable register usage and unit availability. The Python script converts instructions to opcodes, embeds them into Processor.v, compiles, and runs it, displaying register changes (except Program Counter).
