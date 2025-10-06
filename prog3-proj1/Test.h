#pragma once

template<typename T> void EvaluationTest(int iItem, std::string filename)
{
	// iTem - index of the item, 1...10
	// filename - name and path of the file used for storing the items from data structure
	char c;
	const char* colours[] = { "Verekarva Punane", "Tumedam Sinine", "Punakas Kollane", "Heledam Roheline", "Absoluutne Must", "Pimestav Valge" };
	char buf[20];
	//
	// Test 1
	//
	cout << "Test constructor DataStructure(n), printout and number of items" << endl;
	DataStructure* pds1 = new DataStructure(35);  // test constructor
	cout << *pds1 << endl;  // test printout
	cout << "Number of items is " << pds1->GetItemsNumber() << endl;  // test number of items
	delete pds1; // test destructor
	cin.get(c);
	//
	// Test 2
	//
	cout << endl << "Test default constructor and operator+=" << endl;
	DataStructure* pds2 = new DataStructure;  // test default constructor
	for (int i = 0; i < 5; i++)
	{
		*pds2 += (T*)GetItem(iItem);  // test operator+=
	}
	cout << *pds2 << endl;
	try
	{
		strcpy_s(buf, 20, "Taevakarva Sinine");
		*pds2 += (T*)GetItem(iItem, buf);
		*pds2 += (T*)GetItem(iItem, buf);
	}
	catch (exception& e)
	{
		cout << e.what() << endl;
	}
	cout << endl << *pds2 << endl;
	delete pds2;
	cin.get(c);
	//
	// Test 3
	//
	cout << endl << "Test copy constructor and operator==" << endl;
	DataStructure* pds3 = new DataStructure(5);
	DataStructure ds3 = *pds3;  // test copy constructor 
	cout << ds3 << endl;
	cout << ((*pds3 == ds3) ? "structures are identical" : "structures are not identical") << endl; // test operator==
	*pds3 += (T*)GetItem(iItem);
	cout << ((*pds3 == ds3) ? "structures are identical" : "structures are not identical") << endl;
	delete pds3;
	cin.get(c);
	//
	// Test 4
	//
	cout << endl << "Test operator=" << endl;
	DataStructure* pds4 = new DataStructure(5);  // test operator=
	cout << *pds4 << endl;
	DataStructure* pds5 = new DataStructure(10);
	cout << *pds5 << endl;
	*pds5 = *pds4;
	cout << *pds5 << endl;
	cout << ((*pds4 == *pds5) ? "structures are identical" : "structures are not identical") << endl;
	cin.get(c);
	//
	// Test 5
	//
	cout << endl << "Test operator-=" << endl;
	DataStructure* pds6 = new DataStructure;
	for (int i = 0; i < 6; i++)
	{
		strcpy_s(buf, 20, colours[i]);
		ITEM5* p = (T*)GetItem(iItem, buf);
		*pds6 += (T*)GetItem(iItem, buf);  // test operator+=
	}
	cout << *pds6 << endl;
	try
	{
		strcpy_s(buf, 20, "Pleekinud Pruun");
		*pds6 -= buf;
	}
	catch (exception& e)
	{
		cout << e.what() << endl;
	}
	for (int i = 0; i < 6; i++)
	{
		strcpy_s(buf, 20, colours[i]);
		ITEM5* p = (T*)GetItem(iItem, buf);
		*pds6 -= buf;  // test operator-=
	}
	cout << "Number of items is " << pds6->GetItemsNumber() << endl; // test number of items
	//
	// Test 6
	//
	cout << endl << "TestGetItem" << endl;
	DataStructure* pds7 = new DataStructure;
	for (int i = 0; i < 6; i++)
	{
		strcpy_s(buf, 20, colours[i]);
		ITEM5* p = (T*)GetItem(iItem, buf);
		*pds7 += (T*)GetItem(iItem, buf);  // test operator+=
	}
	strcpy_s(buf, 20, "Heledam Roheline");
	cout << (pds7->GetItem(buf) ? "Item found" : "Item not found") << endl;
	strcpy_s(buf, 20, "Pleekinud Pruun");
	cout << (pds7->GetItem(buf) ? "Item found" : "Item not found") << endl;
	//
	// Test 7
	//
	cout << endl << "Test Write and constructor DataStructure(filename)" << endl;
	try
	{
		DataStructure* pds8 = new DataStructure(10);
		pds8->Write(filename);  // test Write
		cout << "File stored" << endl;
		DataStructure* pds9 = new DataStructure(filename);  // test constructor
		cout << "New structure created, the structures are ";
		cout << ((*pds8 == *pds9) ? "identical" : "not identical") << endl;
	}
	catch (exception& e)
	{
		cout << e.what() << endl;
	}
}
