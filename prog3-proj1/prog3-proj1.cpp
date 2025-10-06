#include <iostream>
#include <exception>
#include "DateTime.h"
#include "Items.h"
#include "Headers.h"
#include "DataSource.h"
using namespace std;
// IMPORTANT: follow the given order of *.h files: DataSource.h must be the last
#define NITEM 4 // define you item
int main()
{
	try // uncomment you Struct
	{
		//HEADER_B* p1 = GetStruct1(NITEM,100);
		//HEADER_C* p2 = GetStruct2(NITEM, 100);
		//HEADER_A** pp3 = GetStruct3(NITEM, 100);
		//HEADER_D* p4 = GetStruct4(NITEM, 100);
		HEADER_E* p5 = GetStruct5(NITEM, 100);
	}
	catch (exception& e)
	{
		cout << e.what() << endl;
	}
	return 0;
}