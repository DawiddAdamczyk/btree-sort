#include "Page.h"
#include "Tree.h"
#include "PageElements.h"

using namespace page;
//================================================================================
//================================================================================
//===================================PAGE=========================================
//================================================================================
//================================================================================
//Page Public Named Constructors  ---------------------------------------------------------------
Page* Page::createNewPage()
{
	Page* newPage = new Page();
	newPage->elem = page::PageElements::createNew();
	if (newPage->elem == nullptr) throw 1;
	newPage->pageNumber = new rec::PageNumber(rec::PageNumber::getNextNumber());
	return newPage;
}
Page* Page::createPageFromFile(pageptr argpagetab)
{
	Page* newPage = new Page();
	newPage->elem = page::PageElements::createFromFile(argpagetab);
	if (newPage->elem == nullptr) throw 1;
	newPage->pageNumber = new rec::PageNumber(argpagetab[0]);
	return newPage;
}
//Page private Constructors ----------------------------------------------
Page::Page()
{
	//this constructor is private
	//never used outside specified "named constructors" static functions
}
//Static Parameters initializer ---------------------------------------
void Page::initializeTree(Tree* myTree)
{
	Page::myTree = myTree;
	Page::MAXIMAL_PAGE_SIZE = Page::HEADER_SIZE + (6 * (uint64_t)myTree->dOrder) + 1;
	PageElements::initializeTable(myTree->dOrder);
}
//Deconstructor-------------------------------------------------------
Page::~Page()
{
	if (this->pageNumber != nullptr)
	{
		delete this->pageNumber;
		this->pageNumber = nullptr;
	}
	if (this->elem != nullptr)
	{
		delete this->elem;
		this->elem = nullptr;
	}
}

// Static Getters: ----------------------------------------------------

unsigned int Page::getMaxPageSize()
{
	return Page::MAXIMAL_PAGE_SIZE;
}
unsigned int Page::getHeaderSize()
{
	return Page::HEADER_SIZE;
}
Tree* Page::getMyTree()
{
	//return Page::myTree;
	return nullptr;
}

// Getters ----------------------------------------------------

rec::PageNumber* Page::getThisPageNumber()
{
	return this->pageNumber;
}

unsigned int Page::getPageRealSize()
{
	return this->elem->getPageRealSize();
}

rec::PageNumber* Page::getPtrBefore(rec::Tag* tag)
{
	unsigned int tagIndex = this->elem->getTagIndex(tag);
	unsigned int pointerTag = this->elem->calcpBt(tagIndex);
	rec::PageNumber* pointer = this->elem->getPointer(pointerTag);
	if (pointer == nullptr) return nullptr;
	return pointer;
}

rec::PageNumber* Page::getPtrAfter(rec::Tag* tag)
{
	unsigned int tagIndex = this->elem->getTagIndex(tag);
	unsigned int pointerTag = this->elem->calcpAt(tagIndex);
	rec::PageNumber* pointer = this->elem->getPointer(pointerTag);
	if (pointer == nullptr) return nullptr;
	return pointer;
}


// Public Methods -------------------------------
bool Page::savePage()
{
	if (this->pageNumber != nullptr && this->pageNumber->goodPage())
	{
		pageptr pagetab = this->elem->saveToFile(this->pageNumber);
		if (pagetab == nullptr)  return false;
		this->myTree->treeFile->savePage(pagetab, this->pageNumber->calculatePageOffset());
		return true;
	}
	return false;
}

void Page::saveEmptyPage()
{
	if (this->pageNumber != nullptr && this->pageNumber->goodPage())
	{
		pageptr pagetab = nullptr;

		if (this->emptyPage())
		{
			pagetab = this->elem->saveZerosToFile();
			delete this->elem;
			this->elem = nullptr;
		}
		else
		{
			throw 1;
		}
		if (pagetab == nullptr) throw 1;
		this->myTree->treeFile->savePage(pagetab, this->pageNumber->calculatePageOffset());
		if (pagetab != nullptr)
		{
			delete pagetab;
			pagetab = nullptr;
		}
		this->pageNumber->removePageNumber();
		delete this->pageNumber;
	}
}

bool Page::addTag(rec::Tag* newTag, rec::PageNumber* childPageNumber)
{
	unsigned int  tagindex = this->elem->getTagIndex(newTag);
	if (this->elem->getTag(tagindex) != nullptr && rec::Tag::equals(newTag->getTag(), this->elem->getTag(tagindex)->getTag()))
	{
		//duplicate
		return false;
	}
	if (this->elem->insertTag(tagindex, newTag))
	{
		unsigned int  pointerindex = this->elem->calcpAt(tagindex);
		bool resultP;
		if (childPageNumber != nullptr)
		{
			resultP = this->elem->insertPointer(pointerindex, childPageNumber);
		}
		else
		{
			resultP = this->elem->insertPointer(pointerindex, new rec::PageNumber(rec::PageNumber::NULL_NUMBER));
		}
		if (resultP)
		{
			return true;
		}
		else
		{
			throw 0;
		}
	}
	else
	{
		return false;
	}
}
void* Page::serchPageForTag(rec::Tag* recordTag, bool& founded)
{
	rec::Tag* foundedRecord = nullptr;
	rec::PageNumber* nextPageNumber = nullptr;
	founded = false;
	unsigned int  tagindex = this->elem->getTagIndex(recordTag);
	foundedRecord = this->elem->getTag(tagindex);
	if (foundedRecord != nullptr && rec::Tag::equals(recordTag->getTag(), foundedRecord->getTag()))
	{
		//founded Tag
		founded = true;
		return (void*)foundedRecord;
	}
	nextPageNumber = this->elem->getPointerBeforeTag(tagindex);
	if (nextPageNumber != nullptr)
	{
		return (void*)nextPageNumber;
	}
	return nullptr;
}
void* Page::serchPageForAlmostTag(rec::Tag* recordTag, bool& founded)
{
	rec::Tag* foundedRecord = nullptr;
	rec::PageNumber* nextPageNumber = nullptr;
	founded = false;
	unsigned int  tagindex = this->elem->getAlmostTagIndex(recordTag);
	foundedRecord = this->elem->getTag(tagindex);
	if (foundedRecord == nullptr) throw 1;
	if (rec::Tag::equals(recordTag->getTag(), foundedRecord->getTag()))
	{
		//founded Tag
		founded = true;
		return (void*)foundedRecord;
	}
	if (rec::Tag::compare(foundedRecord->getTag(), recordTag->getTag()))
	{
		//founded tag smaller than tag
		nextPageNumber = this->elem->getPointerAfrerTag(tagindex);
		if (nextPageNumber != nullptr && nextPageNumber->goodPage())
		{
			return (void*)nextPageNumber;
		}
	}
	else
	{
		//founded tag greater than tag
		nextPageNumber = this->elem->getPointerBeforeTag(tagindex);
		if (nextPageNumber != nullptr && nextPageNumber->goodPage())
		{
			return (void*)nextPageNumber;
		}
	}

	//"founded" Tag
	founded = true;
	return (void*)foundedRecord;
}
bool Page::upadeCopiedTag(rec::Tag* oldtag, rec::Tag* newtag, bool bisection)
{
	//Remove tag wich was copied elsewere
	//And add on it's place new one
	unsigned int  tagindex;
	if (bisection)
	{
		tagindex = this->elem->getTagIndex(oldtag);
	}
	else
	{
		tagindex = this->elem->getTagIndexLinearly(oldtag);
	}

	rec::Tag* retTag = this->elem->removeTag(tagindex);
	if (retTag == nullptr) return false;
	bool ret = this->elem->insertTag(tagindex, newtag);
	if (ret == false) throw 1;
	return true;
}
bool Page::compensate(rec::Tag* tag, rec::Tag*& parentTag, page::Page* siblingPage, bool isLeft, rec::PageNumber* childPageNumber)
{
	if (siblingPage == nullptr) return false;
	if (tag == nullptr) return false;
	if (parentTag == nullptr) return false;

	//check if sibling page is full
	if (siblingPage->elem->getPageRealSize() >= siblingPage->MAXIMAL_PAGE_SIZE) return false;

	//add tag:
	unsigned int tagindex = this->elem->getTagIndex(tag);
	bool insertResult = this->elem->insertTag(tagindex, tag, false);
	if (insertResult == false) return false;
	unsigned int pointerindex = this->elem->calcpAt(tagindex);
	bool pinsertResult;
	if (childPageNumber != nullptr)
	{
		pinsertResult = this->elem->insertPointer(pointerindex, childPageNumber, false);

	}
	else
	{
		pinsertResult = this->elem->insertPointer(pointerindex, new rec::PageNumber(rec::PageNumber::NULL_NUMBER), false);
	}
	if (pinsertResult == false) throw 1;

	//add parentTag
	unsigned int parentTagindex = siblingPage->elem->getTagIndex(parentTag);
	bool parentInsertResult = siblingPage->elem->insertTag(parentTagindex, parentTag, true);
	if (parentInsertResult == false)  throw 1;
	//unsigned int parentPointerindex = this->elem->calcpBt(parentTagindex);
	//bool parentPinsertResult = siblingPage->elem->insertPointer(parentPointerindex, new rec::PageNumber(rec::PageNumber::NULL_NUMBER), false);
	//if (parentPinsertResult == false) throw 1;

	//calculate Tag:
	unsigned int tagSum = this->elem->getTagTabSize() + siblingPage->elem->getTagTabSize();
	unsigned int myNewTagTableSize = tagSum / 2;
	unsigned int sibNewTagTableSize = tagSum / 2 + (tagSum % 2);
	unsigned int myNewRecDif = this->elem->getTagTabSize() - myNewTagTableSize;
	unsigned int sibNewRecDif = sibNewTagTableSize - siblingPage->elem->getTagTabSize();
	if (myNewRecDif != sibNewRecDif) throw 1;
	unsigned int amountTag = myNewRecDif;

	//calculate Pointer
	unsigned int pointerSum = this->elem->getPointerTabSize() + siblingPage->elem->getPointerTabSize();
	unsigned int myNewPointerTableSize = pointerSum / 2 + (pointerSum % 2);
	unsigned int sibNewPointerTableSize = pointerSum / 2;
	unsigned int myNewRecDifP = this->elem->getPointerTabSize() - myNewPointerTableSize;
	unsigned int sibNewRecDifP = sibNewPointerTableSize - siblingPage->elem->getPointerTabSize();
	if (myNewRecDifP != sibNewRecDifP) throw 1;
	unsigned int amountPointer = myNewRecDifP;

	if (isLeft)
	{
		//Left compensation
		rec::Tag** tagVan = this->elem->bulkRemoveTagLeft(amountTag);
		if (tagVan == nullptr) throw 1;
		bool bulkInsertResult = siblingPage->elem->bulkInsertTagLeft(tagVan, amountTag);
		if (bulkInsertResult == false) throw 1;
		parentTag = siblingPage->elem->removeTag(siblingPage->elem->getTagTabSize() - 1, false);
		if (parentTag == nullptr) throw 1;
		rec::PageNumber** pointerVan = this->elem->bulkRemovePointerLeft(amountPointer);
		if (pointerVan == nullptr) throw 1;
		bool bulkInsertResultP = siblingPage->elem->bulkInsertPointerLeft(pointerVan, amountPointer);
		if (bulkInsertResultP == false) throw 1;
	}
	else
	{
		//Right compensation
		rec::Tag** tagVan = this->elem->bulkRemoveTagRight(amountTag);
		if (tagVan == nullptr) throw 1;
		bool bulkInsertResult = siblingPage->elem->bulkInsertTagRight(tagVan, amountTag);
		if (bulkInsertResult == false) throw 1;
		parentTag = siblingPage->elem->removeTag(0, false);
		if (parentTag == nullptr) throw 1;
		rec::PageNumber** pointerVan = this->elem->bulkRemovePointerRight(amountPointer);
		if (pointerVan == nullptr) throw 1;
		bool bulkInsertResultP = siblingPage->elem->bulkInsertPointerRight(pointerVan, amountPointer);
		if (bulkInsertResultP == false) throw 1;
	}
	return true;
}

bool Page::recompensate(rec::Tag*& parentTag, page::Page* siblingPage, bool isLeft)
{
	if (siblingPage == nullptr) return false;
	if (parentTag == nullptr) return false;

	//check if my page is halfempty
	if (this->pageSizeLessThenHalf() == false) return false;
	//check if sibling page is halffull
	if (siblingPage->pageSizeMoreThenHalf() == false) return false;
	//check if sum of pages is good enouth
	if (this->pageSumSizeMaxnMore(siblingPage) == false) return false;

	//add parentTag
	unsigned int parentTagindex = this->elem->getTagIndex(parentTag);
	bool parentInsertResult = this->elem->insertTag(parentTagindex, parentTag, true);
	if (parentInsertResult == false)  throw 1;

	//calculate Tag:
	unsigned int tagSum = this->elem->getTagTabSize() + siblingPage->elem->getTagTabSize();
	unsigned int myNewTagTableSize = tagSum / 2 + (tagSum % 2);
	unsigned int sibNewTagTableSize = tagSum / 2;
	unsigned int myNewRecDif = myNewTagTableSize - this->elem->getTagTabSize();
	unsigned int sibNewRecDif = siblingPage->elem->getTagTabSize() - sibNewTagTableSize;
	if (myNewRecDif != sibNewRecDif) throw 1;
	unsigned int amountTag = myNewRecDif;

	//calculate Pointer
	unsigned int pointerSum = this->elem->getPointerTabSize() + siblingPage->elem->getPointerTabSize();
	unsigned int myNewPointerTableSize = pointerSum / 2;
	unsigned int sibNewPointerTableSize = pointerSum / 2 + (pointerSum % 2);
	unsigned int myNewRecDifP = myNewPointerTableSize - this->elem->getPointerTabSize();
	unsigned int sibNewRecDifP = siblingPage->elem->getPointerTabSize() - sibNewPointerTableSize;
	if (myNewRecDifP != sibNewRecDifP) throw 1;
	unsigned int amountPointer = myNewRecDifP;

	if (isLeft == false)
	{
		//Left compensation
		if (amountTag != 0)
		{
			rec::Tag** tagVan = siblingPage->elem->bulkRemoveTagLeft(amountTag);
			if (tagVan == nullptr) throw 1;
			bool bulkInsertResult = this->elem->bulkInsertTagLeft(tagVan, amountTag);
			if (bulkInsertResult == false) throw 1;
		}
		parentTag = this->elem->removeTag(this->elem->getTagTabSize() - 1, false);
		if (parentTag == nullptr) throw 1;
		if (amountPointer != 0)
		{

			rec::PageNumber** pointerVan = siblingPage->elem->bulkRemovePointerLeft(amountPointer);
			if (pointerVan == nullptr) throw 1;
			bool bulkInsertResultP = this->elem->bulkInsertPointerLeft(pointerVan, amountPointer);
			if (bulkInsertResultP == false) throw 1;
		}
	}
	else
	{
		//Right compensation
		if (amountTag != 0)
		{
			rec::Tag** tagVan = siblingPage->elem->bulkRemoveTagRight(amountTag);
			if (tagVan == nullptr) throw 1;
			bool bulkInsertResult = this->elem->bulkInsertTagRight(tagVan, amountTag);
			if (bulkInsertResult == false) throw 1;
		}
		parentTag = this->elem->removeTag(0, false);
		if (parentTag == nullptr) throw 1;
		if (amountPointer != 0)
		{
			rec::PageNumber** pointerVan = siblingPage->elem->bulkRemovePointerRight(amountPointer);
			if (pointerVan == nullptr) throw 1;
			bool bulkInsertResultP = this->elem->bulkInsertPointerRight(pointerVan, amountPointer);
			if (bulkInsertResultP == false) throw 1;
		}
	}
	return true;
}


bool Page::split(rec::Tag* tag, rec::Tag*& newParentTag, page::Page* newPage, rec::PageNumber* childPageNumber)
{
	if (tag == nullptr) return false;
	if (newPage == nullptr) return false;
	//add tag:
	unsigned int tagindex = this->elem->getTagIndex(tag);
	bool insertResult = this->elem->insertTag(tagindex, tag, false);
	if (insertResult == false) return false;
	unsigned int pointerindex = this->elem->calcpAt(tagindex);
	bool pinsertResult;
	if (childPageNumber != nullptr)
	{
		pinsertResult = this->elem->insertPointer(pointerindex, childPageNumber, false);
	}
	else
	{
		pinsertResult = this->elem->insertPointer(pointerindex, new rec::PageNumber(rec::PageNumber::NULL_NUMBER), false);
	}
	if (pinsertResult == false) throw 1;


	//calculate Tag:
	unsigned int myNewTagTableSize = this->elem->getTagTabSize() / 2 + (this->elem->getTagTabSize() % 2);
	unsigned int amountTag = this->elem->getTagTabSize() - myNewTagTableSize;

	//calculate Pointer
	unsigned int myNewPointerTableSize = this->elem->getPointerTabSize() / 2 + (this->elem->getPointerTabSize() % 2);
	unsigned int amountPointer = this->elem->getPointerTabSize() - myNewPointerTableSize;

	//Delete initial pointer
	rec::PageNumber* initPtr = newPage->elem->removePointer(0);
	if (initPtr != nullptr)
	{
		delete initPtr;
		initPtr = nullptr;
	}

	//Split:
	//Tag
	rec::Tag** tagVan = this->elem->bulkRemoveTagRight(amountTag);
	if (tagVan == nullptr) throw 1;
	bool insertSplitRes = newPage->elem->bulkInsertTagRight(tagVan, amountTag);
	if (insertSplitRes == false) throw 1;
	//Pointer
	rec::PageNumber** pointerVan = this->elem->bulkRemovePointerRight(amountPointer);
	if (pointerVan == nullptr) throw 1;
	bool bulkInsertResultP = newPage->elem->bulkInsertPointerRight(pointerVan, amountPointer);
	if (bulkInsertResultP == false) throw 1;

	//Get New Parent Tag
	newParentTag = this->elem->removeTag(this->elem->getTagTabSize() - 1, false);
	if (newParentTag == nullptr) throw 1;

	return true;


}

bool Page::merge(rec::Tag* parentTag, page::Page* siblingPage, bool isLeft)
{
	if (siblingPage == nullptr) return false; //can't merge with null
	if (parentTag == nullptr) return false; //can't merge sibling if parent is null
	if (this->pageSumSizeMaxnLess(siblingPage) == false) return false; // size of two pages is too large

	//add tag:
	unsigned int tagindex = this->elem->getTagIndex(parentTag);
	bool insertResult = this->elem->insertTag(tagindex, parentTag, false);
	if (insertResult == false) return false;


	//calculate Tag:
	unsigned int myNewTagTableSize = this->elem->getTagTabSize() + siblingPage->elem->getTagTabSize();


	//calculate Pointer
	unsigned int myNewPointerTableSize = this->elem->getPointerTabSize() + siblingPage->elem->getTagTabSize();


	//Merge:
	if (isLeft)
	{
		//Tag
		unsigned int sibTagTabSize = siblingPage->elem->getTagTabSize();
		rec::Tag** tagVan = siblingPage->elem->bulkRemoveTagRight(sibTagTabSize);
		if (tagVan == nullptr) throw 1;
		bool insertSplitRes = this->elem->bulkInsertTagRight(tagVan, sibTagTabSize);
		if (insertSplitRes == false) throw 1;
		//Pointer
		unsigned int sibPointerTabSize = siblingPage->elem->getPointerTabSize();
		rec::PageNumber** pointerVan = siblingPage->elem->bulkRemovePointerRight(sibPointerTabSize);
		if (pointerVan == nullptr) throw 1;
		bool bulkInsertResultP = this->elem->bulkInsertPointerRight(pointerVan, sibPointerTabSize);
		if (bulkInsertResultP == false) throw 1;
	}
	else
	{
		//Tag
		unsigned int sibTagTabSize = siblingPage->elem->getTagTabSize();
		rec::Tag** tagVan = siblingPage->elem->bulkRemoveTagLeft(sibTagTabSize);
		if (tagVan == nullptr) throw 1;
		bool insertSplitRes = this->elem->bulkInsertTagLeft(tagVan, sibTagTabSize);
		if (insertSplitRes == false) throw 1;
		//Pointer
		unsigned int sibPointerTabSize = siblingPage->elem->getPointerTabSize();
		rec::PageNumber** pointerVan = siblingPage->elem->bulkRemovePointerLeft(sibPointerTabSize);
		if (pointerVan == nullptr) throw 1;
		bool bulkInsertResultP = this->elem->bulkInsertPointerLeft(pointerVan, sibPointerTabSize);
		if (bulkInsertResultP == false) throw 1;

	}


	return true;
}

rec::PageNumber* Page::getLeftSibling(rec::PageNumber* centerSibling)
{
	bool searchCentIndResult;
	unsigned int centerIndex = this->elem->getPointerIndex(centerSibling, searchCentIndResult);
	if (searchCentIndResult == false) return nullptr;
	if (centerIndex == 0) return nullptr;
	unsigned int leftIndex = centerIndex - 1;
	return this->elem->getPointer(leftIndex);
}

rec::PageNumber* Page::getRightSibling(rec::PageNumber* centerSibling)
{
	bool searchCentIndResult;
	unsigned int centerIndex = this->elem->getPointerIndex(centerSibling, searchCentIndResult);
	if (searchCentIndResult == false) return nullptr;
	if (centerIndex >= this->elem->getPointerTabSize()) return nullptr;
	unsigned int rightIndex = centerIndex + 1;
	return this->elem->getPointer(rightIndex);
}

rec::Tag* Page::getLeftParent(rec::PageNumber* centerSibling)
{
	bool searchCentIndResult;
	unsigned int centerIndex = this->elem->getPointerIndex(centerSibling, searchCentIndResult);
	if (searchCentIndResult == false) return nullptr;
	if (centerIndex == 0) return nullptr;
	unsigned int leftParentIndex = this->elem->calctBp(centerIndex);
	return this->elem->getTag(leftParentIndex);

}

rec::Tag* Page::getRightParent(rec::PageNumber* centerSibling)
{
	bool searchCentIndResult;
	unsigned int centerIndex = this->elem->getPointerIndex(centerSibling, searchCentIndResult);
	if (searchCentIndResult == false) return nullptr;
	if (centerIndex > this->elem->getPointerTabSize()) return nullptr;
	unsigned int rightParentIndex = this->elem->calctAp(centerIndex);
	return this->elem->getTag(rightParentIndex);
}

bool Page::setptrBefore(rec::Tag* tag, rec::PageNumber* pageNumberToInsert)
{
	unsigned int tagIndex = this->elem->getTagIndex(tag);
	unsigned int pointerTag = this->elem->calcpBt(tagIndex);
	rec::PageNumber* pointer = this->elem->removePointer(pointerTag);
	if (pointer == nullptr) return false;
	if (pointer->goodPage() == false)
	{
		//no data lost
		delete pointer;
		pointer = nullptr;
	}
	else
	{
		delete pointer;
		pointer = nullptr;
	}
	this->elem->insertPointer(pointerTag, pageNumberToInsert);
	return true;
}

bool Page::setptrAfter(rec::Tag* tag, rec::PageNumber* pageNumberToInsert)
{
	unsigned int tagIndex = this->elem->getTagIndex(tag);
	unsigned int pointerTag = this->elem->calcpAt(tagIndex);
	rec::PageNumber* pointer = this->elem->removePointer(pointerTag);
	if (pointer == nullptr) return false;
	if (pointer->goodPage() == false)
	{
		//no data lost
		delete pointer;
		pointer = nullptr;
	}
	else
	{
		delete pointer;
		pointer = nullptr;
	}
	this->elem->insertPointer(pointerTag, pageNumberToInsert);
	return true;
}


void Page::printPage(unsigned int currenth, unsigned int& TreeH, bool full)
{
	//Print:
	std::cout << "Page " << this->pageNumber->getPageNumber() << " ";
	std::cout << "Size " << this->elem->getPageRealSize() << " ";
	for (unsigned int i = 0; i < this->elem->getTagTabSize() && i < this->elem->getPointerTabSize(); i++)
	{
		rec::PageNumber* ptr = this->elem->getPointer(i);
		std::cout << "*(" << ptr->getPageNumber() << ") ";
		rec::Tag* tag = this->elem->getTag(i);
		recordmean recordInt = this->myTree->dataFile->getRecord(tag->calculateAddressOffset());
		if (full)
		{
			std::cout << "Key=" << tag->getTag() << " ";
			std::cout << "Address=" << tag->getAddress() << " ";
			std::cout << "Record=" << recordInt << " ";
		}
		else
		{
			std::cout << "k=" << tag->getTag() << " ";
			std::cout << "R=" << recordInt << " ";
		}


	}
	rec::PageNumber* ptr = this->elem->getPointer(this->elem->getPointerTabSize() - 1);
	std::cout << "*(" << ptr->getPageNumber() << ") ";
	std::cout << std::endl;

	//Get Next Page:
	for (unsigned int i = 0; i < this->elem->getPointerTabSize(); i++)
	{
		rec::PageNumber* ptr = this->elem->getPointer(i);
		if (ptr->goodPage())
		{
			page::Page* npage = this->myTree->pageTable->getPageByPageNumber(ptr->getPageNumber());
			if (npage == nullptr)
			{
				npage = page::Page::createPageFromFile(this->myTree->treeFile->getPage(ptr->calculatePageOffset()));
				this->myTree->printTree(npage, currenth + 1, TreeH);
				delete npage;
				npage = nullptr;
			}
			else
			{
				this->myTree->printTree(npage, currenth + 1, TreeH);
			}
		}
	}
}


bool Page::leafPage()
{
	return this->elem->hasOnlyNullPtrs();
}



bool Page::removeTag(rec::Tag* tag, rec::Tag*& removedTag, bool isLeftPtrNull, bool isRoot)
{
	//Arg Error
	if (tag == nullptr) throw 1;
	if (removedTag != nullptr) throw 1;

	unsigned int  tagindex = this->elem->getTagIndex(tag);

	//Errors:
	if (this->elem->getTag(tagindex) == nullptr) throw 1;
	if (rec::Tag::equals(tag->getTag(), this->elem->getTag(tagindex)->getTag()) == false) throw 1; //tags dont match;

	//Remove tag:
	removedTag = this->elem->removeTag(tagindex);

	if (removedTag == nullptr) throw 1;
	if (rec::Tag::equals(tag->getTag(), removedTag->getTag()) == false) throw 1; //tags dont match;

	//Remove Ptr
	unsigned int  pointerindex;
	if (isLeftPtrNull)
	{
		pointerindex = this->elem->calcpBt(tagindex);
	}
	else
	{
		pointerindex = this->elem->calcpAt(tagindex);
	}
	rec::PageNumber* ptr = this->elem->removePointer(pointerindex);

	if (ptr == nullptr || ptr->goodPage()) throw 1;
	delete ptr;
	ptr = nullptr;


	//If Page is good enough:
	if (this->pageSizeHalfnMore() == false && isRoot == false)
	{
		//page has not enough elements to remove
		return false;
	}
	return true;
}

bool  Page::emptyPage()
{
	if (this->elem->getTagTabSize() == 0 && this->elem->getPointerTabSize() == 0) return true;
	else if (this->elem->getTagTabSize() == 0 && this->elem->getPointerTabSize() == 1 && this->elem->getPointer(0)->goodPage() == false)  return true;
	return false;
}

bool  Page::pageSizeHalfnMore()
{
	if (this->elem->getTagTabSize() >= (unsigned int)((double)this->elem->getTagTabMaxSize() * 0.5 * this->myTree->wParam))
	{
		return true;
	}
	return false;
}
bool Page::pageSizeMoreThenHalf()
{
	if (this->elem->getTagTabSize() > (unsigned int)((double)this->elem->getTagTabMaxSize() * 0.5 * this->myTree->wParam))
	{
		return true;
	}
	return false;
}

bool Page::pageSizeLessThenHalf()
{
	if (this->elem->getTagTabSize() < (unsigned int)((double)this->elem->getTagTabMaxSize() * 0.5 * this->myTree->wParam))
	{
		return true;
	}
	return false;
}

bool Page::pageSumSizeMaxnMore(page::Page* otherPage)
{
	if (this->elem->getTagTabSize() + otherPage->elem->getTagTabSize() >= (unsigned int)(2.0 * (double)this->elem->getTagTabMaxSize() * 0.5 * this->myTree->wParam))
	{
		return true;
	}
	return false;
}
//
bool Page::pageSumSizeMaxnLess(page::Page* otherPage)
{
	if (this->elem->getTagTabSize() + otherPage->elem->getTagTabSize() <= (unsigned int)(2.0 * (double)this->elem->getTagTabMaxSize() * 0.5 * this->myTree->wParam))
	{
		return true;
	}
	return false;
}


// Private Methods -------------------------------


//init statix:
Tree* Page::myTree = nullptr;
unsigned int Page::MAXIMAL_PAGE_SIZE = 0;






















