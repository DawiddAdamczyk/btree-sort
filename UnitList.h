#pragma once
template <typename unitint>
class UnitElement
{
public:
    UnitElement<unitint>* prev;
    UnitElement<unitint>* next;
    unitint value;
};


template <typename unitint>
class UnitList
{
private:
    static const unitint NULL_VALUE = 0;
    unitint next;
    UnitElement<unitint>* first;
    UnitElement<unitint>* last;
    unitint ammount;
    bool searchAndDestroy(unitint newElement, UnitElement<unitint>* current)
    {
        if (newElement == NULL_VALUE)
            return false;
        if (current == nullptr)
            return false;
        if (current->value == newElement)
        {
            //delete element
            UnitElement<unitint>* newprev = nullptr;
            UnitElement<unitint>* newnext = nullptr;

            if (current->next != nullptr)
                newnext = current->next;
            if (current->prev != nullptr)
                newprev = current->prev;

            if (newnext != nullptr)
                newnext->prev = newprev;
            if (newprev != nullptr)
                newprev->next = newnext;

            if (current == this->first)
                this->first = newnext;
            if (current == this->last)
                this->last = newprev;

            delete current;
            current = nullptr;
            this->ammount--;
            this->next--;
            return true;
        }
        if (current->prev != nullptr)
            this->searchAndDestroy(newElement, current->prev);
        else return false;

    }

public:
    UnitList()
    {
        this->ammount = 0;
        this->next = 1;
        this->first = nullptr;
        this->last = nullptr;
    }

    bool initialise(unitint* listTab)
    {
        if (listTab != nullptr)
        {
            if (this->first != nullptr || this->last != nullptr)
                return false; //List aready initialised
            this->ammount = listTab[0];
            this->next = listTab[1];
            this->first = new UnitElement<unitint>;
            this->first->prev = nullptr;
            UnitElement<unitint>* current = this->first;
            for (unitint i = 2; current != nullptr && i < this->ammount + 1; i++)
            {
                current->value = listTab[i];
                current->next = new UnitElement<unitint>;
                current->next->prev = current;
                current = current->next;
            }
            this->last = current;
            this->last->value = listTab[this->ammount + 1];
            this->last->next = nullptr;
            return true;
        }
        return false; // null arg;
    }

    unitint* save()
    {
        unitint* listTab = new unitint[this->ammount + 2];
        listTab[0] = this->ammount;
        listTab[1] = this->next;
        UnitElement<unitint>* current = this->first;
        for (unitint i = 2; current != nullptr && i < this->ammount + 2; i++)
        {
            listTab[i] = current->value;
            current = current->next;
        }
        return listTab;
    }

    unitint getNext()
    {
        unitint ret = NULL_VALUE;
        if (this->ammount != 0)
        {
            //list is not empty
            //get from the list
            ret = popFirst();
        }
        if (ret == NULL_VALUE)
        {
            this->next++;
            return this->next - 1;
        }
        return ret;
    }

    unitint getListTableSize()
    {
        return this->ammount + 2;
    }

    unitint popFirst()
    {
        if (this->first != nullptr)
        {
            unitint value = this->first->value;
            if (this->first->next != nullptr)
            {
                this->first->next->prev = nullptr;
            }
            if (this->first == this->last)
            {
                this->last = nullptr;
            }
            delete this->first;
            this->first = nullptr;
            this->ammount--;
            return value;
        }
        else
        {
            return NULL_VALUE;
        }
    }

    void add(unitint newElement)
    {
        if (newElement == NULL_VALUE) return;
        if (newElement >= this->next) return;
        if (newElement == this->next - 1)
        {
            this->next--;
            compensate();
            return;
        }
        if (this->last != nullptr)
        {
            if (this->last->next == nullptr)
            {
                this->last->next = new UnitElement<unitint>;
                this->last->next->prev = this->last;
                this->last->next->next = nullptr;
                this->last->next->value = newElement;
                if (this->last == this->first)
                {
                    this->first->next = this->last->next;
                    this->last->next->prev = this->first;
                }
                this->last = this->last->next;
            }
        }
        else
        {
            //create first element
            this->last = new UnitElement<unitint>;
            this->last->prev = nullptr;
            this->last->next = nullptr;
            this->last->value = newElement;
            if (this->first == nullptr)
                this->first = this->last;

        }
        this->ammount++;
    }

    unitint compensate()
    {
        unitint removed = 0;

        while (true)
        {
            if (this->next - 1 <= 0) return removed;
            if (this->searchAndDestroy(this->next - 1, this->last))
            {
                removed++;
            }
            else
            {
                return removed;
            }
        }


    }
};

