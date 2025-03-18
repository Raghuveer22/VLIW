# scheduler.py
import config
from node import InstructionNode
import os 
from typing import List

def parse_instructions(asm_code):
    instructions = asm_code.split('\n')
    parsed_instructions = []
    registers = []
    
    for instruction in instructions:
        parsed_instructions.append(instruction.split(' ')[0])
        
        if instruction.split(' ')[0] not in ["MOV", "LDR", "STR"]:
            registers.append(instruction.split(' ')[1].split(','))
        else:
            if "#" in instruction.split(' ')[1]:
                registers.append(instruction.split(' ')[1].split(',')[:-1] + ["R0"])
            else:
                registers.append(instruction.split(' ')[1].split(','))
    
    return instructions,parsed_instructions, registers

def schedule_instructions(instructions,parsed_instructions, registers):
    # Initialize node table
    node_table = [InstructionNode(f"R{i}", "") for i in range(32)]
    global_node_table = list(node_table)
    
    for i, instruction in enumerate(parsed_instructions):
        if instruction != "MUL":
            parent_registers = registers[i][1:]
        else:
            parent_registers = registers[i][2:]
            
        register_name = registers[i][0]
        
        node = InstructionNode(register=register_name, instruction_word=instructions[i])
        node.add_instruction(parsed_instructions[i])
        node.add_parents(parent_registers)
        
        for parent in parent_registers:
            global_node_table[int(parent[1:])].add_child(node)
            
        node.add_waw_dependency(global_node_table[int(register_name[1:])])
        
        if instruction == "MUL":
            node.add_waw_dependency(global_node_table[int(registers[i][1][1:])])
            
        node.add_war_dependency(global_node_table[int(register_name[1:])].children)
        
        if instruction == "MUL":
            node.add_war_dependency(global_node_table[int(registers[i][1][1:])].children)
            
        # Remove cyclic WAR dependencies
        while node in node.war_dependencies:
            node.war_dependencies.remove(node)
            
        global_node_table[int(register_name[1:])] = node
        
        if instruction == "MUL":
            global_node_table[int(registers[i][1][1:])] = node
            
    return global_node_table,node_table

def reorder_instructions(node_table:List[InstructionNode]):
    """
    Reorders instructions based on dependencies and functional units.
    
    Parameters:
    - node_table: List of Node objects representing instructions.
    
    Returns:
    - packeted_instructions: List of lists where each sublist contains instructions that can be executed in parallel.
    """
    def get_unit(k):
        for unit in config.FUNCTIONAL_UNITS:
            if k in unit:
                return unit
        return "Unknown"
    
    # Initialize variables
    next_level = []
    packet = []
    packeted_instructions = []
    
  
    while len(node_table) != 0:
        # Dequeue first element
        leader = node_table[0]
        node_table = node_table[1:]
        
        # Check if the unit is free for that clock cycle
        unit_free = all([not(get_unit(i.split(' ')[0]) == get_unit(leader.instruction)) for i in packet])
        
        # Check WAW and WAR dependencies
        waw_cleared = all([i.delay == 0 for i in leader.waw_dependencies])
        war_cleared = all([i.executable for i in leader.war_dependencies])
        
        
        
        # If the instruction is executable, set it as executed and add it to the packet
        if leader.parents == [] and leader.instruction != None and not leader.executable and waw_cleared and war_cleared and unit_free:
            leader.executable = True
            print(leader.instruction_word, "(" + str(leader.delay) + ")", end="\t")
            packet.append(leader.instruction_word)
            
        # If dependencies are not cleared or unit is busy, append leader to next level
        elif leader.parents == [] and ((not waw_cleared) or (not war_cleared) or (not unit_free)):
            next_level.append(leader)
            
        # If leader has no parent dependencies but is not ready to execute, decrease its delay and append back to next level
        elif leader.parents == [] and leader.delay != 0:
            leader.delay -= 1
            if leader.delay > 0:
                next_level.append(leader)
                
        # If leader has completed, push its children to next level and remove it from their parents
        if leader.delay == 0:
            next_level.extend(leader.children)
            for child in leader.children:
                print(child.register,child.instruction,child.parents,leader.register)
                child.parents.remove(leader.register)
                
        # If node table is empty, update it with next level and append current packet to packeted instructions
        if node_table == []:
            node_table = list(set(next_level))
            for i in node_table:
                print(i.register)
            packeted_instructions.append(packet)
            packet = []
            next_level = []
            print()
            
    # First instruction is NOP because of dummy registers
    packeted_instructions = packeted_instructions[1:]
    repacked_instructions=[]
    
    return packeted_instructions

def convert_to_binary(packeted_instructions):
    # Convert instructions to binary format
    binary_instructions = []
    
    for instruction in packeted_instructions:
        if instruction != "NOP":
            print(instruction)
            opcode = config.INSTRUCTION_TYPES[instruction.split(' ')[0]]
            if instruction.split(' ')[0] not in ["MOV", "LDR", "STR"]:
                registers = "".join([bin(int(register[1:]))[2:].zfill(5) for register in instruction.split(' ')[1].split(',')])
            else:
                tmp = [int(register[1:]) for register in instruction.split(' ')[1].split(',')]
                if opcode == "01":
                    registers = bin(tmp[0])[2:].zfill(5) + bin(tmp[1])[2:].zfill(25)
                else:
                    registers = bin(tmp[0])[2:].zfill(5) + bin(tmp[1])[2:].zfill(10)
                    
            binary_instruction = opcode + registers
            binary_instruction += "0" * (32 - len(binary_instruction))
        else:
            binary_instruction = "0" * 32
            
        binary_instructions.append(binary_instruction)
        
    return binary_instructions

def generate_testbench(binary_instructions):
    # Generate testbench file
    # This part is simplified; adjust as needed
    with open(config.PROCESSOR_CODE, 'r') as f:
        testbench_code = f.read()
        
    new_testbench_code = testbench_code[:testbench_code.find("//pyc_pushcode")] + "\n\t\t".join([f"instructionMem[{i}]=" + binary_instruction + ";" for i, binary_instruction in enumerate(binary_instructions)]) + testbench_code[testbench_code.find("//pyc_pushcode"):]
    
    with open(config.NEW_PROCESSOR_CODE, 'w') as f:
        f.write(new_testbench_code)
        
    # Compile and run testbench
    # This part is simplified; adjust as needed
    os.system("iverilog NewProcessor.v")
    os.system("./a.out > output.txt")
    
    # Parse output
    with open("output.txt", 'r') as f:
        dump = f.read()
        
    # Simplified parsing; adjust as needed
    print("Time\tPC\tRegister Values")
    # ... parsing logic ...
