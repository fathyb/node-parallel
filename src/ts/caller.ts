import {addon} from './addon'
import {Callback, IPC, IPCMap} from './types'

export function makeCaller<T extends IPCMap>(calls: T): IPC<T> {
    const dest: IPC<T> = {} as any

    Object.keys(calls).forEach(name => {
        const value = call(name, false) as any

        (dest as any)[name] = value
        value.broadcast = call(name, true)
    })

    return dest
}

let counter = 0
const queue = {
    broadcast: [] as [string, any][],
    dispatch: [] as [string, any][]
}

const listeners = new Map<number, [Callback, boolean, boolean]>()
let scheduled = false


function call(name: string, broadcast: boolean): (...args: any[]) => Promise<any> {
    return (...args: any[]) =>
        new Promise((resolve, reject) =>
            internalCall(name, args, (err: any, result: any) => {
                if(typeof err !== 'undefined') {
                    reject(err)
                }
                else {
                    resolve(result)
                }
            }, broadcast)
        )
}

function processResponses(results: any[]) {
    results.forEach(({error, result, id}) => {
        const entry = listeners.get(id)!
        const [listener, broadcast, counted] = entry

        if(!counted) {
            entry[2] = true
        }

        if(!broadcast) {
            listeners.delete(id)
        }

        if(error) {
            const newError = new Error(error.message)

            newError.stack = error.stack
            newError.name = error.name

            listener(newError, undefined)
        }
        else {
            listener(undefined, result)
        }
    })
}

function internalCall(name: string, args: any[], callback: Callback, broadcast: boolean) {
    const callId = counter++

    args.unshift(callId)
    listeners.set(callId, [callback, broadcast, false])

    if(broadcast) {
        queue.broadcast.push([name, args])
    }
    else {
        queue.dispatch.push([name, args])
    }

    if (!scheduled) {
        scheduled = true
        process.nextTick(() => {
            const {broadcast, dispatch} = queue
            scheduled = false

            if(broadcast.length > 0) {
                addon.queueIPCMessage(broadcast.splice(0), processResponses, true)
            }

            if(dispatch.length > 0) {
                addon.queueIPCMessage(dispatch.splice(0), processResponses, false)
            }
        })
    }
}
