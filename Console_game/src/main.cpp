#include <iostream>
#include <string>
#include <fstream>
#include <conio.h>
#include <vector>
#include <unordered_map>
#include <list>

using std::string;

#define LEFT_SPACE_SIZE 31
#define BORDER_SPACE_SIZE 3
#define LINE_LENGTH 50
#define UPPER_SPACE_SIZE 5
#define FILES_FOLDER_PATH "..\\Console_game\\src\\"

void WriteChar(char c, int number) {
	for (int i = 0; i < number; ++i) {
		std::cout << c;
	}
}

void WriteText(const string& text) {
	WriteChar('\n', UPPER_SPACE_SIZE);
	int lettersOnLine = 0;
	int limit = LINE_LENGTH + BORDER_SPACE_SIZE;
	for (int i = 0; text[i] != '\0'; ++i) {
		
		if (i % LINE_LENGTH == 0) {
			WriteChar(' ', LEFT_SPACE_SIZE);
			std::cout << "{{";
			WriteChar(' ', BORDER_SPACE_SIZE);
		}
		
		std::cout << text[i];
		lettersOnLine++;

		if ((i + 1) % LINE_LENGTH == 0) {
			WriteChar(' ', BORDER_SPACE_SIZE);
			std::cout << "}}\n";
			lettersOnLine = 0;
		}
	}
	if (lettersOnLine != 0) {
		WriteChar(' ', limit - lettersOnLine);
		std::cout << "}}\n";
	}
}

void WriteItem(const string& text, bool isSelected) {
	WriteChar(' ', LEFT_SPACE_SIZE);
	std::cout << ((isSelected) ? ">> " : "   ") << text << '\n';
}

void PrintError(const char* text) {
	std::cout << "ERROR: " << text;
	system("pause");
	exit(0);
}

bool Eof(std::istream& inp) {
	if (inp.eof()) {
		return true;
	}
	int curPos = inp.tellg();
	inp.get();
	if (inp.eof()) {
		return true;
	}
	inp.seekg(curPos);
	return false;
}

char GetNextSymbolWithoutSpace(std::istream& levelFile) {
	char c;
	do {
		if (Eof(levelFile)) {
			PrintError("GetNextSymbolWithoutSpace: Unexpected eof");
		}
		c = levelFile.get();
	} while (c == ' ' || c == '\n' || c == '\t');
	return c;
}

char GetNextSymbol(std::istream& levelFile) {
	char c;
	if (Eof(levelFile)) {
		PrintError("GetNextSymbol: Unexpected eof");
	}
	c = levelFile.get();
	return c;
}

string ReadWhileNot(std::istream& levelFile, char stopChar) {
	string text = "";
	while (true) {
		char c = GetNextSymbol(levelFile);
		if (c != stopChar) {
			text += c;
		}
		else {
			break;
		}
	}
	return text;
}

class Phrase {
public:
	int refId;
	std::vector<string> textArr;

	char returnValue;
	bool close;
	string openPath;

	Phrase() :
		refId(-1),
		textArr(std::vector<string>()),
		returnValue(-1),
		close(false),
		openPath("") {}

	Phrase(int _id, const string& _text) :
		refId(_id),
		textArr(std::vector<string>()),
		returnValue(-1),
		close(false),
		openPath("") 
	{
		textArr.push_back(_text);
	}
};


class GameData {
private:
	std::unordered_map<string, string, std::hash<string>> map;

	string ReadText(std::istream& levelFile) {
		string text;
		char c;
		while (true) {
			c = GetNextSymbol(levelFile);
			if (c != '>') {
				text += c;
			}
			else {
				break;
			}
		}
		return text;
	}

public:
	bool Get(const string& key, string& value) const {
		auto it = map.find(key);
		if (it != map.end()) {
			value = (*it).second;
			return true;
		}
		else {
			return false;
		}
	}
	void Put(const string& key, const string& value) {
		auto it = map.insert(std::make_pair(key, value));
	}

	void Load(const string& path) {
		std::ifstream paramFile;
		paramFile.open(FILES_FOLDER_PATH + path, std::ios::in);

		if (!paramFile.is_open()) {
			PrintError("Load: couldn't open param file");
		}

		map.clear();

		while (!Eof(paramFile)) {
			char c = GetNextSymbolWithoutSpace(paramFile);
			if (c != '<') {
				PrintError("Load: expected \"<\"");
			}
			string key = ReadText(paramFile);
			c = GetNextSymbolWithoutSpace(paramFile);
			if (c != '<') {
				PrintError("Load: expected \"<\"");
			}
			string value = ReadText(paramFile);

			Put(key, value);
		}
	}
};


class Level {
private:
public:
	Phrase* dialogPoints = nullptr;
	std::vector<Phrase>* dialogGraph = nullptr;
	GameData* data = nullptr;

	char StringToInt8(const string& str) {
		int number = 0;
		for (auto it = str.begin(); it != str.end(); ++it) {
			if ((*it) < '0' || (*it) > '9') {
				PrintError("StringToInt8: couldn't convert string to Int8");
			}
			number *= 10;
			number += (*it) - '0';
		}
		char int8Num = (char)number;
		if (int8Num < 0) {
			PrintError("StringToInt8: number too big");
		}
		return int8Num;
	}

	int ReadPointsNumber(std::istream& levelFile) {
		int pointsNum;
		if (!(levelFile >> pointsNum)) {
			PrintError("ReadPointsNum: couldn't read points number");
		}
		return pointsNum;
	}

	int ReadPointNum(std::istream& levelFile) {
		char c = GetNextSymbolWithoutSpace(levelFile);
		if (c != '(') {
			PrintError("ReadPointNum: expected \"(\"");
		}
		int pointNum;
		if (!(levelFile >> pointNum)) {
			PrintError("ReadPointNum: couldn't read point number");
		}
		c = GetNextSymbol(levelFile);
		if (c != ')') {
			PrintError("ReadPointNum: expected \")\"");
		}
		return pointNum;
	}

	bool CheckWord(std::istream& inp, const string& word) {
		for (auto it = word.begin(); it != word.end(); ++it) {
			if (Eof(inp)) {
				PrintError("CheckWord: Enexpected eof");
			}
			if ((*it) != inp.get()) {
				return false;
			}
		}
		return true;
	}

	string ReadReplaceCommand(std::istream& levelFile) {
		if (!CheckWord(levelFile, "replace")) {
			PrintError("ReadReplaceCommand: expected \"replace\"");
		}
		char c = GetNextSymbol(levelFile);
		if (c != '"') {
			PrintError("ReadReplaceCommand: expected \'\"\'");
		}
		string key;
		while (true) {
			c = GetNextSymbol(levelFile);
			if (c == '"') {
				break;
			}
			key += c;
		}

		string value;

		if (!(data->Get(key, value))) {
			PrintError("ReadReplaceCommand: unknwon key");
		}

		return value;
	}

	string ReadPhraseText(std::istream& levelFile) {
		string text;
		char c;
		while (true) {
			c = GetNextSymbol(levelFile);
			if (c == '#') {
				text += ReadReplaceCommand(levelFile);
				continue;
			}
			if (c != '>') {
				text += c;
			}
			else {
				break;
			}
		}
		return text;
	}

	string ReadCommandParams(std::istream& levelFile) {
		char c = GetNextSymbol(levelFile);
		if (c != '(') {
			PrintError("ReadCommandParams: expected \"(\"");
		}
		string params = ReadWhileNot(levelFile, ')');
		return params;
	}

	void ReadPhraseParams(std::istream& levelFile, Phrase& phrase) {
		char c;
		if (!(levelFile >> phrase.refId)) {
			levelFile.clear();
			c = GetNextSymbol(levelFile);
			switch (c) {
			case 'o': {
				if (!CheckWord(levelFile, "pen")) {
					PrintError("ReadPhraseParams: expected \"open\"");
				}
				phrase.openPath = ReadCommandParams(levelFile);
				break;
			}
			case 'c': {
				if (!CheckWord(levelFile, "lose")) {
					PrintError("ReadPhraseParams: expected \"close\"");
				}
				string params = ReadCommandParams(levelFile);
				if (params != "") {
					phrase.returnValue = StringToInt8(params);
				}
				phrase.close = true;
				break;
			}
			default: {
				PrintError("ReadPhraseParams: expected \"open\" or \"close\"");
				break; 
			}
			}
			c = GetNextSymbol(levelFile);
			if (c != ']') {
				PrintError("ReadPhraseParams: expected \"]\"");
			}
			return;
		}
		
		c = GetNextSymbol(levelFile);
		
		if (c == ']') {
			return;
		}

		if (c != ';') {
			PrintError("ReadPhraseParams: expected \"]\" or \";\"");
		}
		
		c = GetNextSymbol(levelFile);
		if (c != 'r') {
			PrintError("ReadPhraseParams: expected \"return\"");
		}
		if (!CheckWord(levelFile, "eturn")) {
			PrintError("ReadPhraseParams: expected \"return\"");
		}

		string params = ReadCommandParams(levelFile);
		if (params == "") {
			PrintError("ReadPhraseParams: empty return params");
		}
		phrase.returnValue = StringToInt8(params);

		c = GetNextSymbol(levelFile);
		if (c != ']') {
			PrintError("ReadPhraseParams: expected \"]\"");
		}
	}

	void ReadChoices(std::istream& levelFile, std::vector<Phrase>& choices) {
		bool isEmpty = true;
		while (true) {
			char c = GetNextSymbolWithoutSpace(levelFile);
			if (c == '}') {
				if (isEmpty) {
					PrintError("ReadChoices: empty choices");
				}
				else {
					break;
				}
			}
			if (c != '<') {
				PrintError("ReadChoices: expected \"<\"");
			}

			isEmpty = false;
			string choiceText = ReadPhraseText(levelFile);
			
			Phrase newChoice;
			newChoice.textArr.push_back(choiceText);

			c = GetNextSymbol(levelFile);

			if (c != '[') {
				PrintError("ReadChoices: expected \"[\"");
			}

			ReadPhraseParams(levelFile, newChoice);

			choices.push_back(newChoice);
		}
	}

	void ReadPointsSet(std::istream& levelFile, Phrase& set) {
		char c;
		bool isEmpty = true;
		while (true) {
			c = GetNextSymbolWithoutSpace(levelFile);
			if (c == '}') {
				if (isEmpty) {
					PrintError("ReadPointsSet: empty set");
				}
				return;
			}
			if (c != '<') {
				PrintError("ReadPointsSet: expected \"<\"");
			}
			isEmpty = false;
			string text = ReadPhraseText(levelFile);
			set.textArr.push_back(text);
		}
		c = GetNextSymbol(levelFile);
		if(c != '[') {
			PrintError("ReadPointSet: expected \"[\"");
		}
		ReadPhraseParams(levelFile, set);
	}

	int Load(const string& levelPath) {

		if(dialogPoints != nullptr) delete[] dialogPoints;
		if(dialogGraph != nullptr) delete[] dialogGraph;

		std::ifstream levelFile;
		levelFile.open(FILES_FOLDER_PATH + levelPath, std::ios::in);
		if(!levelFile.is_open()) {
			PrintError("Couldn't open level file");
		}

		int pointsNumber = ReadPointsNumber(levelFile);

		dialogPoints = new Phrase[pointsNumber];
		dialogGraph = new std::vector<Phrase>[pointsNumber];

		while (!Eof(levelFile)) {
			char c;
			int pointNum = ReadPointNum(levelFile);

			if (pointNum >= pointsNumber) {
				PrintError("Load: point number bigger, then max points number");
			}

			c = GetNextSymbolWithoutSpace(levelFile);
			if (c != '<') {
				if (c != '{') {
					PrintError("Load: expected \"{\" or \"<\"");
				}
				
				Phrase newPointSet;
				ReadPointsSet(levelFile, newPointSet);

				c = GetNextSymbol(levelFile);

				if (c != '[') {
					PrintError("Load: expected \"[\"");
				}

				ReadPhraseParams(levelFile, newPointSet);
				
				int nextPointNum = newPointSet.refId;
				
				dialogGraph[pointNum].push_back(Phrase(nextPointNum, ""));
				dialogPoints[pointNum] = newPointSet;

				continue;
			}
			string text = ReadPhraseText(levelFile);
			dialogPoints[pointNum] = Phrase(pointNum, text);

			c = GetNextSymbolWithoutSpace(levelFile);

			if (c != '{') {
				
				if (c != '[') {
					PrintError("Load: expected \"[\"");
				}

				Phrase nextPoint;
				nextPoint.textArr.push_back("");

				ReadPhraseParams(levelFile, nextPoint);

				dialogGraph[pointNum].push_back(nextPoint);
				continue;
			}
			ReadChoices(levelFile, dialogGraph[pointNum]);
		}

		levelFile.close();
		return 0;
	}
};

class Game {
private:
	GameData* data;
	Level* curLevel;
public:
	Game(const string& levelFilePath, const string& dataFilePath) {

		data = new GameData;
		data->Load(dataFilePath);

		curLevel = new Level;

		curLevel->data = data;

		curLevel->Load(levelFilePath);
	}

	int Run() {
		int curPointNum = 0;
		auto pointTextIt = 0;
		int curItem = 0;
		while (true) {
			system("cls");
			WriteText(curLevel->dialogPoints[curPointNum].textArr[pointTextIt]);
			WriteChar('\n', 2);
			if (pointTextIt == curLevel->dialogPoints[curPointNum].textArr.size() - 1) { 
				
				auto choicesVec = curLevel->dialogGraph[curPointNum];
				auto choiceTextArr = choicesVec[0].textArr;

				if (choiceTextArr[0] != "") {
					for (int i = 0; i < choicesVec.size(); ++i) {
						WriteItem(choicesVec[i].textArr[0], (i == curItem));
					}
				}
				else {
					WriteItem("Next..", true);
				}
			}
			else {
				WriteItem("Next..", true);
			}

			
			int item;
			do {
				item = _getch();
			} while (item != 13 && item != 72 && item != 80);
			
			if (item == 13) {

				Phrase nextPhrase = curLevel->dialogGraph[curPointNum][curItem];

				//if (nextPhrase.returnValue != -1) {
				//	// your code)))
				//}

				if (nextPhrase.close) {
					break;
				}
				if (nextPhrase.openPath != "") {
					delete curLevel;
					curLevel = new Level;
					curLevel->data = data;
					curLevel->Load(nextPhrase.openPath);
					curPointNum = 0;
					curItem = 0;
					pointTextIt = 0;
					continue;
				}

				if (pointTextIt == curLevel->dialogPoints[curPointNum].textArr.size() - 1) {
					curPointNum = curLevel->dialogGraph[curPointNum][curItem].refId;
					pointTextIt = 0;
				}
				else {
					pointTextIt++;
				}
				curItem = 0;
				continue;
			}

			if (item == 72 && curItem > 0) {
				curItem--;
			}

			if (item == 80 && curItem < curLevel->dialogGraph[curPointNum].size() - 1) {
				curItem++;
			}
		}
		return 0;
	}
};

int main() {
	
	setlocale(LC_ALL, "Russian");

	Game game("main_menu", "game_params");

	game.Run();

	return 0;
}