const {createIPC, isMaster} = require('.')

const ipc = createIPC({
    fibonacci: async (n) =>
        n < 1
            ? 0
            : n <= 2
                ? 1
                : (await ipc.fibonacci(n - 1)) + (await ipc.fibonacci(n - 2)),

    test(data, start) {
        const result = {data, pid: process.pid, latency: Date.now() - start}

        return result
        return new Promise(resolve =>
            setTimeout(() => resolve(result), 0 * Math.random())
        )
    }
})

setTimeout(() => null, 50 * 1000)
if(isMaster) {
    void async function() {
        const test = 15

        return console.log('fib(%s) = ', test, await ipc.fibonacci(test))


        const promise = []
        const now = Date.now()
        const calls = 300000
        let tests = 0

        for(let i = 0; i < calls; i++) {
            const mock = (i * Math.random()).toString(36)
            const start = Date.now()
            let called = false

            promise.push(
                ipc
                    .test(mock, start)
                    .then(({data, pid, latency}) => {
                        if(data !== mock) {
                            throw new Error('Memory corrupted')
                        }

                        if(called) {
                            throw new Error('IPC response duplication')
                        }

                        called = true

                        tests++

                        const progress = tests / calls * 100

                        if(Math.floor(progress) === progress) {
                            console.log('Progress %s%', progress)
                        }

                        return {pid, latency}
                    })
            )
        }

        const result = await Promise.all(promise)

        if(tests !== calls) {
            throw new Error('Missing callback calls')
        }

        const stats = result.reduce((acc, {pid, latency}) => {
            if(pid in acc) {
                acc[pid]++
            }
            else {
                acc[pid] = 1
            }

            if(acc.latency === undefined) {
                acc.latency = latency
                acc.latencyMin = latency
                acc.latencyMax = latency
            }
            else {
                acc.latency = (acc.latency + latency) / 2
                acc.latencyMin = Math.min(latency, acc.latencyMin)
                acc.latencyMax = Math.max(latency, acc.latencyMax)
            }

            return acc
        }, {})
        const diff = (Date.now() - now) / 1000
        const callsPerSec = calls / diff

        console.log('\n')
        console.log('%s seconds (%s calls per second), latency %s ms', diff, callsPerSec, stats.latency, stats)
    }().catch(err => console.error(err))
}

