all :

init:
	lux-ai-2021 main.js main.js --out=replay.json

run:
	g++ main.cpp -O3 -std=c++11 -o main.out
	lux-ai-2021 main.out main.out --out=replay.json 


help:
	lux-ai-2021 --help