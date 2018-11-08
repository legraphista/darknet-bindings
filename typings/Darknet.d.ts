import {DarknetImage} from "./DarknetImage";
import {DarknetDetections} from "./DarknetDetections";

export class Darknet {
    constructor(options: {
        cfg: string,
        weights: string,
        names: string[],
        memoryCount?: number,
        thresh?: number,
        hier_thresh?: number
    })

    readonly netWidth: number;
    readonly netHeight: number;

    resetMemory(count?: number): void;

    predict(image: DarknetImage): Promise<DarknetDetections>;
}