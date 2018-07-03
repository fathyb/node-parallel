# `node-multiprocess`

Node.js multiprocess library on steroids.

> WIP: super experimental

## What's in the box

- A ultra-fast IPC backed by native shared memory and v8 serializer
- A keyed shared memory `Buffer` primitive
- A key-value data store to store anything in shared memory, with atomic operations

#### Freebies

- Booting a bunch of workers is faster than any Node.js API
- TypeScript friendly, and almost entirely written in C++ for low-overhead
- Kernel backed Copy-On-Write: it only uses physical (and reported) memory when you write it
- Everything is managed by the garbage collector and reference counted, it's also
reference counted based on each process lifecycle

#### Downsides

- It needs node.js as a static library, I'm working on scripts to fetch
and build node.js binaries for all platforms
- It needs to be launched from our executable :
    - `node-workers a` instead of `node a`
    - this executable behaves exactly like `node`
    - you **will** get a `segfault` if you use `node`
    - this is because we need to modify the node.js process virtual address space before booting it,
    this could be resolved by supporting `fork` inside node.js (which `libuv` does)
- It's currently very niche, I wrote it for use-cases like Parcel
- Windows not supported (soon, should be trivial to implement)
- If one of the worker doesn't die gracefully (eg. segfault) we crash everything.
This is done by safety to prevent any memory corruption and potential deadlocks.
    - This may be fixed in the future by using more lock-free structures

## Usage

```js
import {createIPC, isMaster, store} from 'multiprocess'

const fibonacci = n =>
    n < 1
        ? 0
        : n <= 2
            ? 1
            : fibonacci(n - 1) + fibonacci(n - 2)

// Create a IPC controller
const ipc = createIPC({
    fibonacci(n) {
        console.log('Computing fib(%s) in process %s', n, process.pid)

        return fibonacci(n)
    }
})

// Here we call from master but even workers can call the IPC
if(isMaster) {
    Promise
        .all(
            [10, 15, 20, 25, 30].map(number =>
                ipc.fibonacci(number)
            )
        )
        .then(result => console.log('Result', result))
        .catch(err => console.error(err))
}

// Create a cross-process virtual fs
const readFile = name => {
    console.log('Fetching %s')

    // Returns the value if set or sets it atomically
    return store.getOrSet(name, () => {
        // The reading task is distributed across processes
        // but only done one time per file
        console.log('Reading %s on pid %s', name, process.pid)

        return fs.readFile(name, 'utf-8')
    })
}


readFile('/tmp/foo').then(foo => console.log('Foo', foo))
readFile('/tmp/bar').then(bar => console.log('Bar', bar))
```

```bash
$ node-workers index.js --workers=8

Computing fib(30) in process 4685
Computing fib(10) in process 4689
Computing fib(25) in process 4690
Computing fib(15) in process 4686
Computing fib(20) in process 4688
Result [ 55, 610, 6765, 75025, 832040 ]
```

## Developement

The C++ follows the Google C++ Style Convention, I know, I know,
me neither. This makes the code blend more naturally with v8 and Node.js APIs.

### Wheels

That needed to be reinvented, somehow :

- Memory allocator
- Shared pointer
- SpinLocks
