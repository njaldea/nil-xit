export type Action<T> = (target: T) => {
    destroy: () => void;
};
export type Writable<T> = {
    set: (v: T) => void;
    subscribe: (cb: (v: T) => void) => () => void;
    update: (cb: (v: T) => T) => void;
};
export type CoDec = {
    encode: (o: object) => Uint8Array;
    decode: (a: Uint8Array) => object;
};
export type Loader = {
    one: (f: string) => Action<HTMLDivElement>;
    all: (f: string[]) => Action<HTMLDivElement>;
};
export type Xit = {
    binding: {
        boolean: (t: string, v: boolean) => Writable<boolean>;
        double: (t: string, v: number) => Writable<number>;
        number: (t: string, v: number) => Writable<number>;
        string: (t: string, v: string) => Writable<string>;
        buffer: (t: string, v: Uint8Array) => Writable<Uint8Array>;
        json: (t: string, v: object, codec: CoDec) => Writable<object>;
    };
    listeners: {
        none: (t: string) => () => void;
        boolean: (t: string) => (v: boolean) => void;
        double: (t: string) => (v: number) => void;
        number: (t: string) => (v: number) => void;
        string: (t: string) => (v: string) => void;
        buffer: (t: string) => (v: Uint8Array) => void;
        json: (t: string, encoder: CoDec["encode"]) => (v: object) => void;
    };
    loader: Loader;
};
export declare const json_string: CoDec;