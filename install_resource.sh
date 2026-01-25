#!/bin/bash

package_array=( "git" "make" "ffmpeg" "build-essential" )
file_array=( "res/bg01.wav" "res/bg02.wav" "res/bg03.wav" "res/failure.wav" "res/victory.wav" "res/intro.wav" )
link_array=( "https://youtu.be/xSjnN39Qhmc?si=h6uX36a7Tnss8pe7" "https://youtu.be/YaDxGpzSVKU?si=uLfhk-sHFGBS57_X" "https://youtu.be/5yYQIa-4rLE?si=Li1f1YtYCuwCJ3Zp" \
"https://www.youtube.com/watch?v=e5tEoIrXK6o" "https://youtu.be/itgSd5JIIBs?si=krUy8Afv2v6eOrvo" "https://www.youtube.com/watch?v=xJB3aRd-q74" )
not_ok_array=()

for p in ${package_array[@]}; do
    package_query_value="$(dpkg-query -W --showformat='${db:Status-Status}' $p 2>&1)"
    if [ $package_query_value == "installed" ]; then
        echo "[$p -> ok]" 
    else
        echo "[$p -> not ok]"
        not_ok_array+=("$p")
    fi
done
if [ ${#not_ok_array[@]} -eq 0 ]; then
    echo "[All packages are ok]"
else
    echo "[ERROR: Install packages following: ${not_ok_array[@]}]"
    exit
fi

if [ ${#file_array[@]} -ne ${#link_array[@]} ]; then
    echo "[ERROR: Internal error]"
    exit
fi
array_num=${#file_array[@]}
git clone https://github.com/yt-dlp/yt-dlp.git
make -C yt-dlp
cp yt-dlp/yt-dlp temp-yt-dlp
chmod +x ./yt-dlp
for ((i=0;i<array_num;i++)); do
    ./temp-yt-dlp -x --audio-format wav \
    -o "${file_array[$i]}" "${link_array[$i]}"
done
rm ./temp-yt-dlp
rm -rf yt-dlp

echo "[Done]"
exit
