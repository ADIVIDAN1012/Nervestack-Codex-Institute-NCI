# lexer.py

class Token:
    def __init__(self, type, value=None):
        self.type = type
        self.value = value

    def __repr__(self):
        if self.value:
            return f"Token({self.type}, {self.value})"
        return f"Token({self.type})"

class Lexer:
    KEYWORDS = {
        'firm': 'FIRM',
        'show': 'SHOW',
        'spec': 'SPEC',
        'note': 'NOTE',
        'forward': 'FORWARD',
        'giving': 'GIVING',
        'when': 'WHEN',
        'otherwise': 'OTHERWISE',
        'done': 'DONE',
        'traverse': 'TRAVERSE',
        'from': 'FROM',
        'to': 'TO',
        'until': 'UNTIL',
        'halt': 'HALT',
        'proceed': 'PROCEED',
        'wait': 'WAIT',
        'attempt': 'ATTEMPT',
        'trap': 'TRAP',
        'always': 'ALWAYS',
        'trigger': 'TRIGGER',
        'blame': 'BLAME',
        'peek': 'PEEK',
        'Num': 'NUM', # Types
        'Text': 'TEXT',
        'On': 'ON',
        'Off': 'OFF',
        'Nil': 'NIL',
        'is': 'IS',
        'as': 'AS',
        'a': 'A',
        'an': 'AN',
        'blueprint': 'BLUEPRINT',
        'when': 'WHEN',
        'otherwise': 'OTHERWISE',
        'has': 'HAS',
        'does': 'DOES',
        'make': 'MAKE',
        'own': 'OWN',
        'adopt': 'ADOPT',
        'spawn': 'SPAWN',
        'module': 'MODULE',
        'share': 'SHARE',
        'bring': 'BRING',
        'contract': 'CONTRACT',
        'needs': 'NEEDS',
        'fulfills': 'FULFILLS',
        'both': 'BOTH',
        'either': 'EITHER',
        'and': 'AND',
        'or': 'OR',
        'ask': 'ASK',
        'ask': 'ASK',
        'with': 'WITH',
        'signal': 'SIGNAL',
        'listen': 'LISTEN',
        'pack': 'PACK',
        'unpack': 'UNPACK',
        'prep': 'PREP',
    }

    def __init__(self, source_code: str):
        self.source_code = source_code
        self.position = 0
        self.current_char = self.source_code[self.position] if self.source_code else None

    def advance(self):
        self.position += 1
        if self.position < len(self.source_code):
            self.current_char = self.source_code[self.position]
        else:
            self.current_char = None

    def peek(self):
        peek_pos = self.position + 1
        if peek_pos < len(self.source_code):
            return self.source_code[peek_pos]
        return None

    def tokenize(self):
        tokens = []
        while self.current_char is not None:
            if self.current_char.isspace():
                self.skip_whitespace()
                continue
            elif self.current_char.isdigit():
                tokens.append(self.get_number())
            elif self.current_char.isalpha() or self.current_char == '_':
                tokens.append(self.get_word())
            elif self.current_char == '<':
                if self.peek() == '^':
                    self.skip_multi_line_comment()
                    continue
                elif self.peek() == '=':
                    tokens.append(Token('LESS_THAN_EQUAL', '<='))
                    self.advance()
                    self.advance()
                else:
                    # Check if it's a single-line comment (ends with > on the same line)
                    is_comment = False
                    temp_pos = self.position + 1
                    while temp_pos < len(self.source_code) and self.source_code[temp_pos] != '\n':
                        if self.source_code[temp_pos] == '>':
                            is_comment = True
                            break
                        temp_pos += 1
                    
                    if is_comment:
                        self.skip_single_line_comment()
                        continue
                    else:
                        tokens.append(Token('LESS_THAN', '<'))
                        self.advance()
            elif self.current_char == '+':
                tokens.append(Token('PLUS', '+'))
                self.advance()
            elif self.current_char == '-':
                tokens.append(Token('MINUS', '-'))
                self.advance()
            elif self.current_char == '*':
                tokens.append(Token('MULTIPLY', '*'))
                self.advance()
            elif self.current_char == '/':
                tokens.append(Token('DIVIDE', '/'))
                self.advance()
            elif self.current_char == '=':
                if self.peek() == '=':
                    tokens.append(Token('EQUALS', '=='))
                    self.advance()
                    self.advance()
                else:
                    tokens.append(Token('ASSIGN', '='))
                    self.advance()
            elif self.current_char == '>':
                if self.peek() == '=':
                    tokens.append(Token('GREATER_THAN_EQUAL', '>='))
                    self.advance()
                    self.advance()
                else:
                    tokens.append(Token('GREATER_THAN', '>'))
                    self.advance()

            elif self.current_char == "'":
                if self.peek() == '=':
                    tokens.append(Token('NOT_EQUALS', "'="))
                    self.advance()
                    self.advance()
                else:
                    tokens.append(Token('NOT', "'"))
                    self.advance()
            elif self.current_char == '(':
                tokens.append(Token('LPAREN', '('))
                self.advance()
            elif self.current_char == ')':
                tokens.append(Token('RPAREN', ')'))
                self.advance()
            elif self.current_char == '[':
                tokens.append(Token('LBRACKET', '['))
                self.advance()
            elif self.current_char == ']':
                tokens.append(Token('RBRACKET', ']'))
                self.advance()
            elif self.current_char == '{':
                tokens.append(Token('LBRACE', '{'))
                self.advance()
            elif self.current_char == '}':
                tokens.append(Token('RBRACE', '}'))
                self.advance()
            elif self.current_char == ',':
                tokens.append(Token('COMMA', ','))
                self.advance()
                # The original code had a block here for '~>' access which is now handled by the '~' block.
                # The original code also had a deprecated dot access error here, which is now handled by the '.' block.
            elif self.current_char == '.':
                dot_count = 0
                while self.current_char == '.':
                    dot_count += 1
                    self.advance()
                
                if dot_count == 2:
                    tokens.append(Token('RANGE', '..'))
                elif dot_count >= 3:
                    tokens.append(Token('ELLIPSIS', '.' * dot_count))
                else:
                    # Deprecated dot access (single dot)
                    start = max(0, self.position - 10)
                    end = min(len(self.source_code), self.position + 10)
                    context = self.source_code[start:end]
                    raise Exception(f"Unexpected character '.' at pos {self.position} near '{context}' (Use '~>' for access)")
            elif self.current_char == '~':
                if self.peek() == '>':
                    tokens.append(Token('ACCESS', '~>'))
                    self.advance()
                    self.advance()
                else:
                    raise Exception(f"Unknown character: {self.current_char}")
            elif self.current_char == ':':
                tokens.append(Token('COLON', ':'))
                self.advance()
            elif self.current_char == '"':  # Handle string literals
                tokens.extend(self.get_string_tokens())
            else:
                raise Exception(f"Unknown character: {self.current_char}")
        # Add EOF token
        tokens.append(Token('EOF', None))
        return tokens

    def get_string_tokens(self):
        tokens = []
        self.advance()  # Consume opening quote
        current_string_part = ''
        while self.current_char is not None and self.current_char != '"':
            if self.current_char == '|':
                if current_string_part:
                    tokens.append(Token('STRING', current_string_part))
                    current_string_part = ''
                self.advance()  # Consume first '|'
                expression_str = ''
                while self.current_char is not None and self.current_char != '|':
                    expression_str += self.current_char
                    self.advance()
                if not expression_str:
                    raise Exception("Expected expression after '|' in interpolated string")
                
                # Instead of tokenizing here, we will let the parser handle it.
                # For now, we can just create a special token for the expression.
                tokens.append(Token('INTERPOLATION', expression_str))

                if self.current_char != '|':
                    raise Exception("Expected '|' after expression in interpolated string")
                self.advance()  # Consume second '|'
            else:
                current_string_part += self.current_char
                self.advance()
        self.advance()  # Consume closing quote
        if current_string_part:
            tokens.append(Token('STRING', current_string_part))
        return tokens

    def skip_whitespace(self):
        while self.current_char is not None and self.current_char.isspace():
            self.advance()

    def skip_single_line_comment(self):
        self.advance() # Consume '<'
        while self.current_char is not None and self.current_char != '>':
            self.advance()
        self.advance() # Consume '>'

    def skip_multi_line_comment(self):
        self.advance() # Consume '<'
        self.advance() # Consume '^'
        while self.current_char is not None:
            if self.current_char == '^':
                self.advance()
                if self.current_char == '>':
                    self.advance() # Consume '>'
                    return
            self.advance()
        raise Exception("Unterminated multi-line comment")

    def get_word(self):
        result = ''
        while self.current_char is not None and (self.current_char.isalnum() or self.current_char == '_'):
            result += self.current_char
            self.advance()
        token_type = self.KEYWORDS.get(result, 'WORD')
        return Token(token_type, result)

    def get_number(self):
        result = ''
        while self.current_char is not None and self.current_char.isdigit():
            result += self.current_char
            self.advance()
        if self.current_char == '.':
            # Check if it's a range operator '..'
            if self.peek() == '.':
                return Token('NUMBER', float(result))
            
            result += self.current_char
            self.advance()
            while self.current_char is not None and self.current_char.isdigit():
                result += self.current_char
                self.advance()
        return Token('NUMBER', float(result))
