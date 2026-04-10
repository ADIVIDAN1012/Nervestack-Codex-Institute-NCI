# parser.py

from symbol_table import SymbolTable
from lexer import Token
from nervestack_ast import (
    ProgramNode, NumberNode, BinaryOpNode, UnaryOpNode, VarAccessNode, VarAssignNode, ConstantDeclNode, 
    ShowStatementNode, NickDeclNode, FunctionDeclNode, ReturnStatementNode, FunctionCallNode, MethodCallNode, 
    StringNode, ExpressionStatementNode, DocstringNode, CheckStatementNode, TraverseNode, UntilNode, 
    AttemptTrapConcludeNode, BlueprintNode, AttributeAccessNode, SpawnNode, DenNode, ConvertNode, 
    ToolkitNode, PlugNode, BridgeNode, InletNode, LinkNode, InterpolatedStringNode, HaltNode, ProceedNode, 
    WaitNode, TriggerNode, BlameNode, TypeNode, KindNode, NickNode, HiddenNode, ShieldedNode, InternalNode, 
    EmbedNode, ParalNode, HoldNode, SignalNode, ListenNode, ConstructorNode, AskNode, AuthenNode, 
    TransformNode, CondenseNode, PackNode, UnpackNode, BooleanNode, NilNode,
    ModuleNode, BringNode, ContractNode
)




class Parser:
    def __init__(self, tokens: list[Token]):
        self.tokens = tokens
        self.position = 0
        self.current_token = self.tokens[self.position] if self.tokens else None
        self.symbol_table = SymbolTable()

    def advance(self):
        self.position += 1
        self.current_token = self.tokens[self.position] if self.position < len(self.tokens) else None

    def eat(self, token_type):
        if self.current_token and self.current_token.type == token_type:
            self.advance()
        else:
            raise Exception(f"Expected {token_type}, got {self.current_token.type if self.current_token else 'EOF'}")

    def parse(self):
        return self.program()

    def program(self):
        statements = []
        while self.current_token.type != 'EOF':
            statements.append(self.statement())
        return ProgramNode(statements)

    def statement(self):
        if self.current_token.type == 'FIRM':
            return self.parse_constant_declaration()

        elif self.current_token.type == 'NOTE':
            self.eat('NOTE')
            if self.current_token.type == 'STRING':
                docstring_value = self.current_token.value
                self.eat('STRING')
                return DocstringNode(docstring_value)
            else:
                raise Exception("Expected STRING after NOTE for docstring")
        elif self.current_token.type == 'SPEC' or self.current_token.type == 'PREP': # Changed from KEYWORD and value == 'spec'
            return self.parse_function_declaration()
        elif self.current_token.type == 'FORWARD': # Changed from KEYWORD and value == 'forward'
            self.eat('FORWARD') # Consume 'forward'
            expression = self.expression()
            return ReturnStatementNode(expression)
        elif self.current_token.type == 'SHOW':
            return self.parse_show_statement()
        elif self.current_token.type == 'WHEN':
            return self.parse_when_statement()
        elif self.current_token.type == 'TRAVERSE':
            return self.parse_traverse_statement()
        elif self.current_token.type == 'MODULE':
             return self.parse_module_declaration()
        elif self.current_token.type == 'BRING':
             return self.parse_bring_statement()
        elif self.current_token.type == 'CONTRACT':
             return self.parse_contract_declaration()
        elif self.current_token.type == 'UNTIL':
            return self.parse_until_statement()
        elif self.current_token.type == 'ATTEMPT':
            return self.parse_attempt_trap_conclude_statement()
        elif self.current_token.type == 'BLUEPRINT':
            return self.parse_blueprint_declaration()
        elif self.current_token.type == 'TOOLKIT':
            return self.parse_toolkit_declaration()
        elif self.current_token.type == 'PLUG':
            return self.parse_plug_statement()
        elif self.current_token.type == 'BRIDGE':
            return self.parse_bridge_declaration()
        elif self.current_token.type == 'INLET':
            return self.parse_inlet_block()
        elif self.current_token.type == 'LINK':
            return self.parse_link_statement()
        elif self.current_token.type == 'EXPOSE':
            return self.parse_expose_statement()
        elif self.current_token.type == 'SHARE':
            return self.parse_share_statement()
        elif self.current_token.type == 'HALT':
            return self.parse_halt_statement()
        elif self.current_token.type == 'PROCEED':
            return self.parse_proceed_statement()
        elif self.current_token.type == 'WAIT':
            return self.parse_wait_statement()
        elif self.current_token.type == 'TRIGGER':
            return self.parse_trigger_statement()
        elif self.current_token.type == 'BLAME':
            return self.parse_blame_statement()
        elif self.current_token.type == 'TYPE':
            return self.parse_type_statement()
        elif self.current_token.type == 'KIND':
            return self.parse_kind_statement()
        elif self.current_token.type == 'NICK':
            return self.parse_nick_statement()
        elif self.current_token.type == 'HIDDEN':
            return self.parse_hidden_statement()
        elif self.current_token.type == 'SHIELDED':
            return self.parse_shielded_statement()
        elif self.current_token.type == 'INTERNAL':
            return self.parse_internal_statement()
        elif self.current_token.type == 'EMBED':
            return self.parse_embed_statement()
        elif self.current_token.type == 'PARAL':
            return self.parse_paral_statement()
        elif self.current_token.type == 'HOLD':
            return self.parse_hold_statement()
        elif self.current_token.type == 'SIGNAL':
            return self.parse_signal_statement()
        elif self.current_token.type == 'LISTEN':
            return self.parse_listen_statement()
        elif self.current_token.type == 'ASK':
            return self.parse_ask_statement()
        elif self.current_token.type == 'AUTHEN':
            return self.parse_authen_statement()
        elif self.current_token.type == 'TRANSFORM':
            return self.parse_transform_statement()
        elif self.current_token.type == 'CONDENSE':
            return self.parse_condense_statement()
        elif self.current_token.type == 'PACK':
            return self.parse_pack_statement()
        elif self.current_token.type == 'UNPACK':
            return self.parse_unpack_statement()
        elif self.current_token.type == 'FUNCALL':
            return self.parse_function_call_statement()
        elif self.current_token.type == 'WORD' and self.current_token.value == 'funcall':
            self.eat('WORD')
            expr = self.expression()
            if isinstance(expr, VarAccessNode):
                expr = FunctionCallNode(expr.var_name, [])
            if not isinstance(expr, (FunctionCallNode, AttributeAccessNode)):
                raise Exception(f"Expected a function call or method call after 'funcall', but got {type(expr)}")
            return ExpressionStatementNode(expr)
        else:
            left = self.expression()
            if self.current_token and self.current_token.type == 'ASSIGN':
                self.eat('ASSIGN')
                right = self.expression()
                if not isinstance(left, (VarAccessNode, AttributeAccessNode)):
                    raise Exception("Invalid assignment target.")
                return VarAssignNode(target=left, value=right)
            return ExpressionStatementNode(left)

    # parse_function_call_statement removed

    def parse_function_declaration(self, exposed=False, shared=False):
        func_type = self.current_token.type
        if func_type != 'SPEC':
            raise Exception(f"Expected SPEC, got {func_type}")
        self.eat(func_type)

        function_name = None
        if self.current_token.type == 'WORD':
            function_name = self.current_token.value
            self.eat('WORD')
        
        # Params: 'with a, b'
        params = []
        if self.current_token.type == 'WITH':
            self.eat('WITH')
            # Check keywords if parameter name matches keyword or just WORD?
            if self.current_token.type in ('WORD', 'OWN'):
                params.append(self.current_token.value)
                if self.current_token.type == 'OWN':
                    self.eat('OWN')
                else: 
                     self.eat('WORD')

                while self.current_token.type == 'COMMA':
                    self.eat('COMMA')
                    if self.current_token.type in ('WORD', 'OWN'):
                         params.append(self.current_token.value)
                         if self.current_token.type == 'OWN':
                             self.eat('OWN')
                         else:
                             self.eat('WORD')
                    else:
                         raise Exception("Expected parameter name")

        # Return type? 'giving Type'
        if self.current_token.type == 'GIVING':
             self.eat('GIVING')
             # Consume type
             if self.current_token.type in ('NUM', 'TEXT', 'ON', 'OFF', 'NIL', 'WORD'):
                 self.advance()
             elif self.current_token.type in ('A', 'AN'): 
                 self.advance()
                 self.advance()

        self.eat('COLON')

        docstring = None
        if self.current_token.type == 'NOTE':
            self.eat('NOTE')
            if self.current_token.type == 'STRING':
                docstring = self.current_token.value
                self.eat('STRING')
            else:
                raise Exception("Expected STRING after NOTE")

        body = []
        while self.current_token.type not in ('DONE', 'EOF'):
            body.append(self.statement())
        self.eat('DONE')
        
        return FunctionDeclNode(function_name, params, body, func_type.lower(), exposed, shared, docstring=docstring)

    def parse_show_statement(self):
        self.eat('SHOW')
        # Allow no parens: show expr, expr
        expressions = []
        expressions.append(self.expression())
        while self.current_token.type == 'COMMA':
             self.eat('COMMA')
             expressions.append(self.expression())
        # No RPAREN check
        return ShowStatementNode(expressions)

    def parse_when_statement(self):
        self.eat('WHEN')
        condition = self.expression()
        self.eat('COLON')
        body = []
        while self.current_token.type not in ('OTHERWISE', 'DONE', 'EOF'):
            body.append(self.statement())
        
        alter_clauses = []
        altern_clause = None
        
        while self.current_token.type == 'OTHERWISE':
            self.eat('OTHERWISE')
            if self.current_token.type == 'WHEN':
                self.eat('WHEN')
                cond = self.expression()
                self.eat('COLON')
                block = []
                while self.current_token.type not in ('OTHERWISE', 'DONE', 'EOF'):
                    block.append(self.statement())
                alter_clauses.append((cond, block))
            else:
                self.eat('COLON')
                altern_clause = []
                while self.current_token.type not in ('DONE', 'EOF'):
                    altern_clause.append(self.statement())
                break # Only one else block allowed at end

        self.eat('DONE')
        return CheckStatementNode(condition, body, alter_clauses, altern_clause)

    def parse_traverse_statement(self):
        self.eat('TRAVERSE')
        
        # Allow .. or ... as dummy variable name
        if self.current_token.type in ('RANGE', 'ELLIPSIS'):
            var_name = "_"
            self.advance()
        else:
            var_name = self.current_token.value
            self.eat('WORD')
            
        self.eat('FROM')
        start_val = self.expression()
        self.eat('TO')
        end_val = self.expression()
        self.eat('COLON')
        
        body = []
        while self.current_token.type not in ('DONE', 'EOF'):
            body.append(self.statement())
        self.eat('DONE')
        
        # Synthetic range node using '..' operator token
        # We manually create the Token object as it's no longer produced by Lexer
        range_op = Token('RANGE', '..')
        iterable = BinaryOpNode(start_val, range_op, end_val)
        
        return TraverseNode(var_name, iterable, body)

    def expression(self):
        return self.logic_or()

    def logic_or(self):
        # Support 'either ... or ...'
        if self.current_token.type == 'EITHER':
             self.eat('EITHER')
             left = self.logic_and()
             # Must have OR now? 'either x or y'.
             # If strictly enforcing structure:
             if self.current_token.type == 'OR':
                 op = self.current_token
                 self.eat('OR')
                 right = self.logic_or() # recurse or logic_and? usually logic_or allows chaining
                 return BinaryOpNode(left=left, op=op, right=right)
             else:
                 # 'either' used but no 'or'? 
                 # Maybe 'either' acts as 'exists'? No.
                 # Allow 'either A' (truthy)?
                 # Plan says `either x == 0 or y == 0`.
                 # Let's start with standard logic_or loop, but if EITHER was eaten, we EXPECT OR?
                 pass

        node = self.logic_and()
        while self.current_token and self.current_token.type == 'OR':
            op = self.current_token
            self.eat('OR')
            right = self.logic_and()
            node = BinaryOpNode(left=node, op=op, right=right)
        return node

    def logic_and(self):
        # Support 'both ... and ...'
        if self.current_token.type == 'BOTH':
             self.eat('BOTH')
             left = self.comparison()
             if self.current_token.type == 'AND':
                 op = self.current_token
                 self.eat('AND')
                 right = self.logic_and() 
                 return BinaryOpNode(left=left, op=op, right=right)
             else:
                 pass
        
        node = self.comparison()
        while self.current_token and self.current_token.type == 'AND':
            op = self.current_token
            self.eat('AND')
            right = self.comparison()
            node = BinaryOpNode(left=node, op=op, right=right)
        return node

    def range_expr(self):
        node = self.addition()
        if self.current_token and self.current_token.type == 'RANGE':
             op = self.current_token
             self.eat('RANGE')
             right = self.addition()
             node = BinaryOpNode(left=node, op=op, right=right)
        return node

    def comparison(self):
        node = self.range_expr()
        while self.current_token and self.current_token.type in ('EQUALS', 'NOT_EQUALS', 'LESS_THAN', 'GREATER_THAN', 'LESS_THAN_EQUAL', 'GREATER_THAN_EQUAL', 'IS'):
            op = self.current_token
            if op.type == 'IS':
                self.eat('IS')
                if self.current_token.type in ('A', 'AN'):
                    self.eat(self.current_token.type)
                # Parse type
                target_type = self.current_token.type
                if target_type not in ('NUM', 'TEXT', 'ON', 'OFF', 'NIL', 'WORD'): # WORD for custom types
                     raise Exception(f"Expected type after IS, got {target_type}")
                # We need to treat this as a TypeCheckNode or BinaryOp
                # Let's create a specialized BinaryOp or TypeCheckNode
                # For now using BinaryOp with special right operand (TypeNode?)
                type_node = TypeNode(target_type if target_type != 'WORD' else self.current_token.value)
                self.advance()
                node = BinaryOpNode(left=node, op=op, right=type_node)
            else:
                self.eat(op.type)
                right = self.addition()
                node = BinaryOpNode(left=node, op=op, right=right)
        return node

    def addition(self):
        node = self.multiplication()
        while self.current_token and self.current_token.type in ('PLUS', 'MINUS'):
            op = self.current_token
            self.eat(op.type)
            right = self.multiplication()
            node = BinaryOpNode(left=node, op=op, right=right)
        return node

    def multiplication(self):
        node = self.unary()
        while self.current_token and self.current_token.type in ('MULTIPLY', 'DIVIDE'):
            op = self.current_token
            self.eat(op.type)
            right = self.unary()
            node = BinaryOpNode(left=node, op=op, right=right)
        return node

    def unary(self):
        if self.current_token and self.current_token.type == 'NOT':
            op = self.current_token
            self.eat('NOT')
            node = self.unary()
            return UnaryOpNode(op=op, right=node)
        return self.conversion()

    def conversion(self):
        node = self.primary()
        if self.current_token and self.current_token.type == 'AS':
            self.eat('AS')
            if self.current_token.type in ('A', 'AN'):
                self.eat(self.current_token.type)
            
            if self.current_token.type in ('NUM', 'TEXT', 'ON', 'OFF', 'NIL', 'WORD'):
                target_type = self.current_token.value if self.current_token.type == 'WORD' else self.current_token.type
                self.eat(self.current_token.type)
                return ConvertNode(node, target_type)
            else:
                raise Exception(f"Expected type for conversion, got {self.current_token.type}")
        return node

    def primary(self):
        token = self.current_token

        if token.type == 'NUMBER':
            self.eat('NUMBER')
            return NumberNode(token.value)
        elif token.type == 'STRING':
            # Handling code for string interpolation...
            if self.position + 1 < len(self.tokens) and self.tokens[self.position + 1].type == 'INTERPOLATION':
                parts = []
                while self.current_token.type in ('STRING', 'INTERPOLATION'):
                    if self.current_token.type == 'STRING':
                        parts.append(StringNode(self.current_token.value))
                        self.eat('STRING')
                    elif self.current_token.type == 'INTERPOLATION':
                        interp_expr = self.current_token.value
                        if interp_expr.strip() == 'peek':
                            parts.append(VarAccessNode('peek'))
                        else:
                            from lexer import Lexer
                            temp_lexer = Lexer(interp_expr)
                            temp_parser = self.__class__(temp_lexer.tokenize())
                            parts.append(temp_parser.expression())
                        self.eat('INTERPOLATION')
                return InterpolatedStringNode(parts)
            else:
                node = StringNode(self.current_token.value)
                self.eat('STRING')
                return node
        elif token.type == 'ON':
            self.eat('ON')
            return BooleanNode(True)
        elif token.type == 'OFF':
            self.eat('OFF')
            return BooleanNode(False)
        elif token.type == 'NIL':
            self.eat('NIL')
            return NilNode()
        elif token.type == 'SPAWN':
            return self.parse_spawn_expression()
        elif token.type == 'PACK':
            self.eat('PACK')
            self.eat('LPAREN')
            items = []
            if self.current_token.type != 'RPAREN':
                items.append(self.expression())
                while self.current_token.type == 'COMMA':
                    self.eat('COMMA')
                    items.append(self.expression())
            self.eat('RPAREN')
            return PackNode(items)
        
        # Removed DEN, CONVERT, KIND (legacy)

        node = None
        if token.type == 'LPAREN':
            self.eat('LPAREN')
            node = self.expression()
            self.eat('RPAREN')
        elif token.type == 'ASK':
            self.eat('ASK')
            if self.current_token.type == 'LPAREN':
                self.eat('LPAREN')
                prompt = None
                if self.current_token.type != 'RPAREN':
                    prompt = self.expression()
                self.eat('RPAREN')
                return AskNode([ExpressionStatementNode(prompt)] if prompt else [])
            else:
               # Support legacy ASK "prompt" or ASK identifier?
               # New syntax: simple function call ?
               # If ASK is used as expression, handle here.
               pass 

        elif token.type == 'WORD' or token.type == 'OWN':
            if token.type == 'WORD' and self.position + 1 < len(self.tokens) and self.tokens[self.position + 1].type == 'LPAREN':
                node = self.parse_function_call()
            else:
                self.eat(token.type)
                node = VarAccessNode(token.value)
        else:
            # Check if ASK handled above returned something, if not valid token
            if not isinstance(token.type, str) or token.type not in ('NUMBER', 'STRING', 'ON', 'OFF', 'NIL', 'SPAWN', 'LPAREN', 'ASK', 'WORD', 'OWN'):
                 raise Exception(f"Unexpected token in primary: {token.type}")

        while self.current_token and self.current_token.type == 'ACCESS':
            self.eat('ACCESS') # Consume ~>
            attribute = self.current_token.value
            self.eat('WORD')
            
            # Check for method call
            if self.current_token.type == 'LPAREN':
                 # It's a method call: obj~>method()
                 self.eat('LPAREN')
                 args = []
                 if self.current_token.type != 'RPAREN':
                     args.append(self.expression())
                     while self.current_token.type == 'COMMA':
                         self.eat('COMMA')
                         args.append(self.expression())
                 self.eat('RPAREN')
                 node = MethodCallNode(node, attribute, args)
            else:
                 node = AttributeAccessNode(node, attribute)
        
        return node


    def parse_constant_declaration(self):
        self.eat('FIRM') # Consume 'firm'
        const_name = self.current_token.value
        self.eat('WORD') # Consume constant name
        self.eat('ASSIGN') # Consume '='
        value = self.expression()
        self.symbol_table.define(const_name, 'FIRM')
        return ConstantDeclNode(const_name, value)

    def parse_function_call(self):
        function_name = self.current_token.value
        self.eat('WORD') # Consume function name
        self.eat('LPAREN') # Consume '('
        arguments = []
        if self.current_token.type != 'RPAREN':
            arguments.append(self.expression())
            while self.current_token.type == 'COMMA':
                self.eat('COMMA')
                arguments.append(self.expression())
        self.eat('RPAREN') # Consume ')'
        return FunctionCallNode(function_name, arguments)

    def parse_spawn_expression(self):
        self.eat('SPAWN')
        blueprint_name = self.current_token.value
        self.eat('WORD')
        
        arguments = []
        if self.current_token.type == 'WITH':
            self.eat('WITH')
            arguments.append(self.expression())
            while self.current_token.type == 'COMMA':
                self.eat('COMMA')
                arguments.append(self.expression())
        
        return SpawnNode(blueprint_name, arguments)

    # parse_traverse_statement removed (superceded by parse_each_statement at line 420)

    def parse_until_statement(self):
        self.eat('UNTIL')
        condition = self.expression()
        self.eat('COLON')
        body = []
        while self.current_token.type not in ('DONE', 'EOF'):
            body.append(self.statement())
        self.eat('DONE')
        return UntilNode(condition, body)

    def parse_attempt_trap_conclude_statement(self):
        self.eat('ATTEMPT')
        self.eat('COLON')
        attempt_body = []
        while self.current_token.type not in ('TRAP', 'ALWAYS', 'DONE', 'EOF'):
            attempt_body.append(self.statement())
        
        trap_clauses = []
        peek = False # Deprecated? Plan didn't mention peek. Assuming no peek logic or handled in trap.
        
        while self.current_token.type == 'TRAP':
            self.eat('TRAP')
            error_type = self.current_token.value
            self.eat('WORD')
            self.eat('COLON')
            trap_body = []
            while self.current_token.type not in ('TRAP', 'ALWAYS', 'DONE', 'EOF'):
                trap_body.append(self.statement())
            trap_clauses.append((error_type, trap_body))
        
        conclude_clause = None
        if self.current_token.type == 'ALWAYS':
            self.eat('ALWAYS')
            self.eat('COLON')
            conclude_clause = []
            while self.current_token.type not in ('DONE', 'EOF'):
                conclude_clause.append(self.statement())
        
        self.eat('DONE')
        return AttemptTrapConcludeNode(attempt_body, trap_clauses, conclude_clause, peek=peek)

    def parse_each_statement(self):
        self.eat('EACH')
        var_name = self.current_token.value
        self.eat('WORD')
        self.eat('IN')
        iterable = self.expression()
        self.eat('COLON')
        body = []
        while self.current_token.type not in ('DONE', 'EOF'):
            body.append(self.statement())
        self.eat('DONE')
        return TraverseNode(var_name, iterable, body)

    def parse_until_statement(self):
        self.eat('UNTIL')
        condition = self.expression()
        self.eat('COLON')
        body = []
        while self.current_token.type not in ('DONE', 'EOF'):
            body.append(self.statement())
        self.eat('DONE')
        return UntilNode(condition, body)

    def parse_blueprint_declaration(self):
        self.eat('BLUEPRINT')
        name = self.current_token.value
        self.eat('WORD')

        parent = None
        if self.current_token.type == 'ADOPT':
            self.eat('ADOPT')
            parent = self.current_token.value
            self.eat('WORD')
        
        contracts = []
        if self.current_token.type == 'FULFILLS':
            self.eat('FULFILLS')
            contracts.append(self.current_token.value)
            self.eat('WORD')
            while self.current_token.type == 'COMMA':
                self.eat('COMMA')
                contracts.append(self.current_token.value)
                self.eat('WORD')

        self.eat('COLON')
        
        docstring = None
        if self.current_token.type == 'NOTE':
            self.eat('NOTE')
            if self.current_token.type == 'STRING':
                docstring = self.current_token.value
                self.eat('STRING')
        
        attributes = []
        methods = []
        constructor = None
        
        while self.current_token.type not in ('DONE', 'EOF'):
            if self.current_token.type == 'HAS':
                self.eat('HAS')
                attr_name = self.current_token.value
                self.eat('WORD')
                value = None
                if self.current_token.type == 'ASSIGN':
                    self.eat('ASSIGN')
                    value = self.expression()
                attributes.append(VarAssignNode(VarAccessNode(attr_name), value))
            elif self.current_token.type in ('DOES', 'SPEC'):
                func_type = 'does' if self.current_token.type == 'DOES' else 'spec'
                self.eat(self.current_token.type)
                method_name = self.current_token.value
                self.eat('WORD')
                params = []
                if self.current_token.type == 'WITH':
                    self.eat('WITH')
                    if self.current_token.type in ('WORD', 'OWN'):
                        params.append(self.current_token.value)
                        if self.current_token.type == 'OWN':
                            self.eat('OWN')
                        else:
                            self.eat('WORD')
                    
                    while self.current_token.type == 'COMMA':
                        self.eat('COMMA')
                        if self.current_token.type in ('WORD', 'OWN'):
                             params.append(self.current_token.value)
                             if self.current_token.type == 'OWN':
                                 self.eat('OWN')
                             else:
                                 self.eat('WORD')
                        else:
                             raise Exception("Expected parameter name")
                
                # Method return type? 'does name ... giving Type'
                if self.current_token.type == 'GIVING':
                     self.eat('GIVING')
                     if self.current_token.type in ('NUM', 'TEXT', 'ON', 'OFF', 'NIL', 'WORD'):
                         self.advance()
                     elif self.current_token.type in ('A', 'AN'): 
                         self.advance()
                         self.advance()

                self.eat('COLON')
                m_docstring = None
                if self.current_token.type == 'NOTE':
                    self.eat('NOTE')
                    m_docstring = self.current_token.value
                    self.eat('STRING')

                body = []
                while self.current_token.type not in ('DONE', 'EOF'):
                    body.append(self.statement())
                self.eat('DONE')
                
                params.insert(0, 'own')
                methods.append(FunctionDeclNode(method_name, params, body, func_type, docstring=m_docstring))
            elif self.current_token.type in ('MAKE', 'PREP'):
                 self.eat(self.current_token.type)
                 params = []
                 # PREP uses (params), MAKE uses WITH params
                 if self.current_token.type == 'LPAREN':
                     self.eat('LPAREN')
                     # Handle empty params or first param
                     if self.current_token.type != 'RPAREN':
                         if self.current_token.type in ('WORD', 'OWN'):
                             params.append(self.current_token.value)
                             self.eat(self.current_token.type)
                         else:
                             raise Exception(f"Expected parameter name, got {self.current_token.type}")
                         
                         while self.current_token.type == 'COMMA':
                             self.eat('COMMA')
                             if self.current_token.type in ('WORD', 'OWN'):
                                 params.append(self.current_token.value)
                                 self.eat(self.current_token.type)
                             else:
                                 raise Exception(f"Expected parameter name after comma, got {self.current_token.type}")
                     self.eat('RPAREN')
                 elif self.current_token.type == 'WITH':
                     self.eat('WITH')
                     if self.current_token.type in ('WORD', 'OWN'):
                         params.append(self.current_token.value)
                         if self.current_token.type == 'OWN':
                             self.eat('OWN')
                         else:
                             self.eat('WORD')
                     while self.current_token.type == 'COMMA':
                         self.eat('COMMA')
                         if self.current_token.type in ('WORD', 'OWN'):
                             params.append(self.current_token.value)
                             if self.current_token.type == 'OWN':
                                 self.eat('OWN')
                             else:
                                 self.eat('WORD')
                         else:
                             raise Exception("Expected parameter name")

                 self.eat('COLON')
                 c_body = []
                 while self.current_token.type not in ('DONE', 'EOF'):
                     c_body.append(self.statement())
                 self.eat('DONE')
                 params.insert(0, 'own')
                 constructor = ConstructorNode(params, c_body)
            else:
                 if self.current_token.type == 'DONE':
                     break
                 raise Exception(f"Unexpected token in blueprint: {self.current_token.type}")
        
        self.eat('DONE')
        # Update AST node to accept contracts (needs modification in AST? Using 'parent' field for now or dict?)
        # BlueprintNode definition might need update.
        # For now passing contracts in parent if list, or just updating logic.
        # But wait, parent is single. Contracts is list.
        # I should probably update BlueprintNode to accept contracts.
        # Implementing as dict return for now if BlueprintNode not updated? 
        # No, update BlueprintNode.
        return BlueprintNode(name, attributes, methods, docstring=docstring, constructor=constructor, parent=parent, contracts=contracts)

    def parse_spawn_expression(self):
        self.eat('SPAWN')
        blueprint_name = self.current_token.value
        self.eat('WORD')
        self.eat('LPAREN')
        arguments = []
        if self.current_token.type != 'RPAREN':
            arguments.append(self.expression())
            while self.current_token.type == 'COMMA':
                self.eat('COMMA')
                arguments.append(self.expression())
        self.eat('RPAREN')
        return SpawnNode(blueprint_name, arguments)

    def parse_contract_declaration(self):
        self.eat('CONTRACT')
        name = self.current_token.value
        self.eat('WORD')
        self.eat('COLON')
        body = []
        while self.current_token.type not in ('DONE', 'EOF'):
             if self.current_token.type == 'NEEDS':
                 self.eat('NEEDS')
                 if self.current_token.type in ('SPEC', 'DOES'):
                      spec_type = self.current_token.type.lower()
                      self.eat(self.current_token.type)
                      fname = self.current_token.value
                      self.eat('WORD')
                      
                      params = []
                      if self.current_token.type == 'WITH':
                          self.eat('WITH')
                          if self.current_token.type in ('WORD', 'OWN'):
                              params.append(self.current_token.value)
                              if self.current_token.type == 'OWN':
                                  self.eat('OWN')
                              else:
                                  self.eat('WORD')
                          while self.current_token.type == 'COMMA':
                              self.eat('COMMA')
                              if self.current_token.type in ('WORD', 'OWN'):
                                  params.append(self.current_token.value)
                                  if self.current_token.type == 'OWN':
                                      self.eat('OWN')
                                  else:
                                      self.eat('WORD')
                                      
                      # Return type
                      if self.current_token.type == 'GIVING':
                           self.eat('GIVING')
                           if self.current_token.type in ('NUM', 'TEXT', 'ON', 'OFF', 'NIL', 'WORD'):
                               self.advance()
                           elif self.current_token.type in ('A', 'AN'): 
                               self.advance()
                               self.advance()
                      
                      # No body, just signature. 
                      # Store as FunctionDeclNode with body=None?
                      body.append(FunctionDeclNode(fname, params, None, spec_type))
                 else:
                      raise Exception("Expected SPEC or DOES after NEEDS")
             else:
                  # Allow comments or other statements?
                  # Strict contract?
                  # If we hit NEEDS, handled. If not, try statement?
                  # Maybe restrict to NEEDS only?
                  if self.current_token.type == 'DONE':
                      break
                  # Comments might look like statements (Note?)
                  if self.current_token.type == 'NOTE':
                       self.eat('NOTE')
                       self.eat('STRING')
                       continue
                  raise Exception(f"Unexpected token in contract: {self.current_token.type}")
                  
        self.eat('DONE')
        return ContractNode(name, body)

    def parse_constructor_declaration(self):
        self.eat('PREP') # Consume 'prep'
        self.eat('LPAREN') # Consume '('
        params = []
        if self.current_token.type == 'WORD' or self.current_token.type == 'OWN':
            params.append(self.current_token.value)
            self.eat(self.current_token.type)
            while self.current_token.type == 'COMMA':
                self.eat('COMMA')
                params.append(self.current_token.value)
                self.eat(self.current_token.type)
        self.eat('RPAREN')

        body = []
        self.eat('LBRACE')
        while self.current_token.type != 'RBRACE':
            body.append(self.statement())
        self.eat('RBRACE') # Consume '}'
        
        return ConstructorNode(params, body)

    def parse_den_expression(self):
        self.eat('DEN')
        params = []
        if self.current_token.type == 'WORD':
            params.append(self.current_token.value)
            self.eat('WORD')
            while self.current_token.type == 'COMMA':
                self.eat('COMMA')
                params.append(self.current_token.value)
                self.eat('WORD')
        self.eat('COLON')
        body = self.expression()
        return DenNode(params, body)

    def parse_convert_expression(self):
        self.eat('CONVERT')
        expression = self.expression()
        self.eat('TO')
        if self.current_token.type in ['WORD', 'NUM', 'TEXT', 'ON', 'OFF', 'NIL']:
            target_type = self.current_token.value if self.current_token.type == 'WORD' else self.current_token.type
            self.eat(self.current_token.type)
        else:
            raise Exception(f"Expected type name, got {self.current_token.type}")
        return ConvertNode(expression, target_type)

    def parse_toolkit_declaration(self):
        self.eat('TOOLKIT')
        name = self.current_token.value
        self.eat('WORD')
        self.eat('LBRACE')
        body = []
        while self.current_token.type != 'RBRACE':
            body.append(self.statement())
        self.eat('RBRACE')
        return ToolkitNode(name, body)

    def parse_plug_statement(self):
        self.eat('PLUG')
        toolkit_name = self.current_token.value
        self.eat('WORD')
        self.eat('FROM')
        file_path = self.expression()
        return PlugNode(toolkit_name, file_path)

    def parse_bridge_declaration(self):
        self.eat('BRIDGE')
        name = self.current_token.value
        self.eat('WORD')
        self.eat('LBRACE')
        body = []
        while self.current_token.type != 'RBRACE':
            body.append(self.statement())
        self.eat('RBRACE')
        return BridgeNode(name, body)

    def parse_inlet_block(self):
        self.eat('INLET')
        self.eat('LBRACE')
        body = []
    def parse_expose_statement(self):
        self.eat('EXPOSE')
        if self.current_token.type == 'SPEC':
            return self.parse_function_declaration(exposed=True)
        else:
            raise Exception("Expected SPEC after EXPOSE")

    def parse_share_statement(self):
        self.eat('SHARE')
        if self.current_token.type == 'SPEC':
            return self.parse_function_declaration(shared=True)
        else:
            raise Exception("Expected SPEC after SHARE")

    def parse_halt_statement(self):
        self.eat('HALT')
        return HaltNode()

    def parse_proceed_statement(self):
        self.eat('PROCEED')
        return ProceedNode()

    def parse_wait_statement(self):
        self.eat('WAIT')
        duration = self.expression()
        return WaitNode(duration)

    def parse_trigger_statement(self):
        self.eat('TRIGGER')
        # error type is a bare identifier string
        error_name = self.current_token.value
        self.eat('WORD')
        self.eat('LPAREN')
        message = self.expression()
        self.eat('RPAREN')
        return TriggerNode(error_name, message)

    def parse_shielded_statement(self):
        self.eat('SHIELDED')
        declaration = self.statement()
        return ShieldedNode(declaration)

    def parse_internal_statement(self):
        self.eat('INTERNAL')
        declaration = self.statement()
        return InternalNode(declaration)

    def parse_embed_statement(self):
        self.eat('EMBED')
        block = []
        if self.current_token.type == 'LBRACE':
            self.eat('LBRACE')
            while self.current_token.type != 'RBRACE':
                block.append(self.statement())
            self.eat('RBRACE')
        else:
            expr = self.expression()
            block.append(ExpressionStatementNode(expr))
        return EmbedNode(block)

    def parse_paral_statement(self):
        self.eat('PARAL')
        block = []
        if self.current_token.type == 'LBRACE':
            self.eat('LBRACE')
            while self.current_token.type != 'RBRACE':
                block.append(self.statement())
            self.eat('RBRACE')
        else:
            expr = self.expression()
            block.append(ExpressionStatementNode(expr))
        return ParalNode(block)

    def parse_hold_statement(self):
        self.eat('HOLD')
        block = []
        if self.current_token.type == 'LBRACE':
            self.eat('LBRACE')
            while self.current_token.type != 'RBRACE':
                block.append(self.statement())
            self.eat('RBRACE')
        else:
            expr = self.expression()
            block.append(ExpressionStatementNode(expr))
        return HoldNode(block)

    def parse_signal_statement(self):
        self.eat('SIGNAL')
        block = []
        # optional event expression
        if self.current_token.type != 'LBRACE':
            expr = self.expression()
            block.append(ExpressionStatementNode(expr))
        if self.current_token and self.current_token.type == 'LBRACE':
            self.eat('LBRACE')
            while self.current_token.type != 'RBRACE':
                block.append(self.statement())
            self.eat('RBRACE')
        return SignalNode(block)

    def parse_listen_statement(self):
        self.eat('LISTEN')
        block = []
        # event expression followed by optional block
        if self.current_token.type != 'LBRACE':
            expr = self.expression()
            block.append(ExpressionStatementNode(expr))
        if self.current_token and self.current_token.type == 'LBRACE':
            self.eat('LBRACE')
            while self.current_token.type != 'RBRACE':
                block.append(self.statement())
            self.eat('RBRACE')
        return ListenNode(block)

    def parse_ask_statement(self):
        self.eat('ASK')
        block = []
        if self.current_token.type == 'LBRACE':
            self.eat('LBRACE')
            while self.current_token.type != 'RBRACE':
                block.append(self.statement())
            self.eat('RBRACE')
        else:
            expr = self.expression()
            block.append(ExpressionStatementNode(expr))
        return AskNode(block)

    def parse_authen_statement(self):
        self.eat('AUTHEN')
        block = []
        if self.current_token.type == 'LBRACE':
            self.eat('LBRACE')
            while self.current_token.type != 'RBRACE':
                block.append(self.statement())
            self.eat('RBRACE')
        else:
            expr = self.expression()
            block.append(ExpressionStatementNode(expr))
        return AuthenNode(block)

    def parse_transform_statement(self):
        self.eat('TRANSFORM')
        block = []
        if self.current_token.type == 'LBRACE':
            self.eat('LBRACE')
            while self.current_token.type != 'RBRACE':
                block.append(self.statement())
            self.eat('RBRACE')
        else:
            expr = self.expression()
            block.append(ExpressionStatementNode(expr))
        return TransformNode(block)

    def parse_condense_statement(self):
        self.eat('CONDENSE')
        block = []
        if self.current_token.type == 'LBRACE':
            self.eat('LBRACE')
            while self.current_token.type != 'RBRACE':
                block.append(self.statement())
            self.eat('RBRACE')
        else:
            expr = self.expression()
            block.append(ExpressionStatementNode(expr))
        return CondenseNode(block)

    def parse_pack_statement(self):
        self.eat('PACK')
        self.eat('LBRACE')  # {
        items = []
        if self.current_token.type != 'RBRACE':
            items.append(self.expression())
            while self.current_token.type == 'COMMA':
                self.eat('COMMA')
                items.append(self.expression())
        self.eat('RBRACE')  # }
        return PackNode(items)
    def parse_share_statement(self):
        self.eat('SHARE')
        stmt = self.statement()
        if hasattr(stmt, 'shared'):
            stmt.shared = True
        return stmt

    def parse_module_declaration(self):
        self.eat('MODULE')
        name = self.current_token.value
        self.eat('WORD')
        self.eat('COLON')
        body = []
        while self.current_token.type not in ('DONE', 'EOF'):
            body.append(self.statement())
        self.eat('DONE')
        return ModuleNode(name, body)

    def parse_bring_statement(self):
        self.eat('BRING')
        modules = []
        modules.append(self.current_token.value)
        self.eat('WORD')
        while self.current_token.type == 'COMMA':
            self.eat('COMMA')
            modules.append(self.current_token.value)
            self.eat('WORD')
        
        source = None
        if self.current_token.type == 'FROM':
            self.eat('FROM')
            source = self.current_token.value
            self.eat('STRING')
            
        return BringNode(modules, source)

    def parse_contract_declaration(self):
        self.eat('CONTRACT')
        name = self.current_token.value
        self.eat('WORD')
        self.eat('COLON')
        body = []
        while self.current_token.type not in ('DONE', 'EOF'):
             if self.current_token.type == 'NEEDS':
                 self.eat('NEEDS')
                 # Expect function signature: "needs name with params [giving type]"
                 if self.current_token.type == 'WORD':
                     fname = self.current_token.value
                     self.eat('WORD')
                 else:
                     raise Exception("Expected function name in contract needs")
                     
                 params = []
                 if self.current_token.type == 'WITH':
                     self.eat('WITH')
                     if self.current_token.type in ('WORD', 'OWN'):
                         params.append(self.current_token.value)
                         if self.current_token.type == 'OWN':
                             self.eat('OWN')
                         else:
                             self.eat('WORD')
                     while self.current_token.type == 'COMMA':
                         self.eat('COMMA')
                         if self.current_token.type in ('WORD', 'OWN'):
                             params.append(self.current_token.value)
                             if self.current_token.type == 'OWN':
                                 self.eat('OWN')
                             else:
                                 self.eat('WORD')
                                 
                 # Return type
                 if self.current_token.type == 'GIVING':
                      self.eat('GIVING')
                      if self.current_token.type in ('NUM', 'TEXT', 'ON', 'OFF', 'NIL', 'WORD'):
                          self.advance()
                      elif self.current_token.type in ('A', 'AN'): 
                          self.advance()
                          self.advance()
                 
                 # Store as FunctionDeclNode with body=None
                 body.append(FunctionDeclNode(fname, params, None, 'needs'))
             else:
                  if self.current_token.type == 'DONE':
                      break
                  if self.current_token.type == 'NOTE':
                       self.eat('NOTE')
                       if self.current_token.type == 'STRING':
                           self.eat('STRING')
                       continue
                  # If we hit statements, that's an issue for contracts generally, but let's be strict
                  raise Exception(f"Unexpected token in contract: {self.current_token.type}")
                  
        self.eat('DONE')
        return ContractNode(name, body)
