#include "Tree.h"


//===============================================================
//Initialisation
Tree::Tree() : pageTable()
{
	page::Page::initializeTree(this);
	this->rootFile = new RootFile(this->dOrder);
	this->dataFile = new DataFile(this->rootFile->getDataFileSize());
	this->treeFile = new TreeFile(page::Page::getMaxPageSize(), this->dataFile);
	this->rootPage = nullptr;
	this->thisPage = nullptr;
	this->foundTag = nullptr;
	this->pageTable = new PageTable;
	this->thisH = 0;
	this->treeFileReads = 0;
	this->treeFileWrites = 0;
	this->init();//initialize root
}

Tree::~Tree()
{
	pnumint rootNumber;
	if (this->rootPage != nullptr)
	{
		rootNumber = this->rootPage->getThisPageNumber()->getPageNumber();
	}
	if (this->rootPage == nullptr || this->rootPage->emptyPage())
	{
		rootNumber = rec::PageNumber::NULL_NUMBER;
	}
	this->rootFile->setDataFileSize(this->dataFile->getFileSize());
	this->pageTable->~PageTable();
	this->pageTable = nullptr;
	this->thisPage = nullptr;
	this->rootPage = nullptr;
	this->foundTag = nullptr;
	if (this->dataFile != nullptr)
	{
		delete this->dataFile;
		this->dataFile = nullptr;
	}
	if (this->treeFile != nullptr)
	{
		delete this->treeFile;
		this->treeFile = nullptr;
	}
	if (this->rootFile != nullptr)
	{
		this->rootFile->save(rootNumber, this->dOrder);
		delete this->rootFile;
		this->rootFile = nullptr;
	}

}

void Tree::init()
{
	//on start of the program find root
	if (this->rootFile->ifEmpty() == false && this->treeFile->ifEmpty() == false)
	{
		this->rootPage = page::Page::createPageFromFile(this->treeFile->getPage(this->rootFile->getRootPageNumber() - 1));
		this->pageTable->savePage(this->rootPage);
	}
	else
	{
		//clear all
		this->dataFile->clearFile();
		this->treeFile->clearFile();
		this->rootFile->clearFile();
	}
}




//===============================================================
//Public Methods:

bool Tree::search(rec::Record* record, bool standalone)
{
	if (record == nullptr || record->getTag() == nullptr)
	{
		std::cout << "Searching Interrupted. Wrong Arguments!" << std::endl;
		this->treeFile->resetCounts();
		return false;
	}
	if (record->getTag()->goodTag() == false)
	{
		std::cout << "Searching Interrupted. Wrong Tag! Tag needs to be != 0" << std::endl;
		this->treeFile->resetCounts();
		return false;
	}
	if (this->rootPage == nullptr || this->rootPage->getThisPageNumber()->goodPage() == false)
	{
		this->thisPage = nullptr;
		this->thisH = 0;
		if (standalone)
		{
			std::cout << "Tree is empty" << std::endl;
		}
		return false; //tree is empty
	}

	this->thisPage = this->rootPage;
	this->thisH = 0;
	this->foundTag = nullptr;

	bool found = this->searchTag(record->getTag());
	if (found)
	{
		if (standalone)
		{
			this->treeFileReads += this->treeFile->getPageReads();
			this->treeFileWrites += this->treeFile->getPageWrites();
			if (this->foundTag == nullptr) throw 1;
			std::cout << "Search successfull, Record k=" << record->getTag()->getTag() << " R=" << this->dataFile->getRecord(this->foundTag->calculateAddressOffset()) << " found, i/o = ";
			this->treeFile->getCounts(true, true);
			std::cout << std::endl;
		}
		return true;
	}
	else
	{
		if (standalone)
		{
			this->treeFileReads += this->treeFile->getPageReads();
			this->treeFileWrites += this->treeFile->getPageWrites();

			std::cout << "Search unsuccessfull, Record k=" << record->getTag()->getTag() << " not found, i/o = ";
			this->treeFile->getCounts(true, true);
			std::cout << std::endl;
		}
		return false;
	}





}

bool Tree::add(rec::Record* newRecord, bool print, bool standalone)
{
	if (newRecord == nullptr || newRecord->getTag() == nullptr)
	{
		std::cout << "Adding Interrupted. Wrong Arguments!" << std::endl;
		this->treeFile->resetCounts();
		return false;
	}
	if (newRecord->getTag()->goodTag() == false)
	{
		std::cout << "Adding Interrupted. Wrong Tag! Tag needs to be != 0" << std::endl;
		this->treeFile->resetCounts();
		return false;
	}
	bool duplicatExists = this->search(newRecord, false);
	if (duplicatExists)
	{
		if (print)
			std::cout << "Duplicate founded error - adding aborted, i/o = ";
		this->thisPage = nullptr;
		this->treeFile->getCounts(print, true);
		std::cout << std::endl;
		return false;
	}
	else
	{
		unsigned int testTab2 = 0;
		unsigned int testTab = newRecord->getTag()->getTag();
		newRecord->getTag()->initAddress();
		recordmean newRecordInt = newRecord->getRecord();
		addressint newAddress = newRecord->getTag()->calculateAddressOffset();
		tagint newTag = newRecord->getTag()->getTag();
		bool goodAdd = this->addTag(newRecord->getTag());
		newRecord->setTag(nullptr);
		if (goodAdd)
		{
			this->dataFile->saveRecord(newAddress, newRecordInt);
		}
		else
		{
			std::cout << "Adding impossible, i/o = ";
			this->treeFile->getCounts(print, true);
			std::cout << std::endl;
			return false;
		}
		if (standalone)
		{
			this->treeFileReads += this->treeFile->getPageReads();
			this->treeFileWrites += this->treeFile->getPageWrites();
			if (print)
				std::cout << "Added k=" << newTag << " R=" << newRecordInt << " successfully, i/o = ";
			this->treeFile->getCounts(print, true);
			if (print)
				std::cout << std::endl;
		}
	}

	return true;
}

bool Tree::remove(rec::Record* removedRecord, bool print, bool standalone)
{
	if (removedRecord == nullptr || removedRecord->getTag() == nullptr)
	{
		std::cout << "Removing Interrupted. Wrong Arguments!" << std::endl;
		this->treeFile->resetCounts();
		return false;
	}
	if (removedRecord->getTag()->goodTag() == false)
	{
		std::cout << "Removing Interrupted. Wrong Tag! Tag needs to be != 0" << std::endl;
		this->treeFile->resetCounts();
		return false;
	}
	bool exists = this->search(removedRecord, false);
	if (exists)
	{

		//remove tag
		rec::Tag* removedTag = nullptr;
		bool goodRemove = removeTag(removedTag);
		if (goodRemove && removedTag != nullptr)
		{
			//remove record from file
			if (standalone && print)
			{

				std::cout << "Removed successfully record=";
				std::cout << this->dataFile->getRecord(removedTag->calculateAddressOffset());

			}
			this->dataFile->saveRecord(removedTag->calculateAddressOffset(), rec::Record::INVALID_RECORD_NUMBER);
			if (standalone && print)
			{
				std::cout << " tag=" << removedTag->getTag();
				std::cout << " address=" << removedTag->getAddress();

			}
			removedTag->removeAddress();
			if (removedRecord->getTag() != removedTag)
			{
				delete removedTag;
			}
			removedTag = nullptr;
		}
		else
		{
			std::cout << "Removing Inpossible, i/o = ";
			this->treeFile->getCounts(print, true);
			std::cout << std::endl;
			return false;
		}
	}
	else
	{
		if (print)
			std::cout << "Removing Interrupted. This Record doesn't exist in this Tree, i/o = ";
		this->thisPage = nullptr;
		this->treeFile->getCounts(print, true);
		if (print)
			std::cout << std::endl;
		return false;
	}
	if (standalone)
	{
		this->treeFileReads += this->treeFile->getPageReads();
		this->treeFileWrites += this->treeFile->getPageWrites();
		if (print)
			std::cout << " i/o = ";
		this->treeFile->getCounts(print, true);
		if (print)
			std::cout << std::endl;
	}
	return true;
}

bool Tree::update(rec::Record* record, rec::Record* newRecord)
{
	if (record == nullptr || record->getTag() == nullptr || newRecord == nullptr || newRecord->getTag() == nullptr)
	{
		std::cout << "Update Interrupted. Wrong Arguments!" << std::endl;
		return false;
	}
	if (record->getTag()->goodTag() == false || newRecord->getTag()->goodTag() == false)
	{
		std::cout << "Update Interrupted. Wrong Tag! Tag needs to be != 0" << std::endl;
		return false;
	}

	bool exists = this->search(record, false);
	if (!exists)
	{
		std::cout << "Update Interrupted. This Record doesn't exist in this Tree, i/o = ";
		this->treeFile->getCounts(true, true);
		std::cout << std::endl;
		return false;
	}

	bool remRet = this->remove(record, false, false);
	if (remRet == false)
	{
		std::cout << "Update Interrupted, error while removing record";
		this->treeFile->getCounts(true, true);
		std::cout << std::endl;
		return false;
	}
	tagint newTag = newRecord->getTag()->getTag();
	bool addRet = this->add(newRecord, false, false);
	if (addRet == false)
	{
		std::cout << "Update Interrupted, error while adding record";
		this->treeFile->getCounts(true, true);
		std::cout << std::endl;
		return false;
	}

	this->treeFileReads += this->treeFile->getPageReads();
	this->treeFileWrites += this->treeFile->getPageWrites();
	std::cout << "Update completed, changed k=" << record->getTag()->getTag() << " to k=" << newTag << " i/o = ";
	this->treeFile->getCounts(true, true);
	std::cout << std::endl;
}

void Tree::print(bool full)
{
	if (this->rootPage == nullptr)
	{
		std::cout << "Tree Empty" << std::endl;
		return;
	}
	std::cout << std::endl;
	if (this->thisPage != nullptr)
	{
		this->pageTable->saveAll();
	}
	unsigned int TreeH = 0;
	this->printTree(this->rootPage, 0, TreeH, full);
	std::cout << std::endl;
	std::cout << "Tree H=" << TreeH << " print, i/o = ";
	this->treeFileReads += this->treeFile->getPageReads();
	this->treeFileWrites += this->treeFile->getPageWrites();
	this->treeFile->getCounts(true, true);
	std::cout << std::endl;

}
//===============================================================
//Private Methods:

//Prints:

void Tree::printTree(page::Page* currentPage, unsigned int currentH, unsigned int& TreeH, bool full)
{
	std::cout << "H: " << currentH << " ";
	if (currentH > TreeH) TreeH = currentH;
	currentPage->printPage(currentH, TreeH, full);
}

//Various:

bool Tree::searchTag(rec::Tag* tag, bool equal)
{
	//check if tag is good
	if (tag == nullptr) return false;
	//check if this page exists
	if (this->thisPage == nullptr || this->thisPage->getThisPageNumber()->goodPage() == false)
	{
		return false; //this page is null
	}

	//save this page to buffer 
	savePageToBuf(this->thisPage, this->thisH);

	//search this page:
	bool founded = false;
	void* searchResult = nullptr;
	if (equal)
	{
		searchResult = this->thisPage->serchPageForTag(tag, founded);
	}
	else
	{
		searchResult = this->thisPage->serchPageForAlmostTag(tag, founded);
	}
	if (searchResult == nullptr)
	{
		//search failed
		return false;
	}
	else
	{
		if (founded)
		{
			this->foundTag = (rec::Tag*)searchResult;
			return true;
		}
		else // need to check next page
		{
			rec::PageNumber* nextPageNumber = (rec::PageNumber*)searchResult;
			if (nextPageNumber->goodPage())
			{
				//check if new page is in the buffer:
				this->thisPage = getPageFromBuf(nextPageNumber, this->thisH + 1);
				if (this->thisPage == nullptr)
				{
					//page is not in the buffer take in from file:
					this->thisPage = page::Page::createPageFromFile(this->treeFile->getPage(nextPageNumber->calculatePageOffset()));
					if (this->thisPage->elem == nullptr) throw 1;
				}
				this->thisH++;
				return this->searchTag(tag, equal);
			}
			else
			{
				// next page doesn't exist
				// tag not founded either
				// this means tag can be added to thisPage
				return false;
			}
		}
	}

}
void  Tree::savePageToBuf(page::Page* savePage, unsigned int h)
{
	//save this page to buffer 
	page::Page* bufferPage = this->pageTable->getPageByPosition(h);
	if (bufferPage != savePage)
	{
		if (bufferPage != nullptr)
		{
			this->pageTable->removeFromN(h);
		}
		this->pageTable->savePage(savePage);
	}

}

page::Page* Tree::getPageFromBuf(rec::PageNumber* nextPageNumber, unsigned int h)
{

	page::Page* bufferPage = this->pageTable->getPageByPosition(h);
	if (bufferPage == nullptr) return nullptr;
	if (nextPageNumber == nullptr) return nullptr;
	if (bufferPage->getThisPageNumber()->getPageNumber() == nextPageNumber->getPageNumber())
	{
		return bufferPage;
	}
	return nullptr;
}

//Adds:

bool Tree::addTag(rec::Tag* newTag, rec::PageNumber* childPageNumber)
{
	if (this->thisPage == nullptr)
	{
		//either error or tree is empty
		if (this->rootPage == nullptr)
		{
			//create new blank page
			this->rootPage = page::Page::createNewPage();
			this->thisPage = this->rootPage;
			this->thisH = 0;
			savePageToBuf(this->thisPage, this->thisH);
			return this->addTag(newTag);
		}
		else
		{
			return false;
		}
	}
	bool addResult = this->thisPage->addTag(newTag, childPageNumber);
	if (addResult)
	{
		//successfully added record
		return true;
	}
	else
	{
		page::Page* parentPage = this->pageTable->getParentPage(this->thisH);
		if (parentPage == nullptr && this->thisPage != this->rootPage) throw 0; //error
		//try compensation:
		bool compResult = this->compensation(newTag, parentPage, childPageNumber);
		if (compResult == true)
		{
			//compesation completed
			return true;
		}
		else
		{
			//compensation is impossible
			//try split:
			page::Page* newPage = page::Page::createNewPage(); //create new blank page
			rec::Tag* parentTag = nullptr; // create new parent tag
			bool splitResult = this->split(newTag, parentTag, newPage, parentPage, childPageNumber);
			if (splitResult == false) throw 0; // error
			return true;
		}
	}
}


bool Tree::compensation(rec::Tag* tag, page::Page* parentPage, rec::PageNumber* childPageNumber)
{

	if (parentPage == nullptr) return false;

	bool isLeft = false;
	rec::Tag* parentTag = nullptr;;
	page::Page* siblingPage = nullptr;
	//Get Siblings:
	rec::PageNumber* leftS = parentPage->getLeftSibling(this->thisPage->getThisPageNumber());
	rec::PageNumber* rightS = parentPage->getRightSibling(this->thisPage->getThisPageNumber());
	if (leftS != nullptr && leftS->goodPage() == true)
	{
		//Left Sibling Exists
		siblingPage = page::Page::createPageFromFile(this->treeFile->getPage(leftS->calculatePageOffset()));
		if (siblingPage == nullptr) throw 1;
		if (siblingPage->getPageRealSize() >= page::Page::getMaxPageSize())
		{
			//Sibling Page Full
			delete siblingPage;
			siblingPage = nullptr;
			isLeft = false;
		}
		else
		{
			//Left Sibling can be used to compensate
			parentTag = parentPage->getLeftParent(this->thisPage->getThisPageNumber());
			isLeft = true;
		}
	}

	if (isLeft == false && rightS != nullptr && rightS->goodPage() == true)
	{
		siblingPage = page::Page::createPageFromFile(this->treeFile->getPage(rightS->calculatePageOffset()));
		if (siblingPage == nullptr) throw 1;
		if (siblingPage->getPageRealSize() >= page::Page::getMaxPageSize())
		{
			delete siblingPage;
			siblingPage = nullptr;
			return false;
		}
		else
		{
			//Right Sibling can be used to compensate
			parentTag = parentPage->getRightParent(this->thisPage->getThisPageNumber());
			isLeft = false;
		}

	}
	else if (isLeft == false)
	{
		if (siblingPage != nullptr)
		{
			delete siblingPage;
			siblingPage = nullptr;
		}
		return false;
	}
	//compensate:
	rec::Tag* oldparentTag = parentTag;
	bool success = this->thisPage->compensate(tag, parentTag, siblingPage, isLeft, childPageNumber);
	if (success)
	{
		bool updated = parentPage->upadeCopiedTag(oldparentTag, parentTag);
		if (updated == false) throw 1;
		siblingPage->savePage();
		if (siblingPage != nullptr)
		{
			delete siblingPage;
			siblingPage = nullptr;
		}
		return true;
	}
	else
	{
		if (siblingPage != nullptr)
		{
			delete siblingPage;
			siblingPage = nullptr;
		}
		return false;
	}



}

bool Tree::split(rec::Tag* tag, rec::Tag*& parentTag, page::Page*& newPage, page::Page*& parentPage, rec::PageNumber* childPageNumber)
{
	bool success = this->thisPage->split(tag, parentTag, newPage, childPageNumber);
	if (success == true)
	{
		//success yeay
		if (this->thisPage == this->rootPage)
		{
			//create parent
			if (parentPage != nullptr) throw 0; // error
			parentPage = page::Page::createNewPage();
			rec::PageNumber* thisPageNumberCopy = new rec::PageNumber(*this->thisPage->getThisPageNumber());
			rec::PageNumber* newPageNumberCopy = new rec::PageNumber(*newPage->getThisPageNumber());
			this->rootPage = parentPage;
			//this->thisPage->savePage();
			this->thisPage = parentPage;
			this->thisH = 0;
			savePageToBuf(parentPage, this->thisH);
			bool result = this->addTag(parentTag, nullptr);
			if (result == false) throw 1;
			//Add pointer to parent page:
			parentPage->setptrBefore(parentTag, thisPageNumberCopy);
			parentPage->setptrAfter(parentTag, newPageNumberCopy);
		}
		else
		{
			if (parentPage == nullptr) throw 0; // error
			rec::PageNumber* newPageNumberCopy = new rec::PageNumber(*newPage->getThisPageNumber());
			this->thisPage = parentPage; // thisPage is saved into file
			page::Page* pc = parentPage;
			savePageToBuf(parentPage, this->thisH - 1);
			this->thisH--;
			bool result = this->addTag(parentTag, newPageNumberCopy);
			if (result == false) throw 1;
		}
		newPage->savePage(); //save to file
	}
	else
	{
		//nothing can be done
		if (newPage != nullptr)
		{
			newPage = nullptr;
		}
		if (parentTag != nullptr)
		{
			delete parentTag;
			parentTag = nullptr;
		}
		throw 0; // error
	}
	return true;
}

//Removes:

bool Tree::removeTag(rec::Tag*& removedTag, rec::Tag* tag)
{
	if (removedTag != nullptr) return false;
	if (tag == nullptr)
	{
		tag = this->foundTag;
		this->foundTag = nullptr;
	}
	else
	{
		this->foundTag = nullptr;
	}
	if (this->thisPage == nullptr)
	{
		//empty page 
		//error
		return false;
	}
	//TODO::
	rec::PageNumber* childPtr = nullptr;
	bool childBeforeisNull = false;
	bool isLeaf = this->leafPage(childPtr, childBeforeisNull, tag);
	if (isLeaf == false)
	{
		//Not a Leaf
		//Insert RESERVED tag (to be able to find next closest tag) 
		rec::Tag* MockTag = new rec::Tag(rec::Tag::RESERVED_TAG, rec::Tag::NULL_ADDRESS);
		this->thisPage->upadeCopiedTag(tag, MockTag);
		page::Page* thisPageCopy = this->thisPage;
		unsigned int thishCopy = this->thisH;
		//search for the cloasest tag
		bool searchResult = this->searchTag(tag, false); //sets this->foundTag to the closest tag
		if (searchResult == false) throw 1;
		//swap tag
		page::Page* lastPageCopy = this->pageTable->getPageByPosition(thishCopy);
		bool retUp = lastPageCopy->upadeCopiedTag(MockTag, this->foundTag, false);
		if (retUp == false) throw 1;
		bool retUp2 = this->thisPage->upadeCopiedTag(this->foundTag, tag);
		if (retUp2 == false) throw 1;
		this->foundTag = nullptr;
		lastPageCopy = nullptr;
		if (MockTag != nullptr)
		{
			delete MockTag;
			MockTag = nullptr;
		}
		childPtr = nullptr;
		childBeforeisNull = false;
		isLeaf = this->leafPage(childPtr, childBeforeisNull, tag);
	}



	//try normal removing
	bool isRoot = this->thisPage == this->rootPage;
	bool succesRemoved = this->thisPage->removeTag(tag, removedTag, childBeforeisNull, isRoot);
	if (succesRemoved == true)
	{
		if (this->thisPage->emptyPage())
		{
			page::Page* tobedeletedPage = this->pageTable->removeLast();
			if (tobedeletedPage == nullptr) throw 1;
			if (tobedeletedPage != this->thisPage) throw 1;
			if (tobedeletedPage == this->rootPage) this->rootPage = nullptr;
			delete tobedeletedPage;
			tobedeletedPage = nullptr;
			this->thisPage = nullptr;
		}
		return true;
	}
	else
	{
		//all returns removed tag and bool success
		//if needed remove parent index (no deleting because copied to child page)
		//remove page if needed
		//all pointers?(NO?)
		page::Page* parentPage = this->pageTable->getParentPage(this->thisH);
		if (parentPage == nullptr && this->thisPage != this->rootPage) throw 0; //error
		//try recompensation:
		rec::Tag* parentTag = nullptr;
		bool compResult = this->recompensation(parentPage, parentTag);
		if (compResult == true)
		{
			//recompesation completed
			return true;
		}
		else
		{
			//recompensation is impossible
			//try merge:
			rec::Tag* parentTag = nullptr;
			bool mergeResult = this->merge(parentPage, parentTag);
			if (mergeResult == false)
			{
				throw 0; // error
			}
			else
			{
				//merge well done
				//remove parent tag
				if (parentPage == nullptr) throw 0; // error
				//this->thisPage->savePage();
				this->thisPage = parentPage; // thisPage is saved into file
				this->thisH--;
				rec::Tag* removedParentTag = nullptr;
				bool removedTagBool = this->removeTag(removedParentTag, parentTag);
				if (removedTagBool == false) throw 1;
				if (parentTag == removedParentTag)
				{
					//removing concluded successfully
					//parent tag saved into child page
					removedParentTag = nullptr;
					parentTag = nullptr;
				}
				else
				{
					throw 1;
				}
			}
			return true;
		}
	}


}

bool Tree::leafPage(rec::PageNumber*& childPtr, bool& childBeforeisNull, rec::Tag* tag)
{
	childBeforeisNull = false;
	rec::PageNumber* childPtrBefore = this->thisPage->getPtrBefore(tag);
	rec::PageNumber* childPtrAfter = this->thisPage->getPtrAfter(tag);

	//either tag before is null or tag isn't there
	if (childPtrBefore == nullptr || childPtrAfter == nullptr) throw 1;


	if (childPtrBefore->goodPage() == false)
	{
		childPtr = childPtrBefore;
		childBeforeisNull = true;
		//only before == null and both == null
		return true;
	}
	else if (childPtrAfter->goodPage() == false)
	{
		childPtr = childPtrAfter;
		childBeforeisNull = false;
		//only after == null
		return true;
	}
	return false;
}

bool Tree::recompensation(page::Page* parentPage, rec::Tag*& parentTag)
{

	if (parentPage == nullptr) return false;

	bool isLeft = false;
	if (parentTag != nullptr) throw 1;
	//Get Siblings:
	rec::PageNumber* leftS = parentPage->getLeftSibling(this->thisPage->getThisPageNumber());
	rec::PageNumber* rightS = parentPage->getRightSibling(this->thisPage->getThisPageNumber());
	page::Page* siblingPage = nullptr;
	if (leftS != nullptr && leftS->goodPage() == true)
	{
		//Left Sibling Exists
		siblingPage = page::Page::createPageFromFile(this->treeFile->getPage(leftS->calculatePageOffset()));
		if (siblingPage == nullptr) throw 1;
		if (siblingPage->pageSizeMoreThenHalf() == false)
		{
			//Sibling Page Full
			delete siblingPage;
			siblingPage = nullptr;
			isLeft = false;
		}
		else
		{
			//Left Sibling can be used to compensate			
			isLeft = true;
			parentTag = parentPage->getLeftParent(this->thisPage->getThisPageNumber());
		}
	}

	if (isLeft == false && rightS != nullptr && rightS->goodPage() == true)
	{
		siblingPage = page::Page::createPageFromFile(this->treeFile->getPage(rightS->calculatePageOffset()));
		if (siblingPage == nullptr) throw 1;
		if (siblingPage->pageSizeMoreThenHalf() == false)
		{
			delete siblingPage;
			siblingPage = nullptr;
			return false;
		}
		else
		{
			//Right Sibling can be used to compensate
			isLeft = false;
			parentTag = parentPage->getRightParent(this->thisPage->getThisPageNumber());
		}

	}
	else if (isLeft == false)
	{
		if (siblingPage != nullptr)
		{
			delete siblingPage;
			siblingPage = nullptr;
		}
		parentTag = nullptr;
		return false;
	}

	if (parentTag == nullptr)
	{
		if (siblingPage != nullptr)
		{
			delete siblingPage;
			siblingPage = nullptr;
		}
		return false;
	}


	//recompensate:
	rec::Tag* oldparentTag = parentTag;
	bool success = this->thisPage->recompensate(parentTag, siblingPage, isLeft);
	if (success)
	{
		bool updated = parentPage->upadeCopiedTag(oldparentTag, parentTag);
		oldparentTag = nullptr; //(old parent tag saved in the tagTab)
		if (updated == false) throw 1;
		siblingPage->savePage();
		if (siblingPage != nullptr)
		{
			delete siblingPage;
			siblingPage = nullptr;
		}
		return true;
	}
	else
	{
		if (siblingPage != nullptr)
		{
			delete siblingPage;
			siblingPage = nullptr;
		}
		return false;
	}
}

bool Tree::merge(page::Page* parentPage, rec::Tag*& parentTag)
{
	if (parentPage == nullptr) return false;
	if (parentTag != nullptr) throw 1;

	//Get Siblings:
	rec::PageNumber* leftS = parentPage->getLeftSibling(this->thisPage->getThisPageNumber());
	rec::PageNumber* rightS = parentPage->getRightSibling(this->thisPage->getThisPageNumber());
	rec::PageNumber* otherS = nullptr;
	bool isLeft = false;

	page::Page* siblingPage = nullptr;


	if (siblingPage == nullptr)
	{
		if (leftS != nullptr && leftS->goodPage() == true)
		{
			//Left Sibling Exists
			siblingPage = page::Page::createPageFromFile(this->treeFile->getPage(leftS->calculatePageOffset()));
			if (siblingPage == nullptr) throw 1;
			if (siblingPage->pageSumSizeMaxnLess(this->thisPage) == false)
			{
				//Sibling Page too large
				delete siblingPage;
				siblingPage = nullptr;
				isLeft = false;
			}
			else
			{
				//Left Sibling can be used in merge			
				isLeft = true;
				parentTag = parentPage->getLeftParent(this->thisPage->getThisPageNumber());
			}
		}

		if (isLeft == false && rightS != nullptr && rightS->goodPage() == true)
		{
			siblingPage = page::Page::createPageFromFile(this->treeFile->getPage(rightS->calculatePageOffset()));
			if (siblingPage == nullptr) throw 1;
			if (siblingPage->pageSumSizeMaxnLess(this->thisPage) == false)
			{
				delete siblingPage;
				siblingPage = nullptr;
				return false;
			}
			else
			{
				//Right Sibling can be used to compensate
				isLeft = false;
				parentTag = parentPage->getRightParent(this->thisPage->getThisPageNumber());
			}

		}
		else if (isLeft == false)
		{
			if (siblingPage != nullptr)
			{
				delete siblingPage;
				siblingPage = nullptr;
			}
			return false;
		}
	}


	bool success = this->thisPage->merge(parentTag, siblingPage, isLeft);
	if (success == true)
	{

		if (isLeft)
		{
			parentPage->setptrBefore(parentTag, new rec::PageNumber(rec::PageNumber::NULL_NUMBER));
		}
		else
		{
			parentPage->setptrAfter(parentTag, new rec::PageNumber(rec::PageNumber::NULL_NUMBER));
		}

		if (siblingPage->emptyPage())
		{
			siblingPage->saveEmptyPage();
		}
		else
		{
			siblingPage->savePage(); //save to file
		}
	}
	else
	{
		//nothing can be done
		if (siblingPage != nullptr)
		{
			delete siblingPage;
			siblingPage = nullptr;
		}
		throw 0; // error
	}
	return true;
}

void print_blanks(unsigned n)
{
	while (n--)
		std::cout << ' ';
}


/*void BTreePrinter::visit(page::Page const* node, unsigned level, unsigned child_index)
{
	if (level >= levels.size())
		levels.resize(level + 1);

	LevelInfo& level_info = levels[level];
	NodeInfo info;

	info.text_pos = 0;
	if (!level_info.empty())  // one blank between nodes, one extra blank if left-most child
		info.text_pos = level_info.back().text_end + (child_index == 0 ? 2 : 1);

	info.text = node_text(node->keys, unsigned(node->n));

	if (node->leaf)
	{
		info.text_end = info.text_pos + unsigned(info.text.length());
	}
	else // non-leaf -> do all children so that .text_end for the right-most child becomes known
	{
		for (unsigned i = 0, e = unsigned(node->n); i <= e; ++i)  // one more pointer than there are keys
			visit(node->C[i], level + 1, i);

		info.text_end = levels[level + 1].back().text_end;
	}

	levels[level].push_back(info);
}*/

std::string BTreePrinter::node_text(int const keys[], unsigned key_count)
{
	std::ostringstream os;
	char const* sep = "";

	os << "[";
	for (unsigned i = 0; i < key_count; ++i, sep = " ")
		os << sep << keys[i];
	os << "]";

	return os.str();
}


void BTreePrinter::after_traversal()
{
	for (std::size_t l = 0, level_count = levels.size(); ; )
	{
		auto const& level = levels[l];
		unsigned prev_end = 0;

		for (auto const& node : level)
		{
			unsigned total = node.text_end - node.text_pos;
			unsigned slack = total - unsigned(node.text.length());
			unsigned blanks_before = node.text_pos - prev_end;

			print_blanks(blanks_before + slack / 2);
			std::cout << node.text;

			if (&node == &level.back())
				break;

			print_blanks(slack - slack / 2);

			prev_end += blanks_before + total;
		}

		if (++l == level_count)
			break;

		std::cout << "\n\n";
	}

	std::cout << "\n";
}


