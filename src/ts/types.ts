
export type Callback = (err?: Error, result?: any) => void

export type ResolvePromise<P> = P extends PromiseLike<infer T> ? P : Promise<P>
export type IPCMap = {
    [key: string]: (...args: any[]) => any
}
export type IPC<M extends IPCMap> = {
    [K in keyof M]: WithBroadcast<Promisify<M[K]>>
}

export type WithBroadcast<T> = T & {broadcast: T}

// not very proud of this buddy, but hey, at least it works 99% of the time
export type Promisify<T> =
    T extends (a: infer A) => infer R
    ? (a: A) => ResolvePromise<R>
    : T extends (a: infer A, b: infer B) => infer R
    ? (a: A, b: B) => ResolvePromise<R>
    : T extends (a: infer A, b: infer B, c: infer C) => infer R
    ? (a: A, b: B, c: C) => ResolvePromise<R>
    : T extends (a: infer A, b: infer B, c: infer C, d: infer D) => infer R
    ? (a: A, b: B, c: C, d: D) => ResolvePromise<R>
    : T extends (a: infer A, b: infer B, c: infer C, d: infer D, e: infer E) => infer R
    ? (a: A, b: B, c: C, d: D, e: E) => ResolvePromise<R>
    : T extends (a: infer A, b: infer B, c: infer C, d: infer D, e: infer E, F: infer F) => infer R
    ? (a: A, b: B, c: C, d: D, e: E, f: F) => ResolvePromise<R>
    : T extends (...args: (infer A)[]) => infer R
    ? (...args: A[]) => ResolvePromise<R>
    : never
