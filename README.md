# Task 1

In Task 1 I used a regular expression to create a list of words, simply finding all alphanonnumeric sequences of characters.
For the the Hashtable I used a simple strategy:

    - Use a fixed size array with the amount of unique words found in the text
    - To insert find the hash bucket spot and then keep looking until an empty or deleted slot is found
    - to remove, I marked the slot as deleted. An alternative approach would have been to reinsert all elements after the deleted one until an empty slot is found, but since the size is fixed this would yield no benefit
    - to find, I used a shared loop function that in this case simply skipped over deleted entries
    - Since elements may be deleted, a history of insertions must necessarily be kept (unfortunetly doubling the memory requirements). This history will be searched from the back or the front, until an non deleted entry is found
    - optional and shared pointers are used since they lead to less memory management pain
    - the hash function is a murmur hash downloaded from the official repository


# Task 2

    - In Task 2 libcurl is used to receive data from the api
    - std::chrono::high_resolution_clock is used to measure parser performance, averaging 200ns, which is quite good
    - C++s built in and highly optimized regex parsers are used to process the data. A JSON library would have been a good choice too, but since the data is always in the same format, it may have been slower
    - The complexity is pretty low, since the data is always in the same format and the regex is simple


please visit my Website at [https://marvinwyss.ch](https://marvinwyss.ch) to see all my projects
```

