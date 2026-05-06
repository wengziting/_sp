const assert = require('assert');
const Memory = require('../src/memory');
const CPU = require('../src/cpu');

function testAdd() {
    const mem = new Memory();
    const cpu = new CPU(mem);
    const program = [0x01, 0x00, 0x05, 0x01, 0x01, 0x0A, 0x02, 0x00, 0x01, 0xFF];
    program.forEach((byte, i) => mem.store(i, byte));

    while (!cpu.halted) {
        cpu.step();
    }

    try {
        assert.strictEqual(cpu.regs[0], 15);
        console.log("✅ Test Add: Passed!");
    } catch (e) {
        console.error("❌ Test Add: Failed! Expected 15, got", cpu.regs[0]);
    }
}

testAdd();
