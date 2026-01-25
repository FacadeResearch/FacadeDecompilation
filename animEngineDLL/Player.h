#pragma once

class Player {
	public:
		Player();
		void InitPlayer(const char* name, int arg2, int arg3);
	private:
		const char* name;
};