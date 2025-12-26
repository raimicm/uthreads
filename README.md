# uthreads
A user-level thread library for Linux running on an x86 processor.

## Requirements

To build and use this library, you will need:

- x86 processor
- Linux
- Make
- GCC
- Git

## Build Instructions

### Running examples:

#### Step 1: Clone the Repository

Clone this repository:

```bash
git clone https://github.com/raimicm/uthreads.git
```

#### Step 2: Build

Change directory into the root folder of the cloned repository and run make to compile the examples:

```bash
cd uthreads
make all
```

#### Step 3: Run

Now you can run an example. If you want to run `join_example`, you would use the following commands.
```bash
cd build
./join_example
```