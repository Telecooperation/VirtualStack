#!/bin/bash

valgrind --log-file="valgrind.log" --main-stacksize=10000 "./bin/VirtualStack_snippets"
