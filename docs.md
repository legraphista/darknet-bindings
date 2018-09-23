# Docs

___

## Darknet Class

### Constructor
- options: `{Object}`
	- cfg: `{string}` - path to net config file
	- weights: `{string}` - path to net weights file
	- names: `{string[]}` - array of net class names
	- memoryCount: `{number=3}` - how many net snapshots to average in inference 
	- nms: `{number=0.4}` - threshold for combining multiple detections of the same object into one
	- thresh: `{number=0.5}` - threshold of object detection 
	- hier_thresh: `{number=0.5}` - hierarchical region tree threshold 

### `.resetMemory(count: number = this.memoryCount):void`
Synchronously and dangerously frees the network memory snapshots and creates a new memory layout.  
Useful for when switching scene context, so that the last sequence of frames data doesn't bleed in the new one.  
When calling this method, make sure that all predictions have finished the work, else undetermined behavior will occur.

### `.imageFromRGBBuffer(img: Buffer|Uint8Array, {w: number, h: number}): Promise<DarknetImage>`
Converts image from an RGB buffer into the darknet format and provides back an `DarknetImage` instance to be used later.

### `.imageFromRGBOpenCvMat(mat: cv.Mat): Promise<DarknetImage>`
Converts image from an [OpenCV Mat](https://github.com/justadudewhohacks/opencv4nodejs) into the darknet format and provides back an DarknetImage instance to be used later.

### `.predict(image: DarknetImage): Promise<Detections>`
Predicts a prepared image (or waits for it to be prepared) and returns an instance of `Detections` to be used later for interpreting.  
Calling `.predict` more than once concurrently will result in undetermined behavior or corrupt detections.

## DarknetImage Class

### `.prepare: Promise<void>`
Prepares the image before prediction, can be called before `Darknet.predict()`.  
Calling `.prepare` more than once concurrently will result in undetermined behavior or corrupt detections.

### `get .prepared: boolean`
Flag indicating if image is prepared.

## Detections Class

### `get .count: number`
How many detections have been found.

### `get .image: DarknetImage`
Used darknet image.

### `.doNMS(): Promise<void>`
Does Non-Maximum Suppression for detections, combining detected regions according to threshold.  
Calling `.doNMS` more than once will result in undetermined behavior or corrupt detections.

### `.interpret(): Promise<Interpretation[]>`
Interprets the detections into region boxes and class names.  
Calling `.interpret` more than once will result in undetermined behavior.

### `get .interpreted: boolean`
Flag indicating if detections have been interpreted.


## Interpretation Class

### Properties
- names: `{string[]}` - list of class names detected
- probabilities: `{number[]}` - list of probabilities for each name
- left: `{number}` - left bound of region in pixels
- right: `{number}` - right bound of region in pixels
- top: `{number}` - top bound of region in pixels
- bottom: `{number}` - bottom bound of region in pixels
- x: `{number}` - vertical position of the region center in percentage (0 to 1) 
- y: `{number}` - horizontal position of the region center in percentage (0 to 1)
- w: `{number}` - width of the region in percentage (0 to 1)
- h: `{number}` - height of the region in percentage (0 to 1)

### `get .classes: {name: string, probability: number}[]`
Return the combined list of class names and probabilities

### `get .best:  {name: string, probability: number}`
Returns the highest probable detection from `.classes`
