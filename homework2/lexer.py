import re

class Token:
    def __init__(self, type, value):
        self.type = type
        self.value = value
    def __repr__(self):
        return f"Token({self.type}, {self.value})"

def tokenize(code):
    rules = [
        ('NUMBER',  r'\d+'),
        ('ARROW',   r'->'),
        ('PIPE',    r'\|>'),
        ('LBRACK',  r'\['),
        ('RBRACK',  r'\]'),
        ('COMMA',   r','),
        ('ID',      r'[a-zA-Z_]\w*'),
        ('SKIP',    r'[ \t\n]+'),
        ('MISMATCH',r'.'),
    ]
    tokens = []
    for match in re.finditer('|'.join(f'(?P<{name}>{pattern})' for name, pattern in rules), code):
        kind = match.lastgroup
        value = match.group()
        if kind == 'NUMBER': value = int(value)
        elif kind == 'SKIP': continue
        elif kind == 'MISMATCH': raise RuntimeError(f'Unexpected character: {value}')
        tokens.append(Token(kind, value))
    return tokens
