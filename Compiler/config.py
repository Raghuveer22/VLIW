# config.py
MACHINE_TEST_CODE="/Users/posamokshith/Documents/pca project/VLIW_final/test.vsm"
PROCESSOR_CODE="/Users/posamokshith/Documents/pca project/VLIW_final/Processor.v"
NEW_PROCESSOR_CODE="/Users/posamokshith/Documents/pca project/VLIW_final/NewProcessor.v"

# Instruction types and their opcodes
INSTRUCTION_TYPES = {
    "ADD": "0000",
    "ADDC": "0100",
    "SUB": "1100",
    "SUBB": "1000",
    "MUL": "0000",
    "FADD": "0000",
    "FMUL": "0000",
    "AND": "0000",
    "OR": "0110",
    "NAND": "0100",
    "NOR": "1010",
    "XOR": "0010",
    "XNOR": "1110",
    "NOT": "1000",
    "BLS": "1100",
    "MOV": "01",
    "LDR": "10",
    "STR": "11"
}

# Instruction delays
INSTRUCTION_DELAYS = {
    "ADD": 5,
    "ADDC": 5,
    "SUB": 5,
    "SUBB": 5,
    "MUL": 14,
    "FADD": 5,
    "FMUL": 26,
    "AND": 1,
    "OR": 1,
    "NAND": 1,
    "NOR": 1,
    "XOR": 1,
    "XNOR": 1,
    "NOT": 1,
    "BLS": 1,
    "MOV": 1,
    "LDR": 1,
    "STR": 1
}

# Functional units
FUNCTIONAL_UNITS = [
    ["ADD", "ADDC", "SUB", "SUBB"],
    ["MUL"],
    ["FADD"],
    ["FMUL"],
    ["AND", "OR", "NAND", "NOR", "XOR", "XNOR", "NOT", "BLS"],
    ["MOV", "LDR", "STR"]
]
