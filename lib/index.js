// i don't know why the linker is looking for darknet.so in current working directory
// so we switch to the cwd where the lib exists
// load the bindings (which loads the lib)
// switch back to the original cwd
const path = require('path');
const util = require('util');
const cwd = process.cwd();
process.chdir(path.join(__dirname, '..'));
const {Darknet, DarknetImage, DarknetDetections} = require('bindings/')('darknet-bindings');
process.chdir(cwd);

DarknetImage.fromRGB = util.promisify(DarknetImage.fromRGB);
DarknetImage.prototype.letterbox = util.promisify(DarknetImage.prototype.letterbox);

Darknet.prototype.predict = util.promisify(Darknet.prototype.predict);

DarknetDetections.prototype.doNMS = util.promisify(DarknetDetections.prototype.doNMS);


module.exports = {Darknet, DarknetImage, DarknetDetections};
