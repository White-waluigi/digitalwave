#include <iostream>
#include <string>
#include <curl/curl.h>
#include <vector>
#include <regex>
#include "MurmurHash3.h"
#include <optional>
#include <chrono>

static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
		void *userp) {
	((std::string*) userp)->append((char*) contents, size * nmemb);
	return size * nmemb;
}

std::string downloadAPI() {

	CURL *curl;
	CURLcode res;
	std::string readBuffer;

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL,
				"https://fapi.binance.com/fapi/v1/aggTrades?symbol=BTCUSDT");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

	}

	return readBuffer;
}

struct TradeData {
	long aggTradeId;
	double price;
	double quantity;
	long firstTradeId;
	long lastTradeId;
	long timestamp;
	bool isBuyerMaker;
};
void parseSingleTrade(std::string input, TradeData &tradeData) {

	std::regex tradeRegex(
			R"(\"a\":(\d+),\"p\":\"(\d+\.\d+)\",\"q\":\"(\d+\.\d+)\",\"f\":(\d+),\"l\":(\d+),\"T\":(\d+),\"m\":(true|false))");



	std::smatch match;
	std::regex_search(input, match, tradeRegex);
	tradeData.aggTradeId = std::stol(match[1]);
	tradeData.price = std::stod(match[2]);
	tradeData.quantity = std::stod(match[3]);
	tradeData.firstTradeId = std::stol(match[4]);
	tradeData.lastTradeId = std::stol(match[5]);
	tradeData.timestamp = std::stol(match[6]);
	tradeData.isBuyerMaker = match[7] == "true";


}

//regular array to make it faster
std::vector<TradeData> parseTrades(std::string input) {
	std::vector<TradeData> result;
	//remove [ and ]
	input = input.substr(1, input.size() - 2);
	//capture groups  with {}
	std::regex tradeRegex(R"(\{([^}]*)\})")
	;
	auto tradesBegin = std::sregex_iterator(input.begin(), input.end(),
			tradeRegex);
	for (std::sregex_iterator i = tradesBegin; i != std::sregex_iterator();
			++i) {
		std::smatch match = *i;

		TradeData tradeData;
		parseSingleTrade(match[1], tradeData);

		result.push_back(tradeData);
	}

	return result;

}
void printTrade(TradeData tradeData) {

	std::cout << "\t" << "{" << std::endl;
	std::cout << "\t" << "\t" << "\"a\":" << "\t" << tradeData.aggTradeId
			<< ", // Aggregate tradeId" << std::endl;
	std::cout << "\t" << "\t" << "\"p\":" << "\t" << tradeData.price
			<< ", // Price" << std::endl;
	std::cout << "\t" << "\t" << "\"q\":" << "\t" << tradeData.quantity
			<< ", // Quantity" << std::endl;
	std::cout << "\t" << "\t" << "\"f\":" << "\t" << tradeData.firstTradeId
			<< ", // First tradeId" << std::endl;
	std::cout << "\t" << "\t" << "\"l\":" << "\t" << tradeData.lastTradeId
			<< ", // Last tradeId" << std::endl;
	std::cout << "\t" << "\t" << "\"T\":" << "\t" << tradeData.timestamp
			<< ", // Timestamp" << std::endl;
	std::cout << "\t" << "\t" << "\"m\":" << "\t"
			<< (tradeData.isBuyerMaker ? "true" : "false")
			<< ", // Was the buyer the maker?" << std::endl;
	std::cout << "\t" << "}" << std::endl;

}

int main() {

	while (true) {
		std::string result = downloadAPI();
	    // Start measuring time
	    auto start = std::chrono::high_resolution_clock::now();

		std::vector<TradeData> vec = parseTrades(result);

	    auto end = std::chrono::high_resolution_clock::now();
	    // Calculate the duration
	    std::chrono::duration<double> duration = end - start;

	    // Output the time taken in seconds
	    std::cout << "Function took " << duration.count() << " seconds to execute." << std::endl;


	    /*
		for (TradeData tradeData : vec) {
			std::cout << "[" << std::endl;
			printTrade(tradeData);
			std::cout << "]" << std::endl;
		}
		*/
	}

}
