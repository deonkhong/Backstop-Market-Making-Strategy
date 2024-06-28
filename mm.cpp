#include <string>
#include <curl/curl.h>
#include "json.hpp"
#include <thread>
#include <chrono>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>
#include <iostream>

using json = nlohmann::json;

// Configuration constants
const std::string BINANCE_PRODUCTION_API_URL = "https://api.binance.com"; // Binance production API URL for fetching reference prices
const std::string BINANCE_FUTURES_TESTNET_API_URL = "https://testnet.binancefuture.com"; // Binance testnet API URL for placing orders

const std::string API_KEY = "b1ef4d1eab0f5b8570ffba8cf9c6f8f8201e39dcfdae3ffd6f59581c6e18bc02"; // Your API key
const std::string SECRET_KEY = "b540614dfeb8a9708c20913f130aee000633b8b7a7535c5817575ea83cd7b35b"; // Your secret key
const std::string SYMBOL = "BTCUSDT"; // Trading symbol
const double BUY_BASIS_POINTS = 50.0; // Basis points for buying
const double SELL_BASIS_POINTS = 75.0; // Basis points for selling
const double ORDER_QUANTITY = 0.1; // Quantity of the order
const int REPRICE_INTERVAL_SECONDS = 2; // Interval for repricing in seconds

// Callback function for CURL to write data into a string
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to perform a HTTP GET request
std::string httpGet(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init(); // Initialize CURL
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl); // Perform the request
        curl_easy_cleanup(curl); // Clean up CURL
    }
    return readBuffer;
}

// Function to generate HMAC-SHA256 signature
std::string hmac_sha256(const std::string& key, const std::string& data) {
    unsigned char* digest;
    digest = HMAC(EVP_sha256(), key.c_str(), key.length(), (unsigned char*)data.c_str(), data.length(), NULL, NULL);
    char mdString[SHA256_DIGEST_LENGTH*2+1];
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
    }
    return std::string(mdString);
}

// Function to perform a HTTP POST request
std::string httpPost(const std::string& url, const std::string& payload, const std::string& apiKey) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("X-MBX-APIKEY: " + apiKey).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

    curl = curl_easy_init(); // Initialize CURL
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl); // Perform the request
        curl_easy_cleanup(curl); // Clean up CURL
    }
    curl_slist_free_all(headers); // Free header list
    return readBuffer;
}

// Function to perform a HTTP DELETE request
std::string httpDelete(const std::string& url, const std::string& payload, const std::string& apiKey) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("X-MBX-APIKEY: " + apiKey).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

    curl = curl_easy_init(); // Initialize CURL
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl); // Perform the request
        curl_easy_cleanup(curl); // Clean up CURL
    }
    curl_slist_free_all(headers); // Free header list
    return readBuffer;
}

// Function to cancel an existing order
void cancelOrder(const std::string& symbol, long orderId) {
    std::string endpoint = "/fapi/v1/order";
    std::string url = BINANCE_FUTURES_TESTNET_API_URL + endpoint;

    long recvWindow = 5000;
    long timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    std::ostringstream oss;
    oss << "symbol=" << symbol
        << "&orderId=" << orderId
        << "&recvWindow=" << recvWindow
        << "&timestamp=" << timestamp;

    std::string queryString = oss.str();
    std::string signature = hmac_sha256(SECRET_KEY, queryString);

    std::string payload = queryString + "&signature=" + signature;
    std::string response = httpDelete(url, payload, API_KEY);
    std::cout << "Cancel response: " << response << std::endl;
}

// Function to place a new order
long placeOrder(const std::string& side, double price, double quantity) {
    std::string endpoint = "/fapi/v1/order";
    std::string url = BINANCE_FUTURES_TESTNET_API_URL + endpoint;

    long recvWindow = 5000;
    long timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    std::ostringstream oss;
    oss << "symbol=" << SYMBOL
        << "&side=" << side
        << "&type=LIMIT"
        << "&timeInForce=GTC"
        << "&quantity=" << quantity
        << "&price=" << price
        << "&recvWindow=" << recvWindow
        << "&timestamp=" << timestamp;

    std::string queryString = oss.str();
    std::string signature = hmac_sha256(SECRET_KEY, queryString);

    std::string payload = queryString + "&signature=" + signature;
    std::cout << "Payload: " << payload << std::endl;
    std::string response = httpPost(url, payload, API_KEY);
    std::cout << "Order response: " << response << std::endl;

    auto jsonResponse = json::parse(response);
    if (jsonResponse.contains("orderId")) {
        return jsonResponse["orderId"].get<long>();
    } else {
        std::cerr << "Failed to place order: " << response << std::endl;
        return -1;
    }
}

// Function to fetch and print the account balance
void fetchAndPrintBalance() {
    std::string endpoint = "/fapi/v1/account";
    std::string url = BINANCE_FUTURES_TESTNET_API_URL + endpoint;

    long recvWindow = 5000;
    long timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    std::ostringstream oss;
    oss << "recvWindow=" << recvWindow
        << "&timestamp=" << timestamp;

    std::string queryString = oss.str();
    std::string signature = hmac_sha256(SECRET_KEY, queryString);

    std::string payload = queryString + "&signature=" + signature;
    std::string response = httpGet(url + "?" + payload);
    std::cout << "Account Info: " << response << std::endl;

    auto jsonResponse = json::parse(response);
    for (auto& balance : jsonResponse["assets"]) {
        std::cout << "Asset: " << balance["asset"] << ", Wallet Balance: " << balance["walletBalance"] << ", Unrealized PNL: " << balance["unrealizedProfit"] << std::endl;
    }
}

// Function to fetch the reference price from the production API
double fetchReferencePrice() {
    std::string symbol = SYMBOL;
    std::string url = BINANCE_PRODUCTION_API_URL + "/api/v3/ticker/price?symbol=" + symbol;
    
    auto start = std::chrono::high_resolution_clock::now(); // Start latency measurement
    std::string response = httpGet(url);
    auto end = std::chrono::high_resolution_clock::now(); // End latency measurement
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Latency for fetching price: " << elapsed.count() << " seconds" << std::endl;
    
    auto jsonResponse = json::parse(response);
    double price = std::stod(jsonResponse["price"].get<std::string>());
    return price;
}

// Struct to hold buy and sell prices
struct Prices {
    double buy;
    double sell;
};

// Function to calculate buy and sell prices based on the reference price and basis points
Prices calculatePrices(double referencePrice, double buyBasisPoints, double sellBasisPoints) {
    Prices prices;
    prices.buy = referencePrice * (1 - buyBasisPoints / 10000);
    prices.sell = referencePrice * (1 + sellBasisPoints / 10000);
    return prices;
}

// Function to run the market making strategy
void runMarketMakingStrategy() {
    fetchAndPrintBalance(); // Print account balance at the beginning

    long buyOrderId = -1;
    long sellOrderId = -1;

    while (true) {
        double referencePrice = fetchReferencePrice(); // Fetch reference price from production API
        Prices prices = calculatePrices(referencePrice, BUY_BASIS_POINTS, SELL_BASIS_POINTS); // Calculate buy and sell prices

        // Cancel existing orders if they exist
        if (buyOrderId != -1) cancelOrder(SYMBOL, buyOrderId);
        if (sellOrderId != -1) cancelOrder(SYMBOL, sellOrderId);

        auto start = std::chrono::high_resolution_clock::now(); // Start latency measurement
        // Place new buy and sell orders
        buyOrderId = placeOrder("BUY", prices.buy, ORDER_QUANTITY);
        sellOrderId = placeOrder("SELL", prices.sell, ORDER_QUANTITY);
        auto end = std::chrono::high_resolution_clock::now(); // End latency measurement
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Latency for placing orders: " << elapsed.count() << " seconds" << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(REPRICE_INTERVAL_SECONDS)); // Wait for the defined interval before repricing
    }
}

int main() {
    runMarketMakingStrategy(); // Run the market making strategy
    return 0;
}
