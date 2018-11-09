export class DarknetImage {

    constructor(
        darknet_buffer: Float32Array,
        width: number, height: number, channels: number,
        original_width?: number, original_height?: number);

    release(): void;

    toTGB(): Promise<Uint8Array>;

    letterbox(width: number, height: number): Promise<DarknetImage>;

    static fromRGB(buffer: Uint8Array, width: number, height: number, channels: number): Promise<DarknetImage>;

    static fromPlanarRGB(r: Uint8Array, g: Uint8Array, b: Uint8Array, width: number, height: number): Promise<DarknetImage>;
}