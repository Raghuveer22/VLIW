# main.py
import config
from scheduler import parse_instructions, schedule_instructions, reorder_instructions, convert_to_binary, generate_testbench


def main():
    with open(config.MACHINE_TEST_CODE) as f:
        asm_code = f.read()

    instructions,parsed_instructions, registers = parse_instructions(asm_code)
    print(parsed_instructions,registers)
    global_node_table,node_table = schedule_instructions(instructions,parsed_instructions, registers)
    
    reordered_instructions = reorder_instructions(node_table)
    print(reordered_instructions)
    binary_instructions = convert_to_binary(reordered_instructions)

    generate_testbench(binary_instructions)

if __name__ == "__main__":
    main()
