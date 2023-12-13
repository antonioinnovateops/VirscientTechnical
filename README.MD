# Project Name

Project Name is a simple client-server file transfer application.

## Overview

This project consists of a client and server component for transferring files over a network. The server listens for incoming connections from clients and serves requested files. Clients can connect to the server to request and download files.

## Prerequisites

Before you can run the project, you'll need the following prerequisites:

- A C development environment with GCC compiler
- Git (for cloning the repository)
- CMake 

## Cloning the Repository

To get started, clone this repository to your local machine:

```bash
git clone https://github.com/antonioinnovateops/VirscientTechnical.git
cd VirscientTechnical
cmake -B build/
make -C build/
```


## Generate files for Deployment

```
dd if=/dev/urandom of=./build/file.txt bs=1M count=1

```

## Runing the server

Execut in a terminal the follwinf command:

```
./build/server
```

## Runing multiple clients

First create a output folder for your cliente dum the downloaded files"


```
mkdir -p build/output
```

After that Run the follwing script to deply multiple instance to emulate multiple connectoinf to the server:

```
script.sh file.txt
```

