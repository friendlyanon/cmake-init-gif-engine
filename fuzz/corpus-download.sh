#!/bin/bash

rclone copy "corpus:corpus/data/$1.zip" fuzz/corpus
(cd fuzz/corpus; cmake -E tar xf "$1.zip")
rm "fuzz/corpus/$1.zip"
