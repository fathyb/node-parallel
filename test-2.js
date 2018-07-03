
const addon = require('.')
const fs = require('fs')

const vfsRead = name => addon.Store.getOrSet(name, name => fs.readFileSync(name, 'utf-8'))


void async function() {
    console.log('Pkg length', (await vfsRead(__dirname + '/package.json')).slice(0, 100))
}()

setTimeout(() => null, 500 * 1000)
