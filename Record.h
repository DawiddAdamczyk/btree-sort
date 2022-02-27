#pragma once
#include <cstdint> //for uint32_t
#include <random> //for ranodomiser
#include <iostream>
#include "Exception.h"
#include "UnitList.h"


#define NUM_COUNT 5
typedef float recordmean;
typedef uint32_t tagint;
typedef uint32_t addressint;
typedef uint32_t pnumint;

namespace rec
{
	
	class Tag
	{
	private:
		tagint tag;
		addressint address;
		static const tagint MAX_TAG = ~((tagint)0) - 1;
	public:
		static UnitList<addressint> uniqueAddressList;
		static addressint nextAddress;
		static const addressint NULL_ADDRESS = 0;
		static const addressint NULL_TAG = 0;
		static const addressint RESERVED_TAG = ~((tagint)0);
		Tag(tagint tag, addressint address);
		Tag(tagint tag);
		Tag(std::mt19937* rng);
		tagint getTag() { return this->tag; };
		addressint getAddress();
		void initAddress();
		void removeAddress() { this->uniqueAddressList.add(this->address); };
		addressint calculateAddressOffset();
		bool goodAddress() { return this->address == NULL_ADDRESS ? false : true; };
		bool goodTag() { return this->tag == NULL_TAG || this->tag == RESERVED_TAG ? false : true; };
		static bool compare(tagint t1, tagint t2);
		static bool equals(tagint t1, tagint t2);
	};


	class Record
	{
	private:
		float compressRecord(float arr[], int n); //compresses input floats to g_mean
		recordmean record; //compressed representation of the record (in form of g_mean)
		Tag* tag;

	public:
		static const unsigned int RECORD_BYTE_SIZE = sizeof(recordmean); //Size in Bytes of the record
		static const unsigned int RECORD_MAX_INT = ~((uint32_t)0);
		static const unsigned int RECORD_MAX = 23 * 3600 + 59 * 60 + 59; // Maximal time record is 23:59:59
		static const unsigned int INVALID_RECORD_NUMBER = ~((uint32_t)0);

		void printRecordString(); //prints record's values of key and mean
		Record(Tag* tag, float first, float second, float third, float fourth, float fifth); // creates record from user input 
		Record(Tag* tag, recordmean record); // creates record from file
		Record(std::mt19937* rng); // creates random record
		~Record();
		Tag* getTag(); //extract tag from the record;
		void setTag(Tag* tag);
		recordmean getRecord(); // returns record
	};


	class PageNumber
	{
	private:
		pnumint number;
	public:
		static UnitList<pnumint> uniqueList;
		static pnumint nextPageNumber;
		static const int NULL_NUMBER = 0;
		static const unsigned int PAGE_TAG_BYTE_SIZE = sizeof(pnumint);
		static const unsigned int PAGE_TAG_MAX_INT = ~((pnumint)0);
		PageNumber(pnumint newTag) : number(newTag) {};
		PageNumber();
		bool goodPage() { return this->number == NULL_NUMBER ? false : true; };
		pnumint getPageNumber() { return this->number; };
		pnumint calculatePageOffset() { return this->number - 1; };
		void changeNumber(pnumint newnumber) { this->number = newnumber; };
		void removePageNumber() { this->uniqueList.add(this->number); };
		static pnumint getNextNumber() { return uniqueList.getNext(); };
	};


}
