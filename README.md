
# README

## Overview

This program implements a simple market-making strategy using Binance's futures testnet API. It periodically fetches the reference price of a trading symbol (e.g., BTCUSDT) from Binance's production API, calculates buy and sell prices based on specified basis points, and places limit orders on the Binance futures testnet. The program continuously reprices these orders at defined intervals.

## Features

- Fetches reference price from Binance's production API.
- Calculates buy and sell prices based on specified basis points.
- Places and cancels limit orders on Binance's futures testnet.
- Prints account balance at the beginning.
- Measures and displays latency for fetching prices and placing orders.

## Dependencies

- `libcurl`: Used for making HTTP requests.
- `openssl`: Used for generating HMAC-SHA256 signatures.
- `nlohmann/json`: Used for handling JSON data.

## Installation Instructions

### Installing Dependencies on Linux

#### Install `libcurl`
```bash
sudo apt-get update
sudo apt-get install libcurl4-openssl-dev
```

#### Install `openssl`
```bash
sudo apt-get install openssl libssl-dev
```

#### Install `nlohmann/json`
You can download the single-header version of `nlohmann/json` from [here](https://github.com/nlohmann/json/releases/latest) and include it in your project directory.

### Compile the Code

Save the code into a file named `market_maker.cpp`.

```bash
g++ -o market_maker market_maker.cpp -lcurl -lssl -lcrypto -pthread
```

### Running the Program

```bash
./market_maker
```

## Code Explanation

### Configuration Constants

- `BINANCE_PRODUCTION_API_URL`: URL for fetching reference prices.
- `BINANCE_FUTURES_TESTNET_API_URL`: URL for placing orders.
- `API_KEY` and `SECRET_KEY`: Your Binance API key and secret key.
- `SYMBOL`: Trading symbol, e.g., BTCUSDT.
- `BUY_BASIS_POINTS` and `SELL_BASIS_POINTS`: Basis points for calculating buy and sell prices.
- `ORDER_QUANTITY`: Quantity of the order.
- `REPRICE_INTERVAL_SECONDS`: Interval for repricing in seconds.

### HTTP Request Functions

- `httpGet`: Performs a HTTP GET request.
- `httpPost`: Performs a HTTP POST request.
- `httpDelete`: Performs a HTTP DELETE request.
- `WriteCallback`: Callback function for `CURL` to write data into a string.

### Signature Function

- `hmac_sha256`: Generates HMAC-SHA256 signature.

### Order Functions

- `placeOrder`: Places a new order.
- `cancelOrder`: Cancels an existing order.

### Account Balance Function

- `fetchAndPrintBalance`: Fetches and prints the account balance.

### Reference Price Function

- `fetchReferencePrice`: Fetches the reference price from the production API.

### Price Calculation Function

- `calculatePrices`: Calculates buy and sell prices based on the reference price and basis points.

### Market Making Strategy

- `runMarketMakingStrategy`: Runs the market making strategy, which involves fetching reference prices, calculating buy and sell prices, placing and canceling orders, and repricing at defined intervals.

### Main Function

- `main`: Entry point of the program that starts the market making strategy.

## Notes

- Ensure you have valid API and secret keys from Binance.
- This code uses the Binance futures testnet, which allows you to test without risking real funds.
- Adjust the configuration constants as needed for your strategy.


