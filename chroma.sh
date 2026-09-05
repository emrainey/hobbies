#!/usr/bin/env bash

for mov in ~/Downloads/*.MOV; do
    echo "Processing $mov"
    ./build/homebrew-llvm/projects/vision/chroma-replace \
        -i "$mov" -c green \
        -o "testing/$(basename "$mov" .MOV).mp4" -s 0.25 \
        -t shared -a -V -d 1.0 -F 0.3 -M 0.3 -T 1 -R 3
done


# -t bayesian -a -d 1.0 -M 0.3 -T 2 -R 3
# -t shared -a -d 1.0 -M 0.3 -T 2 -R 3
# -t fused -a -d 1.0 -M 0.3 -T 1 -F 0.3 -R 3
#         -o "testing/$(basename "$mov" .MOV).png" -f 1 \

