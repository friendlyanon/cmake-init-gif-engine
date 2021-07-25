#!/bin/bash

(cd fuzz/corpus
ls -1 | grep -vEe '^files$' > files
cmake -E tar cf "$1.zip" --format=zip --files-from=files
rclone move "corpus:corpus/data/$1.zip" "corpus:corpus/data/$1.old.zip"
rclone copy "$1.zip" corpus:corpus/data
rclone delete "corpus:corpus/data/$1.old.zip")
