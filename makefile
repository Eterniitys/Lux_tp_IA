all : run

run:
	g++ main.cpp -O3 -std=c++11 -o main.out
	lux-ai-2021 main.out main.out --height 12 --width 12 --out=replay.json --seed 23925337

random:
	g++ main.cpp -O3 -std=c++11 -o main.out
	lux-ai-2021 main.out main.out --out=replay.json

help:
	echo "go to https://2021vis.lux-ai.org/ to watch replays"
	lux-ai-2021 --help