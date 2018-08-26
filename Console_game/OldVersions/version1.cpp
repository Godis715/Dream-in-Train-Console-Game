#include <iostream>
#include <string>
#include <fstream>
#include <conio.h>
#include <vector>

using std::string;

#define TAB_SIZE 31
#define BORDER_SPACE_SIZE 3
#define LINE_LENGTH 50

void Space(int number) {
	for (int i = 0; i < number; ++i) {
		std::cout << " ";
	}
}

void WriteText(string text) {
	std::cout << "\n\n\n\n\n";
	int lettersOnLine = 0;
	int limit = LINE_LENGTH + BORDER_SPACE_SIZE;
	for (int i = 0; text[i] != '\0'; ++i) {

		if (i % LINE_LENGTH == 0) {
			Space(TAB_SIZE);
			std::cout << "{{";
			Space(BORDER_SPACE_SIZE);
		}

		std::cout << text[i];
		lettersOnLine++;

		if ((i + 1) % LINE_LENGTH == 0) {
			Space(BORDER_SPACE_SIZE);
			std::cout << "}}\n";
			lettersOnLine = 0;
		}
	}
	if (lettersOnLine != 0) {
		Space(limit - lettersOnLine);
		std::cout << "}}\n";
	}
}

bool Eof(std::ifstream& inp) {
	int curPos = inp.tellg();
	inp.get();
	if (inp.eof()) {
		return true;
	}
	inp.seekg(curPos);
	return false;
}

void PrintError(const char* text) {
	std::cout << "ERROR: " << text;
	system("pause");
	exit(0);
}

char GetNextSymbol(std::istream& inp) {
	char c;
	while (true) {
		if (inp.eof()) {
			PrintError("GetNextSymbol: eof");
		}
		inp.get(c);
		if (c != '\n' && c != '\t') {
			break;
		}
	}
	return c;
}

string ReadText(std::istream& inp) {
	if (inp.eof()) {
		PrintError("ReadText: eof");
	}
	string text;
	char c = inp.get();
	while (c != '>') {
		if (inp.eof()) {
			PrintError("ReadText: unexpected eof");
		}
		text += c;
		inp.get(c);
	}
	return text;
}

class Pair {
public:
	int number;
	string text;
};

typedef Pair DialPoint;
typedef Pair Choice;

class Level {
private:
public:
	DialPoint * dialPoints;
	std::vector<Choice>* dialGraph;

	int Load(string levelPath) {
		std::ifstream levelFile;

		levelFile.open(levelPath, std::ios::in);
		if (!levelFile.is_open()) {
			PrintError("Load level: cannot open file");
		}

		//read the size of dialGraph
		{
			int dialSize;
			if (!(levelFile >> dialSize)) {
				PrintError("Load level: Couldn't read dial size");
			}
			dialGraph = new std::vector<Choice>[dialSize];
			dialPoints = new DialPoint[dialSize];
		}

		// read the dialogue point
		while (!Eof(levelFile)) {
			// read number of point and the text
			int pointNum;
			string pointText;
			char temp;

			if (GetNextSymbol(levelFile) != '<') {
				PrintError("Load level: expected \"<\"");
			}

			if (GetNextSymbol(levelFile) != '(') {
				PrintError("Load level: expected \"(\"");
			}

			if (!(levelFile >> pointNum)) {
				PrintError("Load level: Couldn't read dial size");
			}

			if (GetNextSymbol(levelFile) != ')') {
				PrintError("Load level: expected \")\"");
			}

			string text = ReadText(levelFile);
			dialPoints[pointNum] = DialPoint{ pointNum, text };

			temp = GetNextSymbol(levelFile);
			if (temp != '{') {
				if (temp != '[') {
					PrintError("Load level: expected \"(\" or \"[\"");
				}
				int nextPoint;
				if (!(levelFile >> nextPoint)) {
					levelFile.clear();
					if (GetNextSymbol(levelFile) != '.') {
						PrintError("Load level: Expected \".\" or next point");
					}
					if (GetNextSymbol(levelFile) != '.') {
						PrintError("Load level: Expected \".\"");
					}
					nextPoint = 0;
				}

				dialGraph[pointNum] = std::vector<Choice>(1);
				dialGraph[pointNum][0] = Choice{ nextPoint, "" };
				if (GetNextSymbol(levelFile) != ']') {
					PrintError("Load level: expected \"]\"");
				}
				continue;
			}
			dialGraph[pointNum] = std::vector<Choice>();

			while (true) {
				temp = GetNextSymbol(levelFile);
				if (temp != '<') {
					if (temp != '}') {
						PrintError("Load level: expected \"<\" or \"}\"");
					}
					break;
				}

				if (GetNextSymbol(levelFile) != '[') {
					PrintError("Load level: expected \"[\"");
				}

				int nextPoint;
				if (!(levelFile >> nextPoint)) {
					levelFile.clear();
					if (GetNextSymbol(levelFile) != '.') {
						PrintError("Load level: Expected \".\" or next point");
					}
					if (GetNextSymbol(levelFile) != '.') {
						PrintError("Load level: Expected \".\"");
					}
					nextPoint = 0;
				}
				if (GetNextSymbol(levelFile) != ']') {
					PrintError("Load level: expected \"]\"");
				}
				string text = ReadText(levelFile);

				dialGraph[pointNum].push_back(Choice{ nextPoint, text });
			}
		}

		levelFile.close();
		return 0;
	}
};

class Game {
private:
	Level * curLevel;
	int curPoint;
public:
	Game(string path) {
		curLevel = new Level;
		curLevel->Load(path);
		curPoint = 0;
	}
	int Run() {
		int curItem = 0;
		while (true) {
			system("cls");
			WriteText(curLevel->dialPoints[curPoint].text);
			std::cout << "\n\n";
			if (curLevel->dialGraph[curPoint][0].text != "") {
				int vecSize = curLevel->dialGraph[curPoint].size();
				for (int i = 0; i < vecSize; ++i) {
					Space(TAB_SIZE);
					std::cout << ((i == curItem) ? ">> " : "   ");
					std::cout << curLevel->dialGraph[curPoint][i].text << "\n";
				}
			}
			else {
				Space(TAB_SIZE);
				std::cout << ">> Next..";
			}

			int item = _getch();
			if (item == 13) {
				curPoint = curLevel->dialGraph[curPoint][curItem].number;
				curItem = 0;
				continue;
			}
			if (item == 224) {
				item = _getch();
			}
			if (item == 72 && curItem > 0) {
				curItem--;
				continue;
			}
			if (item == 80 && curItem < curLevel->dialGraph[curPoint].size()) {
				curItem++;
				continue;
			}
		}
	}
};


int not_main() {
	setlocale(LC_ALL, "Russian");
	Game game("game");
	game.Run();

	return 0;
}