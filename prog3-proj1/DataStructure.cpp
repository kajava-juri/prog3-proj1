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

// Destructor
DataStructure::~DataStructure() {
    // Clean up the linked list structure
    HEADER_E* current = pStruct;
    while (current != nullptr) {
        HEADER_E* next = current->pNext;

        // Clean up the items array if it exists
        if (current->ppItems != nullptr) {
            // Traverse the array and delete each ITEM4
            for (int i = 0; current->ppItems[i] != nullptr; i++) {
                ITEM4* item = static_cast<ITEM4*>(current->ppItems[i]);

                if (item != nullptr) {
                    // Free the dynamically allocated strings
                    if (item->pID != nullptr) {
                        delete[] item->pID;
                        item->pID = nullptr;
                    }

                    if (item->pDate != nullptr) {
                        delete[] item->pDate;
                        item->pDate = nullptr;
                    }

                    // Delete the item itself
                    delete item;
                    current->ppItems[i] = nullptr;
                }
            }

            // Delete the array of pointers
            delete[] current->ppItems;
            current->ppItems = nullptr;
        }

        // Delete the header
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
            newHeader->ppItems = new void*[itemCount+1];

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
                
                newItem->pNext = nullptr;
                if (i > 0) {
                    ITEM4* prevNewItem = static_cast<ITEM4*>(newHeader->ppItems[i - 1]);
					prevNewItem->pNext = newItem;
                }
                
                newHeader->ppItems[i] = newItem;
            }

			newHeader->ppItems[itemCount] = nullptr;
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
pointer_to_item DataStructure::GetItem(char* pID) {
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
                    pointer_to_item item = static_cast<pointer_to_item>(current->ppItems[i]);
                    if (item != nullptr && item->pID != nullptr) {
                        // Check if second character matches
                        char itemSecondChar = 0;
                        parseID(item->pID, itemSecondChar);
                        if (itemSecondChar == secondChar && strcmp(item->pID, pID) == 0) {
                            return item; // Return found item
                        }
                    }
                }
            }
        }
        current = current->pNext;
	}

    // If item not found, throw exception or return empty item
    return nullptr;
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

    // Parse the ID to get first character
    char firstChar = pID[0];
    char secondChar = 0;
    parseID(pID, secondChar);

    HEADER_E* current = pStruct;

    // Find the header matching the first character
    while (current != nullptr) {
        if (current->cBegin == firstChar) {
            // Search for the item in this header
            if (current->ppItems != nullptr) {
                // Count items and find the item to remove
                int itemCount = 0;
                int removeIndex = -1;

                for (int i = 0; current->ppItems[i] != nullptr; i++) {
                    ITEM4* item = static_cast<ITEM4*>(current->ppItems[i]);
                    if (item != nullptr && item->pID != nullptr) {
                        if (strcmp(item->pID, pID) == 0) {
                            removeIndex = i;
                        }
                    }
                    itemCount++;
                }

                // If item was found, remove it
                if (removeIndex != -1) {
                    ITEM4* itemToRemove = static_cast<ITEM4*>(current->ppItems[removeIndex]);

                    // Update pNext pointers in the linked list
                    if (removeIndex > 0) {
                        ITEM4* prevItem = static_cast<ITEM4*>(current->ppItems[removeIndex - 1]);
                        if (removeIndex < itemCount - 1) {
                            prevItem->pNext = static_cast<ITEM4*>(current->ppItems[removeIndex + 1]);
                        }
                        else {
                            prevItem->pNext = nullptr;
                        }
                    }

                    // Free memory for the item
                    if (itemToRemove->pID != nullptr) {
                        delete[] itemToRemove->pID;
                    }
                    if (itemToRemove->pDate != nullptr) {
                        delete[] itemToRemove->pDate;
                    }
                    delete itemToRemove;

                    // Create new array without the removed item
                    if (itemCount == 1) {
                        // This was the only item, just delete the array
                        delete[] current->ppItems;
                        current->ppItems = nullptr;
                    }
                    else {
                        // Create smaller array
                        void** newArray = new void* [itemCount]; // itemCount includes nullptr terminator

                        // Copy items before the removed item
                        for (int i = 0; i < removeIndex; i++) {
                            newArray[i] = current->ppItems[i];
                        }

                        // Copy items after the removed item
                        for (int i = removeIndex + 1; i < itemCount; i++) {
                            newArray[i - 1] = current->ppItems[i];
                        }

                        // Null terminator
                        newArray[itemCount - 1] = nullptr;

                        // Delete old array and assign new one
                        delete[] current->ppItems;
                        current->ppItems = newArray;
                    }

                    return; // Successfully removed
                }
            }
        }
        current = current->pNext;
    }

    // If item not found, throw exception
    throw std::runtime_error("Item not found for removal");
}

DataStructure& DataStructure::operator=(const DataStructure& Right) {
    // Check for self-assignment
    if (this == &Right) {
        return *this;
    }

    // Destroy existing contents (same as destructor logic)
    HEADER_E* current = pStruct;
    while (current != nullptr) {
        HEADER_E* next = current->pNext;

        // Clean up the items array if it exists
        if (current->ppItems != nullptr) {
            // Traverse the array and delete each ITEM4
            for (int i = 0; current->ppItems[i] != nullptr; i++) {
                ITEM4* item = static_cast<ITEM4*>(current->ppItems[i]);

                if (item != nullptr) {
                    // Free the dynamically allocated strings
                    if (item->pID != nullptr) {
                        delete[] item->pID;
                        item->pID = nullptr;
                    }

                    if (item->pDate != nullptr) {
                        delete[] item->pDate;
                        item->pDate = nullptr;
                    }

                    // Delete the item itself
                    delete item;
                    current->ppItems[i] = nullptr;
                }
            }

            // Delete the array of pointers
            delete[] current->ppItems;
            current->ppItems = nullptr;
        }

        // Delete the header
        delete current;
        current = next;
    }
    pStruct = nullptr;

    // Copy from Right (same as copy constructor logic)
    if (Right.pStruct == nullptr) {
        return *this;
    }

    HEADER_E* originalCurrent = Right.pStruct;
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
            newHeader->ppItems = new void* [itemCount + 1];

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
                    strcpy_s(newItem->pID, idLen + 1, originalItem->pID);
                }
                else {
                    newItem->pID = nullptr;
                }

                // Deep copy pDate string
                if (originalItem->pDate != nullptr) {
                    int dateLen = strlen(originalItem->pDate);
                    newItem->pDate = new char[dateLen + 1];
                    strcpy_s(newItem->pDate, dateLen + 1, originalItem->pDate);
                }
                else {
                    newItem->pDate = nullptr;
                }

                // Set pNext to nullptr (will be linked in next iteration)
                newItem->pNext = nullptr;

                // Link to previous item if exists
                if (i > 0) {
                    ITEM4* prevItem = static_cast<ITEM4*>(newHeader->ppItems[i - 1]);
                    prevItem->pNext = newItem;
                }

                newHeader->ppItems[i] = newItem;
            }

            // Null terminator
            newHeader->ppItems[itemCount] = nullptr;
        }
        else {
            newHeader->ppItems = nullptr;
        }

        // 3. Link into new list
        if (newPrevious != nullptr) {
            newPrevious->pNext = newHeader;
        }
        else {
            pStruct = newHeader;
        }

        newPrevious = newHeader;
        originalCurrent = originalCurrent->pNext;
    }

    return *this;
}

bool DataStructure::operator==(DataStructure &Other) {
    // First check: compare number of items
    int thisCount = this->GetItemsNumber();
    int otherCount = Other.GetItemsNumber();

    if (thisCount != otherCount) {
        return false;
    }

    // If both are empty, they're equal
    if (thisCount == 0) {
        return true;
    }

    // Second check: verify each item in this structure exists in Other
    // Traverse all headers in this structure
    for (HEADER_E* currentHeader = this->pStruct; currentHeader != nullptr; currentHeader = currentHeader->pNext) {
        if (currentHeader->ppItems != nullptr) {
            // Traverse items in current header
            for (int i = 0; currentHeader->ppItems[i] != nullptr; i++) {
                ITEM4* thisItem = static_cast<ITEM4*>(currentHeader->ppItems[i]);

                if (thisItem != nullptr && thisItem->pID != nullptr) {
                    // Try to find this item in Other structure
                    bool found = false;

                    // Search through all headers in Other
                    for (HEADER_E* otherHeader = Other.pStruct; otherHeader != nullptr; otherHeader = otherHeader->pNext) {
                        if (otherHeader->ppItems != nullptr) {
                            // Search items in this header
                            for (int j = 0; otherHeader->ppItems[j] != nullptr; j++) {
                                ITEM4* otherItem = static_cast<ITEM4*>(otherHeader->ppItems[j]);

                                if (otherItem != nullptr && otherItem->pID != nullptr) {
                                    // Compare IDs
                                    if (strcmp(thisItem->pID, otherItem->pID) == 0) {
                                        found = true;
                                        break;
                                    }
                                }
                            }

                            if (found) {
                                break;
                            }
                        }
                    }

                    // If any item from this structure is not found in Other, they're not equal
                    if (!found) {
                        return false;
                    }
                }
            }
        }
    }

    // All items matched
    return true;
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
		int totalItems = 0;
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
			totalItems += itemCount;
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

DataStructure::DataStructure(std::string Filename) throw(std::exception) : pStruct(nullptr) {
    std::ifstream file(Filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + Filename);
    }

    if (!file.good()) {
        file.close();
		throw std::runtime_error("File is not in good state: " + Filename);
    }

    try {
		HEADER_E* prevHeader = nullptr;
        // Header count
        int headerCount = 0;
		file.read((char*)&headerCount, sizeof(int));

        for (int h = 0; h < headerCount; h++) {
			char cBegin;
			file.read(&cBegin, sizeof(char));

			int itemCount = 0;
			file.read((char*)&itemCount, sizeof(int));

			ITEM4** itemsArray = new ITEM4 * [itemCount + 1]; // +1 for nullptr terminator
			
            for (int i = 0; i < itemCount; i++) {
				ITEM4* item = new ITEM4;
				file.read((char*)&item->Code, sizeof(unsigned long int));

				// for reading strings, read length -> allocate -> read content -> null terminate
				int idLength = 0;
				file.read((char*)&idLength, sizeof(int));
				item->pID = new char[idLength + 1];
                file.read((char*)item->pID, idLength);
				item->pID[idLength] = '\0';

				int dateLength = 0;
                file.read((char*)&dateLength, sizeof(int));
				item->pDate = new char[dateLength + 1];
				file.read((char*)item->pDate, dateLength);
				item->pDate[dateLength] = '\0';
				item->pNext = nullptr;
				itemsArray[i] = item;

                if (i > 0) {
                    itemsArray[i - 1]->pNext = item;
                }
            }
			itemsArray[itemCount] = nullptr; // Null terminator


			HEADER_E *lastHeader = new HEADER_E;
			lastHeader->cBegin = cBegin;
			lastHeader->ppItems = (void**)itemsArray;
			lastHeader->pNext = nullptr;
			lastHeader->pPrior = nullptr;

            if (h > 0) {
                lastHeader->pPrior = prevHeader;
                prevHeader->pNext = lastHeader;
            } else {
                lastHeader->pPrior = nullptr;
			    pStruct = lastHeader;
			}

            prevHeader = lastHeader;

        }

		file.close();
    }
    catch (const std::exception& e) {
        file.close();
        throw std::runtime_error("Error reading from file: " + std::string(e.what()));
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
        
        // Traverse items in current header
        if (currentHeader->ppItems != nullptr) {
            int headerItemCount = 0;
            
            for (int i = 0; currentHeader->ppItems[i] != nullptr; i++) {
                ITEM4* item = static_cast<ITEM4*>(currentHeader->ppItems[i]);
                
                if (item != nullptr) {
                    ostr << totalItems << ")";
                    ostr << (item->pID ? item->pID : "NULL") << " ";
                    ostr << item->Code << " ";
                    ostr << (item->pDate ? item->pDate : "NULL") << " ";
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
        
        //ostr << std::endl;
    }

    ostr << std::endl;
    ostr << "Total items: " << totalItems;
    
    return ostr;
}