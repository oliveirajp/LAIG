#include <iostream>
#include "XMLScene.h"
#include "ANFScene.h"
#include "CGFapplication.h"

using namespace std;

int main(int argc, char* argv[]){


	CGFapplication app = CGFapplication();

	try {
		app.init(&argc, argv);

		app.setScene(new ANFScene("LAIG_TP1_ANF_T06_G03_v1.anf"));
		app.setInterface(new CGFinterface());
		
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