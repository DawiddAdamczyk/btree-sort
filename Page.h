#pragma once
#include <cstdint> // for uint32_t


#include "Record.h"


class Tree;

typedef uint32_t* pageptr;
typedef uint32_t pageint;

namespace page
{
	class PageElements;
	class Page
	{
		/*
			Page Structure:
			Header: 8B
			| pageNumber 4B | pageSize 4B
			Page: 2 * dOrder * 12 + 1 B ( 2d (tag,address), 2d+1 pageNumbers)
			| leftPageNumber 4B |  recordTag 4B | recordAddress 4B | centralPageNumber 4B | recordTag 4B | recordAddress 4B | rightPageNumber 4B | ...
		*/
	private:
		//Maximal page size is equals to 4 * dOrder + 1 + HEADER_SIZE (in uint32)
		static unsigned int MAXIMAL_PAGE_SIZE;
		//Header contains This Page Number [0] and Real Size of Page [1] (in uint32)
		static const unsigned int HEADER_SIZE = 2;
		//Unique number given to every page to recognize and calculate position in the file
		rec::PageNumber* pageNumber;
		//Class of elements of page ( Tags & Pointers )
		//Private Constructor
		Page();
	public:
		static Tree* myTree;
		PageElements* elem;
		//Public Named Constructors:
		static Page* createNewPage();
		static Page* createPageFromFile(pageptr argpagetab);
		//Initializer
		static void initializeTree(Tree* myTree);
		//Deconstructor
		~Page();
		//Static Getters:
		static unsigned int getMaxPageSize();
		static unsigned int getHeaderSize();
		static Tree* getMyTree();
		//Getters:
		rec::PageNumber* getThisPageNumber();
		unsigned int getPageRealSize();
		//Save elements to file
		bool savePage();
		void saveEmptyPage();
		void* serchPageForTag(rec::Tag* recordTag, bool& founded);
		void* serchPageForAlmostTag(rec::Tag* recordTag, bool& founded);
		bool leafPage();
		bool pageSizeHalfnMore();
		bool pageSizeMoreThenHalf();
		bool pageSizeLessThenHalf();
		bool pageSumSizeMaxnMore(page::Page* otherPage);
		bool pageSumSizeMaxnLess(page::Page* otherPage);
		friend class PageElements;
		rec::PageNumber* getPtrBefore(rec::Tag* tag);
		rec::PageNumber* getPtrAfter(rec::Tag* tag);

		bool compensate(rec::Tag* tag, rec::Tag*& parentTag, page::Page* siblingPage, bool isLeft, rec::PageNumber* childPageNumber = nullptr);
		bool recompensate(rec::Tag*& parentTag, page::Page* siblingPage, bool isLeft);
		bool split(rec::Tag* tag, rec::Tag*& newParentTag, page::Page* newPage, rec::PageNumber* childPageNumber = nullptr);
		bool merge(rec::Tag* parentTag, page::Page* siblingPage, bool isLeft);
		bool addTag(rec::Tag* newtag, rec::PageNumber* childPageNumber = nullptr);
		bool upadeCopiedTag(rec::Tag* oldtag, rec::Tag* newtag, bool bisection = true);
		rec::PageNumber* getLeftSibling(rec::PageNumber* centerSibling);
		rec::PageNumber* getRightSibling(rec::PageNumber* centerSibling);
		rec::Tag* getLeftParent(rec::PageNumber* centerSibling);
		rec::Tag* getRightParent(rec::PageNumber* centerSibling);
		bool setptrBefore(rec::Tag* tag, rec::PageNumber* pageNumberToInsert);
		bool setptrAfter(rec::Tag* tag, rec::PageNumber* pageNumberToInsert);
		void printPage(unsigned int currenth, unsigned int& TreeH, bool full = false);
		bool removeTag(rec::Tag* tag, rec::Tag*& removedTag, bool isLeftPtrNull, bool isRoot);
		bool emptyPage();



	};


}