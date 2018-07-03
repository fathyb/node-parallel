import {addon} from './addon'
import {Callback, IPCMap} from './types'

export const isMaster = addon.isMaster()

export function tryAsync(fn: (...args: any[]) => any, args: any[], callback: Callback): void {
    try {
        const result = fn(...args)

        if(result && typeof result.then === 'function') {
            result.then(
                (result: any) =>
                    callback(undefined, result),
                ({name, message, stack}: any) =>
                    callback({name, message, stack}, undefined)
            )
        }
        else {
            callback(undefined, result)
        }
    }
    catch({name, message, stack}) {
        callback({name, message, stack})
    }
}

let called = false
export function listen<T extends IPCMap>(calls: T) {
    if(called) {
        throw new Error('listen should only be called one time')
    }

    called = true

    addon.waitForIPCMessage((messages: any[], done: (r: [any, any][]) => void) => {
        let scheduled = false
        const queue: [any, any][] = []

        function dispatch(handle: any, data: any) {
            queue.push([handle, data])

            if(!scheduled) {
                scheduled = true

                process.nextTick(() => {
                    scheduled = false
                    done(queue.splice(0))
                })
            }
        }

        for(let i = 0; i < messages.length; i++) {
            const [[id, ...args], name, handle] = messages[i]

            tryAsync(
                calls[name],
                args,
                (error, result) =>
                    dispatch(handle, {error, result, id})
            )
        }
    })
}
