const childProcess = require('child_process')

if(!process.send) {
    console.log('Master')
    const childs = []
    const callbacks = []

    while(childs.length < 8) {
        childs.push(
            childProcess
                .fork(__filename)
                .on('message', msg => callbacks[msg.id](msg.data))
        )
    }

    const calls = 5000000
    const promises = []
    const now = Date.now()
    let tests = 0

    for(let i = 0; i < calls; i++) {
        const mock = (i * Math.random()).toString(36)
        const start = Date.now()

        promises.push(
            new Promise(resolve =>
                callbacks.push(resolve)
            ).then(({data, pid, latency}) => {
                if(data !== mock) {
                    throw new Error('Memory corrupted')
                }

                tests++

                const progress = tests / calls * 100

                    console.log('Progress %s%', progress)

                return {pid, latency}
            })
        )
        childs[i % childs.length].send({id: i, data: [mock, start]})
    }

    Promise.all(promises).then(result => {
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
    }).catch(err => console.error(err))
}
else {
    async function work(data, start) {
        const result = {data, pid: process.pid, latency: Date.now() - start}

        return new Promise(resolve =>
            setTimeout(() => resolve(result), 0)
        )
    }

    process.on('message', ({data, id}) =>
        Promise
            .resolve(work(...data))
            .then(data => process.send({data, id}))
            .catch(error => process.send({error, id}))
    )
}
