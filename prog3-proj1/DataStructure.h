#pragma once
#include "Headers.h"
#include "Items.h"
#include <string>
#include <exception>
#include <iostream>

#define ITEM_TYPE 4

class DataStructure {
private:
    HEADER_E* pStruct;

    void ClearStructure();
    void DeleteItem(pItem item);
	HEADER_E* CreateHeaderCopy(const HEADER_E* originalHeader);
	pItem CreateItemCopy(const pItem originalItem);
	int CountItems(void** ppItems);
	void parseID(const char* pID, char& secondChar);

public:
    // Default constructor
    DataStructure();

    // Constructor that takes an integer n
    DataStructure(int n);

    // Constructor that takes a filename and may throw exception
    DataStructure(std::string Filename) throw(std::exception);

    // Destructor
    ~DataStructure();

    // Copy constructor
    DataStructure(const DataStructure& Original);

    // Get number of items
    int GetItemsNumber();

    // Get item by ID
    pItem GetItem(char* pID);

    // Add item operator
    void operator+=(ITEM4 *item) throw(std::exception);

    // Remove item by ID operator
    void operator-=(char* pID) throw(std::exception);

    bool operator==(DataStructure& Other);

    DataStructure& operator=(const DataStructure& Right);

    // Write data structure to binary file
    void Write(std::string Filename) throw(std::exception);

    // Friend function for output stream operator
    friend std::ostream& operator<<(std::ostream& ostr, const DataStructure& str);
};