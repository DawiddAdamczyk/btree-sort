#include "Record.h"
#include "Page.h"

namespace page
{
	class Page;
	class PageElements
	{
	private:
		static unsigned int TAG_TAB_MAX_SIZE;
		static unsigned int POINTER_TAB_MAX_SIZE;
		static unsigned int TAG_TAB_MAX_SIZE_WITH_OVERFLOW;
		static unsigned int POINTER_TAB_MAX_SIZE_WITH_OVERFLOW;

		rec::Tag** tagTab;
		rec::PageNumber** pointerTab;

		unsigned int tagTabRealSize;
		unsigned int pointerTabRealSize;

		PageElements();

	public:
		//Public Named Constructors:
		static PageElements* createNew();
		static PageElements* createFromFile(pageptr pagetab);
		//Initializer:
		void static initializeTable(unsigned int dOrder);
		//Save elements to file
		pageptr saveToFile(rec::PageNumber* thisPageNumber);
		pageptr saveZerosToFile();
		//Deconstructor
		~PageElements();
		void deleteTagVan(rec::Tag**& tagVan, unsigned int size);
		//Getters:
		unsigned int getPageRealSize();
		unsigned int getPointerTabSize();
		unsigned int getTagTabSize();
		unsigned int getPointerTabMaxSize();
		unsigned int getTagTabMaxSize();
		//Pointer:
		rec::PageNumber* getPointer(unsigned int pointerindex);
		unsigned int getPointerIndex(rec::PageNumber* pointer, bool& founded);
		rec::PageNumber* getPointerBeforeTag(unsigned int tagindex);
		rec::PageNumber* getPointerAfrerTag(unsigned int tagindex);
		//Tag:
		rec::Tag* getTag(unsigned int tagindex);
		unsigned int getTagIndex(rec::Tag* tag);
		unsigned int getTagIndexLinearly(rec::Tag* tag);
		unsigned int getAlmostTagIndex(rec::Tag* tag);
		rec::Tag* getTagBefore(unsigned int pointerindex);
		rec::Tag* getTagAfrer(unsigned int pointerindex);
		//Inserts
		bool insertTag(unsigned int index, rec::Tag* tag, bool safe = true);
		bool insertPointer(unsigned int pointerindex, rec::PageNumber* pointer, bool safe = true);
		//Insert many tags to the end (me<-sourse)
		bool bulkInsertTagLeft(rec::Tag**& tagVan, unsigned int amount);
		//Insert many tags to the begining (sourse->me)
		bool bulkInsertTagRight(rec::Tag**& tagVan, unsigned int amount);
		//Insert many pointers to the end (me<-sourse)
		bool bulkInsertPointerLeft(rec::PageNumber**& pointerVan, unsigned int amount);
		//Insert many pointers to the begining (sourse->me)
		bool bulkInsertPointerRight(rec::PageNumber**& pointerVan, unsigned int amount);
		//Removes:
		//Romoves and returns one specified tag
		rec::Tag* removeTag(unsigned int index, bool safe = true);
		rec::PageNumber* removePointer(unsigned int pointerindex, bool safe = true);
		//Removes many Tags from begining (dest<-me)
		rec::Tag** bulkRemoveTagLeft(unsigned int amount);
		//Removes many Tags from end (me->dest)
		rec::Tag** bulkRemoveTagRight(unsigned int amount);
		//Removes many Pointers from begining (dest<-me)
		rec::PageNumber** bulkRemovePointerLeft(unsigned int amount);
		//Removes many Pointers from end (end->dest)
		rec::PageNumber** bulkRemovePointerRight(unsigned int amount);
		//Calcutations:
		//Calculate index of pointer BEFORE tag
		static unsigned int calcpBt(unsigned int tagint);
		//Calculate index of pointer AFTER tag
		static unsigned int calcpAt(unsigned int tagint);
		//Calculate index of tag BEFORE pointer
		static unsigned int calctBp(unsigned int pointerint);
		//Calculate index of tag AFTER pointer
		static unsigned int calctAp(unsigned int pointerint);
		bool hasOnlyNullPtrs();

	};



}