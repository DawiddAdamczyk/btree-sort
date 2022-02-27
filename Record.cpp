#include "Record.h"
using namespace rec;

pnumint PageNumber::nextPageNumber = 1;
addressint Tag::nextAddress = 1;
UnitList<addressint> Tag::uniqueAddressList;
UnitList<pnumint> PageNumber::uniqueList;
//================================================================================
//================================================================================
//=====================================RECORD=====================================
//================================================================================
//================================================================================

Record::Record(Tag* tag, float first, float second, float third, float fourth, float fifth)
{
	/*
	Constructor meant to create Record form user input
	Argument:
		* tag - new tag/key used in the B tree with address of the saved record
		Five floats to create geometric mean from
	*/
	
	//initializing all variables:
	this->tag = tag;
	float num[NUM_COUNT] = { first, second, third, fourth, fifth };
	this->record=compressRecord(num, NUM_COUNT);
}

Record::Record(Tag* tag, recordmean record)
{
	/*
	Constructor meant to create Record objects read from file
	Argument:
	* tag - new tag/key used in the B tree with address of the saved record
	record - geometric mean of the given record
	*/

	this->record = record;
	this->tag = tag;

	if (this->record < 0) // mean cannot be negative
	{
		std::cout << "error - wrong mean number [3] in Record.cpp at:" << __LINE__ << std::endl;
		throw WRONG_FORMAT_NUMBERS;
	}
}

Record::Record(std::mt19937* rng)
{
	/*
	Constructor meant to create random Record objects
	Argument:std::mt19937: pointer to the generator
	*/

	std::random_device rd;
	//specifying range of generation
	constexpr int FLOAT_MIN = 10;
	constexpr int FLOAT_MAX = 1000;
	float num[NUM_COUNT];
	std::default_random_engine eng(rd());
	std::uniform_real_distribution<float> distr(FLOAT_MIN, FLOAT_MAX);

	for (int n = 0; n < NUM_COUNT; ++n) {
		num[n] = distr(eng);
		if (num[n] < 0) n--;
	}

	this->record = compressRecord(num, NUM_COUNT);
	this->tag = new Tag(rng);
}

Record::~Record() //CHECK
{
	//if (this->tag != nullptr) delete this->getTag();
};

void Record::printRecordString()
{
	//prints record to the console
	std::cout << this->tag->getTag() << "-";
	std::cout <<"g_mean: "<< this->record << " ";
	std::cout << ' ';
}


float Record::compressRecord(float arr[], int n)
{
	// declare sum variable and
// initialize it to 0.
	float sum = 0;

	// Compute the sum of all the
	// elements in the array.
	for (int i = 0; i < n; i++)
		sum = sum + log(arr[i]);

	// compute geometric mean through formula
	// antilog(((log(1) + log(2) + . . . + log(n))/n)
	// and return the value to main function.
	sum = sum / n;

	return exp(sum);
}

recordmean Record::getRecord()
{
	return this->record;
}

Tag* Record::getTag()
{
	return this->tag;
}

void Record::setTag(Tag* tag)
{
	this->tag = tag;
}

//================================================================================
//================================================================================
//========================================TAG=====================================
//================================================================================
//================================================================================
Tag::Tag(tagint tag, addressint addres)
{
	this->tag = tag;
	this->address = addres;
}
Tag::Tag(tagint tag)
{
	this->tag = tag;
	this->address = this->NULL_ADDRESS;
}

Tag::Tag(std::mt19937* rng)
{
	this->tag = ((*rng)() % (MAX_TAG - 1)) + 1;
	//this->tag = ((*rng)() % 5000) + 1;
	//this->tag = ((*rng)() % (10000-1)) + 1;
	this->address = this->NULL_ADDRESS;
}

bool Tag::compare(tagint t1, tagint t2)
{
	return (t1 <= t2) ? true : false;
}

bool Tag::equals(tagint t1, tagint t2)
{
	return (t1 == t2) ? true : false;
}


addressint Tag::getAddress()
{

	return this->address;
}

addressint Tag::calculateAddressOffset()
{
	return this->address - 1;
}

void Tag::initAddress()
{
	if (this->address == this->NULL_ADDRESS)
	{
		this->address = this->uniqueAddressList.getNext();
	}
}


//================================================================================
//================================================================================
//================================PAGE_NUMBER=====================================
//================================================================================
//================================================================================

PageNumber::PageNumber()
{
	this->number = nextPageNumber;
	nextPageNumber++;
}




