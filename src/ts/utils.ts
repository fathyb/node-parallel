export class ConditionVariable {
    private readonly listeners: Array<() => void> = []

    public async wait(condition?: () => boolean): Promise<void> {
        do {
            if(condition && condition()) {
                return
            }

            await new Promise(resolve => this.listeners.push(resolve))

            if(!condition) {
                return
            }
        } while(true)
    }

    public notifyAll() {
        const listeners = this.listeners.splice(0)

        while(listeners.length) {
            listeners.pop()!()
        }
    }
}

export class QueuedCallback {

}

