const Memory = require('./memory');
const CPU = require('./cpu');

const mem = new Memory();
const cpu = new CPU(mem);

const program = [
  0x01, 0x00, 0x0F, 
  0x01, 0x01, 0x1B, 
  0x02, 0x00, 0x01, 
  0xFF
];

program.forEach((byte, i) => mem.store(i, byte));

console.log("--- 模擬器啟動 ---");

while (!cpu.halted) {
  console.log(`PC: ${cpu.pc} | Regs: [${cpu.regs.join(', ')}]`);
  cpu.step();
}

console.log("--- 執行結束 ---");
console.log("最終暫存器狀態:", cpu.regs);
console.log("R0 結果 (15 + 27):", cpu.regs[0]);
