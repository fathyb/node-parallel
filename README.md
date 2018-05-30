# `node-shared-buffer`

A native shared `Buffer` implementation.

> WIP: super experimental

## Usage

> Note: `fork` is used but you can use it from anywhere

```js

const {SharedBuffer} = require('node-shared-buffer')
const {isMaster, fork} = require('cluster')

const test = [0xe7, 0x09, 0x23, 0x78]
const buffer = SharedBuffer('fibonnaci-example', test)

console.log(buffer instanceof Buffer) // true

if(isMaster) {
  // copy test to buffer
  Object.assign(buffer, test)

  fork()
}
else {
  // Prints <Buffer e7 09 23 78>
  console.log('Buffer data', buffer)
}
```

## Why

node.js doesn't provide any shared memory logic and `fork` doesn't really `fork`.
This addon adds the possibility to create a `Buffer` and share it with different node.js
processes using a key, even if they don't share any parent process.

This can be useful to create a high-performance IPC implementation when no advanced locking
mechanism is necessary and domain sockets aren't fast enough.

## Performance

The allocation of the `Buffer` is slightly slower, however the read/write performance
are the same as a standard `Buffer`.

No proxies are created, the object is a pristine node.js `Buffer`.

