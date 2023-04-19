Provides the routines for reading keys.

The basic breakdown of the data structures are as follows,

kui_map
 - A kui_map will map a sequence of keys to another sequence of keys.
 - i.e. map abc def (changes abc to def when matched)
 - Contains a key sequence and a value sequence

kui_map_set
 - A kui map_set is many kui_map items.
 - This data structure stores the kui maps in a std::map and a kui tree.
 - The std::map is used to keep track of what kui maps exist
 - The kui tree is used to keep track of what mappings are being
   matched when recieving input.

kuictx
 - A kui context owns a kui map set.
 - It's added benefit is that it reads a key and handles it. It determine's
   if a macro was hit, or if more keys are necessary, etc.

kui manager
 - Contains several kui contexts. the normal user mappings and the
 - One of the kui contexts represents the current user mappings.
   This could be the map or imap mappings depending on cgdb's state.
   The other kui context is the terminal mappings.

Kui Tree and Kui Tree Node
 - The kui tree node, and kui node exist to efficiently determine if
   a mapping has been reached. The mappings are stored in a tree.
   One character at each node. Every time a key is read as input, the
   tree can be traversed one child to know if a mapping was found, or
   if several mappings are still in the running.

- kui_tree_node
  - A node in a kui_tree.
  - A Key/Value pair and a list of children nodes.
    - The key is the charachter
    - The value is the mapping result
    - The children are the list of potential macros still left to match

- kui_tree
  - A list of kui tree nodes
