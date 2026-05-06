class CPU {
  constructor(memory) {
    this.memory = memory;
    this.pc = 0;            
    this.regs = new Uint8Array(4); 
    this.halted = false;     
  }
  step() {
    if (this.halted) return;
    const opcode = this.memory.load(this.pc++);
    switch (opcode) {
      case 0x01: 
        const regIdx = this.memory.load(this.pc++);
        const value = this.memory.load(this.pc++);
        this.regs[regIdx] = value;
        break;
      case 0x02: 
        const rA = this.memory.load(this.pc++);
        const rB = this.memory.load(this.pc++);
        this.regs[rA] = (this.regs[rA] + this.regs[rB]) & 0xFF;
        break;
      case 0xFF:
        this.halted = true;
        break;
      default:
        console.error(`Unknown opcode: 0x${opcode.toString(16)} at PC: ${this.pc-1}`);
        this.halted = true;
    }
  }
}

module.exports = CPU;
