export interface Interpretation {
    names: string[],
    probabilities: number[],

    best: {
        name: string,
        probability: number
    },

    box: {
        x: number,
        y: number,
        w: number,
        h: number
    },

    top: number,
    left: number,
    bottom: number,
    right: number,
    width: number,
    height: number
}

export class DarknetDetections {
    constructor(
        detections: External,
        nb_detections: number,
        names: External,
        nb_classes: number,
        width: number,
        height: number,
        threshold: number
    );

    readonly count: number;

    release(): void;

    doNMS(nms_threshold: number): Promise<undefined>;

    interpret(): Interpretation[];
}