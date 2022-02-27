#include "MyFile.h"

//================================================================================
//================================================================================
//==================================TREE=DATA=====================================
//================================================================================
//================================================================================

TreeFile::TreeFile(unsigned int argPageSize, DataFile* dataFile)
{
	this->filepath = TREE_FILE_PATH;
	this->pageSize = argPageSize;
	this->pageReads = 0;
	this->pageWrites = 0;
	this->dataFile = dataFile;
	initFile();
}
void TreeFile::savePage(pageptr pagetab, unsigned int N)
{
	//opening file:
	this->file.open(this->filepath, std::ios::in | std::ios::out | std::ios::binary);
	if (!this->file.is_open())
	{
		//std::cout << "error - while opening file "<< std::endl;
		this->file.close();
		throw OPEN_FILE_EXEPTION;
	}
	// check file size
	this->file.seekp(0, std::ios::end);
	long size = this->file.tellp();
	if (size < 0 || size < N * this->pageSize * sizeof(pageint))
	{
		//need to add empty pages:
		this->file.close();
		unsigned int pagesAvaliable = (size / this->pageSize);
		unsigned int pagesMissing = N - pagesAvaliable;
		for (unsigned int i = pagesAvaliable; i < N; i++)
		{
			this->insertEmptyPage(i);
		}
		this->file.open(this->filepath, std::ios::in | std::ios::out | std::ios::binary);
	}
	this->file.seekp(((uint64_t)N) * this->pageSize * sizeof(pageint), std::ios::beg);
	//save page:
	try
	{
		this->file.write((char*)pagetab, this->pageSize * sizeof(pageint));
	}
	catch (int exeption)
	{
		throw exeption;
	}
	//closure:
	this->pageWrites++;
	this->empty = false;
	this->file.close();
}
pageptr TreeFile::getPage(unsigned int N)
{
	pageptr pagetab = nullptr;
	this->file.open(this->filepath, std::ios::in | std::ios::binary);
	if (!this->file.is_open())
	{
		//file doesn't exist 
		this->file.close();
		throw OPEN_FILE_EXEPTION;
	}
	// check file size
	this->file.seekg(0, std::ios::end);
	long size = this->file.tellg();
	if (size < 0)
	{
		this->file.close();
		throw EMPTY_FILE_EXEPTION;
	}
	if (size == 0)
	{
		this->file.close();
		throw EMPTY_FILE_EXEPTION;
	}
	if (size < N * this->pageSize * sizeof(pageint))
	{
		//file to small
		this->file.close();
		throw EMPTY_FILE_EXEPTION;
	}
	this->file.seekg(((uint64_t)N) * this->pageSize * sizeof(pageint), std::ios::beg);
	//I have page nmber chceck if numbers are equal if null then save page number
	pagetab = new pageint[this->pageSize];
	try
	{
		this->file.read((char*)pagetab, this->pageSize * sizeof(pageint));
	}
	catch (int exeption)
	{
		delete[] pagetab;
		throw exeption;
	}
	//closure:
	this->pageReads++;
	this->file.close();
	return pagetab;

}
void TreeFile::removeFile()
{
	if (remove(this->filepath.c_str()) != 0)
	{
		throw ERROR_DELETING_FILE;
	}
}
void TreeFile::clearFile()
{
	this->file.open(this->filepath, std::ios::out | std::ios::trunc | std::ios::binary);
	if (!this->file.is_open())
	{
		this->file.close();
		throw OPEN_FILE_EXEPTION;
	}
	this->file.close();

}
void TreeFile::initFile()
{
	this->empty = false;
	this->file.open(this->filepath, std::ios::in | std::ios::binary | std::ios::ate);
	if (!this->file.is_open())
	{
		this->empty = true;
	}
	else
	{
		// check file size
		unsigned int size = (unsigned int)this->file.tellg(); //starts at the end
		if (size == 0)
		{
			this->empty = true;
		}
	}
	this->file.close();
	if (this->empty == true)
	{
		this->clearFile();
	}

}
void TreeFile::insertEmptyPage(unsigned int N)
{
	pageptr pagetab = new pageint[this->pageSize];
	memset(pagetab, 0, this->pageSize * sizeof(pageint));
	//opening file:
	this->file.clear();
	this->file.open(this->filepath, std::ios::in | std::ios::out | std::ios::binary);
	if (!this->file.is_open())
	{
		this->file.close();
		throw OPEN_FILE_EXEPTION;
	}
	this->file.seekp(0, std::ios::end);
	long size = this->file.tellp();
	if (size < 0)
	{
		this->file.close();
		throw EMPTY_FILE_EXEPTION;
	}
	if (size < N * this->pageSize * sizeof(pageint))
	{
		//page to small
		this->file.close();
		throw EMPTY_FILE_EXEPTION;
	}
	this->file.seekp(((uint64_t)N) * this->pageSize * sizeof(pageint), std::ios::beg);
	//save page:
	try
	{
		this->file.write((char*)pagetab, this->pageSize * sizeof(pageint));
	}
	catch (int exeption)
	{
		throw exeption;
	}
	//closure:
	delete[] pagetab;
	this->pageWrites++;
	this->file.close();
}
void TreeFile::getCounts(bool print, bool reset)
{
	if (print)
		std::cout << "Reads: " << this->pageReads << " Writes: " << this->pageWrites;
	if (reset)
	{
		this->pageReads = 0;
		this->pageWrites = 0;
	}
}
void  TreeFile::resetCounts()
{
	this->pageReads = 0;
	this->pageWrites = 0;
}
//================================================================================
//================================================================================
//===================================METADATA=====================================
//================================================================================
//================================================================================


RootFile::RootFile(unsigned int dOrder)
{
	//initialize File
	this->filepath = ROOT_FILE_PATH;
	this->pagetab = new uint8_t[this->listPageSize];
	this->initialised = false;
	this->dOrderCopy = 0;
	this->dataFileSize = 0;
	//read from file:
	this->empty = !readFromFile();
	if (this->empty || this->dOrderCopy != dOrder)
	{
		this->clearFile();
	}
	else
	{
		rec::PageNumber::uniqueList.initialise(this->unuPageNumList);
		rec::Tag::uniqueAddressList.initialise(this->unuAddressList);
	}
}
RootFile::~RootFile()
{
	if (this->pagetab != nullptr)
	{
		delete this->pagetab;
		this->pagetab = nullptr;
	}
}
bool RootFile::fileRead(unsigned int rootpageNumber)
{
	this->file.open(this->filepath, std::ios::in | std::ios::binary);
	if (!this->file.is_open())
	{
		//file doesn't exist 
		this->file.close();
		return false;
	}
	this->file.seekg(0, std::ios::end);
	unsigned int size = (unsigned int)this->file.tellg();
	unsigned int offset = this->listPageSize * rootpageNumber;
	this->file.seekg(offset, std::ios::beg);
	if (size == 0)
	{
		// file is empty
		this->file.close();
		return false;
	}
	if (size <= offset)
	{
		//file to small
		this->file.close();
		return false;
	}
	this->pagetabSize = this->listPageSize;
	if (size - (offset) < this->listPageSize)
	{
		this->pagetabSize = size - (offset);
	}
	try
	{
		this->file.read((char*)this->pagetab, this->pagetabSize);
	}
	catch (int exeption)
	{
		this->file.close();
		throw exeption;
	}
	this->file.close();
	return true;
}
void RootFile::fileWrite(unsigned int rootpageNumber)
{
	this->file.open(this->filepath, std::ios::in | std::ios::out | std::ios::binary);
	if (!this->file.is_open())
	{
		this->file.close();
		throw OPEN_FILE_EXEPTION;
	}
	this->file.seekp(0, std::ios::end);
	unsigned int size = (unsigned int)this->file.tellp();
	unsigned int offset = rootpageNumber * this->listPageSize;
	if (size < offset) return;
	this->file.seekp(offset, std::ios::beg);
	try
	{
		this->file.write((char*)(this->pagetab), this->pagetabSize);
	}
	catch (int exeption)
	{
		this->file.close();
		throw exeption;
	}
	this->file.close();
}
bool RootFile::readFromFile()
{
	//read header:
	bool error = !fileRead(0);
	if (error) return false;
	if (this->pagetabSize < 5) return false;

	//decode header:
	this->rootPageNum = *((pnumint*)this->pagetab);
	if (this->rootPageNum == rec::PageNumber::NULL_NUMBER) return false;
	this->dOrderCopy = *((uint32_t*)(this->pagetab + sizeof(pnumint)));
	this->dataFileSize = *((uint32_t*)(this->pagetab + sizeof(pnumint) + sizeof(uint32_t)));
	this->unuPageNumNumber = *((pnumint*)(this->pagetab + sizeof(pnumint) + 2 * sizeof(uint32_t)));
	this->unuAddressNumber = *((addressint*)(this->pagetab + sizeof(pnumint) + sizeof(pnumint) + 2 * sizeof(uint32_t)));

	//read body:
	this->unuPageNumList = new  pnumint[this->unuPageNumNumber];
	this->unuAddressList = new addressint[this->unuAddressNumber];
	memset((char*)this->unuPageNumList, 0xCD, this->unuPageNumNumber);
	memset((char*)this->unuAddressList, 0xCD, this->unuAddressNumber);

	//read lists:
	unsigned int unuPageNumSize = this->unuPageNumNumber * sizeof(pnumint); // in Bytes
	unsigned int unuAddressSize = this->unuAddressNumber * sizeof(addressint); // in Bytes

	unsigned int unuPageNumOffset = this->headerSize; // in Bytes
	unsigned int unuAddressOffset = unuPageNumOffset + unuPageNumSize; // in Bytes

	unsigned int currentOffset = this->headerSize; // in Bytes
	unsigned int pageOffset = this->headerSize; // in Bytes

	unsigned int unuPageNumByteGot = 0; // in Bytes
	unsigned int unuAddressByteGot = 0; // in Bytes

	unsigned int currentPage = 0;
	//PageNumberList:

	while (true)
	{

		unsigned int possibleBytes = 0;
		if (this->pagetabSize - pageOffset <= unuPageNumSize - unuPageNumByteGot)
		{
			possibleBytes = this->pagetabSize - pageOffset;
		}
		else
		{
			possibleBytes = unuPageNumSize - unuPageNumByteGot;
		}
		memcpy((char*)this->unuPageNumList + unuPageNumByteGot, this->pagetab + pageOffset, possibleBytes);
		unuPageNumByteGot += possibleBytes;
		if (unuPageNumByteGot >= unuPageNumSize) break;
		currentPage++;
		bool error = !fileRead(currentPage);
		if (error) return false;
		pageOffset = 0;

	}

	//Address List:
	if (unuAddressOffset % this->listPageSize == 0)
	{
		currentPage++;
		bool error = !fileRead(currentPage);
		if (error) return false;
	}
	pageOffset = unuAddressOffset % this->listPageSize;

	while (true)
	{

		unsigned int possibleBytes = 0;
		if (this->pagetabSize - pageOffset <= unuAddressSize - unuAddressByteGot)
		{
			possibleBytes = this->pagetabSize - pageOffset;
		}
		else
		{
			possibleBytes = unuAddressSize - unuAddressByteGot;
		}
		memcpy((char*)this->unuAddressList + unuAddressByteGot, this->pagetab + pageOffset, possibleBytes);
		unuAddressByteGot += possibleBytes;
		if (unuAddressByteGot >= unuAddressSize) break;
		currentPage++;
		bool error = !fileRead(currentPage);
		if (error) return false;
		pageOffset = 0;

	}

	error = !fileRead(currentPage);
	if (error) return false;
	pageOffset = 0;

	this->initialised = true;
	return true;

}
void RootFile::writeToFile()
{
	if (this->listPageSize <= 5) return;

	this->unuPageNumList = rec::PageNumber::uniqueList.save();
	this->unuPageNumNumber = rec::PageNumber::uniqueList.getListTableSize();

	this->unuAddressList = rec::Tag::uniqueAddressList.save();
	this->unuAddressNumber = rec::Tag::uniqueAddressList.getListTableSize();

	*((uint32_t*)this->pagetab) = this->rootPageNum;
	*((uint32_t*)(this->pagetab + sizeof(pnumint))) = this->dOrderCopy;
	*((uint32_t*)(this->pagetab + sizeof(pnumint) + sizeof(uint32_t))) = this->dataFileSize;
	*((uint32_t*)(this->pagetab + sizeof(pnumint) + 2 * sizeof(uint32_t))) = this->unuPageNumNumber;
	*((uint32_t*)(this->pagetab + 2 * sizeof(pnumint) + 2 * sizeof(uint32_t))) = this->unuAddressNumber;

	//read lists:
	unsigned int unuPageNumSize = this->unuPageNumNumber * sizeof(pnumint); // in Bytes
	unsigned int unuAddressSize = this->unuAddressNumber * sizeof(addressint); // in Bytes

	unsigned int unuPageNumOffset = this->headerSize; // in Bytes
	unsigned int unuAddressOffset = unuPageNumOffset + unuPageNumSize; // in Bytes

	unsigned int pageOffset = this->headerSize; // in Bytes

	unsigned int unuPageNumByteGot = 0; // in Bytes
	unsigned int unuAddressByteGot = 0; // in Bytes

	unsigned int currentPage = 0;

	this->pagetabSize = listPageSize;

	//Page Number List:

	while (true)
	{

		unsigned int possibleBytes = 0;
		if ((this->listPageSize - pageOffset) <= unuPageNumSize - unuAddressByteGot)
		{
			possibleBytes = (this->pagetabSize - pageOffset);
		}
		else
		{
			possibleBytes = unuPageNumSize - unuAddressByteGot;
		}
		memcpy(this->pagetab + pageOffset, (char*)this->unuPageNumList + unuAddressByteGot, possibleBytes);
		unuAddressByteGot += possibleBytes;
		if (unuAddressByteGot >= unuPageNumSize) break;
		fileWrite(currentPage);
		currentPage++;
		pageOffset = 0;
		possibleBytes = 0;
	}

	//Address List:

	if (unuAddressOffset % this->listPageSize == 0)
	{
		fileWrite(currentPage);
		currentPage++;
		pageOffset = 0;
	}
	else
	{
		pageOffset = unuAddressOffset % this->listPageSize;
	}



	while (true)
	{

		unsigned int possibleBytes = 0;
		if ((this->listPageSize - pageOffset) <= unuAddressSize - unuPageNumByteGot)
		{
			possibleBytes = (this->pagetabSize - pageOffset);
			this->pagetabSize = this->listPageSize;
		}
		else
		{
			possibleBytes = unuAddressSize - unuPageNumByteGot;
			this->pagetabSize = possibleBytes + pageOffset;
		}
		memcpy(this->pagetab + pageOffset, (char*)this->unuAddressList + unuPageNumByteGot, possibleBytes);
		unuPageNumByteGot += possibleBytes;
		if (unuPageNumByteGot >= unuAddressSize) break;
		fileWrite(currentPage);
		currentPage++;
		pageOffset = 0;
		possibleBytes = 0;
	}

	fileWrite(currentPage);
	this->pagetabSize = this->listPageSize;
}
void RootFile::save(pnumint rootNumber, unsigned int dOrder)
{
	if (rootNumber != rec::PageNumber::NULL_NUMBER)
	{
		this->rootPageNum = rootNumber;
	}
	else
	{
		this->rootPageNum = rec::PageNumber::NULL_NUMBER;
	}
	this->dOrderCopy = dOrder;
	this->writeToFile();
}
void RootFile::clearFile()
{
	this->file.open(this->filepath, std::ios::out | std::ios::trunc | std::ios::binary);
	if (!this->file.is_open())
	{
		std::cout << "error - while opening file in Diskfile.cpp at:" << __LINE__ << std::endl;
		this->file.close();
		throw OPEN_FILE_EXEPTION;
	}
	this->file.close();
}



//================================================================================
//================================================================================
//=====================================DATA=======================================
//================================================================================
//================================================================================

DataFile::DataFile(unsigned int argfileSize)
{
	this->bufferstate = initial;
	this->buffer = new recordmean[BUFFER_MAX_RECORD_NUMBER]; // create buffer for the new object
	this->filepath = DATA_FILE_PATH;
	this->bufferPageNumber = 0;
	this->filereadtimes = 0;
	this->filewritetimes = 0;
	this->fileSize = argfileSize;
	this->ifCount = true;
	memset(this->buffer, (uint8_t)rec::Record::INVALID_RECORD_NUMBER, DATA_PAGE_SIZE); //works because INVALD RECORD is 32b of "1"
	initFile();
}
DataFile::~DataFile()
{
	if (this->buffer != nullptr)
	{
		if (this->bufferstate == write)
		{
			//save buffer before exit
			this->writeFile(this->bufferPageNumber);
			this->bufferstate = initial;
		}
		delete[] this->buffer;
	}
}
void DataFile::writeFile(unsigned int recordPageNumber)
{
	//opening file:
	this->file.open(this->filepath, std::ios::in | std::ios::out | std::ios::binary);
	if (!this->file.is_open())
	{
		std::cout << "error - while opening file in Diskfile.cpp at:" << __LINE__ << std::endl;
		this->file.close();
		throw OPEN_FILE_EXEPTION;
	}
	//check size of the file
	this->file.seekp(0, std::ios::end); // append p pointer to the end of the file
	long fileSizeByte = this->file.tellp();

	if (fileSizeByte < 0) throw 1;

	//calculate where to save
	long offsetSize = recordPageNumber * DATA_PAGE_SIZE;

	//checks:
	if (this->fileSize == (long)recordPageNumber)
	{
		this->fileSize++;
	}
	else if (this->fileSize < (long)recordPageNumber)
	{
		//add empty pages;
		long emptyNum = (long)recordPageNumber - this->fileSize;
		recordmean* emptyBuff = new recordmean[BUFFER_MAX_RECORD_NUMBER];
		memset(emptyBuff, (uint8_t)rec::Record::INVALID_RECORD_NUMBER, DATA_PAGE_SIZE);
		for (long i = 0; i < emptyNum; i++)
		{
			long offsetSize = (this->fileSize + i) * DATA_PAGE_SIZE;
			this->file.seekp(offsetSize, std::ios::beg); // append p pointer to the offset
			try
			{
				this->file.write((char*)emptyBuff, DATA_PAGE_SIZE);
			}
			catch (int exeption)
			{
				throw exeption;
			}
		}
		this->fileSize += emptyNum;
		this->file.seekp(0, std::ios::end);
		fileSizeByte = this->file.tellp();
		this->fileSize++;
	}

	if (fileSizeByte < offsetSize)
	{
		throw 1; //error
	};


	this->file.seekp(offsetSize, std::ios::beg); // append p pointer to the offset

	//copy whole buffer into file:
	try
	{
		this->file.write((char*)this->buffer, DATA_PAGE_SIZE);
	}
	catch (int exeption)
	{
		throw exeption;
	}

	if (this->ifCount) this->filewritetimes++;
	this->empty = false;
	this->file.close();
}
void DataFile::readFile(unsigned int recordPageNumber)
{
	//open file
	this->file.open(this->filepath, std::ios::in | std::ios::binary);
	if (!this->file.is_open())
	{
		std::cout << "error - while opening file at:" << __LINE__ << std::endl;
		this->file.close();
		throw OPEN_FILE_EXEPTION;
	}

	//calculate reading offset
	long offsetSize = (long)recordPageNumber * DATA_PAGE_SIZE;


	//check size of the file :
	this->file.seekg(0, std::ios::end);
	long fileSize = this->file.tellg();
	if (fileSize <= 0)
	{
		std::cout << "error - cannot read, file is empty at:" << __LINE__ << std::endl;
		this->file.close();
		throw EMPTY_FILE_EXEPTION;
	}

	//read:
	this->file.seekg(offsetSize, std::ios::beg); // move g pointer to the first record to be read
	try
	{
		this->file.read((char*)this->buffer, DATA_PAGE_SIZE); //save into buffer
	}
	catch (int exeption)
	{
		throw exeption;
	}

	//closure:
	if (this->ifCount) this->filereadtimes++;
	try
	{
		this->file.close();
	}
	catch (int er)
	{
		throw er;
	}
}
recordmean DataFile::getRecord(unsigned int recordAddressNumber)
{
	recordmean record;
	//translate record address
	unsigned int recordPageNumber = recordAddressNumber / DATA_PAGE_RECORD_NUMBER;
	unsigned int recordOffsetNumber = recordAddressNumber % DATA_PAGE_RECORD_NUMBER;

	bool doReadFromFile = false;
	//check if buffer is empty:
	if (this->bufferstate == initial)
	{
		//need to read from file
		doReadFromFile = true;
	}
	else if (this->bufferPageNumber == recordPageNumber)
	{   //check if present buffer is useful:
		//read from buffer
		doReadFromFile = false;
	}
	else
	{
		//present buffer is not useful 
		if (this->bufferstate == write)
		{
			//need to save buffer first 
			try
			{
				this->writeFile(this->bufferPageNumber);
			}
			catch (int exeption)
			{
				std::cout << "error - while writing to the file [" << exeption << "] at:" << __LINE__ << std::endl;
				throw WRITING_FILE_EXEPTION;
			}
		}
		doReadFromFile = true;
	}


	// read from file
	if (doReadFromFile == true)
	{
		this->bufferstate = read;

		//reading from the file:
		try { this->readFile(recordPageNumber); }
		catch (int exeption)
		{
			std::cout << "error - while reading the file at:" << __LINE__ << std::endl;
			throw READING_FILE_EXEPTION;
		}
		this->bufferPageNumber = recordPageNumber;
	}

	// we can read from the buffer:
	record = this->buffer[recordOffsetNumber];

	if (record == rec::Record::INVALID_RECORD_NUMBER)
	{
		//error
		throw 1;
	}

	return record;

}
void DataFile::saveRecord(unsigned int recordAddressNumber, recordmean record)
{
	//translate record address
	unsigned int recordPageNumber = recordAddressNumber / DATA_PAGE_RECORD_NUMBER;
	unsigned int recordOffsetNumber = recordAddressNumber % DATA_PAGE_RECORD_NUMBER;

	if (this->bufferstate == write)
	{
		if (this->bufferPageNumber != recordPageNumber)
		{
			//need to save buffer to file before
			try
			{
				this->writeFile(this->bufferPageNumber);
			}
			catch (int exeption)
			{
				std::cout << "error - while writing to the datafile [" << exeption << "] at:" << __LINE__ << std::endl;
				throw WRITING_FILE_EXEPTION;
			}

		}

	}

	if (this->bufferPageNumber != recordPageNumber || this->bufferstate == initial)
	{
		if (this->fileSize > recordPageNumber)
		{
			//page exists - read proper page
			try
			{
				this->readFile(this->bufferPageNumber);
			}
			catch (int exeption)
			{
				std::cout << "error - while reading datafile [" << exeption << "] at:" << __LINE__ << std::endl;
				throw WRITING_FILE_EXEPTION;
			}
		}
		else
		{
			//prepere new page
			this->bufferPageNumber = recordPageNumber;
			this->bufferstate = write;
			memset(this->buffer, (uint8_t)rec::Record::INVALID_RECORD_NUMBER, DATA_PAGE_SIZE);
		}
	}

	//write into buffer
	this->bufferstate = write;
	this->bufferPageNumber = recordPageNumber;
	this->buffer[recordOffsetNumber] = record;


}
void DataFile::removeFile()
{

	if (remove(this->filepath.c_str()) != 0)
	{
		throw ERROR_DELETING_FILE;
		std::cout << "Error deleting " << this->filepath << "file" << std::endl;
	}
}
void DataFile::clearFile()
{
	this->file.open(this->filepath, std::ios::out | std::ios::trunc | std::ios::binary);
	if (!this->file.is_open())
	{
		std::cout << "error - while opening file in Diskfile.cpp at:" << __LINE__ << std::endl;
		this->file.close();
		throw OPEN_FILE_EXEPTION;
	}
	this->file.close();
	this->fileSize = 0;
}
void DataFile::resetCout()
{
	this->filereadtimes = 0;
	this->filewritetimes = 0;
}
void DataFile::saveBuf()
{
	if (this->bufferstate == write)
	{
		//saving final records from buffer:
		try
		{
			this->writeFile(this->bufferPageNumber);
			this->bufferstate = initial;
			this->bufferPageNumber = 0;
		}
		catch (int exeption)
		{
			std::cout << "error - while writing to the file [" << exeption << "] at:" << __LINE__ << std::endl;
			throw WRITING_FILE_EXEPTION;
		}
	}
}
void DataFile::initFile()
{
	this->empty = false;
	this->file.open(this->filepath, std::ios::in | std::ios::binary);
	if (!this->file.is_open())
	{
		this->empty = true;
	}
	else
	{
		// check file size
		this->file.seekg(0, std::ios::end);
		unsigned int size = (unsigned int)this->file.tellg();
		if (size == 0)
		{
			this->empty = true;
		}
	}
	this->file.close();
	if (this->empty == true)
	{
		this->clearFile();
	}
}
