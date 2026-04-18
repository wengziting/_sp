class FluxInterpreter:
    def __init__(self):
        # 內建函數庫
        self.registry = {
            "sum": sum,
            "max": max,
            "min": min,
            "output": lambda x: print(f"🔥 Flux Output: {x}")
        }

    def run(self, tokens):
        data = []
        i = 0
        
        # 簡單的解析與執行循環
        while i < len(tokens):
            token = tokens[i]
            
            # 1. 處理陣列輸入 [1, 2, 3]
            if token.type == 'LBRACK':
                i += 1
                while tokens[i].type != 'RBRACK':
                    if tokens[i].type == 'NUMBER':
                        data.append(tokens[i].value)
                    i += 1
            
            # 2. 處理管道操作 |>
            elif token.type == 'PIPE':
                i += 1
                func_name = tokens[i].value
                if func_name in self.registry:
                    data = self.registry[func_name](data)
                else:
                    raise NameError(f"Function {func_name} is not defined.")
            
            # 3. 處理最終流向 ->
            elif token.type == 'ARROW':
                i += 1
                sink_name = tokens[i].value
                if sink_name in self.registry:
                    self.registry[sink_name](data)
                else:
                    raise NameError(f"Sink {sink_name} is not defined.")
            
            i += 1

# 測試代碼
if __name__ == "__main__":
    example_code = "[10, 20, 30] |> sum -> output"
    from lexer import tokenize
    tokens = tokenize(example_code)
    interpreter = FluxInterpreter()
    interpreter.run(tokens)
