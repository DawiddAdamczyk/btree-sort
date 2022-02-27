#pragma once
#include <string>
#include <fstream> //for files
#include <cmath> 
#include "Exception.h"
#include "Record.h"

#include "Page.h"


class DataFile;

#define TREE_FILE_PATH  "Data/tree.txt"
#define ROOT_FILE_PATH  "Data/root.txt";
#define DATA_FILE_PATH  "Data/data.txt";

class TreeFile
{
private:
	std::string filepath;
	unsigned int pageSize;
	std::fstream file;
	unsigned int pageReads;
	unsigned int pageWrites;
	DataFile* dataFile;
	bool empty;
	void insertEmptyPage(unsigned int N);
public:
	TreeFile(unsigned int argPageSize, DataFile* dataFile);
	void savePage(pageptr pagetab, unsigned int N);
	pageptr getPage(unsigned int N);
	void removeFile();
	void clearFile();
	void initFile();
	void getCounts(bool print, bool reset = false);
	void resetCounts();
	bool ifEmpty() { return this->empty; };
	unsigned int getPageReads() { return pageReads; }
	unsigned int getPageWrites() { return pageWrites; }
};

class RootFile
{
private:
	std::string filepath;
	std::fstream file;

	pnumint unuPageNumNumber;
	addressint unuAddressNumber;
	addressint* unuAddressList;
	uint32_t dOrderCopy;
	uint32_t dataFileSize;

	uint8_t* pagetab;
	unsigned int pagetabSize; // in Bytes
	bool empty;
	bool initialised;


	static const int headerSize = 2 * sizeof(pnumint) + sizeof(addressint) + sizeof(uint32_t) + sizeof(uint32_t); // in Bytes
	static const int listPageSize = 20; // in Bytes
	bool fileRead(unsigned int rootpageNumber);
	void fileWrite(unsigned int rootpageNumber);

	bool readFromFile();
public:
	pnumint rootPageNum;
	void writeToFile();
	pnumint* unuPageNumList;
	RootFile(unsigned int dOrder);
	~RootFile();
	pnumint getRootPageNumber() { return this->rootPageNum; };
	addressint getNextAddress() { return this->unuPageNumNumber; };
	pnumint getNextPageNumber() { return this->unuAddressNumber; };
	uint32_t getDataFileSize() { return this->dataFileSize; };
	void setDataFileSize(uint32_t argdataFileSize) { this->dataFileSize = argdataFileSize; };
	bool ifEmpty() { return this->empty; };
	void save(pnumint rootNumber, unsigned int dOrder);
	void clearFile();
};

class DataFile
{
private:
	std::string filepath;
	std::fstream file;
	enum state { read, write, initial };
	state bufferstate;
	recordmean* buffer;
	unsigned int fileSize;
	unsigned int bufferPageNumber; // Page which is saved on the buffer
	static const int DATA_PAGE_SIZE = 8;  // This is maximal size of an buffer in Bytes 
	static const int DATA_PAGE_RECORD_NUMBER = DATA_PAGE_SIZE / rec::Record::RECORD_BYTE_SIZE;  // This is maximal size of an buffer in Bytes 
	static const int BUFFER_SIZE = DATA_PAGE_SIZE; //This is maximal size of the buffer in bytes
	static const int BUFFER_MAX_RECORD_NUMBER = DATA_PAGE_SIZE / rec::Record::RECORD_BYTE_SIZE; // Size of the Record
	unsigned int filereadtimes;
	unsigned int filewritetimes;
	bool ifCount;
	bool empty;
	void writeFile(unsigned int pagesOffset);
	void readFile(unsigned int recordPage);
public:
	DataFile(unsigned int argfileSize);
	~DataFile();
	void saveRecord(unsigned int recordAddressNumber, recordmean record);
	recordmean getRecord(unsigned int recordAddress);
	void enableCounting() { this->ifCount = true; };
	void disableCounting() { this->ifCount = false; };
	bool checkCounting() { return this->ifCount; };
	void removeFile();
	void clearFile();
	void saveBuf();
	void initFile();
	void resetCout();
	bool ifEmpty() { return this->empty; };
	unsigned int getFileSize() { return this->fileSize; };
};

