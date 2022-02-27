# btree-sort
Sorting records stored on disk using B Tree structure. Records are sorted according to suplementary keys. 
Useful in big database structures, where you cannot afford storing the whole file in operative memory.

There are two modes of operation:

1.Interactive:
| Command | Description |
| --- | --- |
| add <key> <record> | Adding a new record |
| remove <key> | Deleting an existing record |
| search <key> | Searching for the existing record |
| update <old key> <new key> <record> | Change of record |
| gen <N> <key> | Generate N operations of adding a random record |
| print | Display tree |
| get | Display the number of disk operations |
| exit | Exit the program |
2. File-based commands


Written as a part of "Database Structures" course taken at Gdańsk University of Technology, 2021.
