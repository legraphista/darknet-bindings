const path = require('path');

const rootDir = __dirname;

console.log([
  `-Wl,-rpath,${path.join(rootDir, 'libdarknet.so')}`
].join(' '));
