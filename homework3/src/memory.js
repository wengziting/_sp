class Memory {
  constructor(size = 256) {
    this.data = new Uint8Array(size);
  }
  load(address) {
    return this.data[address];
  }
  store(address, value) {
    this.data[address] = value & 0xFF; 
  }
}
module.exports = Memory;
