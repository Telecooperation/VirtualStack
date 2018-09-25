#!/bin/bash

valgrind --tool=callgrind --dump-instr=yes --separate-threads=yes "./bin/VirtualStack_snippets"
