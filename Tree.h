#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include "Record.h"
#include "MyFile.h"
#include "Page.h"

namespace page
{
	class Page;
}

class PageTable;
class Tree
{
private:
	//params
	unsigned const int dOrder = 2;
	const double wParam = 1.0;
	//merge
	TreeFile* treeFile;
	RootFile* rootFile;
	DataFile* dataFile;
	PageTable* pageTable;
	page::Page* rootPage;
	page::Page* thisPage;
	unsigned int thisH;
	rec::Tag* foundTag;
	unsigned int treeFileReads;
	unsigned int treeFileWrites;
	void init();
	bool searchTag(rec::Tag* tag, bool equal = true);
	bool addTag(rec::Tag* tag, rec::PageNumber* childPageNumber = nullptr);
	bool removeTag(rec::Tag*& removedTag, rec::Tag* tag = nullptr);
	bool leafPage(rec::PageNumber*& childPtr, bool& childBeforeisNull, rec::Tag* tag);
	bool compensation(rec::Tag* tag, page::Page* parentPage, rec::PageNumber* childPageNumber = nullptr);
	bool recompensation(page::Page* parentPage, rec::Tag*& parentTag);
	bool split(rec::Tag* tag, rec::Tag*& parentTag, page::Page*& newPage, page::Page*& parentPage, rec::PageNumber* childPageNumber = nullptr);
	bool merge(page::Page* parentPage, rec::Tag*& parentTag);
	void savePageToBuf(page::Page* savePage, unsigned int h);
	page::Page* getPageFromBuf(rec::PageNumber* nextPageNumber, unsigned int h);
	void printTree(page::Page* currentPage, unsigned int currentH, unsigned int& TreeH, bool full = false);
public:
	Tree();
	~Tree();
	bool search(rec::Record* newRecord, bool standalone = true);
	bool add(rec::Record* newRecord, bool print, bool standalone = true);
	bool remove(rec::Record* removedRecord, bool print, bool standalone = true);
	bool update(rec::Record* record, rec::Record* newRecord);
	void print(bool full = false);
	unsigned int getIOs()
	{
		std::cout << "Reads=" << this->treeFileReads << " Writes=" << this->treeFileWrites << std::endl;
		return this->treeFileReads + this->treeFileWrites;
	}
	void resetIOs() { this->treeFileReads = 0; this->treeFileWrites = 0; };
	friend class page::Page;
	friend class BTreePrinter;

};


class PageTable
{
private:
	page::Page** pointer;
	pnumint* numberPointer;
	unsigned int tableSize = 10;
	unsigned int resizeSize = 10;
	unsigned int nextPage;
public:
	PageTable()
	{
		pointer = new page::Page * [tableSize];
		numberPointer = new pnumint[tableSize];
		nextPage = 0;
	}
	void savePage(page::Page* newPage)
	{
		if (nextPage >= tableSize)
		{

			page::Page** newp = new page::Page * [(uint64_t)tableSize + resizeSize]; 
			pnumint* newnumberPointer = new pnumint[(uint64_t)tableSize + resizeSize];
			memcpy(newp, pointer, tableSize);
			memcpy(newnumberPointer, numberPointer, tableSize);
			memcpy(newnumberPointer, numberPointer, tableSize);
			tableSize += resizeSize;
			delete[] pointer;
			delete numberPointer;
			pointer = newp;
			numberPointer = newnumberPointer;
		}
		pointer[nextPage] = newPage;
		nextPage++;
	}

	page::Page* getPageByPosition(unsigned int n)
	{
		if (n < this->tableSize && n < nextPage)
		{
			return pointer[n];
		}
		else
		{
			return nullptr;
		}
	}
	page::Page* getPageByPageNumber(pnumint number)
	{
		for (unsigned int i = 0; i < nextPage; i++)
		{
			if (numberPointer[i] == number)
			{
				return pointer[i];
			}
		}
		return nullptr;
	}
	page::Page* getParentPage(unsigned myIndex)
	{
		if (myIndex > 0)
		{
			unsigned parentIndex = myIndex - 1;
			if (pointer[parentIndex] != nullptr)
			{
				return pointer[parentIndex];
			}
			return nullptr;
		}
		return nullptr;
	}

	page::Page* removeLast()
	{
		if (nextPage != 0 && pointer[nextPage - 1] != nullptr)
		{
			page::Page* retpage = pointer[nextPage - 1];
			pointer[nextPage - 1] = nullptr;
			nextPage--;
			return retpage;
		}
		return nullptr;
	}
	page::Page* getLastish(unsigned int n)
	{
		// n is offset from last (ex n=1 means last-1)
		// nextPage = last + 1 (index of the next one/ real size of table)
		if (nextPage == 0) return nullptr;
		if (nextPage - 1 >= n)
		{
			unsigned int index = nextPage - n - 1;
			return pointer[index];
		}
		return nullptr;
	}
	void removeFromN(unsigned int N)
	{
		unsigned int removed = 0;
		for (unsigned int i = N; i < nextPage; i++)
		{
			if (pointer[i] != nullptr)
			{
				if (pointer[i]->emptyPage() == false)
				{
					//save page:
					pointer[i]->savePage();
				}
				else
				{
					pointer[i]->saveEmptyPage();
				}
				numberPointer[i] = 0;
				delete pointer[i];
				pointer[i] = nullptr;
			}
			removed++;
		}
		nextPage -= removed;
	}
	void saveAll()
	{
		for (unsigned int i = 0; i < nextPage; i++)
		{
			if (pointer[i] != nullptr)
			{
				//save page:
				if (pointer[i]->emptyPage())
				{
					pointer[i]->saveEmptyPage();
				}
				else
				{
					pointer[i]->savePage();
				}
			}
		}
	}
	~PageTable()
	{
		removeFromN(0);
		delete[] pointer;
		delete numberPointer;
	}

};

class BTreePrinter
{
	struct NodeInfo
	{
		std::string text;
		unsigned text_pos, text_end;  // half-open range
	};

	typedef std::vector<NodeInfo> LevelInfo;

	std::vector<LevelInfo> levels;

	std::string node_text(int const keys[], unsigned key_count);

	void before_traversal()
	{
		levels.resize(0);
		levels.reserve(10);   // far beyond anything that could usefully be printed
	}

	//void visit(page::Page const* node, unsigned level = 0, unsigned child_index = 0);

	void after_traversal();

public:
	void print(Tree const& tree)
	{
		before_traversal();
		//visit(tree.rootPage);
		after_traversal();
	}
};
