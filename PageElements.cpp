#include "PageElements.h"


using namespace page;
//================================================================================
//================================================================================
//=============================PAGE==ELEMENTS=====================================
//================================================================================
//================================================================================

//Constructor----------------------------------------------------------------------------------------------
PageElements::PageElements()
{
	this->tagTab = new rec::Tag * [this->TAG_TAB_MAX_SIZE_WITH_OVERFLOW];
	this->pointerTab = new rec::PageNumber * [this->POINTER_TAB_MAX_SIZE_WITH_OVERFLOW];
	memset(this->tagTab, 0, this->TAG_TAB_MAX_SIZE_WITH_OVERFLOW * sizeof(rec::Tag*));
	memset(this->pointerTab, 0, this->POINTER_TAB_MAX_SIZE_WITH_OVERFLOW * sizeof(rec::PageNumber*));
	this->tagTabRealSize = 0;
	this->pointerTabRealSize = 0;
}

//Public Named Constructors--------------------------------------------------------------------------------
PageElements* PageElements::createNew()
{
	//check if tab initialized: (need to initialize tab first)
	if (PageElements::TAG_TAB_MAX_SIZE == 0) return nullptr;
	if (PageElements::POINTER_TAB_MAX_SIZE == 0) return nullptr;
	//construct element
	PageElements* newTab = new PageElements();
	//add one null pointer
	newTab->pointerTabRealSize = 1;
	newTab->pointerTab[0] = new rec::PageNumber(rec::PageNumber::NULL_NUMBER);
	return newTab;
}
PageElements* PageElements::createFromFile(pageptr pagetab)
{
	//check if arg is good
	if (pagetab == nullptr) return nullptr;
	//check if tab initialized: (need to initialize tab first)
	if (PageElements::TAG_TAB_MAX_SIZE == 0) return nullptr;
	if (PageElements::POINTER_TAB_MAX_SIZE == 0) return nullptr;
	//read number of elements from file:
	pageint pageRealSize = pagetab[1];
	if (pageRealSize % 3 != 0) throw 1;
	pageint checkPageRealSize = page::Page::HEADER_SIZE;
	if (pageRealSize > page::Page::MAXIMAL_PAGE_SIZE) return nullptr;
	if (pageRealSize + 1 < page::Page::HEADER_SIZE) return nullptr;
	//construct element
	PageElements* newTab = new PageElements();
	//read all elements from file:
	for (unsigned int i = page::Page::HEADER_SIZE; i < pageRealSize && i < page::Page::MAXIMAL_PAGE_SIZE; i++)
	{
		unsigned int index = (i - page::Page::HEADER_SIZE) / 3;
		switch ((i - page::Page::HEADER_SIZE) % 3)
		{
		case 0:
		{
			//Page Number (pointer)
			newTab->pointerTab[index] = new rec::PageNumber(pagetab[i]);
			newTab->pointerTabRealSize++;
			checkPageRealSize++;
			break;
		}
		case 1:
		{
			//Tag && Address
			newTab->tagTab[index] = new rec::Tag(pagetab[i], pagetab[i + 1]);
			newTab->tagTabRealSize++;
			checkPageRealSize += 2;
			i++;
			break;
		}

		}
	}
	if (checkPageRealSize != pageRealSize)
	{
		//check (should equals)
		delete newTab;
		newTab = nullptr;
		return nullptr;
	}
	return newTab;

}

//Initializer----------------------------------------------------------------------------------------------
void PageElements::initializeTable(unsigned int dOrder)
{
	PageElements::TAG_TAB_MAX_SIZE = dOrder * 2;
	PageElements::POINTER_TAB_MAX_SIZE = 1 + dOrder * 2;
	PageElements::TAG_TAB_MAX_SIZE_WITH_OVERFLOW = PageElements::TAG_TAB_MAX_SIZE + 1;
	PageElements::POINTER_TAB_MAX_SIZE_WITH_OVERFLOW = PageElements::POINTER_TAB_MAX_SIZE + 1;
}

//Save--To--File------------------------------------------------------------------------------------------
pageptr PageElements::saveToFile(rec::PageNumber* thisPageNumber)
{
	//allocate pagetab:
	pageptr pagetab = new pageint[page::Page::MAXIMAL_PAGE_SIZE];
	memset(pagetab, 0, page::Page::MAXIMAL_PAGE_SIZE * sizeof(pageint));
	//add number of this page:
	pagetab[0] = thisPageNumber->getPageNumber();
	//add size of this page
	pageint pageRealSize = this->getPageRealSize();
	if (pageRealSize % 3 != 0) throw 1;
	pagetab[1] = (pageint)pageRealSize;
	pageint checkPageRealSize = page::Page::HEADER_SIZE;
	//add elements:
	for (unsigned int i = page::Page::HEADER_SIZE; i < pageRealSize && i < page::Page::MAXIMAL_PAGE_SIZE; i++)
	{
		unsigned int index = (i - page::Page::HEADER_SIZE) / 3;
		switch ((i - page::Page::HEADER_SIZE) % 3)
		{
		case 0:
		{
			//Page Number (pointer)
			pagetab[i] = this->pointerTab[index]->getPageNumber();
			checkPageRealSize++;
			break;
		}
		case 1:
		{
			//Tag && Address
			pagetab[i] = this->tagTab[index]->getTag();
			pagetab[i + 1] = this->tagTab[index]->getAddress();
			checkPageRealSize += 2;
			i++;
			break;
		}
		}
	}
	if (checkPageRealSize != checkPageRealSize)
	{
		//check (should equals)
		delete[] pagetab;
		pagetab = nullptr;
		return nullptr;
	}
	return pagetab;
}

pageptr PageElements::saveZerosToFile()
{
	pageptr pagetab = new pageint[page::Page::MAXIMAL_PAGE_SIZE];
	memset(pagetab, 0, page::Page::MAXIMAL_PAGE_SIZE * sizeof(pageint));
	return pagetab;
}

//Deconstructor-------------------------------------------------------------------------------------------
PageElements::~PageElements()
{
	//Delete Tag 
	if (tagTab != nullptr)
	{
		for (unsigned int i = page::Page::HEADER_SIZE; i < this->tagTabRealSize && i < this->TAG_TAB_MAX_SIZE_WITH_OVERFLOW; i++)
		{
			if (tagTab[i] != nullptr)
			{
				delete tagTab[i];
				tagTab[i] = nullptr;
			}
		}
		delete[] tagTab;
		tagTab = nullptr;
	}
	//Delete Page Numbers (pointer)
	if (pointerTab != nullptr)
	{
		for (unsigned int i = page::Page::HEADER_SIZE; i < this->pointerTabRealSize && i < this->POINTER_TAB_MAX_SIZE_WITH_OVERFLOW; i++)
		{
			if (pointerTab[i] != nullptr)
			{
				delete pointerTab[i];
				pointerTab[i] = nullptr;
			}
		}
		delete[] pointerTab;
		pointerTab = nullptr;
	}
}
void PageElements::deleteTagVan(rec::Tag**& tagVan, unsigned int size)
{
	if (tagVan == nullptr) return;
	if (size == 0) return;
	for (unsigned int i = 0; i < size; i++)
	{
		if (tagVan[i] != nullptr)
		{
			delete tagVan[i];
			tagVan[i] = nullptr;
		}
	}
	delete[] tagVan;
	tagVan = nullptr;
}
//Getters-------------------------------------------------------------------------------------------------
unsigned int PageElements::getPageRealSize()
{
	return 2 * this->tagTabRealSize + this->pointerTabRealSize + page::Page::HEADER_SIZE;
}
unsigned int PageElements::getPointerTabSize()
{
	return this->pointerTabRealSize;
}
unsigned int PageElements::getTagTabSize()
{
	return this->tagTabRealSize;
}
unsigned int PageElements::getPointerTabMaxSize()
{
	return this->POINTER_TAB_MAX_SIZE;
}
unsigned int PageElements::getTagTabMaxSize()
{
	return this->TAG_TAB_MAX_SIZE;
}

//Pointer:
rec::PageNumber* PageElements::getPointer(unsigned int pointerindex)
{
	if (pointerindex >= this->pointerTabRealSize) return nullptr;
	if (pointerindex >= this->POINTER_TAB_MAX_SIZE_WITH_OVERFLOW) return nullptr;
	if (this->pointerTab == nullptr) return nullptr;
	return this->pointerTab[pointerindex];
}
unsigned int PageElements::getPointerIndex(rec::PageNumber* pointer, bool& founded)
{
	founded = false;
	if (this->POINTER_TAB_MAX_SIZE == 0) return 0;
	if (this->pointerTabRealSize == 0) return 0;
	for (unsigned int i = 0; i < this->pointerTabRealSize && i < this->POINTER_TAB_MAX_SIZE; i++)
	{
		if (this->pointerTab[i]->getPageNumber() == pointer->getPageNumber())
		{
			//founded
			founded = true;
			return i;
		}
	}
	return 0;

}
rec::PageNumber* PageElements::getPointerBeforeTag(unsigned int tagindex)
{
	if (tagindex > this->tagTabRealSize) return nullptr;
	if (tagindex > this->TAG_TAB_MAX_SIZE) return nullptr;
	unsigned int pointerindex = tagindex;
	if (pointerindex >= this->POINTER_TAB_MAX_SIZE) return nullptr;
	if (pointerindex >= this->pointerTabRealSize) return nullptr;
	return this->pointerTab[pointerindex];
}
rec::PageNumber* PageElements::getPointerAfrerTag(unsigned int tagindex)
{
	if (tagindex >= this->tagTabRealSize) return nullptr;
	if (tagindex >= this->TAG_TAB_MAX_SIZE) return nullptr;
	unsigned int pointerindex = tagindex + 1;
	if (pointerindex >= this->POINTER_TAB_MAX_SIZE) return nullptr;
	if (pointerindex >= this->pointerTabRealSize) return nullptr;
	return this->pointerTab[pointerindex];
}

//Tag:
rec::Tag* PageElements::getTag(unsigned int tagindex)
{
	if (tagindex >= this->tagTabRealSize) return nullptr;
	if (tagindex >= this->TAG_TAB_MAX_SIZE_WITH_OVERFLOW) return nullptr;
	if (this->tagTab == nullptr) return nullptr;
	return this->tagTab[tagindex];
}
unsigned int PageElements::getTagIndex(rec::Tag* tag)
{
	//Bysection:
	if (this->TAG_TAB_MAX_SIZE == 0) return 0;
	if (this->tagTabRealSize == 0) return 0;
	unsigned int start = 0;
	unsigned int end = this->tagTabRealSize - 1;
	unsigned int midpoint = (start + end) / 2;
	for (unsigned int i = 0; i < this->tagTabRealSize; i++)
	{
		if (rec::Tag::equals(tag->getTag(), this->tagTab[midpoint]->getTag()))
		{
			//founded
			return midpoint;
		}
		if (rec::Tag::compare(tag->getTag(), this->tagTab[midpoint]->getTag()))
		{
			// tag < current
			end = midpoint;

		}
		else
		{
			// tag > current
			start = midpoint;
		}
		if (((end - start) / 2) <= 0)
		{
			if (rec::Tag::compare(tag->getTag(), this->tagTab[start]->getTag()))
			{
				return start;
			}
			if (rec::Tag::compare(tag->getTag(), this->tagTab[end]->getTag()))
			{
				return end;
			}
			return end + 1;
		}
		midpoint = (start + end) / 2;
	}
	return midpoint;
}
unsigned int PageElements::getTagIndexLinearly(rec::Tag* tag)
{
	if (this->TAG_TAB_MAX_SIZE == 0) return 0;
	if (this->tagTabRealSize == 0) return 0;
	for (unsigned int i = 0; i < this->tagTabRealSize && i < this->TAG_TAB_MAX_SIZE; i++)
	{
		if (rec::Tag::compare(tag->getTag(), this->tagTab[i]->getTag()))
		{
			//founded

			return i;

		}
	}
	return this->tagTabRealSize;

}
unsigned int PageElements::getAlmostTagIndex(rec::Tag* tag)
{
	//USED for finding greatest tag smaller than this tag only!
	if (this->TAG_TAB_MAX_SIZE == 0) return 0;
	if (this->tagTabRealSize == 0) return 0;
	for (unsigned int i = 0; i < this->tagTabRealSize && i < this->TAG_TAB_MAX_SIZE; i++)
	{
		if (rec::Tag::compare(tag->getTag(), this->tagTab[i]->getTag()))
		{
			//founded
			if (i > 0)
			{
				return i - 1;
			}
			return 0;
		}
	}
	return this->tagTabRealSize - 1;

}
rec::Tag* PageElements::getTagBefore(unsigned int pointerindex)
{
	if (pointerindex >= this->POINTER_TAB_MAX_SIZE) return nullptr;
	if (pointerindex >= this->pointerTabRealSize) return nullptr;
	if (pointerindex == 0) return nullptr;
	unsigned int tagindex = pointerindex - 1;
	if (tagindex >= this->tagTabRealSize) return nullptr;
	if (tagindex >= this->TAG_TAB_MAX_SIZE) return nullptr;
	return this->tagTab[tagindex];
}
rec::Tag* PageElements::getTagAfrer(unsigned int pointerindex)
{
	if (pointerindex >= this->POINTER_TAB_MAX_SIZE) return nullptr;
	if (pointerindex >= this->pointerTabRealSize) return nullptr;
	unsigned int tagindex = pointerindex;
	if (tagindex >= this->tagTabRealSize) return nullptr;
	if (tagindex >= this->TAG_TAB_MAX_SIZE) return nullptr;
	return this->tagTab[tagindex];
}

//Inserts---------------------------------------------------------------------

//Insert one tag into specified position
bool PageElements::insertTag(unsigned int tagindex, rec::Tag* tag, bool safe)
{
	if (safe == true)
	{
		if (this->tagTabRealSize >= this->TAG_TAB_MAX_SIZE) return false; //page full
		if (tagindex >= this->TAG_TAB_MAX_SIZE) return false;
	}
	else
	{
		if (this->tagTabRealSize >= this->TAG_TAB_MAX_SIZE_WITH_OVERFLOW) return false; //page totally full
		if (tagindex >= this->TAG_TAB_MAX_SIZE_WITH_OVERFLOW) return false;
	}
	if (tagindex > this->tagTabRealSize) return false;
	if (tagindex == this->tagTabRealSize)
	{
		// just add at the end
		if (this->tagTab[tagindex] != nullptr) throw 1;
		this->tagTab[tagindex] = tag;
		this->tagTabRealSize++;
		return true;
	}
	if (this->tagTab == nullptr) return false;
	rec::Tag** tagTabCpy = new rec::Tag * [this->TAG_TAB_MAX_SIZE_WITH_OVERFLOW];
	//Initialize with zeros
	memset(tagTabCpy, 0, this->TAG_TAB_MAX_SIZE_WITH_OVERFLOW * sizeof(rec::Tag*));
	//Copy all elements before
	memcpy(tagTabCpy, this->tagTab, tagindex * sizeof(rec::Tag*));
	//Add element
	tagTabCpy[tagindex] = tag;
	//Copy the rest of them
	memcpy(tagTabCpy + tagindex + 1, this->tagTab + tagindex, (this->tagTabRealSize - tagindex) * sizeof(rec::Tag*));
	//Copy the copy
	delete[] this->tagTab;
	this->tagTab = tagTabCpy;
	tagTabCpy = nullptr;
	this->tagTabRealSize++;
	return true;
}

//Insert one pointer into specified position
bool PageElements::insertPointer(unsigned int pointerindex, rec::PageNumber* pointer, bool safe)
{
	if (safe == true)
	{
		if (this->pointerTabRealSize >= this->POINTER_TAB_MAX_SIZE) return false; //page full
		if (pointerindex >= this->POINTER_TAB_MAX_SIZE) return false;
	}
	else
	{
		if (this->pointerTabRealSize >= this->POINTER_TAB_MAX_SIZE_WITH_OVERFLOW) return false; //page totally full
		if (pointerindex >= this->POINTER_TAB_MAX_SIZE_WITH_OVERFLOW) return false;
	}
	if (pointerindex > this->pointerTabRealSize) return false;
	if (pointerindex == this->pointerTabRealSize)
	{
		// just add at the end
		if (this->pointerTab[pointerindex] != nullptr) throw 1;
		this->pointerTab[pointerindex] = pointer;
		this->pointerTabRealSize++;
		return true;
	}
	if (this->pointerTab == nullptr) return false;
	rec::PageNumber** pointerTableCpy = new rec::PageNumber * [this->POINTER_TAB_MAX_SIZE_WITH_OVERFLOW];
	//Initialize with zeros
	memset(pointerTableCpy, 0, this->POINTER_TAB_MAX_SIZE_WITH_OVERFLOW * sizeof(rec::PageNumber*));
	//Copy all elements before
	memcpy(pointerTableCpy, this->pointerTab, pointerindex * sizeof(rec::PageNumber*));
	//Add element
	pointerTableCpy[pointerindex] = pointer;
	//Copy the rest of them
	memcpy(pointerTableCpy + pointerindex + 1, this->pointerTab + pointerindex, (this->tagTabRealSize - pointerindex) * sizeof(rec::PageNumber*));
	//Copy the copy
	delete[] this->pointerTab;
	this->pointerTab = pointerTableCpy;
	pointerTableCpy = nullptr;
	this->pointerTabRealSize++;
	return true;

}

//Insert many tags to the end (me<-sourse)
bool PageElements::bulkInsertTagLeft(rec::Tag**& tagVan, unsigned int amount)
{
	if (tagVan == nullptr) return false;
	if (amount == 0 || this->tagTabRealSize + amount > this->TAG_TAB_MAX_SIZE_WITH_OVERFLOW)
	{
		throw 2; //error 
	}
	//Insert to the end
	unsigned int offset = this->tagTabRealSize;
	//me<-sourse
	memcpy(this->tagTab + offset, tagVan, amount * sizeof(rec::Tag*));
	this->tagTabRealSize += amount;
	delete[] tagVan; // pointer done it's job
	tagVan = nullptr;
	return true;
}

//Insert many tags to the begining (sourse->me)
bool PageElements::bulkInsertTagRight(rec::Tag**& tagVan, unsigned int amount)
{
	if (tagVan == nullptr) return false;
	if (amount == 0) return false;
	if (amount == 0 || this->tagTabRealSize + amount > this->TAG_TAB_MAX_SIZE_WITH_OVERFLOW)
	{
		throw 2; //error 
	}
	//Insert to the begining
	//sourse->me
	rec::Tag** tagTabCpy = new rec::Tag * [this->TAG_TAB_MAX_SIZE_WITH_OVERFLOW];
	memcpy(tagTabCpy, tagVan, amount * sizeof(rec::Tag*));
	memcpy(tagTabCpy + amount, this->tagTab, this->tagTabRealSize * sizeof(rec::Tag*));
	this->tagTabRealSize += amount;
	delete[] this->tagTab;
	this->tagTab = tagTabCpy;
	tagTabCpy = nullptr;
	delete[] tagVan; // pointer done it's job
	tagVan = nullptr;
	return true;
}

//Insert many pointers to the end (me<-sourse)
bool PageElements::bulkInsertPointerLeft(rec::PageNumber**& pointerVan, unsigned int amount)
{
	if (pointerVan == nullptr) return false;
	if (amount == 0 || this->pointerTabRealSize + amount > this->POINTER_TAB_MAX_SIZE_WITH_OVERFLOW)
	{
		throw 2; //error 
	}
	//Insert to the end
	unsigned int offset = this->pointerTabRealSize;
	//me<-sourse
	memcpy(this->pointerTab + offset, pointerVan, amount * sizeof(rec::PageNumber*));
	this->pointerTabRealSize += amount;
	delete[] pointerVan; // pointer done it's job
	pointerVan = nullptr;
	return true;
}

//Insert many pointers to the begining (sourse->me)
bool PageElements::bulkInsertPointerRight(rec::PageNumber**& pointerVan, unsigned int amount)
{
	if (pointerVan == nullptr) return false;
	if (amount == 0) return false;
	if (amount == 0 || this->pointerTabRealSize + amount > this->POINTER_TAB_MAX_SIZE_WITH_OVERFLOW)
	{
		throw 2; //error 
	}
	//Insert to the begining
	//sourse->me
	rec::PageNumber** pointerTabCpy = new rec::PageNumber * [this->POINTER_TAB_MAX_SIZE_WITH_OVERFLOW];
	memcpy(pointerTabCpy, pointerVan, amount * sizeof(rec::PageNumber*));
	memcpy(pointerTabCpy + amount, this->pointerTab, this->pointerTabRealSize * sizeof(rec::PageNumber*));
	this->pointerTabRealSize += amount;
	delete[] this->pointerTab;
	this->pointerTab = pointerTabCpy;
	pointerTabCpy = nullptr;
	delete[] pointerVan; // pointer done it's job
	pointerVan = nullptr;
	return true;
}

//Removes----------------------------------------------------------------------

//Romoves and returns one specified tag
rec::Tag* PageElements::removeTag(unsigned int tagindex, bool safe)
{
	if (safe == true)
	{
		if (tagindex >= this->TAG_TAB_MAX_SIZE) return nullptr;;
	}
	else
	{
		if (tagindex >= this->TAG_TAB_MAX_SIZE_WITH_OVERFLOW) return nullptr;
	}
	if (tagindex > this->tagTabRealSize) return nullptr;
	if (this->tagTab == nullptr)  return nullptr;
	if (tagindex == this->tagTabRealSize)
	{
		// just remove last
		rec::Tag* rettag = this->tagTab[tagindex];
		this->tagTab[tagindex] = nullptr;
		this->tagTabRealSize--;
		return rettag;
	}
	rec::Tag** tagTabCpy = new rec::Tag * [this->TAG_TAB_MAX_SIZE_WITH_OVERFLOW];
	//Initialize with zeros
	memset(tagTabCpy, 0, this->TAG_TAB_MAX_SIZE_WITH_OVERFLOW * sizeof(rec::Tag*));
	//Copy all elements before
	memcpy(tagTabCpy, this->tagTab, tagindex * sizeof(rec::PageNumber*));
	//Remove element
	rec::Tag* rettag = this->tagTab[tagindex];
	this->tagTab[tagindex] = nullptr;
	//Copy the rest of them
	memcpy(tagTabCpy + (tagindex), this->tagTab + (tagindex + 1), (this->tagTabRealSize - (tagindex + 1)) * sizeof(rec::Tag*));
	//Copy the copy
	delete[] this->tagTab;
	this->tagTab = tagTabCpy;
	tagTabCpy = nullptr;
	this->tagTabRealSize--;
	return rettag;
}

rec::PageNumber* PageElements::removePointer(unsigned int pointerindex, bool safe)
{
	if (safe == true)
	{
		if (pointerindex >= this->POINTER_TAB_MAX_SIZE) return nullptr;;
	}
	else
	{
		if (pointerindex >= this->POINTER_TAB_MAX_SIZE_WITH_OVERFLOW) return nullptr;
	}
	if (pointerindex >= this->pointerTabRealSize) return nullptr;
	if (this->pointerTab == nullptr) return nullptr;
	if (pointerindex == this->pointerTabRealSize - 1)
	{
		// just remove last
		rec::PageNumber* retptr = this->pointerTab[pointerindex];
		this->pointerTab[pointerindex] = nullptr;
		this->pointerTabRealSize--;
		return retptr;
	}
	rec::PageNumber** pointerTableCpy = new rec::PageNumber * [this->POINTER_TAB_MAX_SIZE_WITH_OVERFLOW];
	//Initialize with zeros
	memset(pointerTableCpy, 0, this->POINTER_TAB_MAX_SIZE_WITH_OVERFLOW * sizeof(rec::PageNumber*));
	//Copy all elements before
	memcpy(pointerTableCpy, this->pointerTab, pointerindex * sizeof(rec::PageNumber*));
	//Remove element
	rec::PageNumber* retptr = this->pointerTab[pointerindex];
	this->pointerTab[pointerindex] = nullptr;
	//Copy the rest of them
	memcpy(pointerTableCpy + (pointerindex), this->pointerTab + (pointerindex + 1), (this->pointerTabRealSize - (pointerindex + 1)) * sizeof(rec::PageNumber*));
	//Copy the copy
	delete[] this->pointerTab;
	this->pointerTab = pointerTableCpy;
	pointerTableCpy = nullptr;
	this->pointerTabRealSize--;
	return retptr;
}

//Removes many Tags from begining (dest<-me)
rec::Tag** PageElements::bulkRemoveTagLeft(unsigned int amount)
{
	if (amount == 0) return nullptr;
	if (amount > this->tagTabRealSize)  return nullptr;
	//Remove from the begining
	//dest<-me
	rec::Tag** tagVan = new rec::Tag * [amount];
	memcpy(tagVan, this->tagTab, amount * sizeof(rec::Tag*));
	rec::Tag** tagTabCpy = new rec::Tag * [this->TAG_TAB_MAX_SIZE_WITH_OVERFLOW];
	memset(tagTabCpy, 0, this->TAG_TAB_MAX_SIZE_WITH_OVERFLOW * sizeof(rec::Tag*));
	memcpy(tagTabCpy, this->tagTab + amount, (this->TAG_TAB_MAX_SIZE_WITH_OVERFLOW - amount) * sizeof(rec::Tag*));
	delete[] this->tagTab;
	this->tagTab = tagTabCpy;
	tagTabCpy = nullptr;
	this->tagTabRealSize -= amount;
	return tagVan;
}

//Removes many Tags from end (me->dest)
rec::Tag** PageElements::bulkRemoveTagRight(unsigned int amount)
{
	if (amount == 0) return nullptr;
	if (amount > this->tagTabRealSize)  return nullptr;
	//Remove from the end
	unsigned int offset = this->tagTabRealSize - amount;
	//me->dest
	rec::Tag** tagVan = new rec::Tag * [amount];
	memcpy(tagVan, this->tagTab + offset, amount * sizeof(rec::Tag*));
	memset(this->tagTab + offset, 0, amount * sizeof(rec::Tag*));
	this->tagTabRealSize -= amount;
	return tagVan;
}

//Removes many Pointers from begining (dest<-me)
rec::PageNumber** PageElements::bulkRemovePointerLeft(unsigned int amount)
{
	if (amount == 0) return nullptr;
	if (amount > this->pointerTabRealSize)  return nullptr;
	//Remove from the begining
	rec::PageNumber** pointerVan = new rec::PageNumber * [amount];
	memcpy(pointerVan, this->pointerTab, amount * sizeof(rec::PageNumber*));
	rec::PageNumber** pointerTabCpy = new rec::PageNumber * [this->POINTER_TAB_MAX_SIZE_WITH_OVERFLOW];
	memset(pointerTabCpy, 0, this->POINTER_TAB_MAX_SIZE_WITH_OVERFLOW * sizeof(rec::PageNumber*));
	memcpy(pointerTabCpy, this->pointerTab + amount, (this->POINTER_TAB_MAX_SIZE_WITH_OVERFLOW - amount) * sizeof(rec::PageNumber*));
	delete[] this->pointerTab;
	this->pointerTab = pointerTabCpy;
	pointerTabCpy = nullptr;
	this->pointerTabRealSize -= amount;
	return pointerVan;
}

//Removes many Pointers from end (end->dest)
rec::PageNumber** PageElements::bulkRemovePointerRight(unsigned int amount)
{
	if (amount == 0) return nullptr;
	if (amount > this->pointerTabRealSize)  return nullptr;
	//Remove from the end
	unsigned int offset = this->pointerTabRealSize - amount;
	rec::PageNumber** pointerVan = new rec::PageNumber * [amount];
	memcpy(pointerVan, this->pointerTab + offset, amount * sizeof(rec::PageNumber*));
	memset(this->pointerTab + offset, 0, amount * sizeof(rec::PageNumber*));
	this->pointerTabRealSize -= amount;
	return pointerVan;
}

//Calculations-----------------------------------------------------------------

//Calculate index of pointer BEFORE tag
unsigned int PageElements::calcpBt(unsigned int tagint)
{
	return tagint;
}
//Calculate index of pointer AFTER tag
unsigned int PageElements::calcpAt(unsigned int tagint)
{
	return tagint + 1;
}
//Calculate index of tag BEFORE pointer
unsigned int PageElements::calctBp(unsigned int pointerint)
{
	if (pointerint == 0) return 0;
	return pointerint - 1;
}
//Calculate index of tag AFTER pointer
unsigned int PageElements::calctAp(unsigned int pointerint)
{
	return pointerint;
}


bool PageElements::hasOnlyNullPtrs()
{
	if (this->pointerTabRealSize <= 0) return false;
	for (unsigned int i = 0; i < this->pointerTabRealSize; i++)
	{
		rec::PageNumber* ptr = this->pointerTab[i];
		if (ptr == nullptr) continue;
		if (ptr->goodPage() == true) return false;
	}
	return true;
}

//init statix:
unsigned int PageElements::TAG_TAB_MAX_SIZE = 0;
unsigned int PageElements::POINTER_TAB_MAX_SIZE = 0;
unsigned int PageElements::TAG_TAB_MAX_SIZE_WITH_OVERFLOW = 1;
unsigned int PageElements::POINTER_TAB_MAX_SIZE_WITH_OVERFLOW = 1;