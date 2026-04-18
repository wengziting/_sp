import unittest
from src.lexer import tokenize

class TestLexer(unittest.TestCase):
    def test_tokens(self):
        code = "[1, 2] |> sum"
        tokens = tokenize(code)
        self.assertEqual(tokens[0].type, 'LBRACK')
        self.assertEqual(tokens[-1].value, 'sum')

if __name__ == '__main__':
    unittest.main()
