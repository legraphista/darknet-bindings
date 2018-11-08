require('segfault-handler').registerHandler("crash.log");
require('expose-gc');

const {Darknet, DarknetImage, DarknetDetections} = require('./lib');

const fs = require('fs');
const path = require('path');

console.log({Darknet, DarknetDetections, DarknetImage});

const cv = require('opencv4nodejs');
const video = new cv.VideoCapture('faces.mp4');

const darnket = new Darknet({
  cfg: path.join(__dirname, 'darknet', 'cfg', 'yolov3.cfg'),
  names: fs
    .readFileSync(path.join(__dirname, 'darknet', 'data', 'coco.names'))
    .toString()
    .trim()
    .split('\n'),
  weights: "yolov3.weights",
  thresh: 0.5,
  hier_thresh: 0.5,
  memoryCount: 3
});


function printMem() {
  const {
    rss,
    heapTotal,
    heapUsed,
    external
  } = process.memoryUsage();

  console.log((rss / 2 ** 20).toFixed(2));
  // console.log('RSS', (rss / 2 ** 20).toFixed(2));
  // console.log('total', (heapTotal / 2 ** 20).toFixed(2));
  // console.log('used', (heapUsed / 2 ** 20).toFixed(2));
  // console.log('external', (external / 2 ** 20).toFixed(2));
}

(async () => {

  video.set(cv.CAP_PROP_POS_FRAMES, 100);
  const frame = video.read().cvtColor(cv.COLOR_BGR2RGB);
  video.release();

  for (let i = 0; i < 1000; i++) {
    global.gc();
    const image = await DarknetImage.fromRGB(
      frame.getData(),
      frame.cols,
      frame.rows,
      3
    );
    global.gc();
    const letterboxed = await image.letterbox(darnket.netWidth, darnket.netHeight);
    global.gc();
    const detections = await darnket.predict(letterboxed);
    global.gc();
    await detections.doNMS(0.4);
    global.gc();
    const interpretations = detections.interpret();
    global.gc();
  }

})();
