const memory = new Uint8Array(256); 
let regA = 0;                      
let regB = 0;                      
let pc = 0;                        
let running = true;

memory[0] = 0x01; // LOAD 指令
memory[1] = 50;   // Operand: 資料 50
memory[2] = 0x00; // HALT 指令

console.log("=== 虛擬電腦開始運作 ===");

while (running) {
    let instruction = memory[pc]; 
    console.log(`[Fetch] 讀取地址 0x${pc.toString(16).toUpperCase()}: 指令 0x${instruction.toString(16).toUpperCase()}`);
    pc++;
    switch (instruction) {
        case 0x01: // LOAD
            regA = memory[pc];
            console.log(`[Execute] LOAD 執行：把數值 ${regA} 搬入暫存器 A`);
            pc++;
            break;
            
        case 0x02: // ADD
            regA = regA + regB;
            console.log(`[Execute] ADD 執行：A = A + B，當前 A = ${regA}`);
            break;
            
        case 0x00: // HALT
            console.log("[Execute] HALT 執行：接收到停機訊號");
            running = false;
            break;
            
        default:
            console.error(`[Error] 未知指令: 0x${instruction.toString(16)}`);
            running = false;
            break;
    }
}

console.log(`=== 執行完畢。暫存器 A 最終數值: ${regA} ===`);
