# node.py

class InstructionNode:
    def __init__(self, register, instruction_word):
        self.register = register
        self.instruction = None
        self.instruction_word = instruction_word
        self.delay = 0
        self.parents = []
        self.children = []
        self.waw_dependencies = []
        self.war_dependencies = []
        self.executable = False

    def add_child(self, child):
        self.children.append(child)

    def add_parents(self, parents):
        self.parents = parents

    def add_waw_dependency(self, dependency):
        if isinstance(dependency, list):
            self.waw_dependencies.extend(dependency)
        else:
            self.waw_dependencies.append(dependency)

    def add_war_dependency(self, dependency):
        if isinstance(dependency, list):
            self.war_dependencies.extend(dependency)
        else:
            self.war_dependencies.append(dependency)

    def add_instruction(self, instruction):
        self.instruction = instruction

    def print_node(self):
        print(f"Register Name: {self.register}")
        print(f"Instruction: {self.instruction}")
        print("WAR Dependencies:")
        for dep in self.war_dependencies:
            print(f" -> {dep.register} ({dep.instruction})")
        print("WAW Dependencies:")
        for dep in self.waw_dependencies:
            print(f" -> {dep.register} ({dep.instruction})")
        print("Children:")
        for child in self.children:
            print(f" -> {child.register} ({child.instruction})")
