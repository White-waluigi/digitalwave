#include <iostream>
#include <string>
#include <curl/curl.h>
#include <vector>
#include <regex>
#include "MurmurHash3.h"
#include <optional>



static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

std::string downloadBook ()
{


	//download book using curl
	CURL *curl;
	CURLcode res;
	std::string readBuffer;

	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://www.gutenberg.org/files/98/98-0.txt");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);



	}

	return readBuffer;
}



std::vector<std::string> splitBook(std::string input) {


	std::vector<std::string> result;
	std::regex wordRegex("[a-zA-Z]+");
	auto wordsBegin = std::sregex_iterator(input.begin(), input.end(), wordRegex);
	auto wordsEnd = std::sregex_iterator();

	for (std::sregex_iterator i = wordsBegin; i != wordsEnd; ++i) {
		result.push_back(i->str());
	}

	return result;
}

std::vector<std::string> removeDoubles(std::vector<std::string> words) {
	std::vector<std::string> result;
	for (std::string word : words) {
		if (std::find(result.begin(), result.end(), word) == result.end()) {
			result.push_back(word);
		}
	}
	return result;

}



struct Tuple {
	std::string word;
	int value;
	//marks as deleted, since actually deleting it would invalidate the tree and pulling back the next elements may invalidate it too (different entrypoints)
	bool deleted;
};
struct HashTable {

	std::optional<std::shared_ptr<Tuple>> * data;

	std::vector<std::shared_ptr<Tuple>> history;


	size_t maxSize;
};

std::shared_ptr<HashTable> createHashTable(size_t size) {
	std::shared_ptr<HashTable> table = std::make_shared<HashTable>();
	table->maxSize = size;
	table->data = new std::optional<std::shared_ptr<Tuple>>[size];
	for (int i = 0; i < size; i++) {
		table->data[i] = std::nullopt;
	}
	return table;
}


// common function to find an elment in the tree
size_t find_idx(std::shared_ptr<HashTable> table, std::string word, bool skip_deleted,uint32_t &hash) {

	MurmurHash3_x86_32(word.c_str(), word.size(), 0, &hash);
	int start = hash % table->maxSize;
	int index = start;

	while (table->data[index].has_value() ){
		if (table->data[index].value()->word == word) {
			if (!skip_deleted || !table->data[index].value()->deleted) {
				return index;
			}
		}
		index++;
		if (index >= table->maxSize) {
			index = 0;
		}
		if (index == start) {
			return -1;
		}
	}
	return index;

}

std::optional<std::shared_ptr<Tuple>> insert(std::shared_ptr<HashTable> table, std::string word, int value) {
	std::shared_ptr<Tuple> tuple = std::make_shared<Tuple>();

	tuple->word = word;
	tuple->value = value;
	tuple->deleted = false;

	uint32_t hash;

	bool was_deleted = false;
	size_t index = find_idx(table, word, false,hash);

	if (index == -1) {
		return std::nullopt;
	}




	std::shared_ptr<Tuple> old = table->data[index].has_value() ? table->data[index].value() : nullptr;





	// if overwriting mark as deleted
	if(old&& !old->deleted) {
		table->data[index].value()->deleted = true;
	}

	table->data[index] = tuple;

	table->history.push_back(tuple);

	return tuple;

}
std::optional<std::shared_ptr<Tuple>> get_first(std::shared_ptr<HashTable> table) {
	for (std::shared_ptr<Tuple> tuple : table->history) {
		if (!tuple->deleted) {
			return tuple;
		}
	}
	return std::nullopt;
}


std::optional<std::shared_ptr<Tuple>> get_last(std::shared_ptr<HashTable> table) {
	for (int i = table->history.size() - 1; i >= 0; i--) {
		if (!table->history[i]->deleted) {
			return table->history[i];
		}
	}
	return std::nullopt;
}


std::optional<int> get(std::shared_ptr<HashTable> table, std::string word) {
	uint32_t hash;
	size_t index = find_idx(table, word, true,hash);
	if (index == -1) {
		return std::nullopt;
	}
	return table->data[index].value()->value;
}

std::optional<std::shared_ptr<Tuple>> remove(std::shared_ptr<HashTable> table, std::string word) {
	uint32_t hash;
	size_t index = find_idx(table, word, true,hash);
	if (index == -1) {
		return std::nullopt;
	}
	table->data[index].value()->deleted = true;
	return table->data[index];
}





int main()
{
	std::string book = downloadBook();

	std::vector<std::string> words = splitBook(book);
	std::vector<std::string> uniqueWords = removeDoubles(words);

	std::cout << "Number of unique words: " << uniqueWords.size() << std::endl;

	// create hash table with size of unique words
	
	std::shared_ptr<HashTable> table = createHashTable(uniqueWords.size());

	// insert all unique words into the hash table
	
	for (size_t i = 0; i < uniqueWords.size(); i++) {
		std::cout << "Inserting word: " << uniqueWords[i] << std::endl;
		insert(table, uniqueWords[i], i);
	}
	for (size_t i = 0; i < table->maxSize; i++) {
		//should remain no empty entries
		if (!table->data[i].has_value()) {
			std::cout << "Empty entry: " << i << std::endl;
		}
		//none should be marked as deleted
		if (table->data[i].has_value() && table->data[i].value()->deleted) {
			std::cout << "Deleted entry: " << i << std::endl;
		}
	}
	// inserting random word should yield null
	std::optional<std::shared_ptr<Tuple>> inserted = insert(table, "*random", 0);
	if (!inserted.has_value()) {
		std::cout << "Inserting 'random' failed successfully" << std::endl;
	}
	// check if all are retrieved
	for (size_t i = 0; i < uniqueWords.size(); i++) {
		std::optional<int> value = get(table, uniqueWords[i]);
		if (!value.has_value() || value.value() != i) {
			std::cout << "Retrieving word failed: " << uniqueWords[i]
					<< std::endl;
		}

	}


	// get the first and last element of the hash table
	std::optional<std::shared_ptr<Tuple>> first = get_first(table);

	std::optional<std::shared_ptr<Tuple>> last = get_last(table);

	std::cout << "First word: " << first.value()->word << std::endl;
	std::cout << "Last word: " << last.value()->word << std::endl;
	// deleteing "the" to test remove
	remove(table, "The");
	first = get_first(table);
	std::cout << "First word after removing 'The': " << first.value()->word << std::endl;

	// deleteing "newsletter" to test remove
	remove(table, "newsletter");
	last = get_last(table);
	std::cout << "Last word after removing 'newsletter': " << last.value()->word << std::endl;


	// get the value of the word "the"
	
	std::optional<int> the = get(table, "empties");


	std::cout << "Value of the word 'empties': " << the.value() << std::endl;

	// reinsering "the"
	std::optional<std::shared_ptr<Tuple>> reinserted = insert(table, "the", 99);
	if (reinserted.has_value() && reinserted.value()->word == "the" && reinserted.value()->value == 99){
		std::cout << "Reinserting 'the' succeeded" << std::endl;
	}


	// remove the word "the"
	
	std::optional<std::shared_ptr<Tuple>> removed = remove(table, "the");
	return 0;
}


