#include <string>
#include <iostream>

using namespace std;

class Session {
	string name;
	string currentLine;
	int totalLines = 0;
	string timestamp;

public:
	void newSession(string scrName, string timeCreated);

	string GetName();

	void screen();
};