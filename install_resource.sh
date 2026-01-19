#!/bin/bash
wget https://github.com/yt-dlp/yt-dlp/releases/download/2025.12.08/yt-dlp
chmod +x ./yt-dlp
./yt-dlp -x --audio-format wav -o "res/bg01.wav" "https://youtu.be/xSjnN39Qhmc?si=h6uX36a7Tnss8pe7"
./yt-dlp -x --audio-format wav -o "res/bg02.wav" "https://youtu.be/YaDxGpzSVKU?si=uLfhk-sHFGBS57_X"
./yt-dlp -x --audio-format wav -o "res/bg03.wav" "https://youtu.be/5yYQIa-4rLE?si=Li1f1YtYCuwCJ3Zp"
./yt-dlp -x --audio-format wav -o "res/failure.wav" "https://www.youtube.com/watch?v=e5tEoIrXK6o"
./yt-dlp -x --audio-format wav -o "res/victory.wav" "https://youtu.be/itgSd5JIIBs?si=krUy8Afv2v6eOrvo"
./yt-dlp -x --audio-format wav -o "res/intro.wav" "https://www.youtube.com/watch?v=xJB3aRd-q74"
rm ./yt-dlp
