import {addon} from './addon'
import {makeCaller} from './caller'
import {listen} from './callee'
import {IPCMap, IPC} from './types'

export {Store} from './store'
export const isMaster = addon.isMaster()

// We need to be sure the caller exists in the C++ side, so force the user to explicitly wrap the IPC
export function createIPC<T extends IPCMap>(calls: T): IPC<T> {
    if(!isMaster) {
        listen(calls)
    }

    return makeCaller(calls)
}

process.on('exit', () => console.log('Quitting'))