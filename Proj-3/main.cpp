#include <iostream>
#include <exception>



#include "CGFapplication.h"
//#include "ANFScene.h"
//#include "Interface.h"
#include "PickScene.h"
#include "PickInterface.h"
#include "socket.h"
using std::cout;
using std::exception;



int main(int argc, char* argv[]){
	socketConnect();
	char *s = "comando(1, 2).\n";
	envia(s, strlen(s));
	char ans[128];
	recebe(ans);
	
	CGFapplication app = CGFapplication();

	try {
		app.init(&argc, argv);

		//app.setScene(new ANFScene("LAIG_TP1_ANF_T4_G8_v2 (3).anf"));
		//app.setInterface(new Interface());
		app.setScene(new PickScene());
		app.setInterface(new PickInterface());
		app.run();
	}
	catch(GLexception& ex) {
		cout << "Erro: " << ex.what();
		getchar();
		return -1;
	}
	catch(exception& ex) {
		cout << "Erro inesperado: " << ex.what();
		getchar();
		return -1;
	}
	
	quit();
	return 0;
}