import sys
import os
from src.lexer import tokenize
from src.interpreter import FluxInterpreter

def main():
    # 檢查是否有傳入檔案路徑
    if len(sys.argv) < 2:
        print("Usage: flux <filename.flux>")
        sys.exit(1)

    file_path = sys.argv[1]
    
    if not os.path.exists(file_path):
        print(f"Error: File '{file_path}' not found.")
        sys.exit(1)

    # 讀取並執行
    with open(file_path, 'r', encoding='utf-8') as f:
        code = f.read()
        
    try:
        tokens = tokenize(code)
        interpreter = FluxInterpreter()
        interpreter.run(tokens)
    except Exception as e:
        print(f"❌ Runtime Error: {e}")

if __name__ == "__main__":
    main()
