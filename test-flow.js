const Darknet = require('.');
require('segfault-handler').registerHandler("crash.log");
require('expose-gc');

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
const video = new cv.VideoCapture('faces.mp4');

const darknet = new Darknet({
  // cfg: path.join(__dirname, 'darknet', 'cfg', 'yolov3.cfg'),
  cfg: '/Users/Stefan/Desktop/Work/aive-smartcrop/weights/darknet-face-detection/yolo-obj.cfg',
  // names: fs
  //   .readFileSync(path.join(__dirname, 'darknet', 'data', 'coco.names'))
  //   .toString()
  //   .trim()
  //   .split('\n'),
  // weights: "yolov3.weights",
  names: ['face'],
  weights: "/Users/Stefan/Desktop/Work/aive-smartcrop/weights/darknet-face-detection/yolo-fnd-face.weights",
  // cfg: '/Users/Stefan/Desktop/GitHub/YOLO-Face-detection/cfg/yolo-face.cfg',
  // cfg: '/Users/Stefan/Desktop/GitHub/azFace/net_cfg/tiny-yolo-azface-fddb.cfg',
  // weights: "/Users/Stefan/Desktop/GitHub/YOLO-Face-detection/yolo-face_final.weights",
  // weights: "/Users/Stefan/Desktop/GitHub/azFace/weights/tiny-yolo-azface-fddb_82000.weights",
  // names: ['face'],

  thresh: 0.5,
  hierThresh: 0.5,
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
  printMem();

  video.set(cv.CAP_PROP_POS_FRAMES, 100);

  for (let i = 0; i < 3000; i++) {
    const frame = video.read().cvtColor(cv.COLOR_BGR2RGB);

    gc();
    // console.error('imageFromRGBOpenCvMat');
    const image = await darknet.imageFromRGBOpenCvMat(frame);
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
    // console.log(i, interpretations.map(x => x.classes));
    console.log(i, interpretations);
    console.log(i);
    gc();

    // for (let int of interpretations) {
    //   frame.drawRectangle(
    //     new cv.Rect(int.x, int.y, int.w, int.h),
        // new cv.Rect(int.left, int.top, int.right - int.left, int.bottom - int.top),
        // new cv.Vec3(255, 255, 255),
        // 2
      // );
    // }
    // cv.imshow('frame', frame.cvtColor(cv.COLOR_RGB2BGR));
    // cv.waitKey(1);
    frame.release();
    printMem();
  }

  for (let i = 0; i < 10; i++) {
    gc();
    printMem();
  }
})();
