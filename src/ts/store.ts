import {addon} from './addon'

export namespace Store {
    export type MaybePromise<T> = T | Promise<T>

    export type Result<T> =
        | {type: 'result', data: T}
        | {type: 'error', error: any}

    export function getOrSet<T>(name: string, set: (name: string) => MaybePromise<T>) {
        return new Promise<T>((resolve, reject) => {
            addon.getOrSet(
                name,
                (result: Result<T>) => {
                    if(result.type === 'error') {
                        reject(result.error)
                    }
                    else {
                        resolve(result.data)
                    }
                },
                (done: (data: Result<T>) => void) => {
                    console.log('Set called for ', name)

                    setImmediate(async () => {
                        let result: Result<T> | null = null

                        console.log('Inside nextTick', name)

                        try {
                            result = {
                                type: 'result',
                                data: await Promise.resolve(set(name)).then(r => {
                                console.log('Got async result', name)

                                return r
                            }, err => {
                                console.log('Got async err', err)

                                throw err
                            })
                        }
                        }
                        catch({name, stack, message}) {
                            result = {
                                type: 'error',
                                error: {name, stack, message}
                            }
                        }

                        console.log('Calling done for ', name)
                        done(result)
                    })
                }
            )
        })
    }
}
