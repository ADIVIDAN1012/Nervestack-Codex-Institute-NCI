"""
Nervestack Language Interpreter
Executes Nervestack AST directly without compilation to C
"""

from typing import Any, Dict, List, Optional
from nervestack_ast import *
import sys


class NervestackRuntimeError(Exception):
    """Runtime error during interpretation"""
    def __init__(self, message: str, node: Optional[ASTNode] = None):
        self.message = message
        self.node = node
        super().__init__(message)


class Environment:
    """Manages variable and function scopes"""
    
    def __init__(self, parent: Optional['Environment'] = None):
        self.parent = parent
        self.variables: Dict[str, Any] = {}
        self.constants: Dict[str, Any] = {}
        self.functions: Dict[str, FunctionDeclNode] = {}
    
    def define_variable(self, name: str, value: Any):
        """Define or update a variable"""
        self.variables[name] = value
    
    def define_constant(self, name: str, value: Any):
        """Define a constant (immutable)"""
        if name in self.constants:
            raise NervestackRuntimeError(f"Constant '{name}' already defined")
        self.constants[name] = value
    
    def get_variable(self, name: str) -> Any:
        """Get variable value, checking constants and parent scopes"""
        if name in self.variables:
            return self.variables[name]
        if name in self.constants:
            return self.constants[name]
        if self.parent:
            return self.parent.get_variable(name)
        raise NervestackRuntimeError(f"Undefined variable: '{name}'")
    
    def set_variable(self, name: str, value: Any):
        """Set variable value (cannot modify constants)"""
        if name in self.constants:
            raise NervestackRuntimeError(f"Cannot modify constant '{name}'")
        if name in self.variables:
            self.variables[name] = value
        elif self.parent:
            self.parent.set_variable(name, value)
        else:
            raise NervestackRuntimeError(f"Undefined variable: '{name}'")
    
    def define_function(self, name: str, func_node: FunctionDeclNode):
        """Define a function"""
        self.functions[name] = func_node
    
    def get_function(self, name: str) -> Optional[FunctionDeclNode]:
        """Get function definition"""
        if name in self.functions:
            return self.functions[name]
        if self.parent:
            return self.parent.get_function(name)
        return None


class ReturnValue(Exception):
    """Exception used to handle return statements"""
    def __init__(self, value: Any):
        self.value = value


class BreakLoop(Exception):
    """Exception used to handle break statements"""
    pass


class ContinueLoop(Exception):
    """Exception used to handle continue statements"""
    pass


class Blueprint:
    """Represents a Nervestack blueprint (class)"""
    def __init__(self, node: BlueprintNode, env: Environment):
        self.node = node
        self.env = env
        self.name = node.name

class Instance:
    """Represents an instance of a Nervestack blueprint"""
    def __init__(self, blueprint: Blueprint):
        self.blueprint = blueprint
        self.fields: Dict[str, Any] = {}

    def __repr__(self):
        return f"<instance of {self.blueprint.name}>"

class BoundMethod:
    def __init__(self, instance, method_node, interpreter):
        self.instance = instance
        self.method_node = method_node
        self.interpreter = interpreter

    def __call__(self, *args):
        # Implementation of calling the method with 'own' bound to the instance
        pass

class Interpreter:
    """Nervestack AST Interpreter using visitor pattern"""
    
    def __init__(self):
        self.global_env = Environment()
        self.current_env = self.global_env
        self._setup_builtins()
    
    def _setup_builtins(self):
        """Setup built-in functions and constants"""
        # Built-in constants
        self.global_env.define_constant("On", True)
        self.global_env.define_constant("Off", False)
        self.global_env.define_constant("Nil", None)
    
    def interpret(self, program: ProgramNode):
        """Interpret a Nervestack program"""
        try:
            self.visit(program)
        except NervestackRuntimeError as e:
            print(f"Runtime Error: {e.message}", file=sys.stderr)
            sys.exit(1)
    
    def visit(self, node: ASTNode) -> Any:
        """Dispatch to appropriate visit method"""
        method_name = f'visit_{node.__class__.__name__}'
        method = getattr(self, method_name, self.generic_visit)
        return method(node)
    
    def generic_visit(self, node: ASTNode):
        """Fallback for unimplemented node types"""
        raise NervestackRuntimeError(f"No visit method for {node.__class__.__name__}")
    
    # ===== Program and Statements =====
    
    def visit_ProgramNode(self, node: ProgramNode):
        """Execute program statements"""
        for statement in node.statements:
            self.visit(statement)
    
    def visit_ShowStatementNode(self, node: ShowStatementNode):
        """Execute show statement (print)"""
        values = []
        for expr in node.expressions:
            value = self.visit(expr)
            values.append(self._to_string(value))
        print(' '.join(values))
    
    def visit_ConstantDeclNode(self, node: ConstantDeclNode):
        """Define a constant"""
        value = self.visit(node.value)
        self.current_env.define_constant(node.const_name, value)
    
    def visit_VarAssignNode(self, node: VarAssignNode):
        """Assign to a variable or attribute"""
        value = self.visit(node.value)
        
        if isinstance(node.target, VarAccessNode):
            self.current_env.set_variable(node.target.var_name, value)
        elif isinstance(node.target, AttributeAccessNode):
            obj = self.visit(node.target.obj)
            if not isinstance(obj, Instance):
                raise NervestackRuntimeError(f"Cannot set attribute on non-instance: {type(obj)}")
            obj.fields[node.target.attribute] = value
        elif isinstance(node.target, str):
             self.current_env.define_variable(node.target, value)
        else:
            raise NervestackRuntimeError(f"Invalid assignment target: {node.target}")
    
    def visit_ExpressionStatementNode(self, node):
        """Execute expression statement (e.g., function call)"""
        return self.visit(node.expression)
    
    # ===== Expressions =====

    def visit_NumberNode(self, node: NumberNode) -> float:
        """Evaluate number node"""
        return float(node.value)

    def visit_BooleanNode(self, node: BooleanNode) -> bool:
        """Evaluate boolean node"""
        return bool(node.value)

    def visit_NilNode(self, node: NilNode) -> None:
        """Evaluate nil node"""
        return None

    def visit_StringNode(self, node: StringNode) -> str:
        """Evaluate string node"""
        return str(node.value)

    def visit_VarAccessNode(self, node: VarAccessNode) -> Any:
        """Evaluate variable access"""
        return self.current_env.get_variable(node.var_name)
    
    def visit_AttributeAccessNode(self, node: AttributeAccessNode) -> Any:
        """Evaluate attribute access"""
        obj = self.visit(node.obj)
        if isinstance(obj, Instance):
            if node.attribute in obj.fields:
                return obj.fields[node.attribute]
            # Check for methods?
            for method in obj.blueprint.node.methods:
                if method.name == node.attribute:
                    # Return a bound method
                    return BoundMethod(obj, method, self)
            raise NervestackRuntimeError(f"Attribute '{node.attribute}' not found in instance of {obj.blueprint.name}")
        else:
             raise NervestackRuntimeError(f"Cannot access attribute on non-instance: {type(obj)}")

    def visit_BinaryOpNode(self, node: BinaryOpNode) -> Any:
        """Evaluate binary operation"""
        left = self.visit(node.left)
        right = self.visit(node.right)
        # Extract operator value from Token
        op = node.op.value if hasattr(node.op, 'value') else node.op
        
        # Arithmetic operators
        if op == '+':
            return left + right
        elif op == '-':
            return left - right
        elif op == '*':
            return left * right
        elif op == '/':
            if right == 0:
                raise NervestackRuntimeError("Division by zero")
            return left / right
        elif op == '%':
            return left % right
        elif op == '^':
            return left ** right
        
        # Comparison operators
        elif op == '==' or op == '=':
            return left == right
        elif op == '!=' or op == '≠':
            return left != right
        elif op == '<':
            return left < right
        elif op == '>':
            return left > right
        elif op == '<=' or op == '≤':
            return left <= right
        elif op == '>=' or op == '≥':
            return left >= right
        
        # Logical operators
        elif op == 'and' or op == '&&':
            return left and right
        elif op == 'or' or op == '||':
            return left or right
        elif op == '..':
            return range(int(left), int(right) + 1)
        
        else:
            raise NervestackRuntimeError(f"Unknown binary operator: {op}")
    
    def visit_UnaryOpNode(self, node: UnaryOpNode) -> Any:
        """Evaluate unary operation"""
        operand = self.visit(node.right)
        # Extract operator value from Token
        op = node.op.value if hasattr(node.op, 'value') else node.op
        
        if op == '-':
            return -operand
        elif op == 'not' or op == '!':
            return not operand
        else:
            raise NervestackRuntimeError(f"Unknown unary operator: {op}")

    def visit_InterpolatedStringNode(self, node: InterpolatedStringNode) -> str:
        """Evaluate interpolated string"""
        result = ""
        for part in node.parts:
            val = self.visit(part)
            result += self._to_string(val)
        return result
    
    # ===== Control Flow =====
    
    def visit_CheckStatementNode(self, node: CheckStatementNode):
        """Execute check (if) statement"""
        condition = self.visit(node.condition)
        
        if self._is_truthy(condition):
            for stmt in node.body:
                self.visit(stmt)
        else:
            # Check alter clauses (elif)
            for alter_cond, alter_body in node.alter_clauses:
                if self._is_truthy(self.visit(alter_cond)):
                    for stmt in alter_body:
                        self.visit(stmt)
                    return
            
            # Execute altern clause (else)
            if node.altern_clause:
                for stmt in node.altern_clause:
                    self.visit(stmt)
    
    def visit_TraverseNode(self, node: TraverseNode):
        """Execute traverse (for) loop"""
        iterable = self.visit(node.iterable)
        
        # Create new scope for loop variable
        loop_env = Environment(self.current_env)
        prev_env = self.current_env
        self.current_env = loop_env
        
        try:
            # Handle list/range/etc
            if isinstance(iterable, (range, list, tuple)):
                it = iterable
            else:
                it = iterable

            for i in it:
                loop_env.define_variable(node.var_name, i)
                try:
                    for stmt in node.body:
                        self.visit(stmt)
                except ContinueLoop:
                    continue
                except BreakLoop:
                    break
        finally:
            self.current_env = prev_env

    def visit_UntilNode(self, node: UntilNode):
        """Execute until (while) loop"""
        while True:
            condition = self.visit(node.condition)
            if not self._is_truthy(condition):
                break
            
            try:
                for stmt in node.body:
                    self.visit(stmt)
            except ContinueLoop:
                continue
            except BreakLoop:
                break

    # ===== OOP =====

    def visit_BlueprintNode(self, node: BlueprintNode):
        """Register a blueprint"""
        blueprint = Blueprint(node, self.current_env)
        self.current_env.define_variable(node.name, blueprint)

    def visit_SpawnNode(self, node: SpawnNode) -> Instance:
        """Create an instance of a blueprint"""
        blueprint = self.current_env.get_variable(node.blueprint_name)
        if not isinstance(blueprint, Blueprint):
            raise NervestackRuntimeError(f"'{node.blueprint_name}' is not a blueprint")
        
        instance = Instance(blueprint)
        
        # Initialize attributes
        for attr in blueprint.node.attributes:
            # attr is a VarAssignNode
            val = self.visit(attr.value) if attr.value else None
            attr_name = attr.target.var_name if isinstance(attr.target, VarAccessNode) else attr.target
            instance.fields[attr_name] = val
        
        # Call constructor if exists
        # (Simplified: logic for prep/make)
        
        return instance
    
    def visit_HaltNode(self, node):
        """Execute halt (break) statement"""
        raise BreakLoop()
    
    def visit_ProceedNode(self, node):
        """Execute proceed (continue) statement"""
        raise ContinueLoop()
    
    # ===== Functions =====
    
    def visit_FunctionDeclNode(self, node: FunctionDeclNode):
        """Define a function"""
        self.current_env.define_function(node.name, node)
    
    def visit_FunctionCallNode(self, node: FunctionCallNode) -> Any:
        """Call a function"""
        # Check for built-in functions
        if node.function_name == 'show':
            # Already handled by ShowStatementNode
            values = [self.visit(arg) for arg in node.arguments]
            print(' '.join(self._to_string(v) for v in values))
            return None
        
        if node.function_name == 'input':
            prompt = self.visit(node.arguments[0]) if node.arguments else ""
            return input(self._to_string(prompt))
        
        # User-defined function
        func = self.current_env.get_function(node.function_name)
        if not func:
            raise NervestackRuntimeError(f"Undefined function: '{node.function_name}'")
        
        # Evaluate arguments
        args = [self.visit(arg) for arg in node.arguments]
        
        # Check argument count
        if len(args) != len(func.params):
            raise NervestackRuntimeError(
                f"Function '{node.function_name}' expects {len(func.params)} arguments, got {len(args)}"
            )
        
        # Create new environment for function
        func_env = Environment(self.current_env)
        
        # Bind parameters
        for param, arg in zip(func.params, args):
            func_env.define_variable(param, arg)
        
        # Execute function body
        prev_env = self.current_env
        self.current_env = func_env
        
        try:
            for stmt in func.body:
                self.visit(stmt)
            return None  # No explicit return
        except ReturnValue as ret:
            return ret.value
        finally:
            self.current_env = prev_env
    
    def visit_ReturnStatementNode(self, node: ReturnStatementNode):
        """Execute return statement"""
        value = self.visit(node.expression) if node.expression else None
        raise ReturnValue(value)
    
    # ===== Helper Methods =====
    
    def _is_truthy(self, value: Any) -> bool:
        """Determine if a value is truthy"""
        if value is None:
            return False
        if isinstance(value, bool):
            return value
        if isinstance(value, (int, float)):
            return value != 0
        if isinstance(value, str):
            return len(value) > 0
        return True
    
    def _to_string(self, value: Any) -> str:
        """Convert value to string for output"""
        if value is None:
            return "Nil"
        if isinstance(value, bool):
            return "On" if value else "Off"
        if isinstance(value, float):
            # Remove .0 for whole numbers
            if value.is_integer():
                return str(int(value))
            return str(value)
        return str(value)
