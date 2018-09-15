const path = require('path');

const rootDir = __dirname;

console.log([
  path.join(rootDir, 'libdarknet.so')
].join( ' '));

// console.log(`\
//   ${path.join(rootDir, 'libdarknet.so')} \\
// `);
