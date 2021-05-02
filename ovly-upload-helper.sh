#!/bin/bash

nops /debug $4

nops /fast /bin $1 $2 $4

nops /fast /exe $3 $4

nops /slow $4
