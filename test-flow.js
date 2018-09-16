const Darknet = require('.');
require('segfault-handler').registerHandler("crash.log");

// todo TESTS! TESTS! TESTS!

const gc = () => {
  if (global.gc) {
    global.gc();
    global.gc();
    global.gc();
    global.gc();
    global.gc();
    return;
  }
  console.log('GC not exposed, doing nothing!');
};

const fs = require('fs');
const path = require('path');
const cv = require('opencv4nodejs');
const video = new cv.VideoCapture('video.mp4');

const darknet = new Darknet({
  cfg: path.join(__dirname, 'darknet', 'cfg', 'yolov3.cfg'),
  names: fs
    .readFileSync(path.join(__dirname, 'darknet', 'data', 'coco.names'))
    .toString()
    .trim()
    .split('\n'),
  weights: "yolov3.weights",
  thresh: .25,
  memoryCount: 3
});

function printMem() {
  const {
    rss,
    heapTotal,
    heapUsed,
    external
  } = process.memoryUsage();
  return 'early';
  console.log('RSS', rss / 2 ** 20);
  console.log('total', heapTotal / 2 ** 20);
  console.log('used', heapUsed / 2 ** 20);
  console.log('external', external / 2 ** 20);
}

(async () => {
  printMem();

  for (let i = 0; i < 30; i++) {
    const frame = video.read().cvtColor(cv.COLOR_BGR2RGB);

    gc();
    // console.error('imageFromRGBOpenCvMat');
    const image = await darknet.imageFromRGBOpenCvMat(frame);
    frame.release();
    gc();
    // console.error('prepare');
    await image.prepare();
    gc();
    // console.error('predict');
    const detections = await darknet.predict(image);
    gc();
    // console.error('doNMS');
    await detections.doNMS();
    gc();
    // console.error('interpret');
    const interpretations = await detections.interpret();
    gc();
    console.log(i, interpretations.map(x => x.classes));
    gc();

    printMem();
  }

  for (let i = 0; i < 10; i++) {
    gc();
    printMem();
  }
})();
