#include <iostream>
#include <exception>
#include <fstream>
#include "DateTime.h"
#include "Items.h"
#include "Headers.h"
#include "DataSource.h"
#include "DataStructure.h"

// Constructor that creates empty data structure.
DataStructure::DataStructure() : pStruct(nullptr) {
}

// Constructor that creates data structure of n items. n cannot exceed 100.
DataStructure::DataStructure(int n) : pStruct(nullptr) {
    if (n < 1 || n > 100) {
        throw std::invalid_argument("Invalid number of items. Must be between 1 and 100.");
    }

    pStruct = GetStruct5(ITEM_TYPE, n);
}

// Constructor that takes a filename and may throw exception
DataStructure::DataStructure(std::string Filename) throw(std::exception) : pStruct(nullptr) {
    std::ifstream file(Filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + Filename);
    }

    // TODO: Implement file reading logic based on file format
    // This would need to parse the file and construct the HEADER_E structure

    file.close();
}

// Destructor
DataStructure::~DataStructure() {
    // Clean up the linked list structure
    HEADER_E* current = pStruct;
    while (current != nullptr) {
        HEADER_E* next = current->pNext;

        // Clean up the items array if it exists
        if (current->ppItems != nullptr) {
            // TODO: Need to determine array size and clean up individual items
            // This depends on how the array size is tracked
            delete[] current->ppItems;
        }

        delete current;
        current = next;
    }
    pStruct = nullptr;
}

// Copy constructor
DataStructure::DataStructure(const DataStructure& Original) : pStruct(nullptr) {
    // Deep copy of the structure
    if (Original.pStruct == nullptr) {
        return;
    }

    HEADER_E* originalCurrent = Original.pStruct;
    HEADER_E* newPrevious = nullptr;

    while (originalCurrent != nullptr) {
        // Create new header
        HEADER_E* newHeader = new HEADER_E;
        newHeader->cBegin = originalCurrent->cBegin;
        newHeader->pNext = nullptr;
        newHeader->pPrior = newPrevious;

        // TODO: Deep copy the items array
        // This requires knowing the array size and copying individual items
        newHeader->ppItems = nullptr; // Placeholder

        // Link the new header
        if (newPrevious != nullptr) {
            newPrevious->pNext = newHeader;
        }
        else {
            pStruct = newHeader;
        }

        newPrevious = newHeader;
        originalCurrent = originalCurrent->pNext;
    }
}

// Get number of items
int DataStructure::GetItemsNumber() {
    int count = 0;
    HEADER_E* current = pStruct;

    while (current != nullptr) {
        // TODO: Count items in the array pointed to by ppItems
        // This requires knowing how the array size is tracked
        current = current->pNext;
    }

    return count;
}

// Get item by ID
ITEM4 DataStructure::GetItem(char* pID) {
    if (pID == nullptr) {
        throw std::invalid_argument("ID cannot be null");
    }

    HEADER_E* current = pStruct;
    while (current != nullptr) {
        if (current->ppItems != nullptr) {
            // TODO: Search through the items array
            // This requires knowing the array size and searching individual items
            // for (int i = 0; i < arraySize; i++) {
            //     ITEM4 *item = static_cast<ITEM4*>(current->ppItems[i]);
            //     if (item && item->pID && strcmp(item->pID, pID) == 0) {
            //         return *item;
            //     }
            // }
        }
        current = current->pNext;
    }

    // If item not found, throw exception or return empty item
    throw std::runtime_error("Item not found");
}

// Add item operator
void DataStructure::operator+=(ITEM4 item) throw(std::exception) {
    if (item.pID == nullptr) {
        throw std::invalid_argument("Item ID cannot be null");
    }

    // Determine which header this item belongs to based on cBegin
    char firstChar = item.pID[0];

    HEADER_E* targetHeader = nullptr;
    HEADER_E* current = pStruct;

    // Find existing header with matching cBegin
    while (current != nullptr) {
        if (current->cBegin == firstChar) {
            targetHeader = current;
            break;
        }
        current = current->pNext;
    }

    // If no matching header found, create one
    if (targetHeader == nullptr) {
        targetHeader = new HEADER_E;
        targetHeader->cBegin = firstChar;
        targetHeader->ppItems = nullptr;
        targetHeader->pNext = pStruct;
        targetHeader->pPrior = nullptr;

        if (pStruct != nullptr) {
            pStruct->pPrior = targetHeader;
        }
        pStruct = targetHeader;
    }

    // TODO: Add item to the targetHeader's items array
    // This requires dynamic array management
}

// Remove item by ID operator
void DataStructure::operator-=(char* pID) throw(std::exception) {
    if (pID == nullptr) {
        throw std::invalid_argument("ID cannot be null");
    }

    HEADER_E* current = pStruct;
    while (current != nullptr) {
        if (current->ppItems != nullptr) {
            // TODO: Search and remove item from the array
            // This requires knowing the array size and managing dynamic arrays
        }
        current = current->pNext;
    }

    // If item not found, could throw exception or silently ignore
    throw std::runtime_error("Item not found for removal");
}

// Credit to geeksforgeeks.org https://www.geeksforgeeks.org/cpp/serialize-and-deserialize-an-object-in-cpp/
void DataStructure::Write(std::string Filename) throw(std::exception) {
    // Check if data structure is empty
    if (pStruct == nullptr) {
        throw std::runtime_error("Cannot write empty data structure");
    }

    std::ofstream file(Filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for writing: " + Filename);
    }

    try {
        // Count total headers first
        int headerCount = 0;
        HEADER_E* current = pStruct;
        while (current != nullptr) {
            headerCount++;
            current = current->pNext;
        }

        // Write header count
        file.write(reinterpret_cast<const char*>(&headerCount), sizeof(int));

        // Write each header and its items
        current = pStruct;
        while (current != nullptr) {
            // Write header's cBegin character
            file.write(&current->cBegin, sizeof(char));

            // Count items in this header's array
            int itemCount = 0;
            if (current->ppItems != nullptr) {
                // Count items until we find one with ID starting with 'Z' (sentinel)
                while (current->ppItems[itemCount] != nullptr) {
                    ITEM4* item = static_cast<ITEM4*>(current->ppItems[itemCount]);
                    if (item != nullptr && item->pID != nullptr && item->pID[0] == 'Z') {
                        itemCount++; // Include the sentinel item in count
                        break;
                    }
                    itemCount++;
                }
            }

            // Write item count for this header
            file.write(reinterpret_cast<const char*>(&itemCount), sizeof(int));

            // Write each item in the array
            for (int i = 0; i < itemCount; i++) {
                ITEM4* item = static_cast<ITEM4*>(current->ppItems[i]);
                if (item != nullptr) {
                    // Write item code
                    file.write(reinterpret_cast<const char*>(&item->Code), sizeof(unsigned long int));

                    // Write ID string length and content
                    int idLength = item->pID ? strlen(item->pID) : 0;
                    file.write(reinterpret_cast<const char*>(&idLength), sizeof(int));
                    if (idLength > 0) {
                        file.write(item->pID, idLength);
                    }

                    // Write date string length and content
                    int dateLength = item->pDate ? strlen(item->pDate) : 0;
                    file.write(reinterpret_cast<const char*>(&dateLength), sizeof(int));
                    if (dateLength > 0) {
                        file.write(item->pDate, dateLength);
                    }
                }
            }

            current = current->pNext;
        }

        file.close();

    }
    catch (const std::exception& e) {
        file.close();
        throw std::runtime_error("Error writing to file: " + std::string(e.what()));
    }