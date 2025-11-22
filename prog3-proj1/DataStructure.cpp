#include <iostream>
#include <exception>
#include <fstream>
#include "DateTime.h"
#include "Items.h"
#include "Headers.h"
#include "DataSource.h"
#include "DataStructure.h"

// Get pointers to second word; throws on invalid ID
static void parseID(const char* id, char& second) {
    if (!id)
        throw std::runtime_error("ID is null");
	// Find first space character and perform simple validation
    const char* sp = std::strchr(id, ' ');
    if (!sp || sp == id || *(sp + 1) == '\0')
        throw std::runtime_error("ID must contain two words separated by space");
    second = *(sp + 1);
}

// Constructor that creates empty data structure.
DataStructure::DataStructure() : pStruct(nullptr) {
}

// Constructor that creates data structure of n items. n cannot exceed 100.
DataStructure::DataStructure(int n) : pStruct(nullptr) {
    if (n < 1 || n > 100) {
        throw std::invalid_argument("Invalid number of items. Must be between 1 and 100.");
    }

    int inserted = 0;
    const int batchSize = 32;
    
    while (inserted < n) {
        int toGenerate = std::min(batchSize, n - inserted);
        ITEM4* items[32] = {nullptr};  // Match batchSize
        int generated = 0;
        
        // Generate a batch of items
        for (int i = 0; i < toGenerate; ++i) {
            items[i] = static_cast<ITEM4*>(::GetItem(ITEM_TYPE));
            if (!items[i]) {
                throw std::runtime_error("Failed to generate item");
            }
            items[i]->pNext = nullptr;
            ++generated;
        }
        
        // Insert the batch
        for (int i = 0; i < generated && inserted < n; ++i) {
            try {
                *this += items[i];
                ++inserted;
            } catch (const std::exception&) {
                // Silently skip duplicates or other insertion errors
            }
        }
    }
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
    if (Original.pStruct == nullptr) {
        return;
    }

    HEADER_E* originalCurrent = Original.pStruct;
    HEADER_E* newPrevious = nullptr;

    while (originalCurrent != nullptr) {
        // 1. Create new HEADER_E node
        HEADER_E* newHeader = new HEADER_E;
        newHeader->cBegin = originalCurrent->cBegin;
        newHeader->pNext = nullptr;
        newHeader->pPrior = newPrevious;

        // 2. Deep copy the items array
        if (originalCurrent->ppItems != nullptr) {
            // Count items (using your sentinel pattern)
            int itemCount = 0;
            while (originalCurrent->ppItems[itemCount] != nullptr) {
                itemCount++;
                ITEM4* item = static_cast<ITEM4*>(originalCurrent->ppItems[itemCount - 1]);
                if (item->pID && item->pID[0] == 'Z') break; // Sentinel
            }

            // Allocate new array
            newHeader->ppItems = new void*[itemCount];

            // Deep copy each ITEM4
            for (int i = 0; i < itemCount; i++) {
                ITEM4* originalItem = static_cast<ITEM4*>(originalCurrent->ppItems[i]);
                ITEM4* newItem = new ITEM4;
                
                // Copy the Code
                newItem->Code = originalItem->Code;
                
                // Deep copy pID string
                if (originalItem->pID != nullptr) {
                    int idLen = strlen(originalItem->pID);
                    newItem->pID = new char[idLen + 1];
                    //strcpy(newItem->pID, originalItem->pID);
					strcpy_s(newItem->pID, idLen + 1, originalItem->pID);
                } else {
                    newItem->pID = nullptr;
                }
                
                // Deep copy pDate string
                if (originalItem->pDate != nullptr) {
                    int dateLen = strlen(originalItem->pDate);
                    newItem->pDate = new char[dateLen + 1];
                    //strcpy(newItem->pDate, originalItem->pDate);
					strcpy_s(newItem->pDate, dateLen + 1, originalItem->pDate);
                } else {
                    newItem->pDate = nullptr;
                }
                
                // Copy pNext pointer (if this links to items in the same structure)
                newItem->pNext = originalItem->pNext; // May need adjustment
                
                newHeader->ppItems[i] = newItem;
            }
        } else {
            newHeader->ppItems = nullptr;
        }

        // 3. Link into new list
        if (newPrevious != nullptr) {
            newPrevious->pNext = newHeader;
        } else {
            pStruct = newHeader;
        }

        newPrevious = newHeader;
        originalCurrent = originalCurrent->pNext;
    }
}

// Get number of items
int DataStructure::GetItemsNumber() {
    int count = 0;
    HEADER_E* current = this->pStruct;
    // Traverse headers until nullptr
    for (HEADER_E* currentHeader = this->pStruct; currentHeader; currentHeader = currentHeader->pNext) {
		// Traverse items in the current header's items array until nullptr
        for(ITEM4* currentItem = static_cast<ITEM4*>(currentHeader->ppItems ? currentHeader->ppItems[0] : nullptr); 
            currentItem; 
            currentItem = currentItem->pNext) {
                count++;
		}
    }

    return count;
}

// Get item by ID
ITEM4 DataStructure::GetItem(char* pID) {
    if (pID == nullptr) {
        throw std::invalid_argument("ID cannot be null");
    }

	char firstChar = pID[0];
	char secondChar = 0;
	parseID(pID, secondChar);

    HEADER_E* current = pStruct;
	// Find the header matching the first character
    while (current != nullptr) {
        if (current->cBegin == firstChar) {
            // Search items in this header
            if (current->ppItems != nullptr) {
                for (int i = 0; current->ppItems[i] != nullptr; i++) {
                    ITEM4* item = static_cast<ITEM4*>(current->ppItems[i]);
                    if (item != nullptr && item->pID != nullptr) {
                        // Check if second character matches
                        char itemSecondChar = 0;
                        parseID(item->pID, itemSecondChar);
                        if (itemSecondChar == secondChar && strcmp(item->pID, pID) == 0) {
                            return *item; // Return found item
                        }
                    }
                }
            }
        }
        current = current->pNext;
	}

    // If item not found, throw exception or return empty item
    throw std::runtime_error("Item not found");
}

// Add item operator
void DataStructure::operator+=(ITEM4* pItem){
    if (pItem == nullptr) {
        throw std::invalid_argument("Item pointer cannot be null");
    }
    
    if (pItem->pID == nullptr) {
        throw std::invalid_argument("Item ID cannot be null");
    }

    // Parse the ID to get first and second characters
    char firstChar = pItem->pID[0];
    char secondChar = 0;
    parseID(pItem->pID, secondChar);

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

    // Check if item with same ID already exists in target header
    if (targetHeader != nullptr && targetHeader->ppItems != nullptr) {
        for (int i = 0; targetHeader->ppItems[i] != nullptr; i++) {
            ITEM4* existingItem = static_cast<ITEM4*>(targetHeader->ppItems[i]);
            if (existingItem != nullptr && existingItem->pID != nullptr) {
                if (strcmp(existingItem->pID, pItem->pID) == 0) {
                    throw std::runtime_error("Item with this ID already exists");
                }
            }
        }
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

    // Find the correct position in the array based on second character
    // and count existing items
    int itemCount = 0;
    int insertPosition = -1;
    
    if (targetHeader->ppItems != nullptr) {
        // Count items and find insertion position
        for (int i = 0; targetHeader->ppItems[i] != nullptr; i++) {
            ITEM4* existingItem = static_cast<ITEM4*>(targetHeader->ppItems[i]);
            if (existingItem != nullptr && existingItem->pID != nullptr) {
                char existingSecondChar = 0;
                parseID(existingItem->pID, existingSecondChar);
                
                if (insertPosition == -1 && secondChar < existingSecondChar) {
                    insertPosition = i;
                }
            }
            itemCount++;
        }
        
        if (insertPosition == -1) {
            insertPosition = itemCount; // Insert at end
        }
    } else {
        insertPosition = 0;
    }

    // Create new array with space for one more item
    void** newArray = new void*[itemCount + 2]; // +1 for new item, +1 for nullptr terminator
    
    // Copy items before insertion point
    for (int i = 0; i < insertPosition; i++) {
        newArray[i] = targetHeader->ppItems[i];
    }
    
    // Insert new item
    newArray[insertPosition] = pItem;
    
    // Copy items after insertion point
    if (targetHeader->ppItems != nullptr) {
        for (int i = insertPosition; i < itemCount; i++) {
            newArray[i + 1] = targetHeader->ppItems[i];
        }
    }
    
    // Null terminator
    newArray[itemCount + 1] = nullptr;
    
    // Update pNext pointers in the linked list
    if (insertPosition > 0) {
        ITEM4* prevItem = static_cast<ITEM4*>(newArray[insertPosition - 1]);
        prevItem->pNext = pItem;
    }
    
    if (insertPosition < itemCount) {
        pItem->pNext = static_cast<ITEM4*>(newArray[insertPosition + 1]);
    } else {
        pItem->pNext = nullptr;
    }
    
    // Delete old array and assign new one
    delete[] targetHeader->ppItems;
    targetHeader->ppItems = newArray;
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
void DataStructure::Write(std::string Filename) {
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
}

// Output stream operator
std::ostream& operator<<(std::ostream& ostr, const DataStructure& str) {
    if (str.pStruct == nullptr) {
        ostr << "Empty DataStructure";
        return ostr;
    }

    int totalItems = 0;
    
    // Traverse all headers
    for (HEADER_E* currentHeader = str.pStruct; currentHeader != nullptr; currentHeader = currentHeader->pNext) {
        ostr << "Header [" << currentHeader->cBegin << "]:" << std::endl;
        
        // Traverse items in current header
        if (currentHeader->ppItems != nullptr) {
            int headerItemCount = 0;
            
            for (int i = 0; currentHeader->ppItems[i] != nullptr; i++) {
                ITEM4* item = static_cast<ITEM4*>(currentHeader->ppItems[i]);
                
                if (item != nullptr) {
                    ostr << "  Item " << (totalItems + 1) << ": ";
                    ostr << "ID=\"" << (item->pID ? item->pID : "NULL") << "\", ";
                    ostr << "Code=" << item->Code << ", ";
                    ostr << "Date=\"" << (item->pDate ? item->pDate : "NULL") << "\"";
                    ostr << std::endl;
                    
                    headerItemCount++;
                    totalItems++;
                }
            }
            
            if (headerItemCount == 0) {
                ostr << "  (no items)" << std::endl;
            }
        } else {
            ostr << "  (no items)" << std::endl;
        }
        
        ostr << std::endl;
    }
    
    ostr << "Total items: " << totalItems;
    
    return ostr;
}