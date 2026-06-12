compile:
	g++ -std=c++17 src\main.cpp src\Game.cpp src\Level.cpp src\ConsoleRenderer.cpp src\AssetManager.cpp -o build\main.exe
	cp assets.txt build\assets.txt
	cp level.txt build\level.txt
