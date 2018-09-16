const DarknetClass = require('bindings/')('darknet-bindings').DarknetClass;
const {assert} = require('hoek');
const fs = require('fs');

class DarknetImage {
  /**
   * @param {DarknetClass} darknetModule
   * @param {Buffer|Uint8Array} imageData
   * @param {number} w
   * @param {number} h
   * @param {number} c
   */
  constructor({darknetModule, imageData, w, h, c}) {
    this.w = w;
    this.h = h;
    this.c = c;
    this.data = imageData;
    this._darknet = darknetModule;

    this._letterboxed_data = null;
    this._letterboxed_w = 0;
    this._letterboxed_h = 0;
    this._letterboxed_c = 0;
  }

  /**
   * @return {Promise<void>}
   */
  async prepare() {
    // already prepared
    if (this.prepared) {
      return;
    }

    const {w, h, c, data_pointer} = await new Promise((res, rej) =>
      this._darknet.letterbox(this.data, this.w, this.h, this.c, (e, data) => {
        if (e) {
          return rej(e);
        }
        return res(data);
      })
    );

    this._letterboxed_data = data_pointer;
    this._letterboxed_w = w;
    this._letterboxed_h = h;
    this._letterboxed_c = c;
  }

  /**
   * @return {{w: number, h: number, c: number}}
   */
  get letterboxSize() {
    return {
      w: this._letterboxed_w,
      h: this._letterboxed_h,
      c: this._letterboxed_c,
    }
  }

  /**
   * @return {boolean}
   */
  get prepared() {
    return !!this._letterboxed_data;
  }

  /**
   * @return {{pointer: Buffer, w: number, h: number}}
   * @private
   */
  getDataForPrediction() {
    const toReturn = {
      pointer: this._letterboxed_data,
      w: this.w,
      h: this.h
    };

    this._letterboxed_data = null;
    this._letterboxed_w = 0;
    this._letterboxed_h = 0;
    this._letterboxed_c = 0;

    return toReturn;
  }
}

class Interpretation {
  /**
   * @param {string[]} names
   * @param {number[]} probabilities
   * @param {number} left
   * @param {number} top
   * @param {number} right
   * @param {number} bottom
   * @param {number} x
   * @param {number} y
   * @param {number} w
   * @param {number} h
   */
  constructor({names, probabilities, left, top, right, bottom, x, y, w, h}) {
    this.names = names;
    this.probabilities = probabilities;
    this.left = left;
    this.right = right;
    this.top = top;
    this.right = right;
    this.x = x;
    this.y = y;
    this.w = w;
    this.h = h;
  }

  /**
   * @return {{name: string, probability: number}[]}
   */
  get classes() {
    return this.names.map((name, index) => ({
      name,
      probability: this.probabilities[i]
    }));
  }

  /**
   * @return {{name: string, probability: number}}
   */
  get best() {
    return this
      .classes
      .sort((a, b) => b.probability - a.probability)[0];
  }
}

class Detections {
  /**
   * @param {DarknetImage} image
   * @param {DarknetClass} darknetModule
   * @param {Buffer} data_pointer
   * @param {number} count
   */
  constructor({image, darknetModule, data_pointer, count}) {
    this._count = count;
    this._data_pointer = data_pointer;
    this._darknet = darknetModule;
    this._image = image;

    this._interepretation = null;
  }

  /**
   * @return {number}
   */
  get count() {
    return this._count;
  }

  /**
   * @return {DarknetImage}
   */
  get image() {
    return this._image;
  }

  /**
   * @return {Promise<void>}
   */
  async doNMS() {
    await new Promise((res, rej) =>
      this._darknet.nms(this._data_pointer, this._count, (e) => {
        if (e) {
          return rej(e);
        }
        return res();
      })
    );
  }

  /**
   * @return {boolean}
   */
  get interpreted() {
    return !!this._interepretation;
  }

  /**
   * @return {Promise<*>}
   */
  async interpret() {
    if (this.interpreted) {
      return this._interepretation;
    }

    const _interepretation = await new Promise((res, rej) =>
      this._darknet.interpret(
        this._data_pointer,
        this._count,
        this._image.w,
        this.image.h,
        (e, data) => {
          if (e) {
            return rej(e);
          }
          return res(data);
        }
      )
    );

    this._interepretation = _interepretation.map(x => new Interpretation(x));

    return this._interepretation;
  }
}


class Darknet {
  constructor({
                cfg,
                weights,
                names,
                memoryCount = 3,
                nms = .4,
                thresh = .5,
                hierThresh = .5
              }) {
    assert(fs.existsSync(cfg), 'bad config file location');
    assert(fs.existsSync(weights), 'bad weights file location');
    assert(Array.isArray(names) && names.length > 0, 'bad names');

    this._darknet = new DarknetClass({
      cfg, weights, names,
      memoryCount,
      nms,
      thresh,
      hier_thresh: hierThresh
    });

    this._memoryCount = memoryCount;
  }

  resetMemory(count = this._memoryCount) {
    this._darknet.resetMemory(count);
  }

  async imageFromRGBBuffer(buffer, {w, h}) {
    return this._rgbBufferToDarknetImage(buffer, {w, h, c: 3});
  }

  async imageFromRGBOpenCvMat(mat) {
    assert(mat.rows && mat.cols && mat.channels && mat.getDataAsync,
      'malformed cv.Mat');

    assert(mat.channels === 3, 'unsupported channels number ' + mat.channels);

    return this._rgbBufferToDarknetImage(
      await mat.getDataAsync(),
      {
        w: mat.cols,
        h: mat.rows,
        c: mat.channels
      }
    )
  }

  async _rgbBufferToDarknetImage(buffer, {w, h, c}) {
    assert(buffer instanceof Buffer || buffer instanceof Uint8Array, 'bad buffer type');
    assert(w * h * c === buffer.length, 'buffer lenght doesn\'t match the image dimensions');

    const darknetBuffer = await new Promise((res, rej) =>
      this._darknet.rgbToDarknet(buffer, w, h, c, (e, data) => {
        if (e) {
          return rej(e);
        }
        return res(data);
      })
    );

    return new DarknetImage({
      darknetModule: this._darknet,
      imageData: darknetBuffer,
      w, h, c
    });
  }

  async predict(dnImage) {
    assert(dnImage instanceof DarknetImage, 'bad dn image instance');

    await dnImage.prepare();
    const {pointer: imagePointer, w, h} = dnImage.getDataForPrediction();

    const {count, data_pointer} = await new Promise((res, rej) =>
      this._darknet.predict(imagePointer, w, h, (e, data) => {
        if (e) {
          return rej(e);
        }
        return res(data);
      })
    );

    return new Detections({
      image: dnImage,
      data_pointer,
      count,
      darknetModule: this._darknet
    });
  }
}

module.exports = Darknet;
