#include "Tree.h"
#include "Record.h"

#include "UnitList.h"
#include <iostream>
#include <random>
#include <time.h>

#define GEN_RATIO 5

void menu(Tree* myTree, std::mt19937* rng);
void interak(Tree* myTree, std::mt19937* rng);
void fromFile(Tree* myTree);
void generate(Tree* myTree, unsigned int N, std::mt19937* rng, bool show, bool removesBool = false);

int main()
{
	std::mt19937 rng(time(0));
	Tree myTree;

	menu(&myTree, &rng);

	return 0;
}



void menu(Tree* myTree, std::mt19937* rng)
{
	while (true)
	{

		std::cout << "1. Interaktywnie" << std::endl;
		std::cout << "2. Z pliku" << std::endl;
		int t;
		std::cin >> t;
		if (t == 1)
		{
			interak(myTree, rng);
			return;
		}
		else if (t == 2)
		{
			fromFile(myTree);
			continue;
		}
		else
		{
			std::cout << "BYE";
			return;
		}
	}


}

void interak(Tree* myTree, std::mt19937* rng)
{
	std::string x;
	while (true)
	{
		std::cout << "<< ";
		std::cin >> x;
		if (x == "add")
		{
			//std::cout << " tag= ";
			unsigned int tag;
			std::cin >> tag;
			float first, second, third, fourth, fifth;
			std::cin >> first>>second>>third>>fourth>>fifth;
			rec::Record myRecord(new rec::Tag(tag), first, second, third, fourth, fifth);
			myTree->add(&myRecord, true, true);
		}
		else if (x == "remove")
		{
			//std::cout << " tag= ";
			unsigned int tag;
			std::cin >> tag;
			rec::Record myRecord(new rec::Tag(tag), 0);
			myTree->remove(&myRecord, true, true);
			delete myRecord.getTag();
		}
		else if (x == "search")
		{
			//std::cout<< " tag= ";
			unsigned int tag;
			std::cin >> tag;
			rec::Record myRecord(new rec::Tag(tag), 0);
			myTree->search(&myRecord);
			delete myRecord.getTag();
		}
		else if (x == "update")
		{
			unsigned int tag;
			unsigned int newtag;
			unsigned int newrecord;
			//std::cout << " old tag= ";
			std::cin >> tag;
			//std::cout << " new tag= ";
			std::cin >> newtag;
			float first, second, third, fourth, fifth;
			std::cin >> first>>second>>third>>fourth>>fifth;
			rec::Record myNewRecord(new rec::Tag(newtag), first, second, third, fourth, fifth);
			rec::Record myRecord(new rec::Tag(tag), 0);
			myTree->update(&myRecord, &myNewRecord);
			delete myRecord.getTag();
		}
		else if (x == "gen")
		{
			unsigned int N;
			unsigned int show;
			//1 std::cout << " N= ";
			std::cin >> N;
			std::cin >> show;
			generate(myTree, N, rng, (bool)show, false);
			myTree->print();
		}
		else if (x == "print")
		{
			myTree->print();
		}
		else if (x == "get")
		{
			std::cout << "All I/o Operations = " << myTree->getIOs() << std::endl;
		}
		else if (x == "exit")
		{
			return;
		}
	}
}

void fromFile(Tree* myTree)
{
	std::string filepath = "Data/";
	std::cout << "Insert a FileName" << std::endl;
	std::string pathentry;
	std::cin >> pathentry;
	filepath.append(pathentry);
	filepath.append(".txt");
	std::fstream file;
	file.open(filepath, std::ios::in | std::ios::out);
	if (!file.is_open())
	{
		std::cout << "File " << filepath << " empty or does not exist" << std::endl;
		return;
	}
	while (true)
	{

		std::string c;
		unsigned int tag;
		unsigned int record;
		file >> c;
		switch (c[0])
		{
		case 'a':
		{
			file >> tag;
			float first, second, third, fourth, fifth;
			file >> first;
			file >> second;
			file >> third;
			file >> fourth;
			file>> fifth;
			rec::Record myRecord(new rec::Tag(tag), first, second, third, fourth, fifth);
			myTree->add(&myRecord, true);
			break;
		}
		case 'r':
		{

			file >> tag;
			rec::Record myRecord(new rec::Tag(tag), 0);
			myTree->remove(&myRecord, true);
			delete myRecord.getTag();
			break;
		}
		case 'u':
		{
			unsigned int newtag;
			unsigned int newrecord;
			file >> tag;
			file >> newtag;
			float first, second, third, fourth, fifth;
			file >> first;
			file >> second;
			file >> third;
			file >> fourth;
			file >> fifth;
			rec::Record myNewRecord(new rec::Tag(newtag), first, second, third, fourth, fifth);
			rec::Record myRecord(new rec::Tag(tag), 0);
			myTree->update(&myRecord, &myNewRecord);
			delete myRecord.getTag();
			break;
		}
		case 's':
		{
			file >> tag;
			rec::Record myRecord(new rec::Tag(tag), 0);

			myTree->search(&myRecord);
			delete myRecord.getTag();
			break;
		}
		case 'p':
		{
			myTree->print();
			std::cout << "All I/o Operations = " << myTree->getIOs() << std::endl;
			myTree->resetIOs();
			break;
		}
		case 'q':
		{
			file.close();
			return;
			break;
		}
		}


	}
}

void generate(Tree* myTree, unsigned int N, std::mt19937* rng, bool show, bool removesBool)
{
	unsigned int removes = 0;
	unsigned int adds = 0;
	unsigned int remove = 0;
	bool choosen = false;
	unsigned int lastAdded = 0;
	for (unsigned int i = 0; i < N;)
	{
		rec::Record* myRec;
		bool result;
		if (removesBool)
			remove = (*rng)() % GEN_RATIO;
		if (remove == 1 && lastAdded != 0)
		{
			myRec = new rec::Record(new rec::Tag(lastAdded), 0);
			choosen = false;
			result = myTree->remove(myRec, show, true);
			lastAdded = 0;
			delete myRec->getTag();
			myRec = nullptr;
			if (result == false)
			{
				result = false;
				continue;
			}
			removes++;
		}
		else
		{
			myRec = new rec::Record(rng);
			lastAdded = myRec->getTag()->getTag();
			result = myTree->add(myRec, show, true);
			if (result == false)
			{
				delete myRec;
				myRec = nullptr;
				result = false;
				lastAdded = 0;
				continue;
			}
			adds++;
		}
		i++;
	}
	std::cout << "Added " << adds << " records" << std::endl;
	std::cout << "Removed " << removes << " records" << std::endl;
}