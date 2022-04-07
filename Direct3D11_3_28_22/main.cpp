#include "Application.h"

int main(void) {
	Application app;

	if (!app.init())
		return -1;

	while (!app.is_closing())
		app.run();

	return 0;
}