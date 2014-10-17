#include <iostream>
#include "XMLScene.h"
#include "ANFScene.h"
#include "CGFapplication.h"
#include "Interface.h"

using namespace std;

int main(int argc, char* argv[]){


	CGFapplication app = CGFapplication();

	try {
		app.init(&argc, argv);

		app.setScene(new ANFScene("snowman.anf"));
		app.setInterface(new Interface());
		app.run();
	}
	catch(GLexception& ex) {
		cout << "Erro: " << ex.what();
		return -1;
	}
	catch(exception& ex) {
		cout << "Erro inesperado: " << ex.what();
		return -1;
	}
	return 0;
}